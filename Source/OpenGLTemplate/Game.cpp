/* 
OpenGL Template for INM376 / IN3005
City University London, School of Informatics
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by School of Informatics, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting, 
 different camera controls, different shaders, etc.
 
 Template version 2.0a 31/01/2014
 Dr. Greg Slabaugh (gregory.slabaugh.1@city.ac.uk) 
*/


#include "game.h"


// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "Sphere.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Cube.h"
#include "CatmullRom.h"
#include "Tetrahedron.h"
#include "HeightMapTerrain.h"


// Constructor
Game::Game()
{
	m_pSkybox = NULL;
	m_pCamera = NULL;
	m_pShaderPrograms = NULL;
	m_pPlanarTerrain = NULL;
	m_pFtFont = NULL;
	m_pPlayerMesh = NULL;
	m_pSphere = NULL;
	m_pHighResolutionTimer = NULL;
	m_pCube = NULL;
	m_pWall = NULL;
	m_pObst = NULL;
	m_pHeightmapTerrain = NULL;
	m_pPickUp = NULL;
	m_spacePod = NULL;
	

	m_pCatmullRom = NULL;

	m_dt = 0.0;
	m_iFramesPerSecond = 0;
	playerAngle = 0.f;

	m_boundary = 0.f;

	m_t = 0;
	m_spaceShipPosition = glm::vec3(0);
	m_spaceShipOrientation = glm::mat4(1);
	m_playerPos = glm::vec3(0);
	freeLook = false;

	
	phase = 1.f;

	m_currentDistance = 0.0f;
	m_cameraMovement = 0.0f;
	p_speed = 2.5f;

	time_el = 0.f;

	score = 0;

}

