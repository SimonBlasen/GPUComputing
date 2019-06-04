
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

	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);

	if (GID.x < width && GID.y < height)
	{
		float val = img[GID.y * pitch + GID.x] * ((float)num_hist_bins);
		int index = (int)val;
		if (index < 0)
		{
			index = 0;
		}
		else if (index >= num_hist_bins)
		{
			index = num_hist_bins - 1;
		}
		
		atomic_add(histogram + index, 1);
	}
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
	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);
	
	int threadID = LID.x + LID.y * LSIZE.y;

	if (threadID < num_hist_bins)
	{
		local_hist[threadID] = 0;
	}

	barrier(CLK_LOCAL_MEM_FENCE);



	if (GID.x < width && GID.y < height)
	{
		float val = img[GID.y * pitch + GID.x] * ((float)num_hist_bins);
		int index = (int)val;
		if (index < 0)
		{
			index = 0;
		}
		else if (index >= num_hist_bins)
		{
			index = num_hist_bins - 1;
		}
		
		atomic_add(local_hist + index, 1);
	}

	
	barrier(CLK_LOCAL_MEM_FENCE);

	if (threadID < num_hist_bins)
	{
		int val = local_hist[threadID];
		atomic_add(histogram + threadID, val);
	}

} 
