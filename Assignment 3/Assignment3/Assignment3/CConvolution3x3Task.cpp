/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CConvolution3x3Task.h"

#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CConvolution3x3Task

CConvolution3x3Task::
CConvolution3x3Task(
		const std::string& FileName,
		size_t TileSize[2],
		float ConvKernel[3][3],
		bool Monochrome,
		float Offset
)
	: CConvolutionTaskBase(FileName, Monochrome)
	, m_Offset(Offset)
{
	m_TileSize[0] = TileSize[0];
	m_TileSize[1] = TileSize[1];

	m_KernelWeight = 0;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			m_hConvolutionKernel[i][j] = ConvKernel[i][j];
			m_KernelWeight += ConvKernel[i][j];
		}
	}

	if(m_KernelWeight > 0)
		m_KernelWeight = 1.0f / m_KernelWeight;
	else
		m_KernelWeight = 1.0f;

	m_FileNamePostfix = "3x3";
}

CConvolution3x3Task::~CConvolution3x3Task()
{
	ReleaseResources();
}

bool CConvolution3x3Task::InitResources(cl_device_id Device, cl_context Context)
{
	if(!CConvolutionTaskBase::InitResources(Device, Context))
		return false;

	//we can init the kernel buffer during creation as its contents will not change
	cl_int clError;
	cl_float kernelConstants[11];
	for(int y = 0; y < 3; y++)
		for(int x = 0; x < 3; x++)
			kernelConstants[3*y + x] = m_hConvolutionKernel[y][x];
	kernelConstants[9] = m_KernelWeight;
	kernelConstants[10] = m_Offset;

	m_dKernelConstants = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 11 * sizeof(cl_float), 
		kernelConstants, &clError);
	V_RETURN_FALSE_CL(clError, "Error allocating device kernel constants.");

	string programCode;

	CLUtil::LoadProgramSourceToMemory("Convolution3x3.cl", programCode);
	m_Program = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(m_Program == nullptr) return false;

	//create kernel(s)
	m_ConvolutionKernel = clCreateKernel(m_Program, "Convolution", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel.");
	
	//bind kernel attributes
	clError = clSetKernelArg(m_ConvolutionKernel, 2, sizeof(cl_mem), (void*)&m_dKernelConstants);
	clError |= clSetKernelArg(m_ConvolutionKernel, 3, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_ConvolutionKernel, 4, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_ConvolutionKernel, 5, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting kernel arguments");

	return true;
}

void CConvolution3x3Task::ReleaseResources()
{
	SAFE_RELEASE_MEMOBJECT(m_dKernelConstants);

	SAFE_RELEASE_KERNEL(m_ConvolutionKernel);
	SAFE_RELEASE_PROGRAM(m_Program);

	CConvolutionTaskBase::ReleaseResources();
}

void CConvolution3x3Task::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	// This time we can take a bit less iterations than before, since the image processing itself
	// is more time consuming than the previous tasks
	const int nIterations = 1000;

	//do 1 or 3 convolution steps, based on the number of color channels to process
	unsigned int numChannels = m_Monochrome ? 1 : 3;

	size_t dataSize = m_Pitch * m_Height * sizeof(cl_float);

	//perform the convolution and measure the performance
	double runTime = 0.0f;
	for(unsigned int iChannel = 0; iChannel < numChannels; iChannel++)	
		runTime += ConvolutionChannelGPU(iChannel, Context, CommandQueue, nIterations);


	cout<<"  Average GPU time: "<<runTime<<" ms, throughput: "<< 1.0e-6 * m_Width * m_Height / runTime << " Gpixels/s" <<endl;

	for(unsigned int iChannel = 0; iChannel < numChannels; iChannel++)
	{
		//copy the results back to the CPU
		V_RETURN_CL( clEnqueueReadBuffer(CommandQueue, m_dResultChannels[iChannel], CL_TRUE, 0, dataSize,
									m_hGPUResultChannels[iChannel], 0, NULL, NULL), "Error reading back results from the device!" );
	}


	SaveImage("Images/GPUResult3x3.pfm", m_hGPUResultChannels);
}

void CConvolution3x3Task::ComputeCPU()
{
	//number of channels to compute
	unsigned int numChannels = m_Monochrome ? 1 : 3;

	double runTime = 0.0;

	for(unsigned int iChannel = 0; iChannel < numChannels; iChannel++)
	{
		runTime += ConvolutionChannelCPU(iChannel);
	}

	cout<<"  CPU time: "<<runTime<<" ms, throughput: "<< 1.0e-6 * m_Width * m_Height / runTime << " Gpixels/s" <<endl;

	SaveImage("Images/CPUResult3x3.pfm", m_hCPUResultChannels);
}

double CConvolution3x3Task::ConvolutionChannelCPU(unsigned int Channel)
{
	//also measure the time for the first channel
	CTimer timer;

	const int nIterations = 10;
	
	timer.Start();
	
	for(int iter = 0; iter < nIterations; iter++)
	{

		for(unsigned int y = 0; y < m_Height; y++)
		{
			for(unsigned int x = 0; x < m_Width; x++)
			{
				float value = 0;
				//apply convolution kernel
				for(int offsetY = -1; offsetY < 2; offsetY ++)
				{
					int sy = y + offsetY;
					if(sy >= 0 && sy < int(m_Height))
						for(int offsetX = -1; offsetX < 2; offsetX++)
						{
							int sx = x + offsetX;
							if(sx >= 0 && sx < int(m_Width))
								value += m_hSourceChannels[Channel][sy * m_Pitch + sx] * m_hConvolutionKernel[1 + offsetY][1 + offsetX];
						}
				}
				m_hCPUResultChannels[Channel][y * m_Pitch + x] = value * m_KernelWeight + m_Offset;		
			}
		}

	}

	timer.Stop();

	return timer.GetElapsedMilliseconds();
}

double CConvolution3x3Task::ConvolutionChannelGPU(unsigned int Channel, cl_context Context, 
												cl_command_queue CommandQueue, int NIterations)
{
	size_t globalWorkSize[2] = {CLUtil::GetGlobalWorkSize(m_Width, m_TileSize[0]), CLUtil::GetGlobalWorkSize(m_Height, m_TileSize[1])};
	
	cl_int clErr;
	clErr  = clSetKernelArg(m_ConvolutionKernel, 0, sizeof(cl_mem), (void*)&m_dResultChannels[Channel]);
	clErr |= clSetKernelArg(m_ConvolutionKernel, 1, sizeof(cl_mem), (void*)&m_dSourceChannels[Channel]);
	V_RETURN_0_CL(clErr, "Error setting kernel arguments!");

	return CLUtil::ProfileKernel(CommandQueue, m_ConvolutionKernel, 2, globalWorkSize, m_TileSize, NIterations);
}


///////////////////////////////////////////////////////////////////////////////
