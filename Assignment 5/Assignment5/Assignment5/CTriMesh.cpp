#include "CTriMesh.h"

#define MAX_STR 255

#include <iostream>
#include <fstream>
#include <string.h>

using namespace hlsl;

////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTriMesh

CTriMesh::CTriMesh()
{
	//m_ModelMatrix = identity<float, 4, 4>();
}

CTriMesh::~CTriMesh()
{
	ReleaseGLResources();
}

unsigned int CTriMesh::AddVertex(unsigned int Hash, Vertex* pVertex)
{
	//try to find the vertex in the cache
	bool found = false;
	unsigned int index = 0;

	if(m_VertexLoadCache.size() > Hash)
	{
		CacheEntry* pEntry = m_VertexLoadCache[Hash];

		while(pEntry != NULL)
		{
			Vertex* pCacheVertex = &m_Vertices[ pEntry->Index ];
			if(0 == memcmp(pVertex, pCacheVertex, sizeof(Vertex)))
			{
				found = true;
				index = pEntry->Index;
				break;
			}

			pEntry = pEntry->pNext;
		}
	}

	//if the vertex was not found in the cache
	if(!found)
	{
	  index = (unsigned int)(m_Vertices.size());
		m_Vertices.push_back(*pVertex);

		CacheEntry* pNewEntry = new CacheEntry();
		pNewEntry->Index = index;
		pNewEntry->pNext = NULL;

		//grow the cache if needed
		while(m_VertexLoadCache.size() <= Hash)
		{
			m_VertexLoadCache.push_back(NULL);
		}

		CacheEntry* pCurrEntry = m_VertexLoadCache[Hash];
		if(pCurrEntry == NULL)
		{
			//this is the head element
			m_VertexLoadCache[Hash] = pNewEntry;
		}
		else
		{
			//find the tail
			while(pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;

			pCurrEntry->pNext = pNewEntry;
		}
	}

	return index;
}

void CTriMesh::DeleteCache()
{
	for(size_t iEntry = 0; iEntry < m_VertexLoadCache.size(); iEntry++)
	{
		CacheEntry* pEntry = m_VertexLoadCache[ iEntry ];
		while(pEntry != NULL)
		{
			CacheEntry* pNextEntry = pEntry->pNext;
			delete pEntry;
			pEntry = pNextEntry;
		}
	}

	m_VertexLoadCache.clear();
}

CTriMesh* CTriMesh::LoadFromObj(const std::string& Path, const float4x4 &M)
{

	//temporary storage for the parsed data
	std::vector<float3> positions;
	std::vector<float3> normals;
	std::vector<float2>	texcoords;

	//file input
	char strMatFileName[MAX_STR];
	char strCommand[MAX_STR];

	std::ifstream inFile(Path);
	if(!inFile)
	{
		std::cout<<"File not found: "<<Path;
		return NULL;
	}

	CTriMesh* pNewMesh = new CTriMesh();

	//parse the file
#ifdef _MSC_VER
	#pragma warning(disable: 4127)
	while(true)
	#pragma warning(default: 4127)
#else
	while(true)
#endif
	{
		inFile>>strCommand;
		if(!inFile)
			break;

		if(0 == strcmp( strCommand, "#" ))
		{
			//comment
		}
		else if(0 == strcmp( strCommand, "v" ))
		{
			//vertex position
			float x, y, z;
			inFile>>x>>y>>z;

			float4 vertex = mul(float4(x, y, z, 1.f), M);
			positions.push_back(vertex.xyz);
		}
		else if(0 == strcmp( strCommand, "vt" ))
		{
			//vertex texcoord
			float u, v;
			inFile>>u>>v;

			texcoords.push_back( float2(u, v) );
		}
		else if(0 == strcmp( strCommand, "vn" ))
		{
			//vertex normal
			float x, y, z;
			inFile>>x>>y>>z;

			float4 normal = normalize(mul(float4(x, y, z, 0.f), M));
			normals.push_back( normal.xyz );
		}
		else if(0 == strcmp( strCommand, "f" ))
		{
			//face
			//NOTE: the OBJ format uses 1-based arrays
			unsigned int iPos, iTex, iNorm;

			Vertex CurrVertex;
			Face currFace;

			for(unsigned int iVertex = 0; iVertex < 3; iVertex++)
			{
				memset(&CurrVertex, 0, sizeof(CurrVertex));

				inFile>>iPos;
				CurrVertex.Pos = positions[iPos - 1];

				if( '/' == inFile.peek())
				{
					inFile.ignore();

					if('/' != inFile.peek())
					{
						//optional texture coordinate
						inFile>>iTex;
						CurrVertex.Tex = texcoords[iTex - 1];
					}
					if('/' == inFile.peek())
					{
						inFile.ignore();

						//optional vertex normal
						inFile>>iNorm;
						CurrVertex.Norm = normals[iNorm - 1];
					}
				}

				//add this vertex to the vertex buffer, but avoid duplicates
				unsigned int index = pNewMesh->AddVertex(iPos, &CurrVertex);
				currFace.Verts[iVertex] = index;
			}

			pNewMesh->m_Faces.push_back(currFace);
		}
		else if(0 == strcmp(strCommand, "mtllib"))
		{
			//material library
			inFile>>strMatFileName;
		}

		inFile.ignore( 1000, '\n' );
	}

	pNewMesh->DeleteCache();

	return pNewMesh;
}

CTriMesh* CTriMesh::CreatePlane(unsigned int nVertsX, unsigned int nVertsY)
{
	if(nVertsX < 2 || nVertsY < 2)
	{
		std::cout<<"Invalid plane resolution"<<std::endl;
		return 0;
	}

	CTriMesh* pNewMesh = new CTriMesh();
	
	//vertex data
	unsigned int xSegments = nVertsX - 1;
	unsigned int ySegments = nVertsY - 1;

	float deltaX = 1.0f / xSegments;
	float deltaY = 1.0f / ySegments;

	for(unsigned int y = 0; y < nVertsY; y++)
		for(unsigned int x = 0; x < nVertsX; x++)
		{
			Vertex vert;
			vert.Pos = float3(deltaX * x - 0.5f, 0.6f, -deltaY * y);
			vert.Norm = float3(0, 0, 1.0f);
			vert.Tex = float2(deltaX * x, deltaY * y);
			pNewMesh->m_Vertices.push_back(vert);
		}

	//create indices
	for(unsigned int x = 0; x < xSegments; x++)
		for(unsigned int y = 0; y < ySegments; y++)
		{
			Face A;
			A.Verts[0] = y * nVertsX + x;
			A.Verts[2] = A.Verts[0] + 1;
			A.Verts[1] = A.Verts[0] + nVertsX;

			Face B;
			B.Verts[0] = A.Verts[1];
			B.Verts[2] = A.Verts[2];
			B.Verts[1] = B.Verts[2] + nVertsX;

			pNewMesh->m_Faces.push_back(A);
			pNewMesh->m_Faces.push_back(B);
		}

	return pNewMesh;
}

bool CTriMesh::CreateGLResources()
{
	ReleaseGLResources();


	//Vertex* pVertices = new Vertex[m_Vertices.size()];
	//for(size_t iVert = 0; iVert < m_Vertices.size(); iVert++)
	//{
	//	pVertices[iVert] = m_Vertices[iVert];
	//}
	float4* pVertices = new float4[m_Vertices.size()];
	float4* pNormals = new float4[m_Vertices.size()];
	float2* pTexCoords = new float2[m_Vertices.size()];

	for(size_t iVert = 0; iVert < m_Vertices.size(); iVert++)
	{
		pVertices[iVert] = float4(m_Vertices[iVert].Pos, 1.0f);
		pNormals[iVert] = float4(m_Vertices[iVert].Norm, 0.0f);
		pTexCoords[iVert] = m_Vertices[iVert].Tex;
	}

	glGenBuffers(1, &m_glVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float4) * m_Vertices.size(), pVertices, GL_DYNAMIC_DRAW);
	V_RETURN_OGL_ERROR();
	
	glGenBuffers(1, &m_glNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float4) * m_Vertices.size(), pNormals, GL_DYNAMIC_DRAW);
	V_RETURN_OGL_ERROR();

	glGenBuffers(1, &m_glTexCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glTexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * m_Vertices.size(), pTexCoords, GL_DYNAMIC_DRAW);
	V_RETURN_OGL_ERROR();

	delete [] pVertices;
	delete [] pNormals;
	delete [] pTexCoords;
	
	glGenBuffers(1, &m_glIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);

	unsigned int* pIndices = new unsigned int[m_Faces.size() * 3];

	for(size_t i = 0; i < m_Faces.size(); i++)
	{
		pIndices[3 * i] = m_Faces[i].Verts[0];
		pIndices[3 * i + 1] = m_Faces[i].Verts[1];
		pIndices[3 * i + 2] = m_Faces[i].Verts[2];
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * m_Faces.size(), pIndices, GL_STATIC_DRAW);
	V_RETURN_OGL_ERROR();

	delete [] pIndices;


	return true;
}

