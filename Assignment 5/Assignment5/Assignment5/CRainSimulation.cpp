/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CRainSimulation.h"

#include "../Common/CLUtil.h"

#ifdef min // these macros are defined under windows, but collide with our math utility
#	undef min
#endif
#ifdef max
#	undef max
#endif

#include "HLSLEx.h"

#include <math.h>
#include <string>

#include "CL/cl_gl.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CClothSimulationTask

CRainSimulation::
CRainSimulation(unsigned int TerrainResX, unsigned int TerrainResY)
	: m_TerrainResX(TerrainResX)
	, m_TerrainResY(TerrainResY)
{
	for(int i = 0; i < 255; i++)
		m_KeyboardMask[i] = false;

	cout<<"Press 'i' to inspect the cloth in wireframe mode."<<endl;
	cout<<"Left mouse button pressed: orbit camera"<<endl;
	cout<<"Middle mouse button pressed: move collision sphere"<<endl;
}

CRainSimulation::~CRainSimulation()
{
	ReleaseResources();
}

float CRainSimulation::Gauss2D(float x, float y, float uX, float uY, float variant)
{
	return (1.f / (2.f * CL_M_PI * variant * variant)) * exp(-1.f * ((x - uX) * (x - uX) + (y - uY) * (y - uY)) / (2.f * variant * variant));
}

