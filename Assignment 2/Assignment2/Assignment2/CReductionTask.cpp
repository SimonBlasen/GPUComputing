/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CReductionTask.h"

#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CReductionTask

string g_kernelNames[5] = {
	"interleavedAddressing",
	"sequentialAddressing",
	"kernelDecomposition",
	"kernelDecompositionUnroll",
	"kernelDecompositionAtomics"
};

CReductionTask::CReductionTask(size_t ArraySize)
	: m_N(ArraySize), m_hInput(NULL), 
	m_dPingArray(NULL),
	m_dPongArray(NULL),
	m_Program(NULL), 
	m_InterleavedAddressingKernel(NULL), m_SequentialAddressingKernel(NULL), m_DecompKernel(NULL), m_DecompUnrollKernel(NULL), m_DecompAtomicsKernel(NULL)
{
}

CReductionTask::~CReductionTask()
{
	ReleaseResources();
}

bool CReductionTask::InitResources(cl_device_id Device, cl_context Context)
{
	//CPU resources
	m_hInput = new unsigned int[m_N];

	//fill the array with some values
	for(unsigned int i = 0; i < m_N; i++) 
		//m_hInput[i] = 1;			// Use this for debugging
		m_hInput[i] = rand() & 15;

	//device resources
	cl_int clError, clError2;
	m_dPingArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * m_N, NULL, &clError2);
	clError = clError2;
	m_dPongArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * m_N, NULL, &clError2);
	clError |= clError2;
	V_RETURN_FALSE_CL(clError, "Error allocating device arrays");

	//load and compile kernels
	string programCode;

	CLUtil::LoadProgramSourceToMemory("Reduction.cl", programCode);
	m_Program = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(m_Program == nullptr) return false;

	//create kernels
	m_InterleavedAddressingKernel = clCreateKernel(m_Program, "Reduction_InterleavedAddressing", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_InterleavedAddressing.");

	m_SequentialAddressingKernel = clCreateKernel(m_Program, "Reduction_SequentialAddressing", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_SequentialAddressing.");

	m_DecompKernel = clCreateKernel(m_Program, "Reduction_Decomp", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_Decomp.");

	m_DecompUnrollKernel = clCreateKernel(m_Program, "Reduction_DecompUnroll", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_DecompUnroll.");

	m_DecompAtomicsKernel = clCreateKernel(m_Program, "Reduction_DecompAtomics", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_DecompAtomics.");

	return true;
}

void CReductionTask::ReleaseResources()
{
	// host resources
	SAFE_DELETE_ARRAY(m_hInput);

	// device resources
	SAFE_RELEASE_MEMOBJECT(m_dPingArray);
	SAFE_RELEASE_MEMOBJECT(m_dPongArray);

	SAFE_RELEASE_KERNEL(m_InterleavedAddressingKernel);
	SAFE_RELEASE_KERNEL(m_SequentialAddressingKernel);
	SAFE_RELEASE_KERNEL(m_DecompKernel);
	SAFE_RELEASE_KERNEL(m_DecompUnrollKernel);
	SAFE_RELEASE_KERNEL(m_DecompAtomicsKernel);

	SAFE_RELEASE_PROGRAM(m_Program);
}

void CReductionTask::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 0);
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 1);
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 2);
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 3);
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 4);

	TestPerformance(Context, CommandQueue, LocalWorkSize, 0);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 1);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 2);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 3);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 4);

}

void CReductionTask::ComputeCPU()
{
	CTimer timer;
	timer.Start();

	unsigned int nIterations = 10;
	for(unsigned int j = 0; j < nIterations; j++) {
		m_resultCPU = m_hInput[0];
		for(unsigned int i = 1; i < m_N; i++) {
			m_resultCPU += m_hInput[i]; 
		}
	}

	timer.Stop();

	double ms = timer.GetElapsedMilliseconds() / double(nIterations);
	cout << "  average time: " << ms << " ms, throughput: " << 1.0e-6 * (double)m_N / ms << " Gelem/s" <<endl;
}

bool CReductionTask::ValidateResults()
{
	bool success = true;

	//TODO
	for(int i = 0; i < 5; i++)
		if(m_resultGPU[i] != m_resultCPU)
		{
			cout<<"Validation of reduction kernel "<<g_kernelNames[i]<<" failed." << endl;
			success = false;
		}

	return success;
}

