/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CConvolutionSeparableTask.h"

#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"

#include <sstream>
#include <cstring>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CConvolutionSeparableTask

CConvolutionSeparableTask::CConvolutionSeparableTask(
		const std::string& OutFileName, 
		const std::string& FileName, 
		size_t LocalSizeHorizontal[2],
		size_t LocalSizeVertical[2],
		int StepsHorizontal,
		int StepsVertical,
		int KernelRadius,
		float* pKernelHorizontal,
		float* pKernelVertical
)
	: CConvolutionTaskBase(FileName, false)
	, m_OutFileName(OutFileName)
	, m_StepsHorizontal(StepsHorizontal)
	, m_StepsVertical(StepsVertical)
	, m_KernelRadius(KernelRadius)
{
	m_LocalSizeHorizontal[0] = LocalSizeHorizontal[0];
	m_LocalSizeHorizontal[1] = LocalSizeHorizontal[1];
	m_LocalSizeVertical[0]   = LocalSizeVertical[0];
	m_LocalSizeVertical[1]   = LocalSizeVertical[1];

	const unsigned int kernelSize = 2 * m_KernelRadius + 1;
	m_hKernelHorizontal = new float[kernelSize];
	m_hKernelVertical = new float[kernelSize];
	memcpy(m_hKernelHorizontal, pKernelHorizontal, kernelSize * sizeof(float));
	memcpy(m_hKernelVertical, pKernelVertical, kernelSize * sizeof(float));

	m_dGPUWorkingBuffer = nullptr;
	m_hCPUWorkingBuffer = nullptr;

	m_FileNamePostfix = "Separable_" + OutFileName;
	m_ProgramName = "ConvolutionSeparable.cl";
}

CConvolutionSeparableTask::~CConvolutionSeparableTask()
{
	delete [] m_hKernelHorizontal;
	delete [] m_hKernelVertical;

	ReleaseResources();
}

bool CConvolutionSeparableTask::InitResources(cl_device_id Device, cl_context Context)
{
	if(!CConvolutionTaskBase::InitResources(Device, Context))
		return false;
	

	//create GPU resources
	//we can init the kernel buffer during creation as its contents will not change
	const unsigned int kernelSize = 2 * m_KernelRadius + 1;

	cl_int clError = 0;
	cl_int clErr;
	m_dKernelHorizontal = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kernelSize * sizeof(cl_float), 
		m_hKernelHorizontal, &clErr);
	clError |= clErr;
	m_dKernelVertical = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kernelSize * sizeof(cl_float), 
		m_hKernelVertical, &clErr);
	clError |= clErr;
	V_RETURN_FALSE_CL(clError, "Error allocating device kernel constants.");

	m_dGPUWorkingBuffer = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_Pitch * m_Height * sizeof(cl_float), NULL, &clError);
	V_RETURN_FALSE_CL(clError, "Error allocating device working array");

	m_hCPUWorkingBuffer = new float[m_Height * m_Pitch];

	string programCode;

	CLUtil::LoadProgramSourceToMemory(m_ProgramName, programCode);

	//This time we define several kernel-specific constants that we did not know during
	//implementing the kernel, but we need to include during compile time.
	stringstream compileOptions;
	compileOptions<<"-cl-fast-relaxed-math"
	<<" -D KERNEL_RADIUS="<<m_KernelRadius
	<<" -D H_GROUPSIZE_X="<<m_LocalSizeHorizontal[0]<<" -D H_GROUPSIZE_Y="<<m_LocalSizeHorizontal[1]
	<<" -D H_RESULT_STEPS="<<m_StepsHorizontal
	<<" -D V_GROUPSIZE_X="<<m_LocalSizeVertical[0]<<" -D V_GROUPSIZE_Y="<<m_LocalSizeVertical[1]
	<<" -D V_RESULT_STEPS="<<m_StepsVertical;

	m_Program = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode, compileOptions.str());
	if(m_Program == nullptr) return false;


	return InitKernels();
}

