/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CParticleSystemTask.h"

#include "../Common/CLUtil.h"

#ifdef min // these macros are defined under windows, but collide with our math utility
#	undef min
#endif
#ifdef max
#	undef max
#endif

#include "HLSLEx.h"

#include <string>
#include <algorithm>
#include <string.h>

#include "CL/cl_gl.h"

#define NUM_FORCE_LINES		4096
#define NUM_BANKS	32

using namespace std;
using namespace hlsl;

#ifdef _MSC_VER
// we would like to use fopen...
#pragma warning(disable: 4996)
// unreferenced local parameter: as some code is missing from the startup kit, we don't want the compiler complaining about these
#pragma warning(disable: 4100)
#endif

///////////////////////////////////////////////////////////////////////////////
// CParticleSystemTask

CParticleSystemTask::CParticleSystemTask(
		const std::string& CollisionMeshPath,
		unsigned int NParticles,
		size_t LocalWorkSize[3]
)
	: m_nParticles(NParticles)
	, m_CollisionMeshPath(CollisionMeshPath)
{
	m_RotateX = 0;
	m_RotateY = 0;
	m_TranslateZ = -1.5f;

	for(unsigned int i = 0; i < 3; i++)
		m_LocalWorkSize[i] = LocalWorkSize[i];
	
	// compute the number of levels that we need for the work-efficient algorithm
	m_nLevels = 1;
	unsigned int N = NParticles;
	while (N > 0){
		N /= 2 * LocalWorkSize[0];
		m_nLevels++;
	}
	m_clLevelArrays = new cl_mem[m_nLevels];

	for (unsigned int i = 0; i < m_nLevels; i++)
		m_clLevelArrays[i] = NULL;

	for(int i = 0; i < 255; i++)
	{
		m_KeyboardMask[i] = false;
	}
}

CParticleSystemTask::~CParticleSystemTask()
{
	ReleaseResources();
}

