/******************************************************************************
                         .88888.   888888ba  dP     dP 
                        d8'   `88  88    `8b 88     88 
                        88        a88aaaa8P' 88     88 
                        88   YP88  88        88     88 
                        Y8.   .88  88        Y8.   .8P 
                         `88888'   dP        `Y88888P' 
                                                       
                                                       
   a88888b.                                         dP   oo                   
  d8'   `88                                         88                        
  88        .d8888b. 88d8b.d8b. 88d888b. dP    dP d8888P dP 88d888b. .d8888b. 
  88        88'  `88 88'`88'`88 88'  `88 88    88   88   88 88'  `88 88'  `88 
  Y8.   .88 88.  .88 88  88  88 88.  .88 88.  .88   88   88 88    88 88.  .88 
   Y88888P' `88888P' dP  dP  dP 88Y888P' `88888P'   dP   dP dP    dP `8888P88 
                                88                                        .88 
                                dP                                    d8888P  
******************************************************************************/

#ifndef _CASSIGNMENT4_H
#define _CASSIGNMENT4_H

#include "../Common/CAssignmentBase.h"
#include "../Common/IGUIEnabledComputeTask.h"
#include "../Common/CTimer.h"

#include "GLCommon.h"

//! Assignment4
class CAssignment4 : public CAssignmentBase
{
public:
	virtual ~CAssignment4();

	static CAssignment4* GetSingleton();

	//! We overload this method to define OpenGL callbacks and to support CL-GL context sharing
	virtual bool EnterMainLoop(int argc, char** argv);
	
	virtual bool DoCompute();
	
	virtual bool InitGL(int argc, char** argv);

	virtual void CleanupGL();

	// GLFW callback functions
	// NOTE: these have to be static, as GLFW does not support ptrs to member functions
	static void OnKeyboardCallback(GLFWwindow* Window, int Key, int ScanCode, int Action, int Mods);

	static void OnMouseCallback(GLFWwindow* Window, int Button, int State, int Mods);
	static void OnMouseMoveCallback(GLFWwindow* window, double X, double Y);

	static void OnWindowResizedCallback(GLFWwindow* window, int Width, int Height);

protected:
	// ctor is protected to enforce singleton pattern
	CAssignment4();

	// We also had to overload this, as we use a special context initialization
	// for OpenCL - OpenGL interop
	virtual bool InitCLContext();

	virtual void Render();

	virtual void OnKeyboard(GLFWwindow* pWindow, int Key, int ScanCode, int Action, int Mods);

	virtual void OnMouse(GLFWwindow* pWindow, int Button, int State, int Mods);
	virtual void OnMouseMove(GLFWwindow* pWindow, double X, double Y);

	virtual void OnIdle();

	virtual void OnWindowResized(GLFWwindow* pWindow, int Width, int Height);

protected:
	GLFWwindow*		m_Window;
	int				m_WindowWidth;
	int				m_WindowHeight;

	IGUIEnabledComputeTask*	m_pCurrentTask;
	size_t			m_LocalWorkSize[3];

	static CAssignment4*	s_pSingletonInstance;

	// for the physical simulation we need to keep track of the time between frames rendered
	CTimer			m_FrameTimer;
	double			m_PrevTime;
};

#endif // _CASSIGNMENT4_H
