#include "CatmullRom.h"
#define _USE_MATH_DEFINES
#include <math.h>



CCatmullRom::CCatmullRom()
{
	m_vertexCount = 0;
	texture.Load("resources\\textures\\road.jpg");
}

CCatmullRom::~CCatmullRom()
{}

// Perform Catmull Rom spline interpolation between four points, interpolating the space between p1 and p2
glm::vec3 CCatmullRom::Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	glm::vec3 a = p1;
	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

	return a + b * t + c * t2 + d * t3;

}


void CCatmullRom::SetControlPoints()
{
	// Set control points (m_controlPoints) here, or load from disk
	m_controlPoints.push_back(glm::vec3(0, 1, 0));
	m_controlPoints.push_back(glm::vec3(250, 1, -71));
	m_controlPoints.push_back(glm::vec3(300, 1, -200));
	m_controlPoints.push_back(glm::vec3(350, 1, -400));
	m_controlPoints.push_back(glm::vec3(0, 1, -800));
	m_controlPoints.push_back(glm::vec3(0, 10.f, -1000));
	m_controlPoints.push_back(glm::vec3(0, 20, -1200));
	m_controlPoints.push_back(glm::vec3(0, 10.f, -1400));
	m_controlPoints.push_back(glm::vec3(0, 1.f, -1600));
	m_controlPoints.push_back(glm::vec3(-250, 1, -1400));
	m_controlPoints.push_back(glm::vec3(-300, 1, -1200));
	m_controlPoints.push_back(glm::vec3(-325, 1.f, -1000));
	m_controlPoints.push_back(glm::vec3(-350, 1, -800));
	m_controlPoints.push_back(glm::vec3(-300, 1, -400));
	m_controlPoints.push_back(glm::vec3(-350, 1, -200));




	// Optionally, set upvectors (m_controlUpVectors, one for each control point as well)

}


// Determine lengths along the control points, which is the set of control points forming the closed curve
void CCatmullRom::ComputeLengthsAlongControlPoints()
{
	int M = (int)m_controlPoints.size();

	float fAccumulatedLength = 0.0f;
	m_distances.push_back(fAccumulatedLength);
	for (int i = 1; i < M; i++) {
		fAccumulatedLength += glm::distance(m_controlPoints[i - 1], m_controlPoints[i]);
		m_distances.push_back(fAccumulatedLength);
	}

	// Get the distance from the last point to the first
	fAccumulatedLength += glm::distance(m_controlPoints[M - 1], m_controlPoints[0]);
	m_distances.push_back(fAccumulatedLength);
}


// Return the point (and upvector, if control upvectors provided) based on a distance d along the control polygon
bool CCatmullRom::Sample(float d, glm::vec3& p, glm::vec3 up)
{


	if (d < 0)
		return false;

	int M = (int)m_controlPoints.size();
	if (M == 0)
		return false;


	float fTotalLength = m_distances[m_distances.size() - 1];

	// The the current length along the control polygon; handle the case where we've looped around the track
	float fLength = d - (int)(d / fTotalLength) * fTotalLength;

	// Find the current segment
	int j = -1;
	for (int i = 0; i < (int)m_distances.size(); i++) {
		if (fLength >= m_distances[i] && fLength < m_distances[i + 1]) {
			j = i; // found it!
			break;
		}
	}

	if (j == -1)
		return false;

	// Interpolate on current segment -- get t
	float fSegmentLength = m_distances[j + 1] - m_distances[j];
	float t = (fLength - m_distances[j]) / fSegmentLength;

	// Get the indices of the four points along the control polygon for the current segment
	int iPrev = ((j - 1) + M) % M;
	int iCur = j;
	int iNext = (j + 1) % M;
	int iNextNext = (j + 2) % M;

	// Interpolate to get the point (and upvector)
	p = Interpolate(m_controlPoints[iPrev], m_controlPoints[iCur], m_controlPoints[iNext], m_controlPoints[iNextNext], t);
	if (m_controlUpVectors.size() == m_controlPoints.size())
		up = glm::normalize(Interpolate(m_controlUpVectors[iPrev], m_controlUpVectors[iCur], m_controlUpVectors[iNext], m_controlUpVectors[iNextNext], t));


	return true;
}



// Sample a set of control points using an open Catmull-Rom spline, to produce a set of iNumSamples that are (roughly) equally spaced
void CCatmullRom::UniformlySampleControlPoints(int numSamples)
{
	glm::vec3 p, up;

	// Compute the lengths of each segment along the control polygon, and the total length
	ComputeLengthsAlongControlPoints();
	float fTotalLength = m_distances[m_distances.size() - 1];

	// The spacing will be based on the control polygon
	float fSpacing = fTotalLength / numSamples;

	// Call PointAt to sample the spline, to generate the points
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);

	}


	// Repeat once more for truly equidistant points
	m_controlPoints = m_centrelinePoints;
	m_controlUpVectors = m_centrelineUpVectors;
	m_centrelinePoints.clear();
	m_centrelineUpVectors.clear();
	m_distances.clear();
	ComputeLengthsAlongControlPoints();
	fTotalLength = m_distances[m_distances.size() - 1];
	fSpacing = fTotalLength / numSamples;
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);
	}


}



void CCatmullRom::CreateCentreline()
{
	// Call Set Control Points
	SetControlPoints();

	// Call UniformlySampleControlPoints with the number of samples required
	UniformlySampleControlPoints(600);

	// Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
	glGenVertexArrays(1, &m_vaoCentreline);
	glBindVertexArray(m_vaoCentreline);

	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();

	int M = (int)m_controlPoints.size();


	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < 600; i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = m_centrelinePoints[i];
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));

	}

	// Upload the VBO to the GPU
	vbo.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}


