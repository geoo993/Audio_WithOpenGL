#pragma once

// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"
#include "GameManager.h"
#include "Extensions.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Shaders.h"
#include "Plane.h"
#include "FreeTypeFont.h"
#include "Sphere.h"
#include "OpenAssetImportMesh.h"
#include "CatmullRom.h"
#include "OscillatorDSP.h"
#include "OcclusionDSP.h"
#include "FilterDSP.h"
#include "FIRConvolutionDSP.h"

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
class COscillator;
class CFilterDSP;
class CFIRConvolutionDSP;
class COcclusion;
class CCatmullRom;

class Game {
private:
	// Three main methods used in the game.  Initialise runs once, while Update and Render run repeatedly in the game loop.
	void Initialise();
    void Update();
    void Render();
    void Audio();
    void GameLoop();
    void LoadFromResources(const std::string &path);
    void CreateShaderPrograms(const std::string &path);
    void KeyBoardControls(int &keyPressed, int &keyReleased, int &keyAction);
    void MouseControls(const int &button, const int &action);
    void DisplayFrameRate();

	// Pointers to game objects.  They will get allocated in Game::Initialise()
    CGameWindow m_gameWindow;
    CGameManager m_gameManager;

	CSkybox *m_pSkybox;
	CCamera *m_pCamera;
	vector <CShaderProgram *> *m_pShaderPrograms;
	CPlane *m_pPlanarTerrain;
    CHighResolutionTimer *m_pGameTimer;
    CFreeTypeFont *m_pFtFont;

    // game timers
    GLfloat m_timeSeconds, m_timeMilliSeconds;
    GLdouble m_deltaTime, m_elapsedTime;
    GLint m_framesPerSecond, m_frameCount;

    // inputs
    GLboolean m_mouseButtonDown;
    GLboolean m_enableMouseMovement;
    GLboolean m_isMouseCursorVisible;
    GLboolean m_mouseMouseMoveClickSwitch;
    double m_mouseX;
    double m_mouseY;
    double m_keyPressTime;
    double m_lastKeyPressTime;
    int m_lastKeyPress;
    GLboolean m_isKeyPressRestriction;

    // car
    COpenAssetImportMesh *m_pRacingCar;
    glm::vec3 m_racingCarPosition;
    void RenderRacingCar(CShaderProgram * shaderProgram);

    // helicopter
    COpenAssetImportMesh *m_pHelicopter;
    glm::vec3 m_helicoptePosition;
    glm::mat4 m_helicopteOrientation;
    glm::vec3 m_helicopteVelocity;
    GLfloat m_helicopteRotorRotation;
    GLint m_helicopteRotor;
    GLfloat m_helicopterTimeScalar;
    void RenderHelicopter(CShaderProgram * shaderProgram);

    // helicoper path
    CCatmullRom *m_pPath;

    // audio DSP
    void LoadDSPFromResources(const std::string &path);
    vector <string> m_audioFiles;
    CFIRConvolutionDSP *m_pFIR;
    COscillator *m_pOscillator;
    CFilterDSP *m_pFilter;
    COcclusion *m_pOcclusion;


public:

    Game();
    Game(const Game &other);
    Game &operator=(const Game &other);
    ~Game();

    void Execute(const std::string &filepath);

    void SetUniformBufferObject();
};