void CReductionTask::Reduction_InterleavedAddressing(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	cl_int clErr;
	size_t globalWorkSize[1];
	size_t localWorkSize[1];
	size_t nGroups[2];
	unsigned int stride;

	// TO DO: Implement reduction with interleaved addressing


	if (CLUtil::IsTwoPot(LocalWorkSize[0]) == false)
	{
		cout << "ERROR: Local Work Size is no potence of 2" << endl;
		return;
	}



	unsigned int iterations = CLUtil::LogBase2(m_N);

	//cout << "  Exec Reduction_Interleaved running " << iterations << " iterations" << endl;

	for (unsigned int i = 0; i < iterations; i++)
	{
		stride = CLUtil::TwoPow(i + 1);

		globalWorkSize[0] = m_N / stride;// CLUtil::GetGlobalWorkSize((m_N / stride), LocalWorkSize[0]);
		if (globalWorkSize[0] < LocalWorkSize[0])
		{
			localWorkSize[0] = globalWorkSize[0];
		}
		else
		{
			localWorkSize[0] = LocalWorkSize[0];
		}
		nGroups[0] = globalWorkSize[0] / localWorkSize[0];
		/*
		cout << i << ". Iteration" << endl;
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Groups:  " << nGroups[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  Stride: " << stride << endl;
		*/

		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_InterleavedAddressingKernel, 0, sizeof(cl_mem), (void*)&m_dPingArray);
		//clErr |= clSetKernelArg(m_InterleavedAddressingKernel, 1, sizeof(cl_uint), (void*)&m_N);
		clErr |= clSetKernelArg(m_InterleavedAddressingKernel, 1, sizeof(cl_uint), (void*)&stride);
		clErr |= clFinish(CommandQueue);
		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_InterleavedAddressingKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}

	}


	


	//for (...) {
	//}
}

void CReductionTask::Reduction_SequentialAddressing(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	cl_int clErr;
	size_t globalWorkSize[1];
	size_t localWorkSize[1];
	size_t nGroups[2];
	unsigned int stride;
	unsigned int offset;


	if (CLUtil::IsTwoPot(LocalWorkSize[0]) == false)
	{
		cout << "ERROR: Local Work Size is no potence of 2" << endl;
		return;
	}



	unsigned int iterations = CLUtil::LogBase2(m_N);

	//cout << "  Exec Reduction_Sequential running " << iterations << " iterations" << endl;

	for (unsigned int i = 0; i < iterations; i++)
	{
		stride = CLUtil::TwoPow(i + 1);
		offset = CLUtil::TwoPow(iterations - 1 - i);

		globalWorkSize[0] = m_N / stride;// CLUtil::GetGlobalWorkSize((m_N / stride), LocalWorkSize[0]);
		if (globalWorkSize[0] < LocalWorkSize[0])
		{
			localWorkSize[0] = globalWorkSize[0];
		}
		else
		{
			localWorkSize[0] = LocalWorkSize[0];
		}
		nGroups[0] = globalWorkSize[0] / localWorkSize[0];
		/*
		cout << i << ". Iteration" << endl;
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Groups:  " << nGroups[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  Offset:  " << offset << endl;
		*/

		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_SequentialAddressingKernel, 0, sizeof(cl_mem), (void*)&m_dPingArray);
		clErr |= clSetKernelArg(m_SequentialAddressingKernel, 1, sizeof(cl_uint), (void*)&offset);
		clErr |= clFinish(CommandQueue);
		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_SequentialAddressingKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}

	}

}

void CReductionTask::Reduction_Decomp(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	// TO DO: Implement reduction with kernel decomposition

	// NOTE: make sure that the final result is always in the variable m_dPingArray
	// as this is read back for the correctness check
	// (CReductionTask::ExecuteTask)
	//
	// hint: for example, you can use swap(m_dPingArray, m_dPongArray) at the end of your for loop...


	cl_int clErr;
	size_t globalWorkSize[1];
	size_t localWorkSize[1];


	unsigned int leftElements = m_N;
	unsigned int pingPong = 0;

	while (leftElements >= (LocalWorkSize[0] * 2))
	{
		globalWorkSize[0] = leftElements / 2;
		localWorkSize[0] = LocalWorkSize[0];


		unsigned int n = localWorkSize[0];
		/*
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  LEl:     " << leftElements << endl;
		*/
		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_DecompKernel, pingPong, sizeof(cl_mem), (void*)&m_dPingArray);
		clErr |= clSetKernelArg(m_DecompKernel, 1 - pingPong, sizeof(cl_mem), (void*)&m_dPongArray);
		clErr |= clSetKernelArg(m_DecompKernel, 2, sizeof(cl_uint), (void*)&n);
		clErr |= clSetKernelArg(m_DecompKernel, 3, sizeof(cl_uint) * LocalWorkSize[0], NULL);
		clErr |= clFinish(CommandQueue);

		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_DecompKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}

		pingPong = 1 - pingPong;
		leftElements = leftElements / (LocalWorkSize[0] * 2);
	}

	if (leftElements > 1)
	{
		//cout << "Executing one last kernel" << endl;
		// Execute one last kernel to sum everything left up

		globalWorkSize[0] = leftElements / 2;
		localWorkSize[0] = LocalWorkSize[0];
		if (globalWorkSize[0] < LocalWorkSize[0])
		{
			localWorkSize[0] = globalWorkSize[0];
		}


		unsigned int n = localWorkSize[0];
		/*
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  LEl:     " << leftElements << endl;
		*/
		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_DecompKernel, pingPong, sizeof(cl_mem), (void*)&m_dPingArray);
		clErr |= clSetKernelArg(m_DecompKernel, 1 - pingPong, sizeof(cl_mem), (void*)&m_dPongArray);
		clErr |= clSetKernelArg(m_DecompKernel, 2, sizeof(cl_uint), (void*)&n);
		clErr |= clSetKernelArg(m_DecompKernel, 3, sizeof(cl_uint) * LocalWorkSize[0], NULL);
		clErr |= clFinish(CommandQueue);

		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_DecompKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

		pingPong = 1 - pingPong;
		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}
	}

	//if (pingPong == 1)
	//	cout << "Pong has results" << endl;
	//else
	//	cout << "Ping has results" << endl;

	if (pingPong == 1)
		V_RETURN_CL(clEnqueueCopyBuffer(CommandQueue, m_dPongArray, m_dPingArray, 0, 0, 4, 0, NULL, NULL), "Error swapping ping pong");

}

