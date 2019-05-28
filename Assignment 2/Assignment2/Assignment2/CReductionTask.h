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

#ifndef _CREDUCTION_TASK_H
#define _CREDUCTION_TASK_H

#include "../Common/IComputeTask.h"

//! A2/T1: Parallel reduction
class CReductionTask : public IComputeTask
{
public:
	CReductionTask(size_t ArraySize);

	virtual ~CReductionTask();

	// IComputeTask

	virtual bool InitResources(cl_device_id Device, cl_context Context);
	
	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	virtual void ComputeCPU();

	virtual bool ValidateResults();

protected:

	void Reduction_InterleavedAddressing(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);
	void Reduction_SequentialAddressing(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);
	void Reduction_Decomp(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);
	void Reduction_DecompUnroll(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);
	void Reduction_DecompAtomics(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	void ExecuteTask(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int task);
	void TestPerformance(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int task);

	//NOTE: we have two memory address spaces, so we mark pointers with a prefix
	//to avoid confusions: 'h' - host, 'd' - device

	unsigned int		m_N;

	// input data
	unsigned int		*m_hInput;
	// results
	unsigned int		m_resultCPU;
	unsigned int		m_resultGPU[5];

	cl_mem				m_dPingArray;
	cl_mem				m_dPongArray;

	//OpenCL program and kernels
	cl_program			m_Program;
	cl_kernel			m_InterleavedAddressingKernel;
	cl_kernel			m_SequentialAddressingKernel;
	cl_kernel			m_DecompKernel;
	cl_kernel			m_DecompUnrollKernel;
	cl_kernel			m_DecompAtomicsKernel;

};

#endif // _CREDUCTION_TASK_H
