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

#ifndef _CSIMPLE_ARRAYS_TASK_H
#define _CSIMPLE_ARRAYS_TASK_H

#include "../Common/IComputeTask.h"

//! A1/T1: Simple vector addition
class CSimpleArraysTask : public IComputeTask
{
public:
	CSimpleArraysTask(size_t ArraySize);
	virtual ~CSimpleArraysTask();

	// IComputeTask
	
	virtual bool InitResources(cl_device_id Device, cl_context Context);
	
	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	virtual void ComputeCPU();

	virtual bool ValidateResults();

protected:
	//NOTE: we have two memory address spaces, so we mark pointers with a prefix
	//to avoid confusions: 'h' - host, 'd' - device
	
	//number of array elements
	size_t				m_ArraySize = 0;

	//integer arrays on the CPU
	int					*m_hA = nullptr, *m_hB = nullptr, *m_hC = nullptr;

	//integer arrays on the GPU (and a buffer to read the result back to the host)
	cl_mem				m_dA = nullptr, m_dB = nullptr, m_dC = nullptr;
	int					*m_hGPUResult = nullptr;

	//OpenCL program and kernels
	cl_program			m_Program = nullptr;
	cl_kernel			m_Kernel = nullptr;
};

#endif // _CSIMPLE_ARRAYS_TASK_H
