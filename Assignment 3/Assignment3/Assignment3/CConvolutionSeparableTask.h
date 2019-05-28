/******************************************************************************
                         .88888.   888888ba  dP     dP 
                        d8'   `88  88    `8b 88     88 
                        88        a88aaaa8P' 88     88 
                        88   YP88  88        88     88 
                        Y8.   .88  88        Y8.   .8P 
                         `88888'   dP        `Y88888P' 
                                                       
                                                       
   a88888b.                                         dP   oo                   
  d8'   `88                                         88                        
  88        .d8888b. 88d8b.d8b. 88d888b. dP    dP d8888P dP 88d888b. .d8888b. 
  88        88'  `88 88'`88'`88 88'  `88 88    88   88   88 88'  `88 88'  `88 
  Y8.   .88 88.  .88 88  88  88 88.  .88 88.  .88   88   88 88    88 88.  .88 
   Y88888P' `88888P' dP  dP  dP 88Y888P' `88888P'   dP   dP dP    dP `8888P88 
                                88                                        .88 
                                dP                                    d8888P  
******************************************************************************/

#ifndef _CCONVOLUTION_SEPARABLE_TASK_H
#define _CCONVOLUTION_SEPARABLE_TASK_H

#include "CConvolutionTaskBase.h"

#include <string>

//! A3 / T2 separable convolution
class CConvolutionSeparableTask : public CConvolutionTaskBase
{
public:
	CConvolutionSeparableTask(
			const std::string& OutFileName,
			const std::string& FileName,
			size_t LocalSizeHorizontal[2],
			size_t LocalSizeVertical[2],
			int StepsHorizontal,
			int StepsVertical,
			int KernelRadius,
			float* pKernelHorizontal,
			float* pKernelVertical);

	virtual ~CConvolutionSeparableTask();

	// IComputeTask

	virtual bool InitResources(cl_device_id Device, cl_context Context);
	virtual bool InitKernels();
	
	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	virtual void ComputeCPU();

protected:
	// the return value is the run time in milliseconds
	double ConvolutionChannelCPU(unsigned int Channel);
	// the return value is the run time in milliseconds
	double ConvolutionChannelGPU(unsigned int Channel, cl_context Context, cl_command_queue CommandQueue, int NIterations);

	std::string m_OutFileName;

	//we use different local work sizes during the two convolution kernels
	size_t			m_LocalSizeHorizontal[2];
	size_t			m_LocalSizeVertical[2];
	//how many convolution steps a single thread executes
	int				m_StepsHorizontal = 0;
	int				m_StepsVertical = 0;

	//host data
	float*			m_hKernelHorizontal = nullptr;
	float*			m_hKernelVertical = nullptr;
	int				m_KernelRadius = 0;

	// device data
	cl_mem			m_dGPUWorkingBuffer;
	float*			m_hCPUWorkingBuffer;

	//kernel coefficients
	cl_mem			m_dKernelHorizontal = nullptr;
	cl_mem			m_dKernelVertical = nullptr;

	cl_program		m_Program = nullptr;
	std::string		m_ProgramName;
	//horizontal convolution pass
	cl_kernel		m_HorizontalKernel = nullptr;
	//vertical convolution pass
	cl_kernel		m_VerticalKernel = nullptr;
};

#endif // _CCONVOLUTION_SEPARABLE_TASK_H
