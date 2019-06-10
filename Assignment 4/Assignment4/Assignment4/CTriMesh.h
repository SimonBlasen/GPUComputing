/*
This class handles triangle meshes.
*/

#ifndef CTRIMESH_H
#define CTRIMESH_H

#ifdef min // these macros are defined under windows, but collide with our math utility
#	undef min
#endif
#ifdef max
#	undef max
#endif

#include "HLSLEx.h"

#include "GLCommon.h"

#include <vector>
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTriMesh
{
protected:

	//only for loading from obj
	struct CacheEntry
	{
		unsigned int	Index;
		CacheEntry*		pNext;
	};
	
	struct Vertex
	{
		hlsl::float3		Pos;
		hlsl::float3		Norm;
		hlsl::float2		Tex;
	};

	struct Face
	{
		unsigned int	Verts[3];
	};

	//CPU data
	std::vector<CacheEntry*>m_VertexLoadCache;
	std::vector<Vertex>		m_Vertices;
	std::vector<Face>		m_Faces;

	//transformation
	//float4x4				m_ModelMatrix;

	//rendering data on the GPU
	GLuint					m_glVertexBuffer = 0;
	GLuint					m_glNormalBuffer = 0;
	GLuint					m_glTexCoordBuffer = 0;
	GLuint					m_glIndexBuffer = 0;

	//adds a new vertex to the vertex buffer and returns its index.
	unsigned int AddVertex(unsigned int Hash, Vertex* pVertex);
	
	void DeleteCache();

public:
	CTriMesh();

	~CTriMesh();

	static CTriMesh* LoadFromObj(const std::string& Path, const hlsl::float4x4 &M);

	//create a unit plane with a given resolution
	static CTriMesh* CreatePlane(unsigned int nVertsX, unsigned int nVertsY);

	bool CreateGLResources();

	void ReleaseGLResources();

	GLuint GetVertexBuffer() const;

	GLuint GetNormalBuffer() const;

	//void SetTransform(const float4x4& Matrix);

	//const float4x4* GetTransform() const;

	//returns the WORLD positions of the triangle in the buffer
	//the float array will contain triplets of vertices, each vertex having 4 floats as world coordinate (float4)
	void GetTriangleSoup(float** ppPositionBuffer, unsigned int* pNumVertices);

	//renders the triangles using the current OpenGL program
	void DrawGL(GLenum mode);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
