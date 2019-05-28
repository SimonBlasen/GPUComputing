#include "CHistogramTask.h"
#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"
#include "Pfm.h"
#include <string.h>
#include <cassert>

CHistogramTask::
CHistogramTask(float min_val, float max_val, bool use_local_memory, const std::string &img_path)
	: m_min_val(min_val)
	, m_max_val(max_val)
	, m_img_path(img_path)
	, m_use_local_memory(use_local_memory)
{
}

CHistogramTask::
~CHistogramTask()
{
	ReleaseResources();
}

bool CHistogramTask::
InitResources(cl_device_id dev, cl_context ctx)
{
	cl_int err;
	PFM img;
	if(!img.LoadRGB(m_img_path.c_str())) {
		std::cerr << "Error loading image: \"" << m_img_path << "\"!" << std::endl;
		return false;
	}

	m_img_width  = img.width;
	m_img_height = img.height;
	m_img_stride = img.width % 32 ? (img.width + 32 - img.width % 32) : img.width;
	m_pixels.resize(m_img_stride * m_img_height, 0.0f);
	for(int y = 0; y < m_img_height; y++) {
		for(int x = 0; x < m_img_width; x++) {
			auto &s = m_pixels[y * m_img_stride + x];
			s = 0.0f;
		   	s += img.pImg[(y * img.width + x) * 3 + 0] * 0.3f;
		   	s += img.pImg[(y * img.width + x) * 3 + 1] * 0.59f;
		   	s += img.pImg[(y * img.width + x) * 3 + 2] * 0.11f;
		}
	}
	m_d_pixels = clCreateBuffer(ctx,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(float) * m_pixels.size(),
			m_pixels.data(),
			&err);
	V_RETURN_FALSE_CL(err, "Failed to allocate device memory");

	std::vector<int> zeroes(NUM_HIST_BINS, 0);
	m_d_hist = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_HIST_BINS * sizeof(int),
			zeroes.data(), &err);
	V_RETURN_FALSE_CL(err, "Failed to allocate device memory");


	std::string src;
	if(!CLUtil::LoadProgramSourceToMemory("histogram.cl", src))
		return false;

	m_program = CLUtil::BuildCLProgramFromMemory(dev, ctx, src);
	if(!m_program)
		return false;


	int num_hist_bins = NUM_HIST_BINS;

	m_kernel_histogram = clCreateKernel(
			m_program,
			m_use_local_memory ? "compute_histogram_local_memory" : "compute_histogram",
			&err);
	V_RETURN_FALSE_CL(err, "Failed to create kernel: histogram");

	err = clSetKernelArg(m_kernel_histogram, 0, sizeof(cl_mem), &m_d_hist);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 0");
	err = clSetKernelArg(m_kernel_histogram, 1, sizeof(cl_mem), &m_d_pixels);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 1");
	err = clSetKernelArg(m_kernel_histogram, 2, sizeof(int), &m_img_width);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 2");
	err = clSetKernelArg(m_kernel_histogram, 3, sizeof(int), &m_img_height);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 3");
	err = clSetKernelArg(m_kernel_histogram, 4, sizeof(int), &m_img_stride);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 4");
	err = clSetKernelArg(m_kernel_histogram, 5, sizeof(int), &num_hist_bins);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 5");
	if(m_use_local_memory) {
		err = clSetKernelArg(m_kernel_histogram, 6, sizeof(int) * NUM_HIST_BINS, nullptr);
		V_RETURN_FALSE_CL(err, "Error setting kernel Arg 6");
	}

	m_kernel_set_to_val = clCreateKernel(m_program, "set_array_to_constant", &err);
	V_RETURN_FALSE_CL(err, "Failed to create kernel: set_array_to_constant");
	err = clSetKernelArg(m_kernel_set_to_val, 0, sizeof(cl_mem), &m_d_hist);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 0");
	err = clSetKernelArg(m_kernel_set_to_val, 1, sizeof(int), &num_hist_bins);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 1");
	int zero = 0;
	err = clSetKernelArg(m_kernel_set_to_val, 2, sizeof(int), &zero);
	V_RETURN_FALSE_CL(err, "Error setting kernel Arg 2");

	return true;
}