bool CParticleSystemTask::InitResources(cl_device_id Device, cl_context Context)
{
	//Load mesh
	float4x4 M = float4x4(	1.f, 0.f, 0.f, 0.f,
							0.f, 1.f, 0.f, 0.f,
							0.f, 0.f, 1.f, 0.f,
							0.5f, 0.f, 0.5f, 0.f);
	
	m_pMesh = CTriMesh::LoadFromObj(m_CollisionMeshPath, M);
	
	m_pMesh->GetVertexBuffer();
	if(!m_pMesh)
	{
		cout<<"Failed to load mesh."<<endl;
		return false;
	}
	if(!m_pMesh->CreateGLResources())
	{
		cout<<"Failed to create mesh OpenGL resources"<<endl;
		return false;
	}
	
	//CPU resources
	cl_float4 *pPosLife	= new cl_float4[m_nParticles * 2];
	cl_float4 *pVelMass	= new cl_float4[m_nParticles * 2];
	memset(pPosLife, 0, sizeof(cl_float4) * m_nParticles * 2);
	memset(pVelMass, 0, sizeof(cl_float4) * m_nParticles * 2);

	//fill the array with some values
	for(unsigned int i = 0; i < m_nParticles; i++) {
		pPosLife[i].s[0] = (float(rand()) / float(RAND_MAX) * 0.5f + 0.25f);
		pPosLife[i].s[1] = (float(rand()) / float(RAND_MAX) * 0.5f + 0.25f);
		pPosLife[i].s[2] = (float(rand()) / float(RAND_MAX) * 0.5f + 0.25f);
		pPosLife[i].s[3] = 100.f + 5.f * (float(rand()) / float(RAND_MAX));

		// if (i & 1)
		// 	pPosLife[i].s[3] = 0.f;

		pVelMass[i].s[0] = 0.f;
		pVelMass[i].s[1] = 0.f;
		pVelMass[i].s[2] = 0.f;
		pVelMass[i].s[3] = (1.f + float(rand()) / float(RAND_MAX)) * 1.5f;
	}

	// Device resources
	glGenBuffers(2, m_glPosLife);
	glBindBuffer(GL_ARRAY_BUFFER, m_glPosLife[0]);
	glBufferData(GL_ARRAY_BUFFER, m_nParticles * sizeof(cl_float4) * 2, pPosLife, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m_glPosLife[1]);
	glBufferData(GL_ARRAY_BUFFER, m_nParticles * sizeof(cl_float4) * 2, pPosLife, GL_DYNAMIC_DRAW);
    CHECK_FOR_OGL_ERROR();

	glGenBuffers(2, m_glVelMass);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVelMass[0]);
	glBufferData(GL_ARRAY_BUFFER, m_nParticles * sizeof(cl_float4) * 2, pVelMass, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVelMass[1]);
	glBufferData(GL_ARRAY_BUFFER, m_nParticles * sizeof(cl_float4) * 2, pVelMass, GL_DYNAMIC_DRAW);
    CHECK_FOR_OGL_ERROR();

	//create a texture for the TBO
	glGenTextures(2, m_glTexVelMass);

	glBindTexture(GL_TEXTURE_BUFFER_EXT, m_glTexVelMass[0]);
	glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_RGBA32F_ARB, m_glVelMass[0]);
	glBindBuffer(GL_TEXTURE_BUFFER_EXT, 0);
	glBindTexture(GL_TEXTURE_BUFFER_EXT, 0);
    CHECK_FOR_OGL_ERROR();

	glBindTexture(GL_TEXTURE_BUFFER_EXT, m_glTexVelMass[1]);
	glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_RGBA32F_ARB, m_glVelMass[1]);
	glBindBuffer(GL_TEXTURE_BUFFER_EXT, 0);
	glBindTexture(GL_TEXTURE_BUFFER_EXT, 0);
    CHECK_FOR_OGL_ERROR();


	//scatter force field sampling points
	float* pForceSamples = new float[NUM_FORCE_LINES * 2 * 4];
	for(int i = 0; i < NUM_FORCE_LINES; i++)
	{
		pForceSamples[8 * i] = float(rand()) / float(RAND_MAX);
		pForceSamples[8 * i + 1] = float(rand()) / float(RAND_MAX);
		pForceSamples[8 * i + 2] = float(rand()) / float(RAND_MAX);
		pForceSamples[8 * i + 3] = 0.0f; 

		pForceSamples[8 * i + 4] = pForceSamples[8 * i];
		pForceSamples[8 * i + 5] = pForceSamples[8 * i +1];
		pForceSamples[8 * i + 6] = pForceSamples[8 * i + 2];
		pForceSamples[8 * i + 7] = 1.0f;
	}

	glGenBuffers(1, &m_glForceLines);
	glBindBuffer(GL_ARRAY_BUFFER, m_glForceLines);
	glBufferData(GL_ARRAY_BUFFER, NUM_FORCE_LINES * 2 * 4 * sizeof(float), pForceSamples, GL_DYNAMIC_DRAW);
    CHECK_FOR_OGL_ERROR();

	delete [] pForceSamples;

	cl_int clError, clError2;

	// Particle arrrays
	m_clPosLife[0] = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_glPosLife[0], &clError2);
	clError = clError2;
	m_clPosLife[1] = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_glPosLife[1], &clError2);
	clError |= clError2;
	m_clVelMass[0] = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_glVelMass[0], &clError2);
	clError |= clError2;
	m_clVelMass[1] = clCreateFromGLBuffer(Context, CL_MEM_READ_WRITE, m_glVelMass[1], &clError2);
	clError |= clError2;
	m_clAlive = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_nParticles * sizeof(cl_uint) * 2, NULL, &clError2);
	clError |= clError2;

	float *pTriangles;
	m_pMesh->GetTriangleSoup(&pTriangles, &m_nTriangles);
	m_clTriangleSoup = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, m_nTriangles * 3 * sizeof(cl_float4), pTriangles, &clError2);
	clError |= clError2;
	delete pTriangles;

	m_clPingArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_nParticles * sizeof(cl_uint) * 2, NULL, &clError2);
	clError |= clError2;
	m_clPongArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_nParticles * sizeof(cl_uint) * 2, NULL, &clError2);
	clError |= clError2;
	V_RETURN_FALSE_CL(clError, "Error allocating device arrays");


	SAFE_DELETE_ARRAY(pPosLife);
	SAFE_DELETE_ARRAY(pVelMass);

	// Scan arrays
	unsigned int N = m_nParticles * 2;
	for (unsigned int i = 0; i < m_nLevels; i++) {
		m_clLevelArrays[i] = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * N, NULL, &clError2);
		clError |= clError2;
		N = std::max(N / (2 * m_LocalWorkSize[0]), m_LocalWorkSize[0]);
	}
	V_RETURN_FALSE_CL(clError, "Error allocating device arrays");



	//shader programs

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

	m_VSParticles = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	m_PSParticles = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	if(!CreateShaderFromFile("particles.vert", m_VSParticles))
		return false;
	
	if(!CreateShaderFromFile("particles.frag", m_PSParticles))
		return false;

	m_ProgRenderParticles = glCreateProgramObjectARB();
	glAttachObjectARB(m_ProgRenderParticles, m_VSParticles);
	glAttachObjectARB(m_ProgRenderParticles, m_PSParticles);
	if(!LinkGLSLProgram(m_ProgRenderParticles))
		return false;

	GLint tboSampler = glGetUniformLocationARB(m_ProgRenderParticles, "tboSampler");
	glUseProgramObjectARB(m_ProgRenderParticles);
	glUniform1i(tboSampler, 0);

	m_VSForceField = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	m_PSForceField = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	if(!CreateShaderFromFile("forcefield.vert", m_VSForceField))
		return false;

	if(!CreateShaderFromFile("forcefield.frag", m_PSForceField))
		return false;

    CHECK_FOR_OGL_ERROR();
	m_ProgRenderForceField = glCreateProgramObjectARB();
	glAttachObjectARB(m_ProgRenderForceField, m_VSForceField);
	glAttachObjectARB(m_ProgRenderForceField, m_PSForceField);
	if(!LinkGLSLProgram(m_ProgRenderForceField))
		return false;
    CHECK_FOR_OGL_ERROR();

	GLint texForceField = glGetUniformLocationARB(m_ProgRenderForceField, "texForceField");
	glUseProgramObjectARB(m_ProgRenderForceField);
	glUniform1i(texForceField, 0);

	// Particle kernels
	string programCode;
	CLUtil::LoadProgramSourceToMemory("ParticleSystem.cl", programCode);
	m_PSystemProgram = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(!m_PSystemProgram)
		return false;

	m_IntegrateKernel = clCreateKernel(m_PSystemProgram, "Integrate", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Integrate kernel.");
	m_ClearKernel = clCreateKernel(m_PSystemProgram, "Clear", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Clear kernel.");
	m_ReorganizeKernel = clCreateKernel(m_PSystemProgram, "Reorganize", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Reorganize kernel.");

	// Scan kernels
	CLUtil::LoadProgramSourceToMemory("Scan.cl", programCode);
	m_ScanProgram = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(!m_ScanProgram)
		return false;

	m_ScanKernel = clCreateKernel(m_ScanProgram, "Scan", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Scan kernel.");
	m_ScanAddKernel = clCreateKernel(m_ScanProgram, "ScanAdd", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create ScanAdd kernel.");
	m_ScanNaiveKernel = clCreateKernel(m_ScanProgram, "ScanNaive", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Scan kernel.");

	// Load volume data
	m_volumeRes[0] = m_volumeRes[1] = m_volumeRes[2] = 128;
	FILE *fin = fopen("Assets/helix_float.raw", "rb");
	if (!fin) {
		cout << "Unable to open volumetric data file." << endl;
		return false;
	}
	
	// Flip the y and z axis of the volume and adjust the forces a little
	cl_float4 *pVolume = new cl_float4[m_volumeRes[0] * m_volumeRes[1] * m_volumeRes[2]];
	for (unsigned int z = 0; z < m_volumeRes[2]; z++){
		for (unsigned int y = 0; y < m_volumeRes[1]; y++){
			for (unsigned int x = 0; x < m_volumeRes[0]; x++){
				cl_float4 pForce;
				fread(&pForce, sizeof(float), 3, fin);
				pForce.s[2] *= 2.f;
				float scale = (1.f - (float)z / (float)m_volumeRes[2]) * 1.f;
				pForce.s[0] *= scale;
				pForce.s[1] *= scale;
				pForce.s[2] *= scale * exp(-(float)z / (float)m_volumeRes[2] * 5.f);
				pForce.s[3] = 0.f;

				std::swap(pForce.s[1], pForce.s[2]);
				pVolume[m_volumeRes[0] * m_volumeRes[1] * y + m_volumeRes[0] * z + x] = pForce;

			}
		}
	}
	fclose(fin);

	// Create OpenGL texture
	
    CHECK_FOR_OGL_ERROR();
	glEnable(GL_TEXTURE_3D);
    CHECK_FOR_OGL_ERROR();
	glGenTextures(1, &m_glVolTex3D);
    CHECK_FOR_OGL_ERROR();
    glBindTexture(GL_TEXTURE_3D, m_glVolTex3D);
    CHECK_FOR_OGL_ERROR();
	//if(!glIsEnabled(GL_TEXTURE_3D))
	//	cout<<"3D textures are not supported."<<endl;

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    CHECK_FOR_OGL_ERROR();

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F_ARB, 
					m_volumeRes[0], m_volumeRes[1], m_volumeRes[2],
					0, GL_RGBA, GL_FLOAT, pVolume);
    CHECK_FOR_OGL_ERROR();
    
	cl_image_format volume_format;
    volume_format.image_channel_order = CL_RGBA;
	volume_format.image_channel_data_type = CL_FLOAT;
    m_clVolTex3D = clCreateImage3D(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &volume_format, 
									m_volumeRes[0], m_volumeRes[1], m_volumeRes[2],
									(m_volumeRes[0] * sizeof(cl_float4)), (m_volumeRes[0] * m_volumeRes[1] * sizeof(cl_float4)),
									pVolume, &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create OpenCL 3D texture.");

	SAFE_DELETE(pVolume);

	m_LinearSampler = clCreateSampler(Context, true, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create a sampler.");

	// Set static arguments for the kernels
	clError  = clSetKernelArg(m_IntegrateKernel, 0, sizeof(cl_mem), (void*)&m_clAlive);
	clError |= clSetKernelArg(m_IntegrateKernel, 1, sizeof(cl_mem), (void*)&m_clVolTex3D);
	clError |= clSetKernelArg(m_IntegrateKernel, 2, sizeof(cl_sampler), (void*)&m_LinearSampler);
	clError |= clSetKernelArg(m_IntegrateKernel, 3, sizeof(cl_uint), (void*)&m_nParticles);
	clError |= clSetKernelArg(m_IntegrateKernel, 4, sizeof(cl_uint), (void*)&m_nTriangles);
	V_RETURN_FALSE_CL(clError, "Failed to set args for m_IntegrateKernel");

	clError  = clSetKernelArg(m_ReorganizeKernel, 0, sizeof(cl_mem), (void*)&m_clAlive);
	V_RETURN_FALSE_CL(clError, "Failed to set args for m_ReorganizeKernel");

	//set the modelview matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glTranslatef(0.0f, 0.0f, m_TranslateZ);
	glRotatef(m_RotateY, 0.0f, 1.0f, 0.0f);
	glRotatef(m_RotateX, 1.0f, 0.0f, 0.0f);
	glTranslatef(-0.5, -0.5, -0.5);

	return true;
}

void CParticleSystemTask::ReleaseResources()
{
	if(m_pMesh)
	{
		m_pMesh->ReleaseGLResources();
		delete m_pMesh;
		m_pMesh = NULL;
	}
	
	// Device resources

	SAFE_RELEASE_MEMOBJECT(m_clPosLife[0]);
	SAFE_RELEASE_MEMOBJECT(m_clPosLife[1]);
	SAFE_RELEASE_MEMOBJECT(m_clVelMass[0]);
	SAFE_RELEASE_MEMOBJECT(m_clVelMass[1]);
	SAFE_RELEASE_MEMOBJECT(m_clAlive);
	SAFE_RELEASE_MEMOBJECT(m_clTriangleSoup);
	SAFE_RELEASE_MEMOBJECT(m_clPingArray);
	SAFE_RELEASE_MEMOBJECT(m_clPongArray);
	SAFE_RELEASE_MEMOBJECT(m_clVolTex3D);
	if (m_clLevelArrays)
		for (unsigned int i = 0; i < m_nLevels; i++)
			SAFE_RELEASE_MEMOBJECT(m_clLevelArrays[i]);
	
	SAFE_DELETE_ARRAY(m_clLevelArrays);

	SAFE_RELEASE_SAMPLER(m_LinearSampler);

	SAFE_RELEASE_KERNEL(m_ScanKernel);
	SAFE_RELEASE_KERNEL(m_ScanAddKernel);
	SAFE_RELEASE_KERNEL(m_ScanNaiveKernel);
	SAFE_RELEASE_PROGRAM(m_ScanProgram);

	SAFE_RELEASE_KERNEL(m_IntegrateKernel);
	SAFE_RELEASE_KERNEL(m_ClearKernel);
	SAFE_RELEASE_KERNEL(m_ReorganizeKernel);
	SAFE_RELEASE_PROGRAM(m_PSystemProgram);	

	SAFE_RELEASE_GL_BUFFER(m_glForceLines);
	SAFE_RELEASE_GL_BUFFER(m_glPosLife[0]);
	SAFE_RELEASE_GL_BUFFER(m_glPosLife[1]);
	SAFE_RELEASE_GL_BUFFER(m_glVelMass[0]);
	SAFE_RELEASE_GL_BUFFER(m_glVelMass[1]);

	glDeleteTextures(1, &m_glVolTex3D);
	glDeleteTextures(2, m_glTexVelMass);

	SAFE_RELEASE_GL_SHADER(m_PSParticles);
	SAFE_RELEASE_GL_SHADER(m_VSParticles);
	SAFE_RELEASE_GL_SHADER(m_VSForceField);
	SAFE_RELEASE_GL_SHADER(m_PSForceField);
	SAFE_RELEASE_GL_SHADER(m_VSMesh);
	SAFE_RELEASE_GL_SHADER(m_PSMesh);

	SAFE_RELEASE_GL_PROGRAM(m_ProgRenderParticles);
	SAFE_RELEASE_GL_PROGRAM(m_ProgRenderForceField);
	SAFE_RELEASE_GL_PROGRAM(m_ProgRenderMesh);
}

void CParticleSystemTask::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	glFinish();
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clPosLife[0], 0, NULL, NULL),  "Error acquiring OpenGL buffer.");
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clVelMass[0], 0, NULL, NULL), "Error acquiring OpenGL buffer.");
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clPosLife[1], 0, NULL, NULL),  "Error acquiring OpenGL buffer.");
	V_RETURN_CL(clEnqueueAcquireGLObjects(CommandQueue, 1, &m_clVelMass[1], 0, NULL, NULL), "Error acquiring OpenGL buffer.");

	// Integration with a fixed timestep
	Integrate(Context, CommandQueue, LocalWorkSize, 0.003f);


	//********************************************************
	// GPU Computing only!
	// (If you are doing the smaller course, ignore these lines)
	//********************************************************

	// TO DO: 
	// Enable these once you implement them
	// Stream compaction
	//Scan(Context, CommandQueue, LocalWorkSize);
	//Reorganize(Context, CommandQueue, LocalWorkSize);


	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clPosLife[0], 0, NULL, NULL),  "Error releasing OpenGL buffer.");
	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clVelMass[0], 0, NULL, NULL), "Error releasing OpenGL buffer.");
	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clPosLife[1], 0, NULL, NULL),  "Error releasing OpenGL buffer.");
	V_RETURN_CL(clEnqueueReleaseGLObjects(CommandQueue, 1, &m_clVelMass[1], 0, NULL, NULL), "Error releasing OpenGL buffer.");

	clFinish(CommandQueue);

}