void CReductionTask::Reduction_DecompUnroll(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	// TO DO: Implement reduction with loop unrolling

	// NOTE: make sure that the final result is always in the variable m_dPingArray
	// as this is read back for the correctness check
	// (CReductionTask::ExecuteTask)
	//
	// hint: for example, you can use swap(m_dPingArray, m_dPongArray) at the end of your for loop...

	cl_int clErr;
	size_t globalWorkSize[1];
	size_t localWorkSize[1];


	unsigned int leftElements = m_N;
	unsigned int pingPong = 0;

	while (leftElements >= (LocalWorkSize[0] * 2))
	{
		globalWorkSize[0] = leftElements / 2;
		localWorkSize[0] = LocalWorkSize[0];


		unsigned int n = localWorkSize[0];
		/*
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  LEl:     " << leftElements << endl;
		*/
		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, pingPong, sizeof(cl_mem), (void*)&m_dPingArray);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, 1 - pingPong, sizeof(cl_mem), (void*)&m_dPongArray);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, 2, sizeof(cl_uint), (void*)&n);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, 3, sizeof(cl_uint) * LocalWorkSize[0], NULL);
		clErr |= clFinish(CommandQueue);

		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_DecompUnrollKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}

		pingPong = 1 - pingPong;
		leftElements = leftElements / (LocalWorkSize[0] * 2);
	}

	if (leftElements > 1)
	{
		//cout << "Executing one last kernel" << endl;
		// Execute one last kernel to sum everything left up

		globalWorkSize[0] = leftElements / 2;
		localWorkSize[0] = LocalWorkSize[0];
		if (globalWorkSize[0] < LocalWorkSize[0])
		{
			localWorkSize[0] = globalWorkSize[0];
		}


		unsigned int n = localWorkSize[0];
		/*
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  LEl:     " << leftElements << endl;
		*/
		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, pingPong, sizeof(cl_mem), (void*)&m_dPingArray);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, 1 - pingPong, sizeof(cl_mem), (void*)&m_dPongArray);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, 2, sizeof(cl_uint), (void*)&n);
		clErr |= clSetKernelArg(m_DecompUnrollKernel, 3, sizeof(cl_uint) * LocalWorkSize[0], NULL);
		clErr |= clFinish(CommandQueue);

		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_DecompUnrollKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

		pingPong = 1 - pingPong;
		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}
	}

	//if (pingPong == 1)
	//	cout << "Pong has results" << endl;
	//else
	//	cout << "Ping has results" << endl;

	if (pingPong == 1)
		V_RETURN_CL(clEnqueueCopyBuffer(CommandQueue, m_dPongArray, m_dPingArray, 0, 0, 4, 0, NULL, NULL), "Error swapping ping pong");

}