void CCatmullRom::CreateOffsetCurves()
{
	// Compute the offset curves, one left, and one right.  Store the points in m_leftOffsetPoints and m_rightOffsetPoints respectively
	glm::vec3 p;
	glm::vec3 pNext;
	glm::vec3 l;
	glm::vec3 r;
	float w = 100.f;
	for (int i = 0; i < m_centrelinePoints.size() - 1; i++) {
		p = m_centrelinePoints[i];
		pNext = m_centrelinePoints[i + 1.f];

		//TNB frame
		glm::vec3 T = glm::normalize(pNext - p);
		glm::vec3 N = glm::normalize(glm::cross(T, glm::vec3(0, 1.f, 0)));
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		l = p - (w / 2) * N;
		r = p + (w / 2) * N;

		m_leftOffsetPoints.push_back(l);
		m_rightOffsetPoints.push_back(r);

	}

	// Generate two VAOs called m_vaoLeftOffsetCurve and m_vaoRightOffsetCurve, each with a VBO, and get the offset curve points on the graphics card
	//left
	glGenVertexArrays(1, &m_vaoLeftline);
	glBindVertexArray(m_vaoLeftline);

	CVertexBufferObject vboLeft;
	vboLeft.Create();
	vboLeft.Bind();
	int M = (int)m_leftOffsetPoints.size();


	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < m_leftOffsetPoints.size(); i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = m_leftOffsetPoints[i];
		vboLeft.AddData(&v, sizeof(glm::vec3));
		vboLeft.AddData(&texCoord, sizeof(glm::vec2));
		vboLeft.AddData(&normal, sizeof(glm::vec3));

	}
	// Upload the VBO to the GPU
	vboLeft.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));



	//right
	glGenVertexArrays(1, &m_vaoRightline);
	glBindVertexArray(m_vaoRightline);

	CVertexBufferObject vboRight;
	vboRight.Create();
	vboRight.Bind();
	M = (int)m_rightOffsetPoints.size();



	for (unsigned int i = 0; i < m_rightOffsetPoints.size(); i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = m_rightOffsetPoints[i];
		vboRight.AddData(&v, sizeof(glm::vec3));
		vboRight.AddData(&texCoord, sizeof(glm::vec2));
		vboRight.AddData(&normal, sizeof(glm::vec3));

	}

	// Upload the VBO to the GPU
	vboRight.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));



}


void CCatmullRom::CreateTrack()
{

	// Generate a VAO called m_vaoTrack and a VBO to get the offset curve points and indices on the graphics card
	glGenVertexArrays(1, &m_vaoTrack);
	glBindVertexArray(m_vaoTrack);

	CVertexBufferObject vboTrack;
	vboTrack.Create();
	vboTrack.Bind();

	m_vertexCount = 0.f;

	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < m_leftOffsetPoints.size(); i++) {
		float t = (float)i / 100.0f;

		glm::vec2 leftTex(0.0f, t);
		glm::vec2 rightTex(1.f, t);

		//left
		glm::vec3 l = m_leftOffsetPoints[i];
		vboTrack.AddData(&l, sizeof(glm::vec3));
		vboTrack.AddData(&leftTex, sizeof(glm::vec2));
		vboTrack.AddData(&normal, sizeof(glm::vec3));
		m_vertexCount++;

		//right
		glm::vec3 r = m_rightOffsetPoints[i];
		vboTrack.AddData(&r, sizeof(glm::vec3));
		vboTrack.AddData(&rightTex, sizeof(glm::vec2));
		vboTrack.AddData(&normal, sizeof(glm::vec3));
		m_vertexCount++;

		
	}

	// Upload the VBO to the GPU
	vboTrack.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

}


void CCatmullRom::RenderCentreline()
{
	// Bind the VAO m_vaoCentreline and render it
	glPointSize(10.f);
	glLineWidth(5.0f);
	glBindVertexArray(m_vaoCentreline);
	glDrawArrays(GL_POINTS, 0, 500);
	//glDrawArrays(GL_LINE_LOOP, 0, 250);

}

void CCatmullRom::RenderOffsetCurves()
{
	// Bind the VAO m_vaoLeftOffsetCurve and render it
	glPointSize(10.f);
	glLineWidth(5.0f);
	glBindVertexArray(m_vaoLeftline);
	glDrawArrays(GL_POINTS, 0, 500);
	glDrawArrays(GL_LINE_STRIP, 0, 500);

	// Bind the VAO m_vaoRightOffsetCurve and render it
	glBindVertexArray(m_vaoRightline);
	glDrawArrays(GL_POINTS, 0, 500);
	glDrawArrays(GL_LINE_STRIP, 0, 500);
}


void CCatmullRom::RenderTrack()
{
	// Bind the VAO m_vaoTrack and render it
		// Bind the VAO m_vaoCentreline and render it
	glPointSize(10.f);
	glLineWidth(5.0f);
	glBindVertexArray(m_vaoTrack);
	texture.Bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexCount);

}

int CCatmullRom::CurrentLap(float d)
{

	return (int)(d / m_distances.back());

}

vector<glm::vec3> CCatmullRom::GetTrackPoints() {

	vector<glm::vec3> TrackPoints;
	for (int i = 0; i < m_rightOffsetPoints.size(); i++) {
		TrackPoints.push_back(m_leftOffsetPoints[i]);
		TrackPoints.push_back(m_rightOffsetPoints[i]);
	}

	return TrackPoints;
}