// Destructor
Game::~Game() 
{ 
	//game objects
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
	delete m_pPlayerMesh;
	delete m_pSphere;
	delete m_pCube;
	delete m_pWall;
	delete m_pObst;
	delete m_pHeightmapTerrain;
	delete m_pPickUp;
	delete m_spacePod;

	

	delete m_pCatmullRom;
	

	if (m_pShaderPrograms != NULL) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pHighResolutionTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise() 
{
	// Set the clear colour and depth
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	/// Create objects
	m_pCamera = new CCamera;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector <CShaderProgram *>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pPlayerMesh = new COpenAssetImportMesh;
	m_pPickUp = new COpenAssetImportMesh;
	m_spacePod = new COpenAssetImportMesh;
	m_pSphere = new CSphere;
	m_pCube = new CCube;
	m_pCatmullRom = new CCatmullRom;
	m_pWall = new CCube;
	m_pObst = new CTetrahedron;
	m_pHeightmapTerrain = new CHeightMapTerrain;

	

	RECT dimensions = m_gameWindow.GetDimensions();

	int width = dimensions.right - dimensions.left;
	int height = dimensions.bottom - dimensions.top;

	// Set the orthographic and perspective projection matrices based on the image size
	m_pCamera->SetOrthographicProjectionMatrix(width, height); 
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float) width / (float) height, 0.5f, 5000.0f);

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");
	sShaderFileNames.push_back("sphereShader.vert");
	sShaderFileNames.push_back("sphereShader.frag");
	sShaderFileNames.push_back("lightShader.vert");
	sShaderFileNames.push_back("lightShader.frag");

	for (int i = 0; i < (int) sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int) sShaderFileNames[i].size()-4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
		shader.LoadShader("resources\\shaders\\"+sShaderFileNames[i], iShaderType);
		shShaders.push_back(shader);
	}

	// Create the main shader program
	CShaderProgram *pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	// Create a shader program for fonts
	CShaderProgram *pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	CShaderProgram* pSphereProgram = new CShaderProgram;
	pSphereProgram->CreateProgram();
	pSphereProgram->AddShaderToProgram(&shShaders[4]);
	pSphereProgram->AddShaderToProgram(&shShaders[5]);
	pSphereProgram->LinkProgram();
	m_pShaderPrograms->push_back(pSphereProgram);

	// You can follow this pattern to load additional shaders
	CShaderProgram* pLightProgram = new CShaderProgram;
	pLightProgram->CreateProgram();
	pLightProgram->AddShaderToProgram(&shShaders[6]);
	pLightProgram->AddShaderToProgram(&shShaders[7]);
	pLightProgram->LinkProgram();
	m_pShaderPrograms->push_back(pLightProgram);

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
	//m_pSkybox->Create("resources\\skyboxes\\jajdarkland1\\", "jajdarkland1_ft.jpg", "jajdarkland1_bk.jpg", "jajdarkland1_lf.jpg", "jajdarkland1_rt.jpg", "jajdarkland1_up.jpg", "jajdarkland1_dn.jpg", 2500.0f);
	m_pSkybox->Create("resources\\skyboxes\\space\\", "space_ft.png", "space_bk.png", "space_lf.png", "space_rt.png", "space_up.png", "space_dn.png", 2500.0f);
	// Create the planar terrain
	m_pPlanarTerrain->Create("resources\\textures\\", "grassfloor01.jpg", 4000.0f, 4000.0f, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

	m_pFtFont->LoadSystemFont("arial.ttf", 32);
	m_pFtFont->SetShaderProgram(pFontProgram);

	// Load some meshes in OBJ format
	
	m_pPlayerMesh->Load("resources\\models\\flyingDisk\\Ashtar Flying Disk.obj");  // Downloaded from http://opengameart.org/content/horse-lowpoly on 24 Jan 2013
	m_pPickUp->Load("resources\\models\\PickUp\\PickUp.obj");
	m_spacePod->Load("resources\\models\\ship\\Arc170.obj");

	
	  

	
	
	glEnable(GL_CULL_FACE);

	//Create the wall as cube
	m_pWall->Create("resources\\textures\\gren.jpg");

	

	m_pObst->Create("resources\\textures\\path.jpg");



	

	m_pCatmullRom->CreateCentreline();
	m_pCatmullRom->CreateOffsetCurves();
	m_pCatmullRom->CreateTrack();

	wallPoints = m_pCatmullRom->GetTrackPoints();


	//game starts at lvlOne
	mapMode = true;

	//specifying pickUp points for the sphere
	pickupPoints.push_back(glm::vec3(125.f, 7.f, -20.f));
	pickupPoints.push_back(glm::vec3(320.f, 7.f, -430.f));
	pickupPoints.push_back(glm::vec3(0.f, 7.f, -820.f));
	pickupPoints.push_back(glm::vec3(40.f, 12.f, -1500.f));
	pickupPoints.push_back(glm::vec3(-320.f, 7.f, -1000.f));
	pickupPoints.push_back(glm::vec3(-320.f, 7.f, -200.f));

	pickupPoints2.push_back(glm::vec3(290.f, 7.f, -240.f));
	pickupPoints2.push_back(glm::vec3(100.f, 7.f, -680.f));
	pickupPoints2.push_back(glm::vec3(0.f, 12.f, -990.f));
	pickupPoints2.push_back(glm::vec3(-150.f, 7.f, -1500.f));
	pickupPoints2.push_back(glm::vec3(-320.f, 7.f, -600.f));

	

	finalPickUp = glm::vec3(-320.f, 7.f, -200.f);
	finalPickUp2 = glm::vec3(-320.f, 7.f, -600.f);


	currentPickUp = 0;
	currentPickUp2 = 0;
	gameOver = false;

	m_objPos = pickupPoints[0];
	m_objPos2 = pickupPoints2[0];

	//specifying obstacle points that will be rendered as a tetrahedron
	obstaclePoints.push_back(glm::vec3(210.f, 7.f, -160.f));
	
	glow = 0.f;
	glow_fade = false;

	time_el = 0.f;
	fadeout = 1.f;

	m_pHeightmapTerrain->Create("resources\\textures\\terrainHeightMap201.bmp", "resources\\textures\\back.jpg", glm::vec3(0, 0, 0), 4000.0f, 4000.0f, 50.5f); //http://spiralgraphics.biz
	

}

