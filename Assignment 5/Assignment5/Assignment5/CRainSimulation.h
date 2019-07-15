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


#include "../Common/IGUIEnabledComputeTask.h"

#include "CTriMesh.h"
#include "CGLTexture.h"
#include <math.h>

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
	virtual bool ValidateResults() {return false;};

	// IGUIEnabledComputeTask
	virtual void Render();

	virtual void OnKeyboard(int Key, int Action);

	virtual void OnMouse(int Button, int State);

	virtual void OnMouseMove(int X, int Y);

	virtual void OnIdle(double Time, float ElapsedTime);

	virtual void OnWindowResized(int Width, int Height);

	virtual float Gauss2D(float x, float y, float uX, float uY, float variant);

protected:
	unsigned int			m_TerrainResX = 0;
	unsigned int			m_TerrainResY = 0;
	unsigned int			m_FrameCounter = 0;
	unsigned long			m_TerrainSeed = 0;

	CTriMesh*				m_pTerrainModel = nullptr;
	//CTriMesh*				m_pClothModel = nullptr;
	CTriMesh*				m_pEnvironment = nullptr;
	//CGLTexture*				m_pClothTexture = nullptr;
	CGLTexture*				m_pTerrainTexture = nullptr;
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
	cl_mem					m_clRainArray = nullptr;
	cl_mem					m_clPosArrayAux = nullptr;
	cl_mem					m_clNormalArray = nullptr;
	float*					m_hRainArray;
	hlsl::float4*			m_rainSpots;
	unsigned int			m_n_rainSpots;

	cl_program				m_TerrainSimProgram = nullptr;
	cl_kernel				m_NormalKernel = nullptr;
	cl_kernel				m_IntegrateKernel = nullptr;
	cl_kernel				m_ConstraintKernel = nullptr;
	cl_kernel				m_CollisionsKernel = nullptr;

	cl_kernel				m_InitTerrainKernel = nullptr;

	float					m_ElapsedTime = 0.0f;
	float					m_PrevElapsedTime = 0.0f;
	float					m_simulationTime = 0.0f;

	bool					m_KeyboardMask[255];
	bool					m_InspectCloth = false;

	bool					m_firstRun = true;

	// mouse
	int						m_Buttons = 0;
	int						m_PrevX = 0;
	int						m_PrevY = 0;

	//for camera handling
	float					m_RotateX = 0.0f;
	float					m_RotateY = 0.0f;
	float					m_TranslateZ = -2.0f;


};

