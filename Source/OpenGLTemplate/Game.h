#pragma once

#include "Common.h"
#include "GameWindow.h"

// Classes used in game.  For a new class, declare it here and provide a pointer to an object of this class below.  Then, in Game.cpp, 
// include the header.  In the Game constructor, set the pointer to NULL and in Game::Initialise, create a new object.  Don't forget to 
// delete the object in the destructor.   
class CCamera;
class CSkybox;
class CShader;
class CShaderProgram;
class CPlane;
class CFreeTypeFont;
class CHighResolutionTimer;
class CSphere;
class COpenAssetImportMesh;
class CCube;
class CCatmullRom;
class CTetrahedron;
class CHeightMapTerrain;

class Game {
private:
	// Three main methods used in the game.  Initialise runs once, while Update and Render run repeatedly in the game loop.
	void Initialise();
	void Update();
	void Render();
	
	

	// Pointers to game objects.  They will get allocated in Game::Initialise()
	CSkybox *m_pSkybox;
	CCamera *m_pCamera;
	vector <CShaderProgram *> *m_pShaderPrograms;
	CPlane *m_pPlanarTerrain;
	CFreeTypeFont *m_pFtFont;
	CFreeTypeFont* m_timeEl;
	
	COpenAssetImportMesh *m_pPlayerMesh;
	COpenAssetImportMesh* m_pPickUp;
	float pickUpAng;
	COpenAssetImportMesh* m_spacePod;

	CSphere *m_pSphere;
	CHighResolutionTimer *m_pHighResolutionTimer;
	CCube *m_pCube;
	CHeightMapTerrain* m_pHeightmapTerrain;
	

	CCube* m_pWall;
	vector<glm::vec3> wallPoints;
	float glow;
	bool glow_fade;

	CCatmullRom *m_pCatmullRom;
	bool gameOver;
	float phase;

	// Some other member variables
	double m_dt;
	int m_iFramesPerSecond;
	bool m_bAppActive;
	double time_el;
	

	float m_t;
	glm::vec3 m_spaceShipPosition;
	glm::mat4 m_spaceShipOrientation;

	bool freeLook;
	glm::vec3 m_playerPos;
	glm::vec3 m_prevPos;
	float playerAngle;

	bool mapMode;
	float fadeout;

	float m_currentDistance;
	float m_cameraMovement;


	glm::vec3 m_objPos;
	glm::vec3 m_objPos2;
	int score;
	
	vector<glm::vec3> pickupPoints;
	vector<glm::vec3> pickupPoints2;
	int currentPickUp;
	int currentPickUp2;
	glm::vec3 finalPickUp;
	glm::vec3 finalPickUp2;

	vector<glm::vec3> obstaclePoints;
	CTetrahedron* m_pObst; //obstacles that reduce player score

	//variables for player movement from camera class
	float p_speed;
	glm::vec3 m_StrafeVector;

	//
	float m_boundary;

public:
	Game();
	~Game();
	static Game& GetInstance();
	LRESULT ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param);
	void SetHinstance(HINSTANCE hinstance);
	WPARAM Execute();
	bool collision(glm::vec3 vec1, glm::vec3 vec2);
	

private:
	static const int FPS = 60;
	void DisplayFrameRate();
	void GameLoop();
	GameWindow m_gameWindow;
	HINSTANCE m_hHinstance;


};