// Render method runs repeatedly in a loop
void Game::Render() 
{
	
	// Clear the buffers and enable depth testing (z-buffering)
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Set up a matrix stack
	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	// Use the main shader program 
	CShaderProgram *pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);

	// Set the projection and modelview matrix based on the current camera 	
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	
	
	// Set light and materials in main shader program
	glm::vec4 vPosition(-100, 100, -100, 1);
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 normalMatrix = m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top());

	// Convert light position to eye coordinates, since lighting is done in eye coordinates
	glm::vec4 vLightEye = modelViewMatrixStack.Top()*vPosition;
	pMainProgram->SetUniform("light1.position", vLightEye);		// Position of light source in eye coordinates
	pMainProgram->SetUniform("light1.La", glm::vec3(1.0f));		// Ambient colour of light
	pMainProgram->SetUniform("light1.Ld", glm::vec3(1.0f));		// Diffuse colour of light
	pMainProgram->SetUniform("light1.Ls", glm::vec3(1.0f));		// Specular colour of light
	pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.0f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.0f));	// Specular material reflectance
	pMainProgram->SetUniform("material1.shininess", 15.0f);		// Shininess material property

	//light and materials for the dynamic light
	glm::vec4 lightPosition1(m_playerPos.x, 25, m_playerPos.z, 1); 
	glm::vec4 lightPosition2(125.f, 50, -20.f, 1);

	
		

	// Render the skybox and terrain with full ambient reflectance 
	modelViewMatrixStack.Push();
		// Translate the modelview matrix to the camera eye point so skybox stays centred around camera
		glm::vec3 vEye = m_pCamera->GetPosition();
		modelViewMatrixStack.Translate(vEye);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pSkybox->Render();
	modelViewMatrixStack.Pop();

	

	// Render the planar terrain
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pPlanarTerrain->Render();
	modelViewMatrixStack.Pop();

	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pHeightmapTerrain->Render();
	modelViewMatrixStack.Pop();



	// Turn on diffuse + specular materials
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance	



	CShaderProgram* pSphereProgram = (*m_pShaderPrograms)[2];
	pSphereProgram->UseProgram();
	pSphereProgram->SetUniform("t", m_t);

	pSphereProgram->SetUniform("material1.Ma", glm::vec3(0.0f, 1.0f, 0.0f));
	pSphereProgram->SetUniform("material1.Md", glm::vec3(0.0f, 1.0f, 0.0f));
	pSphereProgram->SetUniform("material1.Ms", glm::vec3(1.0f, 1.0f, 1.0f));
	pSphereProgram->SetUniform("material1.shininess", 50.0f);
	pSphereProgram->SetUniform("light1.La", glm::vec3(0.15f, 0.15f, 0.15f));
	pSphereProgram->SetUniform("light1.Ld", glm::vec3(1.0f, 1.0f, 1.0f));
	pSphereProgram->SetUniform("light1.Ls", glm::vec3(1.0f, 1.0f, 1.0f));
	pSphereProgram->SetUniform("light1.position", viewMatrix * vLightEye);
	pSphereProgram->SetUniform("t", m_t);




	
	




	pSphereProgram->UseProgram();
	// Render the pickup set
		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(m_objPos);
		modelViewMatrixStack.Scale(2.0f);
		modelViewMatrixStack.Rotate(glm::vec3(0, 1, 0), time_el);
		pSphereProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
		pSphereProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pSphereProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pPickUp->Render();
		modelViewMatrixStack.Pop();
	
		pMainProgram->UseProgram();

	//back
	for (float i = -500; i < 500.f; i = i + 10.f) {
		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(i, 10, 250));
		modelViewMatrixStack.Scale(5.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pWall->Render();
		modelViewMatrixStack.Pop();
	}

	// Render the front wall
	for (float i = -500; i < 500.f; i = i + 10.f) {
		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(i, 10, -1700));
		modelViewMatrixStack.Scale(5.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pWall->Render();
		modelViewMatrixStack.Pop();
	}
	//left wall
	for (float i = -1700.f; i < 250.f; i = i + 10.f) {
		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(-500, 10, i));
		modelViewMatrixStack.Rotate(glm::vec3(0, 1, 0), 90.f);
		modelViewMatrixStack.Scale(5.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pWall->Render();
		modelViewMatrixStack.Pop();
	}
	//right wall
	for (float i = -1700; i < 250.f; i = i + 10.f) {
		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(500, 10, i));
		modelViewMatrixStack.Scale(5.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pWall->Render();
		modelViewMatrixStack.Pop();
	}



	//space ship in centre 
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(glm::vec3(0, 5, -400.f));
	modelViewMatrixStack.Scale(0.15f);
	modelViewMatrixStack.Rotate(glm::vec3(0, 1, 0), 90.f);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
	//pMainProgram->SetUniform("bUseTexture", false);
	m_spacePod->Render();
	modelViewMatrixStack.Pop();



	//directional light
	CShaderProgram* pLightProgram = (*m_pShaderPrograms)[0];
	pLightProgram->UseProgram();


	// Set light and materials
	pLightProgram->SetUniform("light1.position", viewMatrix * lightPosition1);
	pLightProgram->SetUniform("light1.La", glm::vec3(1.0f, 1.0f, 1.0f));
	pLightProgram->SetUniform("light1.Ld", glm::vec3(1.0f, 1.0f, 1.0f));
	pLightProgram->SetUniform("light1.Ls", glm::vec3(1.0f, 1.0f, 1.0f));
	pLightProgram->SetUniform("light1.direction", glm::normalize(normalMatrix * glm::vec3(0, -1, 0)));
	pLightProgram->SetUniform("light1.exponent", 20.0f);
	pLightProgram->SetUniform("light1.cutoff", 30.0f);
	pLightProgram->SetUniform("light2.position", viewMatrix * lightPosition2);
	pLightProgram->SetUniform("light2.La", glm::vec3(0.0f, 0.0f, 1.0f));
	pLightProgram->SetUniform("light2.Ld", glm::vec3(0.0f, 0.0f, 1.0f));
	pLightProgram->SetUniform("light2.Ls", glm::vec3(0.0f, 0.0f, 1.0f));
	pLightProgram->SetUniform("light2.direction", glm::normalize(normalMatrix * glm::vec3(0, -1, 0)));
	pLightProgram->SetUniform("light2.exponent", 20.0f);
	pLightProgram->SetUniform("light2.cutoff", 30.0f);
	pLightProgram->SetUniform("material1.shininess", 15.0f);
	pLightProgram->SetUniform("material1.Ma", glm::vec3(glow, 0.f, 0.2f));
	pLightProgram->SetUniform("material1.Md", glm::vec3(1.0f, 1.0f, 1.0f));
	pLightProgram->SetUniform("material1.Ms", glm::vec3(1.0f, 1.0f, 1.0f));

	
	//teetrahedron glow effect
	if (glow >= 1.0f) {
		glow_fade = true;
	}
	else if (glow < 0.f) {
		glow_fade = false;
	}
	if (glow_fade) {
		glow = glow- 0.01f;
	}
	else {
		glow += 0.01f;
	}

	//toon based pickup
	for (float i = 0; i < wallPoints.size(); i++) {
		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(wallPoints[i]);
		modelViewMatrixStack.Scale(8.0f);
		pLightProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pLightProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pObst->Render();
		modelViewMatrixStack.Pop();
	}
	pLightProgram->SetUniform("material1.shininess", 15.0f);
	pLightProgram->SetUniform("material1.Ma", glm::vec3(0.f, 0.f, 0.f));
	pLightProgram->SetUniform("material1.Md", glm::vec3(1.0f, 1.0f, 1.0f));
	pLightProgram->SetUniform("material1.Ms", glm::vec3(1.0f, 1.0f, 1.0f));

	// Render the player 
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(m_playerPos);
	modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 180.f);
	modelViewMatrixStack.Scale(0.05f);
	pLightProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pLightProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pPlayerMesh->Render();
	modelViewMatrixStack.Pop();


	//track
	modelViewMatrixStack.Push();
	pLightProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pLightProgram->SetUniform("matrices.normalMatrix",
		m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pCatmullRom->RenderTrack();
	modelViewMatrixStack.Pop();

	pMainProgram->UseProgram();

	//cube pickup
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(m_objPos2);
	modelViewMatrixStack.Scale(3+5.0f*glow);
	modelViewMatrixStack.Rotate(glm::vec3(0, 1, 0), time_el * 0.1);
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pWall->Render();
	modelViewMatrixStack.Pop();

	pMainProgram->UseProgram();

	CShaderProgram* fontProgram = (*m_pShaderPrograms)[1];

	RECT dimensions = m_gameWindow.GetDimensions();
	int height = dimensions.bottom - dimensions.top;

	fontProgram->UseProgram();
	glDisable(GL_DEPTH_TEST);
	fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
	fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
	fontProgram->SetUniform("vColour", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	m_pFtFont->Render(620, height - 20, 20, "Time elaspsed:%d", int(time_el *0.001f)); // div by thousand for milliseconds

	if (int(time_el * 0.001f) == 60) {
		freeLook = true;
		gameOver = true;
	}


	//controls fade out at the start of the game
	if (int(time_el * 0.001f) < 5) {
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 0.0f, 0.0f, fadeout));
		m_pFtFont->Render(100, height - 300, 50, "Use A and D to control player");
		m_pFtFont->Render(250, height - 400, 30, "You have 60 seconds!");
	}
	fadeout = fadeout - 0.01f;

	//when free look is enabled
	if (freeLook) {
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
		m_pFtFont->Render(250, height - 20, 20, "Game Paused -Free Look On");
	}

	//top down view is active
	if (mapMode && !freeLook) {
			fontProgram->UseProgram();
			glDisable(GL_DEPTH_TEST);
			fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
			fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
			fontProgram->SetUniform("vColour", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			m_pFtFont->Render(275, height - 20, 20, "Top View: On");
	}
	else if(!freeLook) {
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		m_pFtFont->Render(275, height - 20, 20, "Top View: Off");
	}

	fontProgram->UseProgram();
	glDisable(GL_DEPTH_TEST);
	fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
	fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
	fontProgram->SetUniform("vColour", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	m_pFtFont->Render(20, height - 20, 20, "Score: %d", score);

	if (gameOver && freeLook) {
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 0.0f, 0.0f, 1));
		m_pFtFont->Render(250, height - 300, 50, "Game Over");
	}






	
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	SwapBuffers(m_gameWindow.Hdc());		

}

