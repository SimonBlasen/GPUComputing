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

#ifndef _CCONVOLUTION_3X3_TASK_H
#define _CCONVOLUTION_3X3_TASK_H

#include "CConvolutionTaskBase.h"

#include <string>

//! A3 / T1 3x3 convolution
class CConvolution3x3Task : public CConvolutionTaskBase
{
public:
	CConvolution3x3Task(
			const std::string& FileName,
			size_t TileSize[2],
			float ConvKernel[3][3],
			bool Monochrome,
			float Offset);

	virtual ~CConvolution3x3Task();

	// IComputeTask

	virtual bool InitResources(cl_device_id Device, cl_context Context);
	
	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	virtual void ComputeCPU();

protected:
	
	// the return value is the run time in milliseconds
	double ConvolutionChannelCPU(unsigned int Channel);
	//the last parameter is for timing, and the returned value is the average run time in milliseconds
	double ConvolutionChannelGPU(unsigned int Channel, cl_context Context, cl_command_queue CommandQueue, int NIterations);

	size_t			m_TileSize[2];

	// host data
	float			m_hConvolutionKernel[3][3];
	float			m_KernelWeight;
	float			m_Offset;

	//kernel constants
	cl_mem			m_dKernelConstants = nullptr;

	cl_program		m_Program = nullptr;
	cl_kernel		m_ConvolutionKernel = nullptr;

};

#endif // _CCONVOLUTION_3X3_TASK_H
