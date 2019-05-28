
// Rotate the matrix CLOCKWISE

//naive implementation: move the elements of the matrix directly to their destinations
//this will cause unaligned memory accessed which - as we will see - should be avoided on the GPU

__kernel void MatrixRotNaive(__global const float* M, __global float* MR, uint SizeX, uint SizeY)
{

	// Get the global id to know, which pixel (data point) to read and where to find it in the one-dimensional array

	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);

	int posInArray = GID.y * SizeX + GID.x;

	// Rotate the data point 90 degrees clockwise
	int rotPosInArray = GID.x * SizeY + (SizeY - GID.y - 1);

	// Check if data is being accessed outside the bounds
	if (posInArray < SizeY * SizeX && rotPosInArray < SizeY * SizeX)
	{
		MR[rotPosInArray] = M[posInArray];
	}
}

//this kernel does the same thing, however, the local memory is used to
//transform a small chunk of the matrix locally
//then write it back after synchronization in a coalesced access pattern

__kernel void MatrixRotOptimized(__global const float* M, __global float* MR, uint SizeX, uint SizeY,
							__local float* block)
{

	// First get all the ids and sizes of the complete data and local memory

	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);


	// Get the data position of the current thread and the location in local memory to put it in

	int posInArray = GID.y * SizeX + GID.x;
	int posLocal = LID.y * LSIZE.x + LID.x;

	if (posInArray < SizeY * SizeX)
	{
		block[posLocal] = M[posInArray];
	}
	else
	{
		// In case of the local memory going outside the bounds of the data, the local memory should still be filled with something

		block[posLocal] = 0.0f;
	}
	

	// Wait for all threads to fill local memory buffer

	barrier(CLK_LOCAL_MEM_FENCE);


	// Calculate the local location to read from for this thread, so that when writing it later, it is written in horizontal order

	int lReadX = posLocal / LSIZE.y;
	int lReadY = LSIZE.y - (posLocal % LSIZE.y) - 1;


	// Calculate the left top pixels position in global space

	int GIDRootX = GID.x - LID.x;
	int GIDRootY = GID.y - LID.y;


	// Rotate the left top position

	int rotRootPosX = SizeY - GIDRootY - 1;
	int rotRootPosY = GIDRootX;
	rotRootPosX = rotRootPosX - (LSIZE.y - 1);



	// Determine the position to write, so that data is written in horizontal order

	int writePosX = rotRootPosX + (posLocal % LSIZE.y);
	int writePosY = rotRootPosY + (posLocal / LSIZE.y);
	int writePosArray = writePosY * SizeY + writePosX;


	// The read position in the local buffer array
	int localReadPos = lReadY * LSIZE.x + lReadX;



	// Check for accessing data out of bounds, especially in case of threads overlapping the border of the data

	if (writePosX >= 0 && writePosY >= 0 && localReadPos >= 0 && writePosArray < SizeX * SizeY && localReadPos < LSIZE.x * LSIZE.y) //if (writePosX < SizeY && writePosY < SizeX && lReadX < LSIZE.x && lReadY < LSIZE.y)
	{
		MR[writePosArray] = block[localReadPos];
	}



}
 

 




