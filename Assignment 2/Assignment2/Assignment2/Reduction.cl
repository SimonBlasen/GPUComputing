
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_InterleavedAddressing(__global uint* array, uint stride) 
{
	int GID = get_global_id(0);

	
	array[GID * stride] = array[GID * stride] + array[GID * stride + (stride / 2)];
	

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_SequentialAddressing(__global uint* array, uint stride) 
{
	int GID = get_global_id(0);

	array[GID] = array[GID] + array[GID + stride];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_Decomp(const __global uint* inArray, __global uint* outArray, uint N, __local uint* localBlock)
{
	int GID = get_global_id(0);
	int LID = get_local_id(0);
	
	int writeBackPos = GID / N;
	
	localBlock[LID] = inArray[GID * 2] + inArray[GID * 2 + 1];
	
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	
	uint counter = 2;

	while (counter < N)
	{
		if ((LID % counter) == 0)
		{
			localBlock[LID] = localBlock[LID] + localBlock[LID + (counter / 2)];
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);
		counter = counter * 2;
	}
	
	if (LID == 0)
	{
		outArray[writeBackPos] = localBlock[0] + localBlock[N / 2];
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_DecompUnroll(const __global uint* inArray, __global uint* outArray, uint N, __local uint* localBlock)
{
	int GID = get_global_id(0);
	int LID = get_local_id(0);
	
	int writeBackPos = GID / N;
	
	localBlock[LID] = inArray[GID * 2] + inArray[GID * 2 + 1];
	
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if (N > 2)
	{
		if ((LID % 2) == 0)
		{
			localBlock[LID] = localBlock[LID] + localBlock[LID + 1];
		}
	
		barrier(CLK_LOCAL_MEM_FENCE);

		if (N > 4)
		{
			if ((LID % 4) == 0)
			{
				localBlock[LID] = localBlock[LID] + localBlock[LID + 2];
			}
	
			barrier(CLK_LOCAL_MEM_FENCE);

			if (N > 8)
			{
				if ((LID % 8) == 0)
				{
					localBlock[LID] = localBlock[LID] + localBlock[LID + 4];
				}
	
				barrier(CLK_LOCAL_MEM_FENCE);

				if (N > 16)
				{
					if ((LID % 16) == 0)
					{
						localBlock[LID] = localBlock[LID] + localBlock[LID + 8];
					}
	
					barrier(CLK_LOCAL_MEM_FENCE);

					if (N > 32)
					{
						if ((LID % 32) == 0)
						{
							localBlock[LID] = localBlock[LID] + localBlock[LID + 16];
						}
	
						barrier(CLK_LOCAL_MEM_FENCE);

						if (N > 64)
						{
							if ((LID % 64) == 0)
							{
								localBlock[LID] = localBlock[LID] + localBlock[LID + 32];
							}
	
							barrier(CLK_LOCAL_MEM_FENCE);

							if (N > 128)
							{
								if ((LID % 128) == 0)
								{
									localBlock[LID] = localBlock[LID] + localBlock[LID + 64];
								}
	
								barrier(CLK_LOCAL_MEM_FENCE);

								if (N > 256)
								{
									if ((LID % 256) == 0)
									{
										localBlock[LID] = localBlock[LID] + localBlock[LID + 128];
									}
	
									barrier(CLK_LOCAL_MEM_FENCE);

									if (N > 512)
									{
										if ((LID % 512) == 0)
										{
											localBlock[LID] = localBlock[LID] + localBlock[LID + 256];
										}
	
										barrier(CLK_LOCAL_MEM_FENCE);

										if (N > 1024)
										{
											if ((LID % 1024) == 0)
											{
												localBlock[LID] = localBlock[LID] + localBlock[LID + 512];
											}
	
											barrier(CLK_LOCAL_MEM_FENCE);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (LID == 0)
	{
		outArray[writeBackPos] = localBlock[0] + localBlock[N / 2];
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_DecompAtomics(const __global uint* inArray, __global uint* outArray, uint N, __local uint* localSum)
{
	int GID = get_global_id(0);
	int LID = get_local_id(0);
	
	int writeBackPos = GID / N;

	if (LID == 0)
	{
		localSum[0] = 0;
	}

	uint val = inArray[GID];
	
	barrier(CLK_LOCAL_MEM_FENCE);

	atomic_add(localSum, val);
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if (LID == 0)
	{
		outArray[GID / N] = localSum[0];
	}
	/*
	
	
	uint counter = 2;

	while (counter < N)
	{
		if ((LID % counter) == 0)
		{
			localBlock[LID] = localBlock[LID] + localBlock[LID + (counter / 2)];
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);
		counter = counter * 2;
	}
	
	if (LID == 0)
	{
		outArray[writeBackPos] = localBlock[0] + localBlock[N / 2];
	}*/
}
