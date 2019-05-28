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

#ifndef _CCONVOLUTION_BILATERAL_TASK_H
#define _CCONVOLUTION_BILATERAL_TASK_H

#include "CConvolutionSeparableTask.h"

#include <string>
#include <cmath>

#define DEPTH_THRESHOLD	0.025f
#define NORM_THRESHOLD	0.9f

//! A3/T3 bilateral filter
/*!
	This class implements a separable convolution filter, but
	extended with a discontinuity detection pass. Therefore with
	inherit it from the second task...
*/
class CConvolutionBilateralTask: public CConvolutionSeparableTask
{
public:
	// Yes, we have more and more attributes :) But we did not want to complicate it with more "setter" methods...
	CConvolutionBilateralTask(const std::string& FileName, const std::string& NormalFileName,
		const std::string& DepthFileName, size_t LocalSizeHorizontal[2], size_t LocalSizeVertical[2],
		int StepsHorizontal, int StepsVertical, int KernelRadius, float* pKernelHorizontal, float* pKernelVertical);

	virtual ~CConvolutionBilateralTask();

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

	// These helper methods are used to build the discontinuity buffer
	inline bool IsNormalDiscontinuity(const cl_float4 &n1, const cl_float4 &n2) {
		return ::std::fabs(n1.s[0] * n2.s[0] + n1.s[1] * n2.s[1] + n1.s[2] * n2.s[2]) < NORM_THRESHOLD;
	}

	inline bool IsDepthDiscontinuity(float d1, float d2){
		return ::std::fabs(d1 - d2) > DEPTH_THRESHOLD;
	}

	std::string		m_NormalFileName;
	std::string		m_DepthFileName;

	//host data
	cl_float4*		m_hNormDepthBuffer;

	// discontinuity buffers
	cl_int*			m_hCPUDiscBuffer;
	cl_int*			m_hGPUDiscBuffer;

	// device data
	cl_mem			m_dDiscBuffer;
	cl_mem			m_dNormDepthBuffer;

	// kernels for discontinuity detection
	cl_kernel		m_HorizontalDiscKernel;
	cl_kernel		m_VerticalDiscKernel;

};

#endif // _CCONVOLUTION_BILATERAL_TASK_H
