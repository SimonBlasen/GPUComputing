/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CClothSimulationTask.h"

#include "../Common/CLUtil.h"

#ifdef min // these macros are defined under windows, but collide with our math utility
#	undef min
#endif
#ifdef max
#	undef max
#endif

#include "HLSLEx.h"

#include <string>

#include "CL/cl_gl.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CClothSimulationTask

CClothSimulationTask::
CClothSimulationTask(unsigned int ClothResX, unsigned int ClothResY)
	: m_ClothResX(ClothResX)
	, m_ClothResY(ClothResY)
{
	for(int i = 0; i < 255; i++)
		m_KeyboardMask[i] = false;

	cout<<"Press 'i' to inspect the cloth in wireframe mode."<<endl;
	cout<<"Left mouse button pressed: orbit camera"<<endl;
	cout<<"Middle mouse button pressed: move collision sphere"<<endl;
}

CClothSimulationTask::~CClothSimulationTask()
{
	ReleaseResources();
}

bool CClothSimulationTask::InitResources(cl_device_id Device, cl_context Context)
{
	//create cloth model
	m_pClothModel = CTriMesh::CreatePlane(m_ClothResX, m_ClothResY);
	if(!m_pClothModel)
	{
		cout<<"Failed to create cloth."<<endl;
		return false;
	}
	if(!m_pClothModel->CreateGLResources())
	{
		cout<<"Failed to create cloth OpenGL resources"<<endl;
		return false;
	}

	//load cloth texture
	m_pClothTexture = new CGLTexture();
	if(!m_pClothTexture->loadTGA("Assets/clothTexture.tga"))
	{
		cout<<"Failed to load cloth texture"<<endl;
		return false;
	}

	//load environment
	m_pEnvironment = CTriMesh::LoadFromObj("clothscene.obj", hlsl::identity<float,4,4>());
	if(!m_pEnvironment)
	{
		cout<<"Failed to create cloth environment."<<endl;
		return false;
	}
	if(!m_pEnvironment->CreateGLResources())
	{
		cout<<"Failed to create environment OpenGL resources"<<endl;
		return false;
	}


	m_pSphere = gluNewQuadric();
	gluQuadricNormals(m_pSphere, GLU_SMOOTH);

	///////////////////////////////////////////////////////////
	// shader programs

	m_VSCloth = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	m_PSCloth = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	if(!CreateShaderFromFile("meshtextured.vert", m_VSCloth))
		return false;

	if(!CreateShaderFromFile("meshtextured.frag", m_PSCloth))
		return false;

	m_ProgRenderCloth = glCreateProgramObjectARB();
	glAttachObjectARB(m_ProgRenderCloth, m_VSCloth);
	glAttachObjectARB(m_ProgRenderCloth, m_PSCloth);
	if(!LinkGLSLProgram(m_ProgRenderCloth))
		return false;
    CHECK_FOR_OGL_ERROR();

	m_VSMesh = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	m_PSMesh = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	if(!CreateShaderFromFile("mesh.vert", m_VSMesh))
		return false;

	if(!CreateShaderFromFile("mesh.frag", m_PSMesh))
		return false;

	m_ProgRenderMesh = glCreateProgramObjectARB();
	glAttachObjectARB(m_ProgRenderMesh, m_VSMesh);
	glAttachObjectARB(m_ProgRenderMesh, m_PSMesh);
	if(!LinkGLSLProgram(m_ProgRenderMesh))
		return false;
    CHECK_FOR_OGL_ERROR();

	GLuint diffuseSampler = glGetUniformLocationARB(m_ProgRenderCloth, "texDiffuse");
	glUseProgramObjectARB(m_ProgRenderCloth);
	glUniform1i(diffuseSampler, 0);
	CHECK_FOR_OGL_ERROR();
	
	//set the modelview matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0f, m_TranslateZ);
    glRotatef(m_RotateY, 0.0, 1.0, 0.0);
    glRotatef(m_RotateX, 1.0, 0.0, 0.0);

	/////////////////////////////////////////////////////////////
	// OpenCL resources

	cl_int clError, clError2;

	m_clPosArray = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_pClothModel->GetVertexBuffer(), &clError);
	m_clNormalArray = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_pClothModel->GetNormalBuffer(), &clError2);
	clError |= clError2;

	m_clPosArrayAux = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_ClothResX * m_ClothResY * sizeof(hlsl::float4), 0, &clError2);
	clError |= clError2;
	m_clPosArrayOld = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_ClothResX * m_ClothResY * sizeof(hlsl::float4), 0, &clError2);
	clError |= clError2;

	V_RETURN_FALSE_CL(clError, "Error allocating device arrays.");

	string programCode;
	CLUtil::LoadProgramSourceToMemory("clothsim.cl", programCode);
	m_ClothSimProgram = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(m_ClothSimProgram == nullptr)
		return false;


	m_IntegrateKernel = clCreateKernel(m_ClothSimProgram, "Integrate", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Integrate kernel.");
	m_NormalKernel = clCreateKernel(m_ClothSimProgram, "ComputeNormals", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Normal kernel.");
	m_ConstraintKernel = clCreateKernel(m_ClothSimProgram, "SatisfyConstraints", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Constraint kernel.");
	m_CollisionsKernel = clCreateKernel(m_ClothSimProgram, "CheckCollisions", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Collision kernel.");

	// Compute the rest distance between two particles.
	// We scale the distance by 0.9 to get a nicer look for the cloth (more folds).
	float restDistance = 1.f / ((float)m_ClothResX)*0.9f;

	////////////////////////////////////////////////////////////////////////
	// Specify the arguments for each kernel
	clError  = clSetKernelArg(m_IntegrateKernel, 0, sizeof(unsigned int), &m_ClothResX);
	clError |= clSetKernelArg(m_IntegrateKernel, 1, sizeof(unsigned int), &m_ClothResY);
	clError |= clSetKernelArg(m_IntegrateKernel, 2, sizeof(cl_mem), (void*) &m_clPosArray);
	clError |= clSetKernelArg(m_IntegrateKernel, 3, sizeof(cl_mem), (void*) &m_clPosArrayOld);
	V_RETURN_FALSE_CL(clError, "Failed to set integration kernel params");

	clError  = clSetKernelArg(m_ConstraintKernel, 0, sizeof(unsigned int), &m_ClothResX);
	clError |= clSetKernelArg(m_ConstraintKernel, 1, sizeof(unsigned int), &m_ClothResY);
	clError |= clSetKernelArg(m_ConstraintKernel, 2, sizeof(float), &restDistance);
	// The rest of parameters is set before kernel launch (ping-ponging)
	V_RETURN_FALSE_CL(clError, "Failed to set constraint kernel params");

	clError  = clSetKernelArg(m_NormalKernel, 0, sizeof(unsigned int), &m_ClothResX);
    clError |= clSetKernelArg(m_NormalKernel, 1, sizeof(unsigned int), &m_ClothResY);
	clError |= clSetKernelArg(m_NormalKernel, 2, sizeof(cl_mem), (void*) &m_clPosArray);
    clError |= clSetKernelArg(m_NormalKernel, 3, sizeof(cl_mem), (void*) &m_clNormalArray);
	V_RETURN_FALSE_CL(clError, "Failed to set normal computation kernel params");

	clError  = clSetKernelArg(m_CollisionsKernel, 0, sizeof(unsigned int), &m_ClothResX);
    clError |= clSetKernelArg(m_CollisionsKernel, 1, sizeof(unsigned int), &m_ClothResY);
	clError |= clSetKernelArg(m_CollisionsKernel, 2, sizeof(cl_mem), (void*) &m_clPosArray);
	// Dynamic sphere parameters are updated before kernel launch
	V_RETURN_FALSE_CL(clError, "Failed to set collision kernel params");

	return true;
}

void CClothSimulationTask::ReleaseResources()
{
	if(m_pClothModel)
	{
		delete m_pClothModel;
		m_pClothModel = NULL;
	}

	if(m_pEnvironment)
	{
		delete m_pEnvironment;
		m_pEnvironment = NULL;
	}

	if(m_pSphere)
	{
		gluDeleteQuadric(m_pSphere);
		m_pSphere = 0;
	}

	SAFE_RELEASE_MEMOBJECT(m_clPosArrayAux);
	SAFE_RELEASE_MEMOBJECT(m_clPosArrayOld);
	SAFE_RELEASE_MEMOBJECT(m_clNormalArray);
	SAFE_RELEASE_MEMOBJECT(m_clPosArray);

	SAFE_RELEASE_KERNEL(m_IntegrateKernel);
	SAFE_RELEASE_KERNEL(m_NormalKernel);
	SAFE_RELEASE_KERNEL(m_ConstraintKernel);
	SAFE_RELEASE_KERNEL(m_CollisionsKernel);

	SAFE_RELEASE_PROGRAM(m_ClothSimProgram);
	
	SAFE_RELEASE_GL_SHADER(m_PSCloth);
	SAFE_RELEASE_GL_SHADER(m_VSCloth);
	SAFE_RELEASE_GL_SHADER(m_VSMesh);
	SAFE_RELEASE_GL_SHADER(m_PSMesh);
	
	SAFE_RELEASE_GL_PROGRAM(m_ProgRenderCloth);
	SAFE_RELEASE_GL_PROGRAM(m_ProgRenderMesh);
}

void CClothSimulationTask::ComputeGPU(cl_context , cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	cl_int clErr;

	//get global work size
	size_t globalWorkSize[2];
	globalWorkSize[0] = CLUtil::GetGlobalWorkSize(m_ClothResX, LocalWorkSize[0]);
	globalWorkSize[1] = CLUtil::GetGlobalWorkSize(m_ClothResY, LocalWorkSize[1]);

	glFinish();
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clPosArray, 0, NULL, NULL),  "Error acquiring OpenGL vertex buffer.");
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clNormalArray, 0, NULL, NULL), "Error acquiring OpenGL normal buffer.");
	
	clErr  = clSetKernelArg(m_IntegrateKernel, 0, sizeof(unsigned int), &m_ClothResX);
	clErr |= clSetKernelArg(m_IntegrateKernel, 1, sizeof(unsigned int), &m_ClothResY);
	clErr |= clSetKernelArg(m_IntegrateKernel, 2, sizeof(cl_mem), (void*) &m_clPosArray);
	clErr |= clSetKernelArg(m_IntegrateKernel, 3, sizeof(cl_mem), (void*) &m_clPosArrayOld);
	clErr |= clSetKernelArg(m_IntegrateKernel, 4, sizeof(cl_float), (void*) &m_ElapsedTime);
	clErr |= clSetKernelArg(m_IntegrateKernel, 5, sizeof(cl_float), (void*) &m_PrevElapsedTime);
	clErr |= clSetKernelArg(m_IntegrateKernel, 6, sizeof(cl_float), (void*)&m_simulationTime);
	V_RETURN_CL(clErr, "Failed to set integration kernel params");


	// ADD YOUR CODE HERE

	// Execute the integration kernel
	clErr = clEnqueueNDRangeKernel(CommandQueue, m_IntegrateKernel, 2, 0, globalWorkSize, LocalWorkSize, 0, 0, 0);
	V_RETURN_CL(clErr, "Error executing m_IntegrateKernel");

	// Check for collisions


	// Constraint relaxation: use the ping-pong technique and perform the relaxation in several iterations
	for (unsigned int i = 0; i < 2.0 * m_ClothResX; i++) {
	//
	//	 Execute the constraint relaxation kernel
	//

		if (i % 2 == 1)
		{
			clErr = clSetKernelArg(m_ConstraintKernel, 3, sizeof(cl_mem), (void*)&m_clPosArray);
			clErr |= clSetKernelArg(m_ConstraintKernel, 4, sizeof(cl_mem), (void*)&m_clPosArrayAux);
		}
		else
		{
			clErr = clSetKernelArg(m_ConstraintKernel, 3, sizeof(cl_mem), (void*)&m_clPosArrayAux);
			clErr |= clSetKernelArg(m_ConstraintKernel, 4, sizeof(cl_mem), (void*)&m_clPosArray);
		}
		V_RETURN_CL(clErr, "Failed to set m_ConstraintKernel params");
		clErr = clEnqueueNDRangeKernel(CommandQueue, m_ConstraintKernel, 2, 0, globalWorkSize, LocalWorkSize, 0, 0, 0);
		V_RETURN_CL(clErr, "Error executing m_ConstraintKernel");

		if (i % 3 == 0)
		{
			clErr = clSetKernelArg(m_CollisionsKernel, 3, sizeof(hlsl::float4), &m_SpherePos);
			clErr |= clSetKernelArg(m_CollisionsKernel, 4, sizeof(float), &m_SphereRadius);
			V_RETURN_CL(clErr, "Failed to set m_CollisionsKernel params");
			clErr = clEnqueueNDRangeKernel(CommandQueue, m_CollisionsKernel, 2, 0, globalWorkSize, LocalWorkSize, 0, 0, 0);
			V_RETURN_CL(clErr, "Error executing m_CollisionsKernel");
		}
	//		 Occasionally check for collisions
	//
	//	 Swap the ping pong buffers
	}

	// You can check for collisions here again, to make sure there is no intersection with the cloth in the end

	

	//compute correct normals
	clErr = clEnqueueNDRangeKernel(CommandQueue, m_NormalKernel, 2, 0, globalWorkSize, LocalWorkSize, 0, 0, 0);
	V_RETURN_CL(clErr, "Error executing normal computation kernel");


	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clPosArray, 0, NULL, NULL),  "Error releasing OpenGL vertex buffer.");
	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clNormalArray, 0, NULL, NULL), "Error releasing OpenGL normal buffer.");

	clFinish(CommandQueue);
	m_FrameCounter++;
	m_PrevElapsedTime = m_ElapsedTime;
	m_ElapsedTime = 0;
}