bool CRainSimulation::InitResources(cl_device_id Device, cl_context Context)
{
	m_n_rainSpots = 1;
	/*
		x:	uX
		y:	uY
		z:	variant
		w:	scale
	*/
	m_rainSpots = new hlsl::float4[m_n_rainSpots];
	m_rainSpots[0] = hlsl::float4(m_TerrainResX * 0.5f, m_TerrainResY * 0.5f, 5.f, 16.f);

	// Fit every distribution to be max 1
	for (int i = 0; i < m_n_rainSpots; i++)
	{
		m_rainSpots[i].w = (m_rainSpots[i].w) / (m_rainSpots[i].z * m_rainSpots[i].z * CL_M_PI * 2.f);
	}


	m_hRainArray = new unsigned int[m_TerrainResX * m_TerrainResY];

	for (int x = 0; x < m_TerrainResX; x++)
	{
		for (int y = 0; y < m_TerrainResY; y++)
		{
			float gauss = 0.f;
			for (int i = 0; i < m_n_rainSpots; i++)
			{
				gauss += Gauss2D(x, y, m_rainSpots[i].x, m_rainSpots[i].y, m_rainSpots[i].z) * m_rainSpots[i].w;
			}
			m_hRainArray[x + y * m_TerrainResX] = (unsigned int) gauss;
		}
	}


	//create cloth model
	/*m_pClothModel = CTriMesh::CreatePlane(m_ClothResX, m_ClothResY);
	if(!m_pClothModel)
	{
		cout<<"Failed to create cloth."<<endl;
		return false;
	}
	if(!m_pClothModel->CreateGLResources())
	{
		cout<<"Failed to create cloth OpenGL resources"<<endl;
		return false;
	}*/

	//create terrain model
	m_pTerrainModel = CTriMesh::CreatePlane(m_TerrainResX, m_TerrainResY);
	if (!m_pTerrainModel)
	{
		cout << "Failed to create terrain." << endl;
		return false;
	}
	if (!m_pTerrainModel->CreateGLResources())
	{
		cout << "Failed to create terrain OpenGL resources" << endl;
		return false;
	}

	//load cloth texture
	/*m_pClothTexture = new CGLTexture();
	if(!m_pClothTexture->loadTGA("Assets/clothTexture.tga"))
	{
		cout<<"Failed to load cloth texture"<<endl;
		return false;
	}*/

	//load terrain texture
	m_pTerrainTexture = new CGLTexture();
	if (!m_pTerrainTexture->loadTGA("Assets/terrain01.tga"))
	{
		cout << "Failed to load terrain texture" << endl;
		return false;
	}

	//load environment
	/*m_pEnvironment = CTriMesh::LoadFromObj("clothscene.obj", hlsl::identity<float,4,4>());
	if(!m_pEnvironment)
	{
		cout<<"Failed to create cloth environment."<<endl;
		return false;
	}
	if(!m_pEnvironment->CreateGLResources())
	{
		cout<<"Failed to create environment OpenGL resources"<<endl;
		return false;
	}*/


	//m_pSphere = gluNewQuadric();
	//gluQuadricNormals(m_pSphere, GLU_SMOOTH);

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

	m_clPosArray = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_pTerrainModel->GetVertexBuffer(), &clError);
	m_clNormalArray = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_pTerrainModel->GetNormalBuffer(), &clError2);
	clError |= clError2;

	m_clPosArrayAux = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_TerrainResX * m_TerrainResY * sizeof(hlsl::float4), 0, &clError2);
	clError |= clError2;
	m_clPosArrayOld = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_TerrainResX * m_TerrainResY * sizeof(hlsl::float4), 0, &clError2);
	clError |= clError2;

	m_clRainArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_TerrainResX * m_TerrainResY * sizeof(unsigned int), 0, &clError2);
	clError |= clError2;

	V_RETURN_FALSE_CL(clError, "Error allocating device arrays.");

	string programCode;
	CLUtil::LoadProgramSourceToMemory("TerrainSim.cl", programCode);
	m_TerrainSimProgram = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(m_TerrainSimProgram == nullptr)
		return false;


	m_InitTerrainKernel = clCreateKernel(m_TerrainSimProgram, "InitHeightfield", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Init Heightfield kernel.");


	m_IntegrateKernel = clCreateKernel(m_TerrainSimProgram, "Integrate", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Integrate kernel.");
	m_NormalKernel = clCreateKernel(m_TerrainSimProgram, "ComputeNormals", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Normal kernel.");
	m_ConstraintKernel = clCreateKernel(m_TerrainSimProgram, "SatisfyConstraints", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Constraint kernel.");
	m_CollisionsKernel = clCreateKernel(m_TerrainSimProgram, "CheckCollisions", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Collision kernel.");

	// Compute the rest distance between two particles.
	// We scale the distance by 0.9 to get a nicer look for the cloth (more folds).
	float restDistance = 1.f / ((float)m_TerrainResX)*0.9f;

	m_TerrainSeed = 0;

	clError = clSetKernelArg(m_InitTerrainKernel, 0, sizeof(unsigned int), &m_TerrainResX);
	clError |= clSetKernelArg(m_InitTerrainKernel, 1, sizeof(unsigned int), &m_TerrainResY);
	clError |= clSetKernelArg(m_InitTerrainKernel, 2, sizeof(cl_mem), (void*)&m_TerrainSeed);
	clError |= clSetKernelArg(m_InitTerrainKernel, 3, sizeof(cl_mem), (void*)&m_clPosArray);
	V_RETURN_FALSE_CL(clError, "Failed to set init terrain kernel params");




	////////////////////////////////////////////////////////////////////////
	// Specify the arguments for each kernel
	clError  = clSetKernelArg(m_IntegrateKernel, 0, sizeof(unsigned int), &m_TerrainResX);
	clError |= clSetKernelArg(m_IntegrateKernel, 1, sizeof(unsigned int), &m_TerrainResY);
	clError |= clSetKernelArg(m_IntegrateKernel, 2, sizeof(cl_mem), (void*) &m_clPosArray);
	clError |= clSetKernelArg(m_IntegrateKernel, 3, sizeof(cl_mem), (void*) &m_clPosArrayOld);
	clError |= clSetKernelArg(m_IntegrateKernel, 4, sizeof(cl_mem), (void*)& m_clRainArray);
	V_RETURN_FALSE_CL(clError, "Failed to set integration kernel params");

	clError  = clSetKernelArg(m_ConstraintKernel, 0, sizeof(unsigned int), &m_TerrainResX);
	clError |= clSetKernelArg(m_ConstraintKernel, 1, sizeof(unsigned int), &m_TerrainResY);
	clError |= clSetKernelArg(m_ConstraintKernel, 2, sizeof(float), &restDistance);
	// The rest of parameters is set before kernel launch (ping-ponging)
	V_RETURN_FALSE_CL(clError, "Failed to set constraint kernel params");

	clError  = clSetKernelArg(m_NormalKernel, 0, sizeof(unsigned int), &m_TerrainResX);
    clError |= clSetKernelArg(m_NormalKernel, 1, sizeof(unsigned int), &m_TerrainResY);
	clError |= clSetKernelArg(m_NormalKernel, 2, sizeof(cl_mem), (void*) &m_clPosArray);
    clError |= clSetKernelArg(m_NormalKernel, 3, sizeof(cl_mem), (void*) &m_clNormalArray);
	V_RETURN_FALSE_CL(clError, "Failed to set normal computation kernel params");

	clError  = clSetKernelArg(m_CollisionsKernel, 0, sizeof(unsigned int), &m_TerrainResX);
    clError |= clSetKernelArg(m_CollisionsKernel, 1, sizeof(unsigned int), &m_TerrainResY);
	clError |= clSetKernelArg(m_CollisionsKernel, 2, sizeof(cl_mem), (void*) &m_clPosArray);
	// Dynamic sphere parameters are updated before kernel launch
	V_RETURN_FALSE_CL(clError, "Failed to set collision kernel params");



	return true;
}

void CRainSimulation::ReleaseResources()
{
	/*if(m_pClothModel)
	{
		delete m_pClothModel;
		m_pClothModel = NULL;
	}*/
	if (m_pTerrainModel)
	{
		delete m_pTerrainModel;
		m_pTerrainModel = NULL;
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

	SAFE_DELETE_ARRAY(m_hRainArray);
	SAFE_DELETE_ARRAY(m_rainSpots);

	SAFE_RELEASE_MEMOBJECT(m_clPosArrayAux);
	SAFE_RELEASE_MEMOBJECT(m_clPosArrayOld);
	SAFE_RELEASE_MEMOBJECT(m_clNormalArray);
	SAFE_RELEASE_MEMOBJECT(m_clPosArray);
	SAFE_RELEASE_MEMOBJECT(m_clRainArray);

	SAFE_RELEASE_KERNEL(m_IntegrateKernel);
	SAFE_RELEASE_KERNEL(m_NormalKernel);
	SAFE_RELEASE_KERNEL(m_ConstraintKernel);
	SAFE_RELEASE_KERNEL(m_CollisionsKernel);
	SAFE_RELEASE_KERNEL(m_InitTerrainKernel);

	SAFE_RELEASE_PROGRAM(m_TerrainSimProgram);
	
	SAFE_RELEASE_GL_SHADER(m_PSCloth);
	SAFE_RELEASE_GL_SHADER(m_VSCloth);
	SAFE_RELEASE_GL_SHADER(m_VSMesh);
	SAFE_RELEASE_GL_SHADER(m_PSMesh);
	
	SAFE_RELEASE_GL_PROGRAM(m_ProgRenderCloth);
	SAFE_RELEASE_GL_PROGRAM(m_ProgRenderMesh);
}

void CRainSimulation::ComputeGPU(cl_context , cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	cl_int clErr;

	//get global work size
	size_t globalWorkSize[2];
	globalWorkSize[0] = CLUtil::GetGlobalWorkSize(m_TerrainResX, LocalWorkSize[0]);
	globalWorkSize[1] = CLUtil::GetGlobalWorkSize(m_TerrainResY, LocalWorkSize[1]);

	glFinish();


	unsigned int randSeedX = rand();
	unsigned int randSeedY = rand();




	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clPosArray, 0, NULL, NULL), "Error acquiring OpenGL vertex buffer.");
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clNormalArray, 0, NULL, NULL), "Error acquiring OpenGL normal buffer.");


	if (m_firstRun)
	{
		m_firstRun = false;

		V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_clRainArray, CL_FALSE, 0, m_TerrainResX * m_TerrainResY * sizeof(cl_float), m_hRainArray, 0, NULL, NULL), "Error copying data from host to device!");
		clErr = clFinish(CommandQueue);

		clErr = clEnqueueNDRangeKernel(CommandQueue, m_InitTerrainKernel, 2, 0, globalWorkSize, LocalWorkSize, 0, 0, 0);
		V_RETURN_CL(clErr, "Error executing m_InitTerrainKernel");


		cout << "Inited Terrain" << endl;
	}





	clErr = clSetKernelArg(m_IntegrateKernel, 0, sizeof(unsigned int), &m_TerrainResX);
	clErr |= clSetKernelArg(m_IntegrateKernel, 1, sizeof(unsigned int), &m_TerrainResY);
	clErr |= clSetKernelArg(m_IntegrateKernel, 2, sizeof(cl_mem), (void*)& m_clPosArray);
	clErr |= clSetKernelArg(m_IntegrateKernel, 3, sizeof(cl_mem), (void*)& m_clPosArrayOld);
	clErr |= clSetKernelArg(m_IntegrateKernel, 4, sizeof(cl_mem), (void*)& m_clRainArray);
	clErr |= clSetKernelArg(m_IntegrateKernel, 5, sizeof(cl_float), (void*)& m_ElapsedTime);
	clErr |= clSetKernelArg(m_IntegrateKernel, 6, sizeof(cl_float), (void*)& m_PrevElapsedTime);
	clErr |= clSetKernelArg(m_IntegrateKernel, 7, sizeof(cl_float), (void*)& m_simulationTime);
	clErr |= clSetKernelArg(m_IntegrateKernel, 8, sizeof(unsigned int), &randSeedX);
	clErr |= clSetKernelArg(m_IntegrateKernel, 9, sizeof(unsigned int), &randSeedY);
	V_RETURN_CL(clErr, "Failed to set integration kernel params");





	// ADD YOUR CODE HERE

	// Execute the integration kernel
	clErr = clEnqueueNDRangeKernel(CommandQueue, m_IntegrateKernel, 2, 0, globalWorkSize, LocalWorkSize, 0, 0, 0);
	V_RETURN_CL(clErr, "Error executing m_IntegrateKernel");




	/*
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clPosArray, 0, NULL, NULL),  "Error acquiring OpenGL vertex buffer.");
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clNormalArray, 0, NULL, NULL), "Error acquiring OpenGL normal buffer.");
	

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

	
	*/



	//compute correct normals
	clErr = clEnqueueNDRangeKernel(CommandQueue, m_NormalKernel, 2, 0, globalWorkSize, LocalWorkSize, 0, 0, 0);
	V_RETURN_CL(clErr, "Error executing normal computation kernel");


	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clPosArray, 0, NULL, NULL), "Error releasing OpenGL vertex buffer.");
	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clNormalArray, 0, NULL, NULL), "Error releasing OpenGL normal buffer.");


	clFinish(CommandQueue);
	m_FrameCounter++;
	m_PrevElapsedTime = m_ElapsedTime;
	m_ElapsedTime = 0;
}

