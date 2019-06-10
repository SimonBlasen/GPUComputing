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

#ifndef _CPARTICLE_SYSTEM_TASK_H
#define _CPARTICLE_SYSTEM_TASK_H

#include "../Common/IGUIEnabledComputeTask.h"

#include "CTriMesh.h"
#include "CGLTexture.h"

#include <string>

//! A4 / T1 Particle system
/*!
	Note that this task does not implement a CPU reference result, all simulation is
	running on the GPU. The methods related to the evaluation of the correctness of the
	kernel are therefore meaningless (not implemented).
*/
class CParticleSystemTask : public IGUIEnabledComputeTask
{
public:
	CParticleSystemTask(
			const std::string& CollisionMeshPath,
			unsigned int NParticles,
			size_t LocalWorkSize[3]);

	virtual ~CParticleSystemTask();

	// IComputeTask
	virtual bool InitResources(cl_device_id Device, cl_context Context);

	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	// Not implemented!
	virtual void ComputeCPU() {};
	virtual bool ValidateResults() {return false;};

	// IGUIEnabledComputeTask
	virtual void Render();

	virtual void OnKeyboard(int Key, int Action);

	virtual void OnMouse(int Button, int State);

	virtual void OnMouseMove(int X, int Y);

	virtual void OnIdle(double Time, float ElapsedTime);

	virtual void OnWindowResized(int Width, int Height);

protected:

	void Scan(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);
	void Integrate(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], float dT);
	void Reorganize(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

protected:

	unsigned int		m_nParticles = 0;
	unsigned int		m_nTriangles = 0;
	unsigned int		m_volumeRes[3];

	size_t				m_LocalWorkSize[3];

	std::string			m_CollisionMeshPath;

	CTriMesh*			m_pMesh = nullptr;

	// OpenCL memory objects
	// arrays for particle data
	cl_mem				m_clPosLife[2] /*= { nullptr, nullptr }*/;
	cl_mem				m_clVelMass[2] /*= { nullptr, nullptr }*/;
	cl_mem				m_clAlive = nullptr;
	cl_mem				m_clTriangleSoup = nullptr;
	cl_mem				m_clRank = nullptr;
	cl_mem				m_clVolTex3D = nullptr;
	cl_mem				m_clPingArray = nullptr;
	cl_mem				m_clPongArray = nullptr;

	// arrays for each level of the work-efficient scan
	unsigned int		m_nLevels = 0;
	cl_mem				*m_clLevelArrays = nullptr;


	// OpenCL program and kernels
	cl_sampler			m_LinearSampler = nullptr;
	cl_program			m_PSystemProgram = nullptr;
	cl_kernel			m_IntegrateKernel = nullptr;
	cl_kernel			m_ClearKernel = nullptr;
	cl_kernel			m_ReorganizeKernel = nullptr;

	cl_program			m_ScanProgram = nullptr;
	cl_kernel			m_ScanKernel = nullptr;
	cl_kernel			m_ScanAddKernel = nullptr;
	cl_kernel			m_ScanNaiveKernel = nullptr;

	// OpenGL variables
	//these will be used as VBOs
	GLuint				m_glPosLife[2] /*{ 0, 0 }*/;
	//these will be used as TBOs
	GLuint				m_glVelMass[2] /*= { 0, 0 }*/;
	GLuint				m_glVolTex3D = 0;
	//texture objects (like the SRV in DirectX) for the TBOs
	GLuint				m_glTexVelMass[2] /*= { 0, 0 }*/;

	GLhandleARB			m_VSParticles = 0;
	GLhandleARB			m_PSParticles = 0;
	GLhandleARB			m_ProgRenderParticles = 0;

	GLhandleARB			m_VSMesh = 0;
	GLhandleARB			m_PSMesh = 0;
	GLhandleARB			m_ProgRenderMesh = 0;

	GLuint				m_glForceLines = 0;
	GLhandleARB			m_VSForceField = 0;
	GLhandleARB			m_PSForceField = 0;
	GLhandleARB			m_ProgRenderForceField = 0;
	
	bool				m_KeyboardMask[255] /*= { false }*/;
	bool				m_ShowForceField = false;

	// mouse
	int					m_Buttons = 0;
	int					m_PrevX = 0;
	int					m_PrevY = 0;

	//for camera handling
	float				m_RotateX = 0.0f;
	float				m_RotateY = 0.0f;
	float				m_TranslateZ = 0.0f;

};

#endif // _CPARTICLE_SYSTEM_TASK_H