void CClothSimulationTask::Render()
{
	glCullFace(GL_BACK);

	//enable depth test & depth write
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	m_pClothTexture->bind();
	
	//render wireframe cloth
	if(m_InspectCloth)
	{
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
		glUseProgramObjectARB(m_ProgRenderMesh);
		glColor3f(1.0f, 1.0f, 1.0f);
		if(m_pClothModel)
			m_pClothModel->DrawGL(GL_TRIANGLES);

		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
	}

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
	glUseProgramObjectARB(m_ProgRenderCloth);
	if(m_pClothModel)
		m_pClothModel->DrawGL(GL_TRIANGLES);

	if(m_InspectCloth)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
	}

	// Render sphere
	if(m_pSphere)
	{
		glUseProgramObjectARB(m_ProgRenderMesh);
		glColor3f(0.9f,0.4f,0.3f);
		glPushMatrix();
		glTranslatef(m_SpherePos.x, m_SpherePos.y, m_SpherePos.z);
		gluSphere(m_pSphere, m_SphereRadius - 0.005f, 32, 32);
		glPopMatrix();
	}

	//rendre environment
	if(m_pEnvironment)
	{
		glUseProgramObjectARB(m_ProgRenderMesh);
		glColor3f(0.4f,0.4f,0.4f);
		m_pEnvironment->DrawGL(GL_TRIANGLES);
	}
}

