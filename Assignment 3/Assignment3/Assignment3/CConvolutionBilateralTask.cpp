/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CConvolutionBilateralTask.h"

#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"
#include "Pfm.h"

#include <sstream>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CConvolutionBilateralTask

CConvolutionBilateralTask::CConvolutionBilateralTask(
		const std::string& FileName, 
		const std::string& NormalFileName,
		const std::string& DepthFileName,
		size_t LocalSizeHorizontal[2],
		size_t LocalSizeVertical[2],
		int StepsHorizontal,
		int StepsVertical,
		int KernelRadius,
		float* pKernelHorizontal,
		float* pKernelVertical)
	: CConvolutionSeparableTask(
			"bilateral", FileName, LocalSizeHorizontal,
			LocalSizeVertical, StepsHorizontal, StepsVertical,
			KernelRadius, pKernelHorizontal, pKernelVertical)
	, m_NormalFileName(NormalFileName)
	, m_DepthFileName(DepthFileName)
{
	m_FileNamePostfix = "Bilateral";
	m_ProgramName = "ConvolutionBilateral.cl";
}

CConvolutionBilateralTask::~CConvolutionBilateralTask()
{
}

bool CConvolutionBilateralTask::InitResources(cl_device_id Device, cl_context Context)
{
	PFM normalsPFM;
	if (!normalsPFM.LoadRGB(m_NormalFileName.c_str())) {
		cerr<<"Error loading file: " << m_NormalFileName << "." << endl;
		return false;
	}
	PFM depthsPFM;
	if (!depthsPFM.LoadRGB(m_DepthFileName.c_str())) {
		cerr<<"Error loading file: " << m_DepthFileName << "." << endl;
		return false;
	}

	if(	normalsPFM.height != depthsPFM.height ||
		normalsPFM.width != depthsPFM.width)
	{
		cerr<<"Bilateral filtering data mismatch: the feature images must have the same dimensions as the color data"<<endl;
	}

	m_Height = normalsPFM.height;
	m_Width = normalsPFM.width;
	m_Pitch = m_Width;
	if(m_Width % 32 != 0)
		m_Pitch = m_Width + 32 - (m_Width % 32); //This will make sure that the data accesses are ALWAYS coalesced

	m_hNormDepthBuffer = new cl_float4[m_Height * m_Pitch];
	m_hCPUDiscBuffer = new cl_int[m_Height * m_Pitch];
	m_hGPUDiscBuffer = new cl_int[m_Height * m_Pitch];

	unsigned int pixelOffset = 0;
	unsigned int trippleOffset = 0;
	for(unsigned int y = 0; y < m_Height; y++) {
		for(unsigned int x = 0; x < m_Width; x++) {
			m_hNormDepthBuffer[pixelOffset].s[0] = normalsPFM.pImg[trippleOffset    ];
			m_hNormDepthBuffer[pixelOffset].s[1] = normalsPFM.pImg[trippleOffset + 1];
			m_hNormDepthBuffer[pixelOffset].s[2] = normalsPFM.pImg[trippleOffset + 2];
			m_hNormDepthBuffer[pixelOffset].s[3] = depthsPFM.pImg[trippleOffset];

			pixelOffset++;
			trippleOffset += 3;
		}
		pixelOffset += m_Pitch - m_Width;
	}

	cl_int clError = 0;
	cl_int clErr;

	m_dDiscBuffer = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_Pitch * m_Height * sizeof(cl_int),  NULL, &clErr);
	clError = clErr;
	m_dNormDepthBuffer = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, m_Pitch * m_Height * sizeof(cl_float4),  m_hNormDepthBuffer, &clErr);
	clError |= clErr;
	V_RETURN_FALSE_CL(clError, "Error allocating device memory.");

	return CConvolutionSeparableTask::InitResources(Device, Context);
}

