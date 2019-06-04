
//Each thread load exactly one halo pixel
//Thus, we assume that the halo size is not larger than the 
//dimension of the work-group in the direction of the kernel

//to efficiently reduce the memory transfer overhead of the global memory
// (each pixel is lodaded multiple times at high overlaps)
// one work-item will compute RESULT_STEPS pixels

//for unrolling loops, these values have to be known at compile time

/* These macros will be defined dynamically during building the program

#define KERNEL_RADIUS 2

//horizontal kernel
#define H_GROUPSIZE_X		32
#define H_GROUPSIZE_Y		4
#define H_RESULT_STEPS		2

//vertical kernel
#define V_GROUPSIZE_X		32
#define V_GROUPSIZE_Y		16
#define V_RESULT_STEPS		3

*/

#define KERNEL_LENGTH (2 * KERNEL_RADIUS + 1)


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Horizontal convolution filter

/*
c_Kernel stores 2 * KERNEL_RADIUS + 1 weights, use these during the convolution
*/

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(H_GROUPSIZE_X, H_GROUPSIZE_Y, 1)))
void ConvHorizontal(
			__global float* d_Dst,
			__global const float* d_Src,
			__constant float* c_Kernel,
			int Width,
			int Pitch
			)
{
	//The size of the local memory: one value for each work-item.
	//We even load unused pixels to the halo area, to keep the code and local memory access simple.
	//Since these loads are coalesced, they introduce no overhead, except for slightly redundant local memory allocation.
	//Each work-item loads H_RESULT_STEPS values + 2 halo values
	__local float tile[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);
	

	// TODO:
	const int baseX = (GID.x - LID.x) * H_RESULT_STEPS;
	const int baseY = GID.y - LID.y;
	//const int offset = ...


	// Load left halo (check for left bound)
	int xReadLeft = baseX + LID.x - (H_GROUPSIZE_X);
	if (xReadLeft >= 0)
	{
		tile[LID.y][LID.x] = d_Src[((baseY + LID.y) * Pitch) + xReadLeft];
	}
	else
	{
		tile[LID.y][LID.x] = 0;
	}
	


	// Load main data + right halo (check for right bound)
	
	#pragma unroll
	for (int tileID = 1; tileID <= H_RESULT_STEPS + 1; tileID++)
	{
		int globalXPos = baseX + (H_GROUPSIZE_X * (tileID - 1)) + LID.x;
		if (globalXPos >= Width)
		{
			tile[LID.y][LID.x + (H_GROUPSIZE_X * tileID)] = 0;
		}
		else
		{
			tile[LID.y][LID.x + (H_GROUPSIZE_X * tileID)] = d_Src[((baseY + LID.y) * Pitch) + globalXPos];
		}
	}

	

	// Sync the work-items after loading
	barrier(CLK_LOCAL_MEM_FENCE);

	// Convolve and store the result


	
	#pragma unroll
	for (int tileID = 1; tileID <= H_RESULT_STEPS; tileID++)
	{
		int globalXPos = baseX + (H_GROUPSIZE_X * (tileID - 1)) + LID.x;
		if (globalXPos < Width)
		{
			float sum = 0;
			for (int k = 0; k < KERNEL_RADIUS * 2 + 1; k++)
			{
				sum += (tile[LID.y][LID.x + (H_GROUPSIZE_X * tileID) + k - KERNEL_RADIUS]) * (c_Kernel[k]);
			}

			d_Dst[((baseY + LID.y) * Pitch) + globalXPos] = sum;    //((LID.x + (tileID - 1) * H_GROUPSIZE_X) / ((float)(H_GROUPSIZE_X * H_RESULT_STEPS)));
		}
	}
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertical convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(V_GROUPSIZE_X, V_GROUPSIZE_Y, 1)))
void ConvVertical(
			__global float* d_Dst,
			__global const float* d_Src,
			__constant float* c_Kernel,
			int Height,
			int Pitch
			)
{
	__local float tile[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];

	//TO DO:
	// Conceptually similar to ConvHorizontal
	// Load top halo + main data + bottom halo

	// Compute and store results
	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);
	

	// TODO:
	const int baseX = GID.x - LID.x;
	const int baseY = (GID.y - LID.y) * V_RESULT_STEPS;
	//const int offset = ...


	
	// Load left halo (check for left bound)
	int yRead = baseY + LID.y - (V_GROUPSIZE_Y);
	if (yRead >= 0 && (V_GROUPSIZE_Y - LID.y) <= KERNEL_RADIUS)		//  <--  Don't do unused loads of global memory
	{
		tile[LID.y][LID.x] = d_Src[(yRead * Pitch) + baseX + LID.x];
	}
	else
	{
		tile[LID.y][LID.x] = 0;
	}

	
	// Load main data + down halo (check for right bound)
	
	#pragma unroll
	for (int tileID = 1; tileID <= V_RESULT_STEPS + 1; tileID++)
	{
		int globalYPos = baseY + (V_GROUPSIZE_Y * (tileID - 1)) + LID.y;
		if (globalYPos >= Height)
		{
			tile[LID.y + (V_GROUPSIZE_Y * tileID)][LID.x] = 0;
		}
		else
		{
			tile[LID.y + (V_GROUPSIZE_Y * tileID)][LID.x] = d_Src[(globalYPos * Pitch) + baseX + LID.x];
		}
	}


	
	// Sync the work-items after loading
	barrier(CLK_LOCAL_MEM_FENCE);

	// Convolve and store the result


	
	#pragma unroll
	for (int tileID = 1; tileID <= V_RESULT_STEPS; tileID++)
	{
		int globalYPos = baseY + (V_GROUPSIZE_Y * (tileID - 1)) + LID.y;
		if (globalYPos < Height)
		{
			float sum = 0;
			for (int k = 0; k < KERNEL_RADIUS * 2 + 1; k++)
			{
				sum += (tile[LID.y + (V_GROUPSIZE_Y * tileID) + k - KERNEL_RADIUS][LID.x]) * (c_Kernel[k]);
			}

			d_Dst[(globalYPos * Pitch) + baseX + LID.x] = sum;
		}
	}





}
