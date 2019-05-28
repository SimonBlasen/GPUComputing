/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment1.h"

#include "CSimpleArraysTask.h"
#include "CMatrixRotateTask.h"

#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CAssignment1

bool CAssignment1::DoCompute()
{
	// Task 1: simple array addition.
	cout << "Running vector addition example..." << endl << endl;

	// Trying different array sizes and workgroup sizes
	for (int i = 0; i < 4; i++)
	{
		{
			size_t localWorkSize[3] = { 256, 1, 1 };
			CSimpleArraysTask task(1048576 * (i + 1));
			RunComputeTask(task, localWorkSize);
		}
		{
			size_t LocalWorkSize[3] = { 512, 1, 1 };
			CSimpleArraysTask task(1048576 * (i + 1));
			RunComputeTask(task, LocalWorkSize);
		}
		{
			size_t LocalWorkSize[3] = { 8, 1, 1 };
			CSimpleArraysTask task(1048576 * (i + 1));
			RunComputeTask(task, LocalWorkSize);
		}
		{
			size_t LocalWorkSize[3] = { 16, 1, 1 };
			CSimpleArraysTask task(1048576 * (i + 1));
			RunComputeTask(task, LocalWorkSize);
		}
	}


	// Task 2: matrix rotation.
	std::cout << "Running matrix rotation example..." << std::endl << std::endl;

	// I re-pasted the original code here, as I wasn't sure about what Local-Sizes are supported by the ATIS PCs
	{
		size_t LocalWorkSize[3] = { 32, 16, 1 };
		CMatrixRotateTask task(2048, 1025);
		RunComputeTask(task, LocalWorkSize);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