void CParticleSystemTask::Render()
{
	glCullFace(GL_BACK);

	//enable depth test & depth write
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);
	glUseProgramObjectARB(m_ProgRenderMesh);
	glColor4f(0, 0.5f, 0.2f, 1.0f);
	if(m_pMesh)
		m_pMesh->DrawGL(GL_TRIANGLES);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//enable depth test but disable depth write
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	glUseProgramObjectARB(m_ProgRenderParticles);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER_EXT, m_glTexVelMass[0]);

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    //
	// Set up the OpenGL state machine for using point sprites...
	//

    // This is how will our point sprite's size will be modified by 
    // distance from the viewer
    float quadratic[] =  { 1.0f, 0.0f, 0.01f };
    glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

    glPointSize( 10.0f );

    // The alpha of a point is calculated to allow the fading of points 
    // instead of shrinking them past a defined threshold size. The threshold 
    // is defined by GL_POINT_FADE_THRESHOLD_SIZE_ARB and is not clamped to 
    // the minimum and maximum point sizes.

    // Specify point sprite texture coordinate replacement mode for each 
    // texture unit
    glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

    //
	// Render point sprites...
	//

    glEnable( GL_POINT_SPRITE_ARB );

	glBindBuffer(GL_ARRAY_BUFFER, m_glPosLife[0]);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColor4f(1.0, 0.0, 0.0, 0.3f);
    glDrawArrays(GL_POINTS, 0, m_nParticles);
    glDisableClientState(GL_VERTEX_ARRAY);

	glDisable( GL_POINT_SPRITE_ARB );
	glBindTexture(GL_TEXTURE_BUFFER_EXT, 0);

	//render force field
	
	if(m_ShowForceField)
	{
		glUseProgramObjectARB(m_ProgRenderForceField);

		//bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, m_glVolTex3D);
	
		glBindBuffer(GL_ARRAY_BUFFER, m_glForceLines);
		glVertexPointer(4, GL_FLOAT, 0, 0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColor4f(1.0, 0.0, 0.0, 1.0f);
		glDrawArrays(GL_LINES, 0, NUM_FORCE_LINES * 2);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

}

void CParticleSystemTask::OnKeyboard(int Key, int Action)
{
	if(Key >= 255)
		return;
	if (Action == GLFW_PRESS)
	{
		m_KeyboardMask[Key] = true;

		if (Key == GLFW_KEY_F)
			m_ShowForceField = !m_ShowForceField;
	}
	else
	{
		m_KeyboardMask[Key] = false;
	}
}

void CParticleSystemTask::OnMouse(int Button, int State)
{
	if (State == GLFW_PRESS) {
        m_Buttons |= 1<<Button;
	}
	else if (State == GLFW_RELEASE) {
        m_Buttons = 0;
    }
}

void CParticleSystemTask::OnMouseMove(int X, int Y)
{
    int dx, dy;
	dx = X - m_PrevX;
    dy = Y - m_PrevY;

    // left button
	if (m_Buttons & 1) {
		m_RotateX += dy * 0.2f;
		m_RotateY += dx * 0.2f;
    } 
	m_PrevX = X;
	m_PrevY = Y;

    // set view matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glTranslatef(0.0, 0.0, m_TranslateZ);
	glRotatef(m_RotateY, 0.0, 1.0, 0.0);
	glRotatef(m_RotateX, 1.0, 0.0, 0.0);
	glTranslatef(-0.5, -0.5, -0.5);
}

void CParticleSystemTask::OnIdle(double , float ElapsedTime)
{
	//move camera?
	if(m_KeyboardMask[GLFW_KEY_W])
		m_TranslateZ += 2.f * ElapsedTime;
	if(m_KeyboardMask[GLFW_KEY_S])
		m_TranslateZ -= 2.f * ElapsedTime;

	if(m_TranslateZ > 0)
		m_TranslateZ = 0;
	
	//set the modelview matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, m_TranslateZ);
    glRotatef(m_RotateY, 0.0, 1.0, 0.0);
    glRotatef(m_RotateX, 1.0, 0.0, 0.0);
	glTranslatef(-0.5, -0.5, -0.5);
}

void CParticleSystemTask::OnWindowResized(int Width, int Height)
{
    // viewport
    glViewport(0, 0, Width, Height);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)Width / (GLfloat) Height, 0.1, 10.0);
}