void CHistogramTask::
ReleaseResources()
{
	 SAFE_RELEASE_MEMOBJECT(m_d_pixels);
	 SAFE_RELEASE_MEMOBJECT(m_d_hist);
	 SAFE_RELEASE_KERNEL(m_kernel_histogram);
	 SAFE_RELEASE_KERNEL(m_kernel_set_to_val);
}

static void
print_histogram(const std::vector<int> &h)
{
	int max_val = 0;
	for(auto i: h)
		max_val = std::max<int>(max_val, i);

	std::cout << "+";
	for(size_t i = 0; i < h.size(); i++)
		std::cout << "-";
	std::cout << "+\n";
	const int max_height = 8;
	for(int y = max_height - 1; y >= 0; y--) {
		int val = (max_val * y) / max_height;
		std::cout << "|";
		for(auto i: h)
			std::cout << (i >= val ? '#' : ' ');
		std::cout << "|\n";
	}
	std::cout << "+";
	for(size_t i = 0; i < h.size(); i++)
		std::cout << "-";
	std::cout << "+\n";
}

void CHistogramTask::
ComputeGPU(cl_context ctx, cl_command_queue cmdq, size_t lws[3])
{
	size_t local_size_clear = 256;
	size_t global_size_clear = ((NUM_HIST_BINS + local_size_clear - 1) / local_size_clear) * local_size_clear;
	size_t global_size[2] = {
		((m_img_width  + lws[0] - 1) / lws[0]) * lws[0],
		((m_img_height + lws[1] - 1) / lws[1]) * lws[1]
	};

	CTimer timer;
	clFinish(cmdq);
	timer.Start();

	const int num_iterations = 100;
	for(int i = 0; i < num_iterations; i++) {
		clEnqueueNDRangeKernel(cmdq, m_kernel_set_to_val, 1, NULL, &global_size_clear, &local_size_clear, 0, NULL, NULL);

		clEnqueueNDRangeKernel(cmdq, m_kernel_histogram, 2, NULL, global_size, lws, 0, NULL, NULL);
	}
	clFinish(cmdq);
	timer.Stop();

	const char *prefix = m_use_local_memory
		? "  Histogram GPU time (using local memory): "
		: "  Histogram GPU time (no local memory): ";
	std::cout << prefix << timer.GetElapsedMilliseconds() / float(num_iterations) << " ms\n";

	m_histogram_gpu.resize(NUM_HIST_BINS);

	clEnqueueReadBuffer(cmdq, m_d_hist, CL_TRUE, 0, sizeof(int) * NUM_HIST_BINS,
			m_histogram_gpu.data(), 0, nullptr, nullptr);

}

void CHistogramTask::
ComputeCPU()
{
	m_histogram.assign(NUM_HIST_BINS, 0);
	CTimer timer;
	timer.Start();
	for(int y = 0; y < m_img_height; y++) {
		for(int x = 0; x < m_img_width; x++) {
			float p = m_pixels[y * m_img_stride + x] * float(NUM_HIST_BINS);
			int h_idx = std::min<int>(NUM_HIST_BINS - 1, std::max<int>(0, int(p)));
			m_histogram[h_idx]++;
		}
	}
	timer.Stop();

	std::cout << "  Histogram CPU time: " << timer.GetElapsedMilliseconds() << " ms\n";
}

bool CHistogramTask::
ValidateResults()
{
	bool is_same = true;
	assert(m_histogram.size() == m_histogram_gpu.size());
	for(size_t i = 0; i < m_histogram.size(); i++) {
		if(m_histogram[i] != m_histogram_gpu[i])
			is_same = false;
	}
	if(is_same) {
		print_histogram(m_histogram);
	}
	else {
		std::cout << "Results do not match!" << std::endl;
		std::cout << "Histogram CPU:" << std::endl;
		print_histogram(m_histogram);
		std::cout << "Histogram GPU:" << std::endl;
		print_histogram(m_histogram_gpu);

		std::cout << "CPU   GPU" << std::endl;
		for(size_t i = 0; i < m_histogram.size(); i++) {
			std::cout << m_histogram[i] << " " << m_histogram_gpu[i] << std::endl;
		}
	}
	return is_same;
}