bool CConvolutionSeparableTask::InitKernels()
{
	cl_int clError;
	
	//create kernel(s)
	m_HorizontalKernel = clCreateKernel(m_Program, "ConvHorizontal", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create horizontal kernel.");
	
	m_VerticalKernel = clCreateKernel(m_Program, "ConvVertical", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create vertical kernel.");

	//bind kernel attributes
	//the resulting image will be in buffer 1
	clError = clSetKernelArg(m_HorizontalKernel, 2, sizeof(cl_mem), (void*)&m_dKernelHorizontal);
	clError |= clSetKernelArg(m_HorizontalKernel, 3, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_HorizontalKernel, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting horizontal kernel arguments");
		
	//the resulting image will be in buffer 0
	clError = clSetKernelArg(m_VerticalKernel, 2, sizeof(cl_mem), (void*)&m_dKernelVertical);
	clError |= clSetKernelArg(m_VerticalKernel, 3, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_VerticalKernel, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting vertical kernel arguments");

	return true;
}

void CConvolutionSeparableTask::ReleaseResources()
{
	SAFE_DELETE_ARRAY( m_hCPUWorkingBuffer );

	SAFE_RELEASE_MEMOBJECT(m_dGPUWorkingBuffer);
	SAFE_RELEASE_MEMOBJECT(m_dKernelHorizontal);
	SAFE_RELEASE_MEMOBJECT(m_dKernelVertical);

	SAFE_RELEASE_KERNEL(m_HorizontalKernel);
	SAFE_RELEASE_KERNEL(m_VerticalKernel);
	SAFE_RELEASE_PROGRAM(m_Program);
}

void CConvolutionSeparableTask::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	size_t dataSize = m_Pitch * m_Height * sizeof(cl_float);
	int nIterations = 100;

	unsigned int numChannels = 3;

	double runTime = 0.0f;
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
	
	SaveImage("Images/GPUResultSeparable_" + m_OutFileName + ".pfm", m_hGPUResultChannels);
}

void CConvolutionSeparableTask::ComputeCPU()
{
	double runTime = 0.0;
	for(unsigned int iChannel = 0; iChannel < 3; iChannel++)
	{
		runTime += ConvolutionChannelCPU(iChannel);
	}

	cout<<"  CPU time: "<<runTime<<" ms, throughput: "<< 1.0e-6 * m_Width * m_Height / runTime << " Gpixels/s" <<endl;

	SaveImage("Images/CPUResultSeparable_" + m_OutFileName + ".pfm", m_hCPUResultChannels);
}

double CConvolutionSeparableTask::ConvolutionChannelCPU(unsigned int Channel)
{
	CTimer timer;
	timer.Start();

	//horizontal pass
	for(int y = 0; y < (int)m_Height; y++)
		for(int x = 0; x < (int)m_Width; x++)
		{
			float value = 0;
			//apply horizontal kernel
			for(int k = -m_KernelRadius; k <= m_KernelRadius; k++)
			{
				int sx = x + k;
				if(sx >= 0 && sx < (int)m_Width)
					value += m_hSourceChannels[Channel][y * m_Pitch + sx] * m_hKernelHorizontal[m_KernelRadius - k];
			}
			m_hCPUWorkingBuffer[y * m_Pitch + x] = value;
		}

	//vertical pass
	for(int x = 0; x < (int)m_Width; x++)
		for(int y = 0; y < (int)m_Height; y++)
		{
			float value = 0;
			//apply horizontal kernel
			for(int k = -m_KernelRadius; k <= m_KernelRadius; k++)
			{
				int sy = y + k;
				if(sy >= 0 && sy < (int)m_Height)
					value += m_hCPUWorkingBuffer[sy * m_Pitch + x] * m_hKernelVertical[m_KernelRadius - k];
			}
			m_hCPUResultChannels[Channel][y * m_Pitch + x] = value;
		}

	timer.Stop();

	return timer.GetElapsedMilliseconds();
}

double CConvolutionSeparableTask::ConvolutionChannelGPU(unsigned int Channel, cl_context Context, cl_command_queue CommandQueue, int NIterations)
{
	cl_int clErr;

	clErr  = clSetKernelArg(m_HorizontalKernel, 0, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
	clErr |= clSetKernelArg(m_HorizontalKernel, 1, sizeof(cl_mem), (void*)&m_dSourceChannels[Channel]);
	V_RETURN_0_CL(clErr, "Error setting horizontal kernel arguments");

	clErr  = clSetKernelArg(m_VerticalKernel, 0, sizeof(cl_mem), (void*)&m_dResultChannels[Channel]);
	clErr |= clSetKernelArg(m_VerticalKernel, 1, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
	V_RETURN_0_CL(clErr, "Error setting vertical kernel arguments");


	double runTime;	
	
	size_t globalWorkSizeH[2] = {
		CLUtil::GetGlobalWorkSize(m_Width / m_StepsHorizontal, m_LocalSizeHorizontal[0]),
		CLUtil::GetGlobalWorkSize(m_Height, m_LocalSizeHorizontal[1])
	};	
	runTime = CLUtil::ProfileKernel(CommandQueue, m_HorizontalKernel, 2, globalWorkSizeH, m_LocalSizeHorizontal, NIterations);

	size_t globalWorkSizeV[2] = {
		CLUtil::GetGlobalWorkSize(m_Width, m_LocalSizeVertical[0]),
		CLUtil::GetGlobalWorkSize(m_Height / m_StepsVertical, m_LocalSizeVertical[1])
	};
	runTime += CLUtil::ProfileKernel(CommandQueue, m_VerticalKernel, 2, globalWorkSizeV, m_LocalSizeVertical, NIterations);
	
	return runTime;
}

///////////////////////////////////////////////////////////////////////////////