void CReductionTask::Reduction_DecompAtomics(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	cl_int clErr;
	size_t globalWorkSize[1];
	size_t localWorkSize[1];


	unsigned int leftElements = m_N;
	unsigned int pingPong = 0;

	while (leftElements >= (LocalWorkSize[0]))
	{
		globalWorkSize[0] = leftElements;
		localWorkSize[0] = LocalWorkSize[0];


		unsigned int n = localWorkSize[0];
		/*
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  LEl:     " << leftElements << endl;
		*/
		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, pingPong, sizeof(cl_mem), (void*)&m_dPingArray);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, 1 - pingPong, sizeof(cl_mem), (void*)&m_dPongArray);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, 2, sizeof(cl_uint), (void*)&n);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, 3, sizeof(cl_uint) * 1, NULL);
		clErr |= clFinish(CommandQueue);

		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_DecompAtomicsKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}

		pingPong = 1 - pingPong;
		leftElements = leftElements / (LocalWorkSize[0]);
	}

	if (leftElements > 1)
	{
		//cout << "Executing one last kernel" << endl;
		// Execute one last kernel to sum everything left up

		globalWorkSize[0] = leftElements;
		localWorkSize[0] = LocalWorkSize[0];
		if (globalWorkSize[0] < LocalWorkSize[0])
		{
			localWorkSize[0] = globalWorkSize[0];
		}


		unsigned int n = localWorkSize[0];
		/*
		cout << "  Threads: " << globalWorkSize[0] << endl;
		cout << "  Size:    " << localWorkSize[0] << endl;
		cout << "  LEl:     " << leftElements << endl;
		*/
		clErr = clFinish(CommandQueue);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, pingPong, sizeof(cl_mem), (void*)&m_dPingArray);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, 1 - pingPong, sizeof(cl_mem), (void*)&m_dPongArray);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, 2, sizeof(cl_uint), (void*)&n);
		clErr |= clSetKernelArg(m_DecompAtomicsKernel, 3, sizeof(cl_uint) * 1, NULL);
		clErr |= clFinish(CommandQueue);

		clErr |= clEnqueueNDRangeKernel(CommandQueue, m_DecompAtomicsKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

		pingPong = 1 - pingPong;
		if (clErr != CL_SUCCESS)
		{
			cout << "kernel execution failed" << endl;
		}
	}

	//if (pingPong == 1)
	//	cout << "Pong has results" << endl;
	//else
	//	cout << "Ping has results" << endl;

	if (pingPong == 1)
		V_RETURN_CL(clEnqueueCopyBuffer(CommandQueue, m_dPongArray, m_dPingArray, 0, 0, 4, 0, NULL, NULL), "Error swapping ping pong");

}

void CReductionTask::ExecuteTask(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int Task)
{
	//write input data to the GPU
	V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_dPingArray, CL_FALSE, 0, m_N * sizeof(cl_uint), m_hInput, 0, NULL, NULL), "Error copying data from host to device!");

	//run selected task
	switch (Task){
		case 0:
			Reduction_InterleavedAddressing(Context, CommandQueue, LocalWorkSize);
			break;
		case 1:
			Reduction_SequentialAddressing(Context, CommandQueue, LocalWorkSize);
			break;
		case 2:
			Reduction_Decomp(Context, CommandQueue, LocalWorkSize);
			break;
		case 3:
			Reduction_DecompUnroll(Context, CommandQueue, LocalWorkSize);
			break;
		case 4:
			Reduction_DecompAtomics(Context, CommandQueue, LocalWorkSize);
			break;

	}

	//read back the results synchronously.
	m_resultGPU[Task] = 0;
	V_RETURN_CL(clEnqueueReadBuffer(CommandQueue, m_dPingArray, CL_TRUE, 0, 1 * sizeof(cl_uint), &m_resultGPU[Task], 0, NULL, NULL), "Error reading data from device!");
	
}

void CReductionTask::TestPerformance(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int Task)
{
	cout << "Testing performance of task " << g_kernelNames[Task] << endl;

	//write input data to the GPU
	V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_dPingArray, CL_FALSE, 0, m_N * sizeof(cl_uint), m_hInput, 0, NULL, NULL), "Error copying data from host to device!");
	//finish all before we start meassuring the time
	V_RETURN_CL(clFinish(CommandQueue), "Error finishing the queue!");

	CTimer timer;
	timer.Start();

	//run the kernel N times
	unsigned int nIterations = 100;
	for(unsigned int i = 0; i < nIterations; i++) {
		//run selected task
		switch (Task){
			case 0:
				Reduction_InterleavedAddressing(Context, CommandQueue, LocalWorkSize);
				break;
			case 1:
				Reduction_SequentialAddressing(Context, CommandQueue, LocalWorkSize);
				break;
			case 2:
				Reduction_Decomp(Context, CommandQueue, LocalWorkSize);
				break;
			case 3:
				Reduction_DecompUnroll(Context, CommandQueue, LocalWorkSize);
				break;
			case 4:
				Reduction_DecompAtomics(Context, CommandQueue, LocalWorkSize);
				break;
		}
	}

	//wait until the command queue is empty again
	V_RETURN_CL(clFinish(CommandQueue), "Error finishing the queue!");

	timer.Stop();

	double ms = timer.GetElapsedMilliseconds() / double(nIterations);
	cout << "  average time: " << ms << " ms, throughput: " << 1.0e-6 * (double)m_N / ms << " Gelem/s" <<endl;
}

///////////////////////////////////////////////////////////////////////////////
