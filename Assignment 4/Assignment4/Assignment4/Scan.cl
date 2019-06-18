

	//********************************************************
	// GPU Computing only!
	// (If you are doing the smaller course, ignore this file)
	//********************************************************

	// Add your previous parallel prefix sum code here
	
	
#define UNROLL
#define NUM_BANKS			32
#define NUM_BANKS_LOG		5
#define SIMD_GROUP_SIZE		32

// Bank conflicts
#define AVOID_BANK_CONFLICTS
#ifdef AVOID_BANK_CONFLICTS
	#define OFFSET(A) (((A) + ((uint)((A) / NUM_BANKS))))
#else
	#define OFFSET(A) (A)
#endif




__kernel void Scan(__global uint* inArray, __global uint* outArray, __global uint* higherLevelArray, __local uint* localBlock) 
{
	int GID = get_global_id(0);
	int LID = get_local_id(0);
	int LSIZE = get_local_size(0);


	
	localBlock[OFFSET(LID)] = inArray[GID * 2] + inArray[GID * 2 + 1];
	
	uint counter = 2;

	barrier(CLK_LOCAL_MEM_FENCE);

	while (counter <= LSIZE)
	{
		if (((LID + 1) % counter) == 0)
		{
			localBlock[OFFSET(LID)] = localBlock[OFFSET(LID)] + localBlock[OFFSET(LID - (counter / 2))];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		counter = counter * 2;
	}


	if (LID == (LSIZE - 1))
	{
		localBlock[OFFSET(LID)] = 0;
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);

	
	counter = LSIZE;

	while (counter > 1)
	{
		if (((LID + 1) % counter) == 0)
		{
			uint cached = localBlock[OFFSET(LID)];
			localBlock[OFFSET(LID)] = localBlock[OFFSET(LID)] + localBlock[OFFSET(LID - (counter / 2))];
			localBlock[OFFSET(LID - (counter / 2))] = cached;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		counter = counter / 2;
	}
	

	uint cached1 = inArray[GID * 2];
	uint cached2 = inArray[GID * 2 + 1];
	inArray[GID * 2] = localBlock[OFFSET(LID)] + cached1;
	inArray[GID * 2 + 1] = localBlock[OFFSET(LID)] + cached1 + cached2;

	if (LID == (LSIZE - 1))
	{
		outArray[GID / LSIZE] = localBlock[OFFSET(LID)] + cached1 + cached2;
	}
	
}

__kernel void ScanAdd(__global uint* higherLevelArray, __global uint* outArray, __local uint* localBlock) 
{
	int GID = get_global_id(0);
	int LID = get_local_id(0);
	int LSIZE = get_local_size(0);


	outArray[GID + LSIZE * 2] = outArray[GID + LSIZE * 2] + higherLevelArray[(GID) / (LSIZE * 2)];
	
}

__kernel void ScanNaive(const __global uint* inArray, __global uint* outArray, uint N, uint offset) 
{
	int GID = get_global_id(0);
	
	if (GID >= offset)
	{
		outArray[GID] = inArray[GID] + inArray[GID - offset];
	}
	else
	{
		outArray[GID] = inArray[GID];
	}
}

