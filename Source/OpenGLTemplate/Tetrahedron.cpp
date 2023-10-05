#include "Tetrahedron.h"

CTetrahedron::CTetrahedron()
{}

CTetrahedron::~CTetrahedron()
{
	Release();
}

void CTetrahedron::Create(string filename)
{
	m_tTexture.Load(filename);
	m_tTexture.SetSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_tTexture.SetSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_tTexture.SetSamplerParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_tTexture.SetSamplerParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenVertexArrays(1, &m_uiVAO);
	glBindVertexArray(m_uiVAO);
	m_VBO.Create();
	m_VBO.Bind();

	// Write the code to add interleaved point, texture coord, and normal of the cube
	//front
	glm::vec3 v0 = glm::vec3(-1, -1, 1);
	glm::vec3 v1 = glm::vec3(1, -1, 1);
	glm::vec3 v2 = glm::vec3(0, 1, 0);

	//right
	glm::vec3 v3 = glm::vec3(1, -1, 1);
	glm::vec3 v4 = glm::vec3(1, -1, -1);
	glm::vec3 v5 = glm::vec3(0, 1, 0);

	//back face
	glm::vec3 v6 = glm::vec3(0, 1, 0);
	glm::vec3 v7 = glm::vec3(1, -1, -1);
	glm::vec3 v8 = glm::vec3(-1, -1, -1);

	//left face
	glm::vec3 v9 = glm::vec3(0, 1, 0);
	glm::vec3 v10 = glm::vec3(-1, -1, -1);
	glm::vec3 v11 = glm::vec3(-1, -1, 1);

	glm::vec3 n0 = glm::vec3(0, 1, 1);
	glm::vec3 n1 = glm::vec3(1, 1, 0);
	glm::vec3 n2 = glm::vec3(0, 1, -1);
	glm::vec3 n3 = glm::vec3(-1, 1, 0);

	glm::vec2 t0 = glm::vec2(0, 0);
	glm::vec2 t1 = glm::vec2(1, 0);
	glm::vec2 t2 = glm::vec2(1, 1);
	glm::vec2 t3 = glm::vec2(0, 1);



	//front face
	m_VBO.AddData(&v0, sizeof(glm::vec3));
	m_VBO.AddData(&t0, sizeof(glm::vec2));
	m_VBO.AddData(&n0, sizeof(glm::vec3));
	m_VBO.AddData(&v1, sizeof(glm::vec3));
	m_VBO.AddData(&t1, sizeof(glm::vec2));
	m_VBO.AddData(&n0, sizeof(glm::vec3));
	m_VBO.AddData(&v2, sizeof(glm::vec3));
	m_VBO.AddData(&t2, sizeof(glm::vec2));
	m_VBO.AddData(&n0, sizeof(glm::vec3));

	//right face 421

	m_VBO.AddData(&v3, sizeof(glm::vec3));
	m_VBO.AddData(&t0, sizeof(glm::vec2));
	m_VBO.AddData(&n1, sizeof(glm::vec3));
	m_VBO.AddData(&v4, sizeof(glm::vec3));
	m_VBO.AddData(&t1, sizeof(glm::vec2));
	m_VBO.AddData(&n1, sizeof(glm::vec3));
	m_VBO.AddData(&v5, sizeof(glm::vec3));
	m_VBO.AddData(&t2, sizeof(glm::vec2));
	m_VBO.AddData(&n1, sizeof(glm::vec3));

	//back face 

	m_VBO.AddData(&v6, sizeof(glm::vec3));
	m_VBO.AddData(&t0, sizeof(glm::vec2));
	m_VBO.AddData(&n2, sizeof(glm::vec3));
	m_VBO.AddData(&v7, sizeof(glm::vec3));
	m_VBO.AddData(&t1, sizeof(glm::vec2));
	m_VBO.AddData(&n2, sizeof(glm::vec3));
	m_VBO.AddData(&v8, sizeof(glm::vec3));
	m_VBO.AddData(&t2, sizeof(glm::vec2));
	m_VBO.AddData(&n2, sizeof(glm::vec3));

	//left face

	m_VBO.AddData(&v9, sizeof(glm::vec3));
	m_VBO.AddData(&t0, sizeof(glm::vec2));
	m_VBO.AddData(&n3, sizeof(glm::vec3));
	m_VBO.AddData(&v10, sizeof(glm::vec3));
	m_VBO.AddData(&t1, sizeof(glm::vec2));
	m_VBO.AddData(&n3, sizeof(glm::vec3));
	m_VBO.AddData(&v11, sizeof(glm::vec3));
	m_VBO.AddData(&t2, sizeof(glm::vec2));
	m_VBO.AddData(&n3, sizeof(glm::vec3));


	// Upload data to GPU
	m_VBO.UploadDataToGPU(GL_STATIC_DRAW);
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

}

void CTetrahedron::Render()
{
	glBindVertexArray(m_uiVAO);
	m_tTexture.Bind();

	// Call glDrawArrays to render the side
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
	glDrawArrays(GL_TRIANGLE_STRIP, 3, 3);
	glDrawArrays(GL_TRIANGLE_STRIP, 6, 3);
	glDrawArrays(GL_TRIANGLE_STRIP, 9, 3);
}

void CTetrahedron::Release()
{
	m_tTexture.Release();
	glDeleteVertexArrays(1, &m_uiVAO);
	m_VBO.Release();
}
