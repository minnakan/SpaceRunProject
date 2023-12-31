#include "VertexBufferObjectIndexed.h"


// Constructor -- initialise member variable m_bDataUploaded to false
CVertexBufferObjectIndexed::CVertexBufferObjectIndexed()
{
	m_bDataUploaded = false;
}

CVertexBufferObjectIndexed::~CVertexBufferObjectIndexed()
{}


// Create buffer objects for the vertices and indices
void CVertexBufferObjectIndexed::Create()
{
	glGenBuffers(1, &m_uiVBOVertices);
	glGenBuffers(1, &m_uiVBOIndices);
}

// Release the buffers and any associated data
void CVertexBufferObjectIndexed::Release()
{
	glDeleteBuffers(1, &m_uiVBOVertices);
	glDeleteBuffers(1, &m_uiVBOIndices);
	m_bDataUploaded = false;
	m_vertexData.clear();
	m_indexData.clear();
}


// Binds the buffers
void CVertexBufferObjectIndexed::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVBOVertices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiVBOIndices);
}


// Uploads the data to the GPU.  Afterwards, the data can be cleared.  
// iUsageHint - GL_STATIC_DRAW, GL_DYNAMIC_DRAW...
void CVertexBufferObjectIndexed::UploadDataToGPU(int iUsageHint)
{

	glBufferData(GL_ARRAY_BUFFER, m_vertexData.size(), &m_vertexData[0], iUsageHint);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexData.size(), &m_indexData[0], iUsageHint);
	m_bDataUploaded = true;
	m_vertexData.clear();
	m_indexData.clear();
}

// Adds data to the VBO.  
void CVertexBufferObjectIndexed::AddVertexData(void* ptrVertexData, UINT uiVertexDataSize)
{
	m_vertexData.insert(m_vertexData.end(), (BYTE*)ptrVertexData, (BYTE*)ptrVertexData+uiVertexDataSize);
}


// Adds data to the VBO.  
void CVertexBufferObjectIndexed::AddIndexData(void* ptrIndexData, UINT uiIndexDataSize)
{
	m_indexData.insert(m_indexData.end(), (BYTE*)ptrIndexData, (BYTE*)ptrIndexData+uiIndexDataSize);
}

