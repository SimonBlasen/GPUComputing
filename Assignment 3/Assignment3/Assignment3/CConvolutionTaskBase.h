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

#ifndef _CCONVOLUTION_TASK_BASE_H
#define _CCONVOLUTION_TASK_BASE_H

#include "../Common/IComputeTask.h"

#include <string>

//! Abstract base class for all convolution tasks
/*!
	This class does not handle any actual computation, but implements methods used by all
	tasks such as loading and saving images and comparing GPU-CPU results.
*/
class CConvolutionTaskBase : public IComputeTask
{
public:
	CConvolutionTaskBase(const std::string& FileName, bool Monochrome = false);

	virtual ~CConvolutionTaskBase();

	// IComputeTask

	virtual bool InitResources(cl_device_id Device, cl_context Context);
	
	virtual void ReleaseResources();

	virtual bool ValidateResults();

protected:

	void SaveImage(const std::string& FileName, float* Channels[3]);
	void SaveIntImage(const std::string& FileName, int* Channel);

	// helper functions:
	
	// one grayscale floating point value out of RGB
	float RGBToGrayScale(float R, float G, float B);
	// quantize a floating point to a, 8 bit fixed point
	unsigned int To8BitChannel(float Value);

	std::string		m_FileName;
	//if true, only one channel is used
	bool			m_Monochrome;

	// internally used, so different tasks can name their differece images
	// uniquely
	std::string		m_FileNamePostfix;

	unsigned int	m_Height = 0;
	unsigned int	m_Width  = 0;
	unsigned int	m_Pitch  = 0;

	float*			m_hSourceChannels[3]    /*= { nullptr, nullptr, nullptr }*/; //R, G, B channels
	float*			m_hCPUResultChannels[3] /*= { nullptr, nullptr, nullptr }*/; //the convolved image
	float*			m_hGPUResultChannels[3] /*= { nullptr, nullptr, nullptr }*/; //the convolved image

	//we process exactly one channel on the GPU in the same time
	cl_mem			m_dSourceChannels[3] /*= { nullptr, nullptr, nullptr}*/;
	cl_mem			m_dResultChannels[3] /*= { nullptr, nullptr, nullptr}*/;

};

#endif // _CCONVOLUTION_TASK_BASE_H