void CParticleSystemTask::Scan(cl_context , cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{


	//********************************************************
	// GPU Computing only!
	// (If you are doing the smaller course, ignore this function)
	//********************************************************

	// Add your favorite prefix sum code here from Assignment 2 :)


}


void CParticleSystemTask::Integrate(cl_context , cl_command_queue CommandQueue, size_t LocalWorkSize[3], float dT)
{
	cl_int clErr;
	size_t globalWorkSize[1];

	cl_float4 *pVerts = new cl_float4[m_nTriangles * 3];

	V_RETURN_CL(clEnqueueReadBuffer(CommandQueue, m_clTriangleSoup, CL_TRUE, 0, m_nTriangles * 3 * sizeof(cl_float4), pVerts, 0, NULL, NULL), "Error reading data from device!");

	delete pVerts;

	//detemine the necessary number of global work items
	globalWorkSize[0] = CLUtil::GetGlobalWorkSize(m_nParticles, LocalWorkSize[0]);
	//set kernel args
	clErr  = clSetKernelArg(m_IntegrateKernel, 5, LocalWorkSize[0] * sizeof(cl_float4), NULL);
	clErr |= clSetKernelArg(m_IntegrateKernel, 6, sizeof(cl_mem), (void*)&m_clTriangleSoup);
	clErr |= clSetKernelArg(m_IntegrateKernel, 7, sizeof(cl_mem), (void*)&m_clPosLife[0]);
	clErr |= clSetKernelArg(m_IntegrateKernel, 8, sizeof(cl_mem), (void*)&m_clVelMass[0]);
	clErr |= clSetKernelArg(m_IntegrateKernel, 9, sizeof(cl_float), (void*)&dT);
	V_RETURN_CL(clErr, "Failed to set args for m_IntegrateKernel");
	clErr = clEnqueueNDRangeKernel(CommandQueue, m_IntegrateKernel, 1, NULL, globalWorkSize, LocalWorkSize, 0, NULL, NULL);
	V_RETURN_CL(clErr, "Error executing m_IntegrateKernel!");
}

void CParticleSystemTask::Reorganize(cl_context , cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	//********************************************************
	// GPU Computing only!
	// (If you are doing the smaller course, ignore this function)
	//********************************************************

	// Clear
	// ADD YOUR CODE HERE

	// Reorganize (perform the actual compaction)
	// ADD YOUR CODE HERE


	std::swap(m_clPosLife[0],	 m_clPosLife[1]);
	std::swap(m_clVelMass[0],	 m_clVelMass[1]);
	std::swap(m_glPosLife[0],	 m_glPosLife[1]);
	std::swap(m_glVelMass[0],	 m_glVelMass[1]);
	std::swap(m_glTexVelMass[0], m_glTexVelMass[1]);	
}

///////////////////////////////////////////////////////////////////////////////