bool CConvolutionBilateralTask::InitKernels()
{
	cl_int clError;

	//create kernel(s)
	m_HorizontalKernel = clCreateKernel(m_Program, "ConvHorizontal", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create horizontal kernel.");
	
	m_VerticalKernel = clCreateKernel(m_Program, "ConvVertical", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create vertical kernel.");

	m_HorizontalDiscKernel = clCreateKernel(m_Program, "DiscontinuityHorizontal", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create horizontal discontinuity detection kernel.");

	m_VerticalDiscKernel = clCreateKernel(m_Program, "DiscontinuityVertical", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create vertical discontinuity detection kernel.");

	//bind kernel attributes
	clError  = clSetKernelArg(m_HorizontalDiscKernel, 0, sizeof(cl_mem), (void*)&m_dDiscBuffer);
	clError |= clSetKernelArg(m_HorizontalDiscKernel, 1, sizeof(cl_mem), (void*)&m_dNormDepthBuffer);
	clError |= clSetKernelArg(m_HorizontalDiscKernel, 2, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_HorizontalDiscKernel, 3, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_HorizontalDiscKernel, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting horizontal discontinuity kernel arguments");

	clError  = clSetKernelArg(m_VerticalDiscKernel, 0, sizeof(cl_mem), (void*)&m_dDiscBuffer);
	clError |= clSetKernelArg(m_VerticalDiscKernel, 1, sizeof(cl_mem), (void*)&m_dNormDepthBuffer);
	clError |= clSetKernelArg(m_VerticalDiscKernel, 2, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_VerticalDiscKernel, 3, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_VerticalDiscKernel, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting vertical discontinuity kernel arguments");

	clError  = clSetKernelArg(m_HorizontalKernel, 2, sizeof(cl_mem), (void*)&m_dDiscBuffer);
	clError |= clSetKernelArg(m_HorizontalKernel, 3, sizeof(cl_mem), (void*)&m_dKernelHorizontal);
	clError |= clSetKernelArg(m_HorizontalKernel, 4, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_HorizontalKernel, 5, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_HorizontalKernel, 6, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting horizontal kernel arguments");
		
	clError  = clSetKernelArg(m_VerticalKernel, 2, sizeof(cl_mem), (void*)&m_dDiscBuffer);
	clError |= clSetKernelArg(m_VerticalKernel, 3, sizeof(cl_mem), (void*)&m_dKernelVertical);
	clError |= clSetKernelArg(m_VerticalKernel, 4, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_VerticalKernel, 5, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_VerticalKernel, 6, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting vertical kernel arguments");

	return true;
}

void CConvolutionBilateralTask::ReleaseResources()
{
	SAFE_DELETE_ARRAY( m_hCPUDiscBuffer );
	SAFE_DELETE_ARRAY( m_hGPUDiscBuffer );
	SAFE_DELETE_ARRAY( m_hNormDepthBuffer );

	SAFE_RELEASE_MEMOBJECT( m_dDiscBuffer );
	SAFE_RELEASE_MEMOBJECT( m_dNormDepthBuffer );

	SAFE_RELEASE_KERNEL( m_HorizontalDiscKernel );
	SAFE_RELEASE_KERNEL( m_VerticalDiscKernel );
	
	CConvolutionSeparableTask::ReleaseResources();
}

void CConvolutionBilateralTask::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	size_t dataSize = m_Pitch * m_Height * sizeof(cl_float);
	int nIterations = 100;

	unsigned int numChannels = 3;

	double runTime = 0.0f;

	// detect discontinuities
	size_t globalWorkSizeH[2] = {CLUtil::GetGlobalWorkSize(m_Width / m_StepsHorizontal, m_LocalSizeHorizontal[0]), CLUtil::GetGlobalWorkSize(m_Height, m_LocalSizeHorizontal[1])};	
	runTime += CLUtil::ProfileKernel(CommandQueue, m_HorizontalDiscKernel, 2, globalWorkSizeH, LocalWorkSize, nIterations);

	size_t globalWorkSizeV[2] = {CLUtil::GetGlobalWorkSize(m_Width, m_LocalSizeVertical[0]), CLUtil::GetGlobalWorkSize(m_Height / m_StepsVertical, m_LocalSizeVertical[1])};
	runTime += CLUtil::ProfileKernel(CommandQueue, m_VerticalDiscKernel, 2, globalWorkSizeV, LocalWorkSize, nIterations);


	for(unsigned int iChannel = 0; iChannel < numChannels; iChannel++)
	{
		runTime += ConvolutionChannelGPU(iChannel, Context, CommandQueue, nIterations);
	}

	cout<<"  Average GPU time: "<<runTime<<" ms, throughput: "<< 1.0e-6 * m_Width * m_Height / runTime << " Gpixels/s" <<endl;

	for(unsigned int iChannel = 0; iChannel < numChannels; iChannel++)
	{
		//copy the results back to the CPU
		//(this time the data is in the same buffer as the input was, because of the 2 convolution passes)
		V_RETURN_CL( clEnqueueReadBuffer(CommandQueue, m_dResultChannels[iChannel], CL_TRUE, 0, dataSize,
									m_hGPUResultChannels[iChannel], 0, NULL, NULL), "Error reading back results from the device!" );

	}
	V_RETURN_CL( clEnqueueReadBuffer(CommandQueue, m_dDiscBuffer, CL_TRUE, 0, m_Width * m_Height * sizeof(int), 
		m_hGPUDiscBuffer, 0, NULL, NULL), "Error reading back results from the device!" );
	
	SaveImage("Images/GPUResultBilateral.pfm", m_hGPUResultChannels);
	SaveIntImage("Images/GPUDiscontinuities.pfm", m_hGPUDiscBuffer);
}

void CConvolutionBilateralTask::ComputeCPU()
{
	double runTime = 0.0;

	CTimer timer;
	timer.Start();
	
	// Detect discontinuities
	for(unsigned int y = 0; y < m_Height; y++)
		for(unsigned int x = 0; x < m_Width; x++)
		{
			cl_float4 myNormDepth = m_hNormDepthBuffer[y*m_Pitch + x];
			int flag = 0;

			// Left neighbor
			if (x > 0) {
				cl_float4 normDepth = m_hNormDepthBuffer[y*m_Pitch + x - 1];
				if (IsNormalDiscontinuity(myNormDepth, normDepth) || IsDepthDiscontinuity(myNormDepth.s[3], normDepth.s[3]))
					flag |= 1;
			} else
				flag |= 1;

			// Right neighbor
			if (x < m_Width - 1) {
				cl_float4 normDepth  = m_hNormDepthBuffer[y*m_Pitch + x + 1];
				if (IsNormalDiscontinuity(myNormDepth, normDepth) || IsDepthDiscontinuity(myNormDepth.s[3], normDepth.s[3]))
					flag |= 2;
			} else
				flag |= 2;

			// Upper neighbor
			if (y > 0) {
				cl_float4 normDepth = m_hNormDepthBuffer[(y-1)*m_Pitch + x];
				if (IsNormalDiscontinuity(myNormDepth, normDepth) || IsDepthDiscontinuity(myNormDepth.s[3], normDepth.s[3]))
					flag |= 4;
			} else
				flag |= 4;

			// Lower neighbor
			if (y < m_Height - 1) {
				cl_float4 normDepth  = m_hNormDepthBuffer[(y+1)*m_Pitch + x];
				if (IsNormalDiscontinuity(myNormDepth, normDepth) || IsDepthDiscontinuity(myNormDepth.s[3], normDepth.s[3]))
					flag |= 8;
			} else
				flag |= 8;

			m_hCPUDiscBuffer[y * m_Pitch + x] = flag;
		}

	timer.Stop();

	runTime = timer.GetElapsedMilliseconds();

	// Do the convolution on the CPU
	for(unsigned int iChannel = 0; iChannel < 3; iChannel++)
		runTime += ConvolutionChannelCPU(iChannel);

	cout<<"  CPU time: "<<runTime<<" ms, throughput: "<< 1.0e-6 * m_Width * m_Height / runTime << " Gpixels/s" <<endl;

	// Store CPU results
	SaveImage("Images/CPUResultBilateral.pfm", m_hCPUResultChannels);
	SaveIntImage("Images/CPUDiscontinuities.pfm", m_hCPUDiscBuffer);
}

double CConvolutionBilateralTask::ConvolutionChannelCPU(unsigned int Channel)
{
	CTimer timer;
	timer.Start();

	// HORIZONTAL PASS
	for(unsigned int y = 0; y < m_Height; y++) 
	{
		for(unsigned int x = 0; x < m_Width; x++)
		{
			float sum = 0.f;
			float weight = 0.f;

			// Middle pixel
			weight	= m_hKernelHorizontal[m_KernelRadius];
			sum		= m_hSourceChannels[Channel][y * m_Pitch + x] * weight;

			// Left neighborhood
			for(int k = 0; k > -m_KernelRadius; ) {

				int flag = m_hCPUDiscBuffer[y * m_Pitch + x + k];
				// If discontinuity on the left detected, bail out
				if (flag & 1 ||  (int)x+k <= 0)
					break;

				k--; 

				float w = m_hKernelHorizontal[m_KernelRadius - k];
				sum += m_hSourceChannels[Channel][y * m_Pitch + x + k] * w;
				weight += w;
			}

			// Right neighborhood
			for(int k = 0; k < m_KernelRadius; ) {

				int flag = m_hCPUDiscBuffer[y * m_Pitch + x + k];
				// If discontinuity on the right is detected, bail out
				if (flag & 2 || (int)x+k >= (int)m_Width-1)
					break;

				k++; 

				float w = m_hKernelHorizontal[m_KernelRadius - k];
				sum += m_hSourceChannels[Channel][y * m_Pitch + x + k] * w;
				weight += w;
			}

			// Re-normalize
			if (weight != 0.f)
				sum /= weight;
			else
				sum = 0.f;

			m_hCPUWorkingBuffer[y * m_Pitch + x] = sum;
		}
	}

	//VERTICAL PASS
	for(unsigned int x = 0; x < m_Width; x++)
	{
		for(unsigned int y = 0; y < m_Height; y++)
		{
			float sum = 0.f;
			float weight = 0.f;

			// Middle pixel
			weight	= m_hKernelHorizontal[m_KernelRadius];
			sum		= m_hCPUWorkingBuffer[y * m_Pitch + x] * weight;

			// Upper neighborhood
			for(int k = 0; k > -m_KernelRadius; ) {

				int flag = m_hCPUDiscBuffer[(y+k) * m_Pitch + x];
				// If discontinuity on the left detected, bail out
				if (flag & 4 || y+k <= 0)
					break;

				k--; 

				float w = m_hKernelHorizontal[m_KernelRadius - k];
				sum += m_hCPUWorkingBuffer[(y+k) * m_Pitch + x] * w;
				weight += w;
			}

			// Lower neighborhood
			for(int k = 0; k < m_KernelRadius; ) {

				int flag = m_hCPUDiscBuffer[(y+k) * m_Pitch + x];
				// If discontinuity on the right is detected, bail out
				if (flag & 8 || (int)y+k >= (int)m_Height-1)
					break;

				k++; 

				float w = m_hKernelHorizontal[m_KernelRadius - k];
				sum += m_hCPUWorkingBuffer[(y+k) * m_Pitch + x] * w;
				weight += w;
			}

			// Re-normalize
			if (weight != 0.f)
				sum /= weight;
			else
				sum = 0.f;

			m_hCPUResultChannels[Channel][y * m_Pitch + x] = sum;
		}
	}
	

	timer.Stop();
	return timer.GetElapsedMilliseconds();
}

double CConvolutionBilateralTask::ConvolutionChannelGPU(unsigned int Channel, cl_context Context, cl_command_queue CommandQueue, int NIterations)
{
	cl_int clErr;

	double runTime = 0;

	clErr  = clSetKernelArg(m_HorizontalKernel, 1, sizeof(cl_mem), (void*)&m_dSourceChannels[Channel]);
	clErr |= clSetKernelArg(m_HorizontalKernel, 0, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
	V_RETURN_0_CL(clErr, "Error setting horizontal kernel arguments");

	size_t globalWorkSizeH[2] = {CLUtil::GetGlobalWorkSize(m_Width / m_StepsHorizontal, m_LocalSizeHorizontal[0]), CLUtil::GetGlobalWorkSize(m_Height, m_LocalSizeHorizontal[1])};	
	runTime += CLUtil::ProfileKernel(CommandQueue, m_HorizontalKernel, 2, globalWorkSizeH, m_LocalSizeHorizontal, NIterations);

	clErr  = clSetKernelArg(m_VerticalKernel, 1, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
	clErr |= clSetKernelArg(m_VerticalKernel, 0, sizeof(cl_mem), (void*)&m_dResultChannels[Channel]);
	V_RETURN_0_CL(clErr, "Error setting vertical kernel arguments");

	size_t globalWorkSizeV[2] = {CLUtil::GetGlobalWorkSize(m_Width, m_LocalSizeVertical[0]), CLUtil::GetGlobalWorkSize(m_Height / m_StepsVertical, m_LocalSizeVertical[1])};
	runTime += CLUtil::ProfileKernel(CommandQueue, m_VerticalKernel, 2, globalWorkSizeV, m_LocalSizeVertical, NIterations);

	return runTime;
}
