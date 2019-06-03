/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment3.h"

#include "CConvolution3x3Task.h"
#include "CConvolutionSeparableTask.h"
#include "CConvolutionBilateralTask.h"
#include "CHistogramTask.h"

#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CAssignment3

bool CAssignment3::DoCompute()
{
	cout<<"########################################"<<endl;
	cout<<"GPU Computing assignment 3"<<endl<<endl;

	cout<<"IMPORTANT: Make sure you always check the difference images."<<endl;
	cout<<"The CPU 'gold' test is only suitable to catch trivial errors,"<<endl;
	cout<<"A low MSE (mean squared error) might still happen with a few corrupted pixels."<<endl;

	cout<<"########################################"<<endl;
	cout<<"Task 1: 3x3 convolution"<<endl<<endl;
	{
		size_t TileSize[2] = {32, 16};
		float ConvKernel[3][3] = {
			{ -1.0f / 8.0f, -1.0f / 8.0f, -1.0f / 8.0f },
			{ -1.0f / 8.0f,  1.0f,        -1.0f / 8.0f },
			{ -1.0f / 8.0f, -1.0f / 8.0f, -1.0f / 8.0f },
		};
		CConvolution3x3Task convTask("Images/input.pfm", TileSize, ConvKernel, true, 0.0f);
		RunComputeTask(convTask, TileSize);
	}
	/*

	cout<<endl<<"########################################"<<endl;
	cout<<"Task 2: Separable convolution"<<endl<<endl;
	{
		size_t HGroupSize[2] = {32, 16};
		size_t VGroupSize[2] = {32, 16};

	
		{
			//simple box filter
			float ConvKernel[9];
			for(int i = 0; i < 9; i++)
				ConvKernel[i] = 1.0f / 9.0f;

			CConvolutionSeparableTask convTask("box_4x4", "Images/input.pfm", HGroupSize, VGroupSize,
				4, 4, 4, ConvKernel, ConvKernel);
			// note: the last argument is ignored, but our framework requires it
			// for the horizontal and vertical passes different local sizes might be used
			RunComputeTask(convTask, HGroupSize);
		}

		{
			//simple box filter
			float ConvKernel[17];
			for(int i = 0; i < 17; i++)
				ConvKernel[i] = 1.0f / 17.0f;

			CConvolutionSeparableTask convTask("box_8x8", "Images/input.pfm", HGroupSize, VGroupSize,
				4, 4, 8, ConvKernel, ConvKernel);
			RunComputeTask(convTask, HGroupSize);
		}

		{
			// Gaussian blur
			float ConvKernel[7] = {
				0.000817774f, 0.0286433f, 0.235018f, 0.471041f, 0.235018f, 0.0286433f, 0.000817774f
			};
			CConvolutionSeparableTask convTask("gauss_3x3", "Images/input.pfm", HGroupSize, VGroupSize,
				4, 4, 3, ConvKernel, ConvKernel);
			RunComputeTask(convTask, HGroupSize);
		}
	}


	cout<<endl<<"########################################"<<endl;
	cout<<"Task 3: Separable bilateral convolution"<<endl<<endl;
	{
		size_t HGroupSize[2] = {32, 4};
		size_t VGroupSize[2] = {32, 4};

		float ConvKernel[9] = {0.010284844f,	0.0417071f,	0.113371652f,	0.206576619f,	0.252313252f,	0.206576619f,	0.113371652f,	0.0417071f,	0.010284844f};

		CConvolutionBilateralTask convTask("Images/color.pfm", "Images/normals.pfm", "Images/depth.pfm", HGroupSize, VGroupSize,
			4, 4, 4, ConvKernel, ConvKernel);
		RunComputeTask(convTask, HGroupSize);
	}

	cout<<endl<<"########################################"<<endl;
	cout<<"Task 4: Histogram"<<endl<<endl;
	{
		size_t group_size[2] = {16, 16};
		{
			CHistogramTask histogram(0.25f, 0.26f, false, "Images/input.pfm");
			RunComputeTask(histogram, group_size);
		}

		{
			CHistogramTask histogram(0.25f, 0.26f, true, "Images/input.pfm");
			RunComputeTask(histogram, group_size);
		}
	}
	*/
	return true;
}

///////////////////////////////////////////////////////////////////////////////
