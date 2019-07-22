/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment4.h"

#include "CClothSimulationTask.h"
#include "CParticleSystemTask.h"
#include "CRainSimulation.h"

#include <iostream>
#include <string>

#include "GLCommon.h"

#include "../Common/CLUtil.h"
#include <CL/cl_gl.h>

#ifdef __linux__
#include <GL/glx.h>
#endif


using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CAssignment4

CAssignment4* CAssignment4::s_pSingletonInstance = NULL;

CAssignment4* CAssignment4::GetSingleton()
{
	if(!s_pSingletonInstance)
		s_pSingletonInstance = new CAssignment4();

	return s_pSingletonInstance;
}

CAssignment4::CAssignment4()
	: m_Window(nullptr), m_WindowWidth(1024), m_WindowHeight(768), m_PrevTime(-1.0)
{
	// select task here...
	// This time you have to do it during compile time



	cout << "########################################" << endl;
	cout << "TASK 2: Rain Simulation" << endl << endl;

	m_LocalWorkSize[0] = 16;
	m_LocalWorkSize[1] = 16;
	m_LocalWorkSize[2] = 1;
	m_pCurrentTask = new CRainSimulation(1024, 1024);





	/*
#if 0
	cout<<"########################################"<<endl;
	cout<<"TASK 1: Particle System"<<endl<<endl;

	std::string meshPath = "Assets/CubeMonkey.obj";
	// Uncomment this to test your application with more triangles!
	// meshPath = "Assets/cubeMonkey.obj";

	m_LocalWorkSize[0] = 192;
	m_LocalWorkSize[1] = 1;
	m_LocalWorkSize[2] = 1;
	m_pCurrentTask = new CParticleSystemTask(meshPath, 1024 * 192, m_LocalWorkSize);
#else
	cout<<"########################################"<<endl;
	cout<<"TASK 2: Cloth Simulation"<<endl<<endl;

	m_LocalWorkSize[0] = 16;
	m_LocalWorkSize[1] = 16;
	m_LocalWorkSize[2] = 1;
	m_pCurrentTask = new CClothSimulationTask(64, 64);
#endif*/

}

CAssignment4::~CAssignment4()
{
	if(m_pCurrentTask)
	{
		delete m_pCurrentTask;
		m_pCurrentTask = nullptr;
	}
}

bool CAssignment4::EnterMainLoop(int argc, char** argv)
{

	// create CL context with GL context sharing
	if(InitGL(argc, argv) && InitCLContext())
	{
		if(m_pCurrentTask)
			m_pCurrentTask->InitResources(m_CLDevice, m_CLContext);
		
		// the main event loop...
		while(!glfwWindowShouldClose(m_Window))
		{
			OnIdle();
			Render();

			glfwPollEvents();
		}

		if(m_pCurrentTask)
			m_pCurrentTask->ReleaseResources();
	}
	else
	{
		cerr<<"Failed to create GL and CL context, terminating..."<<endl;

	}

	ReleaseCLContext();
	CleanupGL();

	return true;
}

bool CAssignment4::DoCompute()
{
	if(m_pCurrentTask)
		m_pCurrentTask->ComputeGPU(m_CLContext, m_CLCommandQueue, m_LocalWorkSize);

	return true;
}

#define PRINT_INFO(title, buffer, bufferSize, maxBufferSize, expr) { expr; buffer[bufferSize] = '\0'; std::cout << title << ": " << buffer << std::endl; }

