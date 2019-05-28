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

#ifndef _CMATRIX_ROTATE_TASK_H
#define _CMATRIX_ROTATE_TASK_H

#include "../Common/IComputeTask.h"

//! A1/T2: Matrix rotation
class CMatrixRotateTask : public IComputeTask
{
public:
	CMatrixRotateTask(size_t SizeX, size_t SizeY);
	virtual ~CMatrixRotateTask();

	// IComputeTask
	virtual bool InitResources(cl_device_id Device, cl_context Context);
	
	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	virtual void ComputeCPU();

	virtual bool ValidateResults();

protected:
	//NOTE: we have two memory address spaces, so we mark pointers with a prefix
	//to avoid confusions: 'h' - host, 'd' - device

	unsigned int		m_SizeX;
	unsigned int		m_SizeY;

	//float data on the CPU
	//M: original matrix, MR: rotated matrix
	float				*m_hM, *m_hMR;

	//pointers on the GPU
	//(result buffers for both kernels)
	cl_mem				m_dM, m_dMR;
	//(..and a pointer to read back the result)
	float				*m_hGPUResultNaive, *m_hGPUResultOpt;

	//OpenCL program and kernels
	cl_program			m_Program;
	cl_kernel			m_NaiveKernel;
	cl_kernel			m_OptimizedKernel;
};

#endif // _CMATRIX_ROTATE_TASK_H
