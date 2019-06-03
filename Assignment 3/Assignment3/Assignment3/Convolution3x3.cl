/*
We assume a 3x3 (radius: 1) convolution kernel, which is not separable.
Each work-group will process a (TILE_X x TILE_Y) tile of the image.
For coalescing, TILE_X should be multiple of 16.

Instead of examining the image border for each kernel, we recommend to pad the image
to be the multiple of the given tile-size.
*/

//should be multiple of 32 on Fermi and 16 on pre-Fermi...
#define TILE_X 32 

#define TILE_Y 16

// d_Dst is the convolution of d_Src with the kernel c_Kernel
// c_Kernel is assumed to be a float[11] array of the 3x3 convolution constants, one multiplier (for normalization) and an offset (in this order!)
// With & Height are the image dimensions (should be multiple of the tile size)
__kernel __attribute__((reqd_work_group_size(TILE_X, TILE_Y, 1)))
void Convolution(
				__global float* d_Dst,
				__global const float* d_Src,
				__constant float* c_Kernel,
				uint Width,  // Use width to check for image bounds
				uint Height,
				uint Pitch   // Use pitch for offsetting between lines
				)
{
	// OpenCL allows to allocate the local memory from 'inside' the kernel (without using the clSetKernelArg() call)
	// in a similar way to standard C.
	// the size of the local memory necessary for the convolution is the tile size + the halo area
	__local float tile[TILE_Y + 2][TILE_X + 2];
	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);

	// TO DO...

	// Fill the halo with zeros
	// Load halo regions from d_Src (edges and corners separately), check for image bounds!
	
	if (GID.x < Pitch)
	{
		if (LID.y == 0)
		{
			if (GID.y == 0)
			{
				tile[0][LID.x + 1] = 0;
			}
			else
			{
				tile[0][LID.x + 1] = d_Src[(GID.y - 1) * Pitch + GID.x];
			}
		}
		
		if (LID.y == LSIZE.y - 1)
		{
			if (GID.y == Height - 1)
			{
				tile[TILE_Y + 1][LID.x + 1] = 0;
			}
			else
			{
				tile[TILE_Y + 1][LID.x + 1] = d_Src[(GID.y + 1) * Pitch + GID.x];
			}
		}

		
		if (LID.x == 0)
		{
			if (GID.x == 0)
			{
				tile[LID.y + 1][0] = 0;
			}
			else
			{
				tile[LID.y + 1][0] = d_Src[GID.y * Pitch + GID.x - 1];
			}
		}
		
		if (LID.x == LSIZE.x - 1)
		{
			if (GID.x == Width - 1)
			{
				tile[LID.y + 1][TILE_X + 1] = 0;
			}
			else
			{
				tile[LID.y + 1][TILE_X + 1] = d_Src[GID.y * Pitch + GID.x + 1];
			}
		}



		// Fill pitch region with 0s

		if (GID.x >= Width)
		{
			tile[LID.y + 1][LID.x + 1] = 0;
		}

		
		// Four corner pixels
		if (LID.x == 1 && LID.y == 1)
		{
			if (GID.x == 1 || GID.y == 1)
			{
				tile[0][0] = 0;
			}
			else
			{
				tile[0][0] = d_Src[(GID.y - 2) * Pitch + (GID.x - 2)];
			}
		}
		if (LID.x == 1 && LID.y == LSIZE.y - 2)
		{
			if (GID.x == 1 || GID.y == Height - 2)
			{
				tile[TILE_Y + 1][0] = 0;
			}
			else
			{
				tile[TILE_Y + 1][0] = d_Src[(GID.y + 2) * Pitch + (GID.x - 2)];
			}
		}
		if (LID.x == LSIZE.x - 2 && LID.y == 1)
		{
			if (GID.x == Width - 2 || GID.y == 1)
			{
				tile[0][TILE_X + 1] = 0;
			}
			else
			{
				tile[0][TILE_X + 1] = d_Src[(GID.y - 2) * Pitch + (GID.x + 2)];
			}
		}
		if (LID.x == LSIZE.x - 2 && LID.y == LSIZE.y - 2)
		{
			if (GID.x == Width - 2 || GID.y == Height - 2)
			{
				tile[TILE_Y + 1][TILE_X + 1] = 0;
			}
			else
			{
				tile[TILE_Y + 1][TILE_X + 1] = d_Src[(GID.y + 2) * Pitch + (GID.x + 2)];
			}
		}
	}

	


	// Load main filtered area from d_Src
	
	if (GID.x < Width)
	{
		tile[LID.y + 1][LID.x + 1] = d_Src[GID.y * Pitch + GID.x];
	}


	// Sync threads
	barrier(CLK_LOCAL_MEM_FENCE);

	// Perform the convolution and store the convolved signal to d_Dst.
	
	if (GID.x < Width && GID.y < Height)
	{
		d_Dst[GID.y * Pitch + GID.x] =(		tile[LID.y - 1 + 1][LID.x - 1 + 1] * c_Kernel[0]
										+	tile[LID.y - 1 + 1][LID.x + 0 + 1] * c_Kernel[1]
										+	tile[LID.y - 1 + 1][LID.x + 1 + 1] * c_Kernel[2]
										+	tile[LID.y + 0 + 1][LID.x - 1 + 1] * c_Kernel[3]
										+	tile[LID.y + 0 + 1][LID.x + 0 + 1] * c_Kernel[4]
										+	tile[LID.y + 0 + 1][LID.x + 1 + 1] * c_Kernel[5]
										+	tile[LID.y + 1 + 1][LID.x - 1 + 1] * c_Kernel[6]
										+	tile[LID.y + 1 + 1][LID.x + 0 + 1] * c_Kernel[7]
										+	tile[LID.y + 1 + 1][LID.x + 1 + 1] * c_Kernel[8]
										)
											* c_Kernel[9] + c_Kernel[10];
	}
	
	
}