bool CAssignment4::InitCLContext()
{
	// 1. get all platform IDs

	std::vector<cl_platform_id> platformIds;
	const cl_uint c_MaxPlatforms = 16;
	platformIds.resize(c_MaxPlatforms);
	
	cl_uint countPlatforms;
	V_RETURN_FALSE_CL(clGetPlatformIDs(c_MaxPlatforms, &platformIds[0], &countPlatforms), "Failed to get CL platform ID");
	platformIds.resize(countPlatforms);

	// 2. find all available GPU devices
	std::vector<cl_device_id> deviceIds;
	const int maxDevices = 16;
	deviceIds.resize(maxDevices);
	int countAllDevices = 0;


	cl_device_type deviceType = CL_DEVICE_TYPE_GPU;

	for (size_t i = 0; i < platformIds.size(); i++)
	{
		// Getting the available devices.
		cl_uint countDevices;
		clGetDeviceIDs(platformIds[i], deviceType, 1, &deviceIds[countAllDevices], &countDevices);
		countAllDevices += countDevices;
	}
	deviceIds.resize(countAllDevices);

	if (countAllDevices == 0)
	{
		std::cout << "No device of the selected type with OpenCL support was found.";
		return false;
	}
	// Choosing the first available device.
	m_CLDevice = deviceIds[0];
	clGetDeviceInfo(m_CLDevice, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &m_CLPlatform, NULL);

	// Printing platform and device data.
	const int maxBufferSize = 1024;
	char buffer[maxBufferSize];
	size_t bufferSize;
	std::cout << "OpenCL platform:" << std::endl << std::endl;
	PRINT_INFO("Name", buffer, bufferSize, maxBufferSize, clGetPlatformInfo(m_CLPlatform, CL_PLATFORM_NAME, maxBufferSize, (void*)buffer, &bufferSize));
	PRINT_INFO("Vendor", buffer, bufferSize, maxBufferSize, clGetPlatformInfo(m_CLPlatform, CL_PLATFORM_VENDOR, maxBufferSize, (void*)buffer, &bufferSize));
	PRINT_INFO("Version", buffer, bufferSize, maxBufferSize, clGetPlatformInfo(m_CLPlatform, CL_PLATFORM_VERSION, maxBufferSize, (void*)buffer, &bufferSize));
	PRINT_INFO("Profile", buffer, bufferSize, maxBufferSize, clGetPlatformInfo(m_CLPlatform, CL_PLATFORM_PROFILE, maxBufferSize, (void*)buffer, &bufferSize));
	std::cout << std::endl << "Device:" << std::endl << std::endl;
	PRINT_INFO("Name", buffer, bufferSize, maxBufferSize, clGetDeviceInfo(m_CLDevice, CL_DEVICE_NAME, maxBufferSize, (void*)buffer, &bufferSize));
	PRINT_INFO("Vendor", buffer, bufferSize, maxBufferSize, clGetDeviceInfo(m_CLDevice, CL_DEVICE_VENDOR, maxBufferSize, (void*)buffer, &bufferSize));
	PRINT_INFO("Driver version", buffer, bufferSize, maxBufferSize, clGetDeviceInfo(m_CLDevice, CL_DRIVER_VERSION, maxBufferSize, (void*)buffer, &bufferSize));
	cl_ulong localMemorySize;
	clGetDeviceInfo(m_CLDevice, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemorySize, &bufferSize);
	std::cout << "Local memory size: " << localMemorySize << " Byte" << std::endl;
	std::cout << std::endl << "******************************" << std::endl << std::endl;
        
	cl_int clError;

	
		// Define OS-specific context properties and create the OpenCL context
        #if defined (__APPLE__)
            CGLContextObj kCGLContext = CGLGetCurrentContext();
            CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
            cl_context_properties props[] = 
            {
                CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 
                0 
            };
        #else
            #ifndef _WIN32
                cl_context_properties props[] = 
                {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), 
                    CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), 
                    CL_CONTEXT_PLATFORM, (cl_context_properties)m_CLPlatform, 
                    0
                };
            #else // Win32
                cl_context_properties props[] = 
                {
                    CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(), 
                    CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(), 
                    CL_CONTEXT_PLATFORM, (cl_context_properties) m_CLPlatform, 
                    0
                };
            #endif
        #endif

		m_CLContext = clCreateContext(props, 1, &m_CLDevice, NULL, NULL, &clError);
		
	V_RETURN_FALSE_CL(clError, "Failed to create OpenCL context.");

	// Finally, create a command queue. All the asynchronous commands to the device will be issued
	// from the CPU into this queue. This way the host program can continue the execution until some results
	// from that device are needed.

	m_CLCommandQueue = clCreateCommandQueue(m_CLContext, m_CLDevice, 0, &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create the command queue in the context");

	return true;
}

