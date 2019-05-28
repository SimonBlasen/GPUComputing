
__kernel void
set_array_to_constant(
	__global int *array,
	int num_elements,
	int val
)
{
	// There is no need to touch this kernel
	if(get_global_id(0) < num_elements)
		array[get_global_id(0)] = val;
}

__kernel void
compute_histogram(
	__global int *histogram,   // accumulate histogram here
	__global const float *img, // input image
	int width,                 // image width
	int height,                // image height
	int pitch,                 // image pitch
	int num_hist_bins          // number of histogram bins
)
{
	// Insert your kernel code here
} 

__kernel void
compute_histogram_local_memory(
	__global int *histogram,   // accumulate histogram here
	__global const float *img, // input image
	int width,                 // image width
	int height,                // image height
	int pitch,                 // image pitch
	int num_hist_bins,         // number of histogram bins
	__local int *local_hist
)
{
	// Insert your kernel code here
} 
