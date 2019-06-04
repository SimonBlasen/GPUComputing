
#define KERNEL_LENGTH (2 * KERNEL_RADIUS + 1)

#define DEPTH_THRESHOLD	0.025f
#define NORM_THRESHOLD	0.9f

// These functions define discontinuities
bool IsNormalDiscontinuity(float4 n1, float4 n2){
	return fabs(dot(n1, n2)) < NORM_THRESHOLD;
}

bool IsDepthDiscontinuity(float d1, float d2){
	return fabs(d1 - d2) > DEPTH_THRESHOLD;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
// Horizontal convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(H_GROUPSIZE_X, H_GROUPSIZE_Y, 1)))
void DiscontinuityHorizontal(
			__global int* d_Disc,
			__global const float4* d_NormDepth,
			int Width,
			int Height,
			int Pitch
			)
{

	// TODO: Uncomment code and fill in the missing code. 
	// You don't have to follow the provided code. Feel free to adjust it if you want.

	// The size of the local memory: one value for each work-item.
	// We even load unused pixels to the halo area, to keep the code and local memory access simple.
	// Since these loads are coalesced, they introduce no overhead, except for slightly redundant local memory allocation.
	// Each work-item loads H_RESULT_STEPS values + 2 halo values
	// We split the float4 (normal + depth) into an array of float3 and float to avoid bank conflicts.

	__local float tileNormX[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	__local float tileNormY[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	__local float tileNormZ[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	__local float tileDepth[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);
	
	const int baseX = (GID.x - LID.x) * H_RESULT_STEPS;
	const int baseY = GID.y - LID.y;
	//const int offset = ...

	//Load left halo (each thread loads exactly one)
	float4 nd;
	int globalXPos = baseX + LID.x - H_GROUPSIZE_X;
	if (globalXPos >= 0)
	{
		nd = d_NormDepth[(GID.y * Pitch) + globalXPos];
	}
	else
	{
		nd = (float4) (0, 0, 0, 0);
	}

	tileNormX[LID.y][LID.x] = nd.x;
	tileNormY[LID.y][LID.x] = nd.y;
	tileNormZ[LID.y][LID.x] = nd.z;
	tileDepth[LID.y][LID.x] = nd.w;

	// Load main data + right halo
	// pragma unroll is not necessary as the compiler should unroll the short loops by itself.
	//#pragma unroll
	for (int tileID = 1; tileID <= H_RESULT_STEPS + 1; tileID++) {
		int xPos = baseX + LID.x + (H_GROUPSIZE_X * (tileID - 1));
		float4 ndR;
		if (xPos < Width)
		{
			ndR = d_NormDepth[(GID.y * Pitch) + xPos];
		}
		else
		{
			ndR = (float4) (0, 0, 0, 0);
		}
		tileNormX[LID.y][LID.x + tileID * H_GROUPSIZE_X] = ndR.x;
		tileNormY[LID.y][LID.x + tileID * H_GROUPSIZE_X] = ndR.y;
		tileNormZ[LID.y][LID.x + tileID * H_GROUPSIZE_X] = ndR.z;
		tileDepth[LID.y][LID.x + tileID * H_GROUPSIZE_X] = ndR.w;
	}

	// Sync threads
	barrier(CLK_LOCAL_MEM_FENCE);

	// Identify discontinuities
	//#pragma unroll
	for (int tileID = 1; tileID <= H_RESULT_STEPS; tileID++) {
		int flag = 0;

		float   myDepth = tileDepth[LID.y][LID.x + tileID * H_GROUPSIZE_X];
		float4  myNorm  = (float4) (tileNormX[LID.y][LID.x + tileID * H_GROUPSIZE_X], tileNormY[LID.y][LID.x + tileID * H_GROUPSIZE_X], tileNormZ[LID.y][LID.x + tileID * H_GROUPSIZE_X], 0);



		// Check the left neighbor
		float leftDepth	= tileDepth[LID.y][LID.x + tileID * H_GROUPSIZE_X - 1];
		float4 leftNorm	= (float4) (tileNormX[LID.y][LID.x + tileID * H_GROUPSIZE_X - 1], tileNormY[LID.y][LID.x + tileID * H_GROUPSIZE_X - 1], tileNormZ[LID.y][LID.x + tileID * H_GROUPSIZE_X - 1], 0);



		if (IsDepthDiscontinuity(myDepth, leftDepth) || IsNormalDiscontinuity(myNorm, leftNorm))
			flag |= 1;

		// Check the right neighbor
		float rightDepth	= tileDepth[LID.y][LID.x + tileID * H_GROUPSIZE_X + 1];
		float4 rightNorm	= (float4) (tileNormX[LID.y][LID.x + tileID * H_GROUPSIZE_X + 1], tileNormY[LID.y][LID.x + tileID * H_GROUPSIZE_X + 1], tileNormZ[LID.y][LID.x + tileID * H_GROUPSIZE_X + 1], 0);



		if (IsDepthDiscontinuity(myDepth, rightDepth) || IsNormalDiscontinuity(myNorm, rightNorm))
			flag |= 2;


		// Write the flag out

		d_Disc[GID.y * Pitch + baseX + LID.x + (H_GROUPSIZE_X * (tileID - 1))] = flag;


	}

	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertical convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(V_GROUPSIZE_X, V_GROUPSIZE_Y, 1)))
void DiscontinuityVertical(
			__global int* d_Disc,
			__global const float4* d_NormDepth,
			int Width,
			int Height,
			int Pitch
			)
{
	// Comments in the DiscontinuityHorizontal should be enough.
	// TODO

	// WARNING: For profiling reasons, it might happen that the framework will run
	// this kernel several times.

	// You need to make sure that the output of this kernel DOES NOT influence the input.
	// In this case, we are both reading and writing the d_Disc[] buffer.

	// here is a proposed solution: use separate flags for the vertical discontinuity
	// and merge this with the global discontinuity buffer, using bitwise OR.
	// This way do do not depent on the number of kernel executions.

	//int flag = 0;

	// if there is a discontinuity:
	// flag |= 4...

	//d_Disc['index'] |= flag; // do NOT use '='
	
	__local float tileNormX[V_GROUPSIZE_Y * (V_RESULT_STEPS + 2)][V_GROUPSIZE_X];
	__local float tileNormY[V_GROUPSIZE_Y * (V_RESULT_STEPS + 2)][V_GROUPSIZE_X];
	__local float tileNormZ[V_GROUPSIZE_Y * (V_RESULT_STEPS + 2)][V_GROUPSIZE_X];
	__local float tileDepth[V_GROUPSIZE_Y * (V_RESULT_STEPS + 2)][V_GROUPSIZE_X];
	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);
	
	const int baseX = GID.x - LID.x;
	const int baseY = (GID.y - LID.y) * V_RESULT_STEPS;
	//const int offset = ...

	//Load left halo (each thread loads exactly one)
	float4 nd;
	int globalYPos = baseY + LID.y - V_GROUPSIZE_Y;
	if (globalYPos >= 0 && (V_GROUPSIZE_Y - LID.y) <= 1)
	{
		nd = d_NormDepth[(globalYPos * Pitch) + GID.x];
	}
	else
	{
		nd = (float4) (0, 0, 0, 0);
	}

	tileNormX[LID.y][LID.x] = nd.x;
	tileNormY[LID.y][LID.x] = nd.y;
	tileNormZ[LID.y][LID.x] = nd.z;
	tileDepth[LID.y][LID.x] = nd.w;

	// Load main data + right halo
	// pragma unroll is not necessary as the compiler should unroll the short loops by itself.
	//#pragma unroll
	for (int tileID = 1; tileID <= V_RESULT_STEPS + 1; tileID++) {
		int yPos = baseY + LID.y + (V_GROUPSIZE_Y * (tileID - 1));
		float4 ndR;
		if (yPos < Height)
		{
			ndR = d_NormDepth[(yPos * Pitch) + GID.x];
		}
		else
		{
			ndR = (float4) (0, 0, 0, 0);
		}
		tileNormX[LID.y + tileID * V_GROUPSIZE_Y][LID.x] = ndR.x;
		tileNormY[LID.y + tileID * V_GROUPSIZE_Y][LID.x] = ndR.y;
		tileNormZ[LID.y + tileID * V_GROUPSIZE_Y][LID.x] = ndR.z;
		tileDepth[LID.y + tileID * V_GROUPSIZE_Y][LID.x] = ndR.w;
	}

	// Sync threads
	barrier(CLK_LOCAL_MEM_FENCE);

	// Identify discontinuities
	//#pragma unroll
	for (int tileID = 1; tileID <= V_RESULT_STEPS; tileID++) {
		int flag = 0;

		float   myDepth = tileDepth[LID.y + tileID * V_GROUPSIZE_Y][LID.x];
		float4  myNorm  = (float4) (tileNormX[LID.y + tileID * V_GROUPSIZE_Y][LID.x], tileNormY[LID.y + tileID * V_GROUPSIZE_Y][LID.x], tileNormZ[LID.y + tileID * V_GROUPSIZE_Y][LID.x], 0);



		// Check the upper neighbor
		float upperDepth	= tileDepth[LID.y + tileID * V_GROUPSIZE_Y - 1][LID.x];
		float4 upperNorm	= (float4) (tileNormX[LID.y + tileID * V_GROUPSIZE_Y - 1][LID.x], tileNormY[LID.y + tileID * V_GROUPSIZE_Y - 1][LID.x], tileNormZ[LID.y + tileID * V_GROUPSIZE_Y - 1][LID.x], 0);



		if (IsDepthDiscontinuity(myDepth, upperDepth) || IsNormalDiscontinuity(myNorm, upperNorm))
			flag |= 4;

		// Check the down neighbor
		float downDepth	= tileDepth[LID.y + tileID * V_GROUPSIZE_Y + 1][LID.x];
		float4 downNorm	= (float4) (tileNormX[LID.y + tileID * V_GROUPSIZE_Y + 1][LID.x], tileNormY[LID.y + tileID * V_GROUPSIZE_Y + 1][LID.x], tileNormZ[LID.y + tileID * V_GROUPSIZE_Y + 1][LID.x], 0);



		if (IsDepthDiscontinuity(myDepth, downDepth) || IsNormalDiscontinuity(myNorm, downNorm))
			flag |= 8;


		// Write the flag out

		d_Disc[(baseY + (V_GROUPSIZE_Y * (tileID - 1))) * Pitch + GID.x] |= flag;


	}

}









//////////////////////////////////////////////////////////////////////////////////////////////////////
// Horizontal convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(H_GROUPSIZE_X, H_GROUPSIZE_Y, 1)))
void ConvHorizontal(
			__global float* d_Dst,
			__global const float* d_Src,
			__global const int* d_Disc,
			__constant float* c_Kernel,
			int Width,
			int Height,
			int Pitch
			)
{

	// TODO
	// This will be very similar to the separable convolution, except that you have
	// also load the discontinuity buffer into the local memory
	// Each work-item loads H_RESULT_STEPS values + 2 halo values
	//__local float tile[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	//__local int   disc[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];

	// Load data to the tile and disc local arrays

	// During the convolution iterate inside-out from the center pixel towards the borders.
	//for (...) // Iterate over tiles

	// When you iterate to the left, check for 'left' discontinuities. 
	//for (... > -KERNEL_RADIUS...)
	// If you find relevant discontinuity, stop iterating

	// When iterating to the right, check for 'right' discontinuities.
	//for (... < KERNEL_RADIUS...)
	// If you find a relevant discontinuity, stop iterating

	// Don't forget to accumulate the weights to normalize the kernel (divide the pixel value by the summed weights)
	


	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);


	d_Dst[GID.y * Pitch + GID.x] = d_Disc[GID.y * Pitch + GID.x];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertical convolution filter



//require matching work-group size
__kernel __attribute__((reqd_work_group_size(V_GROUPSIZE_X, V_GROUPSIZE_Y, 1)))
void ConvVertical(
			__global float* d_Dst,
			__global const float* d_Src,
			__global const int* d_Disc,
			__constant float* c_Kernel,
			int Width,
			int Height,
			int Pitch
			)
{

	// TODO

	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);


	d_Dst[GID.y * Pitch + GID.x] = d_Disc[GID.y * Pitch + GID.x];
}




