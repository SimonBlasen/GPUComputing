/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment1.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	CAssignment1 myAssignment;

	myAssignment.EnterMainLoop(argc, argv);

#ifdef _MSC_VER
	cout << "Press 'Enter'..." << endl;
	cin.get();
#endif
}