void CRainSimulation::Render()
{
	glCullFace(GL_BACK);

	//enable depth test & depth write
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	//m_pClothTexture->bind();
	m_pTerrainTexture->bind();
	
	//render wireframe cloth
	if(m_InspectCloth)
	{
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
		glUseProgramObjectARB(m_ProgRenderMesh);
		glColor3f(1.0f, 1.0f, 1.0f);
		if(m_pTerrainModel)
			m_pTerrainModel->DrawGL(GL_TRIANGLES);

		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
	}

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
	glUseProgramObjectARB(m_ProgRenderCloth);
	if(m_pTerrainModel)
		m_pTerrainModel->DrawGL(GL_TRIANGLES);

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

void CRainSimulation::OnKeyboard(int Key, int KeyAction)
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

void CRainSimulation::OnMouse(int Button, int State)
{
	if (State == GLFW_PRESS) {
        m_Buttons |= 1<<Button;
	}
	else if (State == GLFW_RELEASE) {
        m_Buttons = 0;
    }
}

void CRainSimulation::OnMouseMove(int X, int Y)
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

void CRainSimulation::OnIdle(double , float ElapsedTime)
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

void CRainSimulation::OnWindowResized(int Width, int Height)
{
    // viewport
    glViewport(0, 0, Width, Height);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (GLfloat)Width / (GLfloat) Height, 0.1f, 10.0f);
}

///////////////////////////////////////////////////////////////////////////////
