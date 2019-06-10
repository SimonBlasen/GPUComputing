

	//********************************************************
	// GPU Computing only!
	// (If you are doing the smaller course, ignore this file)
	//********************************************************

	// Add your previous parallel prefix sum code here

__kernel void Scan(__global uint* inArray, __global uint* outArray, __global uint* higherLevelArray, __local uint* localBlock) 
{}

__kernel void ScanAdd(__global uint* higherLevelArray, __global uint* outArray, __local uint* localBlock) 
{}

__kernel void ScanNaive(const __global uint* inArray, __global uint* outArray, uint N, uint offset) 
{}