// Update method runs repeatedly with the Render method
void Game::Update() 
{

	if (freeLook) {
		m_pCamera->Update(m_dt, true);

	}
	
	else {
		m_prevPos = m_pCamera->GetPosition();
		

		time_el += m_dt;

		m_currentDistance += m_dt * 0.1f;
		glm::vec3 p;

		m_pCatmullRom->Sample(m_currentDistance, p);

		glm::vec3 pNext;
		m_pCatmullRom->Sample(m_currentDistance + 1.0f, pNext);

		glm::vec3 T = glm::normalize(pNext - p);
		glm::vec3 N = glm::normalize(glm::cross(T, glm::vec3(0, 1.f, 0)));
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		glm::vec3 tempP = p; //temp variable to not affect camera when moving player

		glm::vec3 look_at = tempP + 10.0f * T;

		p.x += m_cameraMovement *N.x;
		p.z += m_cameraMovement * N.z;

		//updating pickup points set 1 

		if (abs(m_objPos.z) > abs(p.z)) {
			if (abs(m_objPos.x) - abs(p.x) <= 5 && abs(m_objPos.z) - abs(p.z) <= 5) { // checking for collision
				if (currentPickUp < pickupPoints.size() - 1) {
					currentPickUp += 1;
					score++;
				}
				m_objPos = pickupPoints[currentPickUp];

				if (currentPickUp == pickupPoints.size() - 1) {
					currentPickUp = 0;
				}

			}
		}
		//updating set 2
		if (abs(m_objPos2.z) > abs(p.z)) {
			if (abs(m_objPos2.x) - abs(p.x) <= 5 && abs(m_objPos2.z) - abs(p.z) <= 5) { // checking for collision
				if (currentPickUp2 < pickupPoints.size() - 1) {
					currentPickUp2 += 1;
					score++;
				}
				m_objPos2 = pickupPoints2[currentPickUp2];

				if (currentPickUp2 == pickupPoints2.size() - 1) {
					currentPickUp2 = 0;
				}

			}
		}




		// Update the camera using the amount of time that has elapsed to avoid framerate dependent motion
		m_pCamera->Update(m_dt, false);



		if (mapMode) { //setting top down view while still having player controls 
			

				m_playerPos = p;
				m_pCamera->Set(glm::vec3(tempP.x, 300, tempP.z), look_at, glm::vec3(0, 1, 0));

		}
		else {
			
				m_playerPos = p;
				m_pCamera->Set(glm::vec3(tempP.x,20,tempP.z + 100.f), look_at, glm::vec3(0, 1, 0));

				if (pNext.z > tempP.z) {
					m_pCamera->Set(glm::vec3(tempP.x, 20, tempP.z - 100.f), look_at, glm::vec3(0, 1, 0));
				}
		}


	}
	}





