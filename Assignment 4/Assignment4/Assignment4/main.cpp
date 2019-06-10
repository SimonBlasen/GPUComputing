/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment4.h"

#include <iostream>

using namespace std;


int main(int argc, char** argv)
{
	CAssignment4* pAssignment = CAssignment4::GetSingleton();

	if(pAssignment)
	{
		pAssignment->EnterMainLoop(argc, argv);
		delete pAssignment;
	}
	
#ifdef _MSC_VER
	cout<<"Press any key..."<<endl;
	cin.get();
#endif

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


