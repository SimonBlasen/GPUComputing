
#ifndef _CCLOTH_SIMULATION_TASK_H
#define _CCLOTH_SIMULATION_TASK_H

#include "../Common/IGUIEnabledComputeTask.h"

#include "CTriMesh.h"
#include "CGLTexture.h"

//! A4 / T2 Cloth simulation
/*!
Note that this task does not implement a CPU reference result, all simulation is
running on the GPU. The methods related to the evaluation of the correctness of the
kernel are therefore meaningless (not implemented).
*/
class CRainSimulation : public IGUIEnabledComputeTask
{
public:
	CRainSimulation(unsigned int ClothResX, unsigned int ClothResY);

	virtual ~CRainSimulation();

	// IComputeTask
	virtual bool InitResources(cl_device_id Device, cl_context Context);

	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	// Not implemented!
	virtual void ComputeCPU() {};
	virtual bool ValidateResults() { return false; };

	// IGUIEnabledComputeTask
	virtual void Render();

	virtual void OnKeyboard(int Key, int Action);

	virtual void OnMouse(int Button, int State);

	virtual void OnMouseMove(int X, int Y);

	virtual void OnIdle(double Time, float ElapsedTime);

	virtual void OnWindowResized(int Width, int Height);

protected:
	unsigned int			m_ClothResX = 0;
	unsigned int			m_ClothResY = 0;
	unsigned int			m_FrameCounter = 0;

	CTriMesh*				m_pClothModel = nullptr;
	CTriMesh*				m_pEnvironment = nullptr;
	CGLTexture*				m_pClothTexture = nullptr;
	GLUquadricObj*			m_pSphere = nullptr;
	float					m_SphereRadius = 0.2f;
	hlsl::float4			m_SpherePos = hlsl::float4(0.0f, 0.0f, 0.0f, 1.0f);

	// OpenGL variables
	GLhandleARB				m_VSCloth = 0;
	GLhandleARB				m_PSCloth = 0;
	GLhandleARB				m_ProgRenderCloth = 0;

	GLhandleARB				m_VSMesh = 0;
	GLhandleARB				m_PSMesh = 0;
	GLhandleARB				m_ProgRenderMesh = 0;

	// OpenCL variables

	//device buffers for cloth simulation
	cl_mem					m_clPosArray = nullptr; //current position of the particles (shared with OpenGL)
	cl_mem					m_clPosArrayOld = nullptr;
	cl_mem					m_clPosArrayAux = nullptr;
	cl_mem					m_clNormalArray = nullptr;

	cl_program				m_ClothSimProgram = nullptr;
	cl_kernel				m_NormalKernel = nullptr;
	cl_kernel				m_IntegrateKernel = nullptr;
	cl_kernel				m_ConstraintKernel = nullptr;
	cl_kernel				m_CollisionsKernel = nullptr;

	float					m_ElapsedTime = 0.0f;
	float					m_PrevElapsedTime = 0.0f;
	float					m_simulationTime = 0.0f;

	bool					m_KeyboardMask[255];
	bool					m_InspectCloth = false;

	// mouse
	int						m_Buttons = 0;
	int						m_PrevX = 0;
	int						m_PrevY = 0;

	//for camera handling
	float					m_RotateX = 0.0f;
	float					m_RotateY = 0.0f;
	float					m_TranslateZ = -2.0f;


};

#endif // _CCLOTH_SIMULATION_TASK_H