void Game::DisplayFrameRate()
{
	static int frameCount = 0;
	static double elapsedTime = 0.0f;

	CShaderProgram *fontProgram = (*m_pShaderPrograms)[1];

	RECT dimensions = m_gameWindow.GetDimensions();
	int height = dimensions.bottom - dimensions.top;

	// Increase the elapsed time and frame counter
	elapsedTime += m_dt;
    frameCount++;
	

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
    if(elapsedTime > 1000 )
    {
		elapsedTime = 0;
		m_iFramesPerSecond = frameCount;

		time_el += 1.f;

		// Reset the frames per second
        frameCount = 0;
		
    }

	if (m_iFramesPerSecond > 0) {
		// Use the font shader program and render the text
		/*fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_pFtFont->Render(20, height - 20, 20, "FPS: %d", m_iFramesPerSecond);*/
		
	}

	


}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	/*
	// Fixed timer
	dDt = pHighResolutionTimer->Elapsed();
	if (dDt > 1000.0 / (double) Game::FPS) {
		pHighResolutionTimer->Start();
		Update();
		Render();
	}
	*/
	
	
	// Variable timer
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();
	

}


WPARAM Game::Execute() 
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hHinstance);

	if(!m_gameWindow.Hdc()) {
		return 1;
	}

	Initialise();

	m_pHighResolutionTimer->Start();

	
	MSG msg;

	while(1) {													
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			if(msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		} else if (m_bAppActive) {
			GameLoop();
		} 
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	m_gameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param) 
{
	LRESULT result = 0;

	switch (message) {


	case WM_ACTIVATE:
	{
		switch(LOWORD(w_param))
		{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				m_bAppActive = true;
				m_pHighResolutionTimer->Start();
				break;
			case WA_INACTIVE:
				m_bAppActive = false;
				break;
		}
		break;
		}

	case WM_SIZE:
			RECT dimensions;
			GetClientRect(window, &dimensions);
			m_gameWindow.SetDimensions(dimensions);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;

	case WM_KEYDOWN:
		switch(w_param) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case '1':
			
				m_pCamera->Set(glm::vec3(m_prevPos.x, 50.f, m_prevPos.z), m_playerPos, glm::vec3(0, 1, 0));
				freeLook = true;;
				break;
		case '2':
				m_pCamera->SetPosition(m_prevPos);
				freeLook = false;
				break;
		case 'M':
			mapMode = true;
			break;
		case 'V':
			mapMode = false;
			break;
		case 'D':
			m_cameraMovement += m_dt * 0.1f;
		
			break;
		case 'A':
			m_cameraMovement -= m_dt * 0.1f;
			
			break;
		}

		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(window, message, w_param, l_param);
		break;
	}

	return result;
}

Game& Game::GetInstance() 
{
	static Game instance;

	return instance;
}

void Game::SetHinstance(HINSTANCE hinstance) 
{
	m_hHinstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int) 
{
	Game &game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return game.Execute();
}