void CTriMesh::ReleaseGLResources()
{
	SAFE_RELEASE_GL_BUFFER(m_glVertexBuffer);
	CHECK_FOR_OGL_ERROR();
	SAFE_RELEASE_GL_BUFFER(m_glNormalBuffer);
	CHECK_FOR_OGL_ERROR();
	SAFE_RELEASE_GL_BUFFER(m_glTexCoordBuffer);
	CHECK_FOR_OGL_ERROR();
	SAFE_RELEASE_GL_BUFFER(m_glIndexBuffer);
	CHECK_FOR_OGL_ERROR();
}

void CTriMesh::DrawGL(GLenum mode)
{
	glEnable(GL_TEXTURE_2D);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	CHECK_FOR_OGL_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
	glVertexPointer(3, GL_FLOAT, sizeof(float4), 0);
	glBindBuffer(GL_ARRAY_BUFFER, m_glNormalBuffer);
	glNormalPointer(GL_FLOAT, sizeof(float4), (char*)0);
	glBindBuffer(GL_ARRAY_BUFFER, m_glTexCoordBuffer);
	glClientActiveTexture(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float2), (char*)0);

	CHECK_FOR_OGL_ERROR();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
	CHECK_FOR_OGL_ERROR();
	glDrawElements(mode, m_Faces.size() * 3, GL_UNSIGNED_INT, 0);
	CHECK_FOR_OGL_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	CHECK_FOR_OGL_ERROR();
}

GLuint CTriMesh::GetVertexBuffer() const
{
	return m_glVertexBuffer;
}

GLuint CTriMesh::GetNormalBuffer() const
{
	return m_glNormalBuffer;
}

//void CTriMesh::SetTransform(const float4x4& Matrix)
//{
//	m_ModelMatrix = Matrix;
//}
//
//const float4x4* CTriMesh::GetTransform() const
//{
//	return &m_ModelMatrix;
//}

void CTriMesh::GetTriangleSoup(float** ppPositionBuffer, unsigned int* pNumTriangles)
{
	*ppPositionBuffer = new float[4 * 3 * m_Faces.size()];
	float4* pPositionBuffer = (float4*)*ppPositionBuffer;
	*pNumTriangles = m_Faces.size();

	//transform all the vertices to world space before copying
	for(unsigned int iFace = 0; iFace < m_Faces.size(); iFace++)
	{
		for(unsigned int i = 0; i < 3; i++)
		{
			float4 mpos(m_Vertices[ m_Faces[iFace].Verts[i] ].Pos, 1.0f);
			//float4 wpos = mul(mpos, m_ModelMatrix);

			*pPositionBuffer = mpos;
			pPositionBuffer++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
