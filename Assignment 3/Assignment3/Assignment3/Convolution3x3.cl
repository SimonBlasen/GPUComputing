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

	// TO DO...

	// Fill the halo with zeros

	// Load main filtered area from d_Src

	// Load halo regions from d_Src (edges and corners separately), check for image bounds!

	// Sync threads

	// Perform the convolution and store the convolved signal to d_Dst.
}