bool CAssignment4::InitGL(int , char** )
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "CL Visual Computing Demo", NULL, NULL);
	if (!m_Window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(m_Window);

	glfwSetKeyCallback(m_Window, OnKeyboardCallback);
	glfwSetMouseButtonCallback(m_Window, OnMouseCallback);
	glfwSetCursorPosCallback(m_Window, OnMouseMoveCallback);
	glfwSetWindowSizeCallback(m_Window, OnWindowResizedCallback);

	// initialize necessary OpenGL extensions
    glewInit();
   if(!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"))
   {
	   cout<<"Missing OpenGL extension: GL_VERSION_2_0 GL_ARB_pixel_buffer_object"<<endl;
	   return false;
   }

    // default initialization
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
    glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    // viewport
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)m_WindowWidth / (GLfloat) m_WindowHeight, 0.1f, 10.0f);

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -2.0f);


	glEnable(GL_TEXTURE_2D);

	cout<<"OpenGL context initialized."<<endl;

	return true;
}

void CAssignment4::CleanupGL()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void CAssignment4::Render()
{
	if(m_pCurrentTask)
		m_pCurrentTask->Render();
	
	glfwSwapBuffers(m_Window);
}

void CAssignment4::InitTerrain()
{

}

void CAssignment4::OnIdle()
{
	if(m_PrevTime < 0)
	{
		m_FrameTimer.Start();
		m_PrevTime = 0.0;
	}
	else
	{
		m_FrameTimer.Stop();
		// we need the time in seconds
		double time = m_FrameTimer.GetElapsedMilliseconds() * 0.001;
		float elapsedTime = (float)( time - m_PrevTime );

		// threshold elapsed time
		// (a too large value might blow up the physics simulation.
		//  this happens for example when dragging the window, 
		//  when our application stops receiving messages for a while.
		if(elapsedTime > 0.05f) elapsedTime = 0.05f;

		m_PrevTime = time;

		if(m_pCurrentTask)
			m_pCurrentTask->OnIdle(time, elapsedTime);

		DoCompute();
	}
}

void CAssignment4::OnWindowResizedCallback(GLFWwindow* pWindow, int Width, int Height)
{
	GetSingleton()->OnWindowResized(pWindow, Width, Height);
}

void CAssignment4::OnWindowResized(GLFWwindow* , int Width, int Height)
{
	if(m_pCurrentTask)
		m_pCurrentTask->OnWindowResized(Width, Height);
}

void CAssignment4::OnKeyboardCallback(GLFWwindow* pWindow, int Key, int ScanCode, int Action, int Mods)
{
	GetSingleton()->OnKeyboard(pWindow, Key, ScanCode, Action, Mods);
}

void CAssignment4::OnKeyboard(GLFWwindow* , int Key, int , int Action, int )
{
	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_Window, GL_TRUE);

	if(m_pCurrentTask)
		m_pCurrentTask->OnKeyboard(Key, Action);
}

void CAssignment4::OnMouseCallback(GLFWwindow* pWindow, int Button, int State, int Mods)
{
	GetSingleton()->OnMouse(pWindow, Button, State, Mods);
}

void CAssignment4::OnMouse(GLFWwindow* , int Button, int State, int )
{
	if(m_pCurrentTask)
		m_pCurrentTask->OnMouse(Button, State);
}

void CAssignment4::OnMouseMoveCallback(GLFWwindow* pWindow, double X, double Y)
{
	GetSingleton()->OnMouseMove(pWindow, X, Y);
}

void CAssignment4::OnMouseMove(GLFWwindow* , double X, double Y)
{
	if(m_pCurrentTask)
		m_pCurrentTask->OnMouseMove((int)X, (int)Y);
}

///////////////////////////////////////////////////////////////////////////////