void CClothSimulationTask::OnKeyboard(int Key, int KeyAction)
{
	if(Key >= 255)
		return;
	m_KeyboardMask[Key] = true;

	if (KeyAction == GLFW_PRESS)
	{
		if (Key == GLFW_KEY_I)
		{
			m_InspectCloth = !m_InspectCloth;
		}
	}
	else if (KeyAction == GLFW_RELEASE)
	{
		m_KeyboardMask[Key] = false;
	}
}

void CClothSimulationTask::OnMouse(int Button, int State)
{
	if (State == GLFW_PRESS) {
        m_Buttons |= 1<<Button;
	}
	else if (State == GLFW_RELEASE) {
        m_Buttons = 0;
    }
}

void CClothSimulationTask::OnMouseMove(int X, int Y)
{
    int dx, dy;
	dx = X - m_PrevX;
    dy = Y - m_PrevY;

    if (m_Buttons & 1) {
		m_RotateX += dy * 0.2f;
		m_RotateY += dx * 0.2f;
    } 
	if(m_Buttons & 4)
	{
		m_SpherePos.z += dy * 0.002f;
	}
	m_PrevX = X;
	m_PrevY = Y;

    // set view matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, m_TranslateZ);
    glRotatef(m_RotateY, 0.0f, 1.0f, 0.0f);
    glRotatef(m_RotateX, 1.0f, 0.0f, 0.0f);
}

void CClothSimulationTask::OnIdle(double , float ElapsedTime)
{
	//move camera?
	if(m_KeyboardMask[GLFW_KEY_W])
		m_TranslateZ += 0.5f * ElapsedTime;
	if(m_KeyboardMask[GLFW_KEY_S])
		m_TranslateZ -= 0.5f * ElapsedTime;

	if(m_TranslateZ > 0)
		m_TranslateZ = 0;

	m_ElapsedTime += ElapsedTime;
	m_simulationTime += ElapsedTime;
	
	//set the modelview matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, m_TranslateZ);
    glRotatef(m_RotateY, 0.0f, 1.0f, 0.0f);
    glRotatef(m_RotateX, 1.0f, 0.0f, 0.0f);
}

void CClothSimulationTask::OnWindowResized(int Width, int Height)
{
    // viewport
    glViewport(0, 0, Width, Height);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (GLfloat)Width / (GLfloat) Height, 0.1f, 10.0f);
}

///////////////////////////////////////////////////////////////////////////////
