/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment3.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	CAssignment3 myAssignment;

	myAssignment.EnterMainLoop(argc, argv);

#ifdef _MSC_VER
	cout<<"Press any key..."<<endl;
	cin.get();
#endif
}
