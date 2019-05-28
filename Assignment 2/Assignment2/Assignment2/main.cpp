/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment2.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	CAssignment2 myAssignment;

	auto success = myAssignment.EnterMainLoop(argc, argv);

#ifdef _MSC_VER
	cout<<"Press any key..."<<endl;
	cin.get();
#endif

	return success ? 0 : 1;
}
