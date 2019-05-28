#ifndef  __CPIXELCOUNTTASK_H__
#define  __CPIXELCOUNTTASK_H__

#include <string>
#include <vector>
#include "../Common/IComputeTask.h"

class CHistogramTask : public IComputeTask
{
public:
	enum { NUM_HIST_BINS = 64 };
	CHistogramTask(float min_val, float max_val, bool use_local_memory, const std::string &img_path);
	virtual ~CHistogramTask();

	virtual bool InitResources(cl_device_id Device, cl_context Context) override;
	virtual void ReleaseResources() override;
	virtual void ComputeGPU(cl_context ctx, cl_command_queue cmdq, size_t lws[3]) override;
	virtual void ComputeCPU() override;
	virtual bool ValidateResults() override;

protected:
	float m_min_val = 0.0f, m_max_val = 1.0f;
	const std::string m_img_path;
	const bool m_use_local_memory;
	int m_img_width = 0, m_img_height = 0, m_img_stride = 0;

	cl_program m_program = nullptr;
	cl_kernel m_kernel_histogram = nullptr, m_kernel_set_to_val = nullptr;
	cl_mem m_d_pixels = nullptr;
	cl_mem m_d_hist = nullptr;

	std::vector<int> m_histogram, m_histogram_gpu;
	std::vector<float> m_pixels;
};


#endif  /*__CPIXELCOUNTTASK_H__*/
