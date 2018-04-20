/* 
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting, 
 different camera controls, different shaders, etc.
 
 Template version 5.0a 29/01/2017
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk) 
*/


#include "Game.h"

//keyboard and mouse controlls
static int keyPressedCode = -1;
static int keyReleasedCode = -1;
static int keyPressedAction = -1;
static int mouseButton = -1;
static int mouseAction = -1;

// Constructor
Game::Game()
{
    m_pGameTimer = nullptr;
    m_pShaderPrograms = nullptr;

	m_pSkybox = nullptr;
	m_pCamera = nullptr;
	m_pPlanarTerrain = nullptr;
	m_pFtFont = nullptr;

    // game timers
    m_framesPerSecond = 0;
    m_frameCount = 0;
    m_elapsedTime = 0.0f;
    m_deltaTime = 0.5f;
    m_time = 0.0f;

    // inputs
    m_mouseButtonDown = false;
    m_enableMouseMovement = true;
    m_isMouseCursorVisible = true;
    m_mouseMouseMoveClickSwitch = false;
    m_mouseX = 0.0;
    m_mouseY = 0.0;
    m_keyPressTime = 0.0;
    m_lastKeyPressTime = 0.0f;
    m_lastKeyPress = -1;
    m_isKeyPressRestriction = true;

    // helicopter
    m_pHelicopter = nullptr;
    m_helicopteRotor = 0;
    m_helicopteRotorRotation = 0.0f;

    // audio DSP
    m_pFIR = nullptr;
    m_pOscillator = nullptr;
    m_pFilter = nullptr;
    m_audioFiles.reserve(6);
}

// Destructor
Game::~Game() 
{ 
	//game objects
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
    delete m_pHelicopter;
	delete m_pFIR;
    delete m_pOscillator;
    delete m_pFilter;

	if (m_pShaderPrograms != nullptr) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pGameTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise() 
{
	/// Create objects
    m_pGameTimer = new CHighResolutionTimer;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector <CShaderProgram *>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pHelicopter = new COpenAssetImportMesh;
	m_pFIR = new CFIRConvolutionDSP;
    m_pOscillator = new COscillator;
    m_pFilter = new CFilterDSP;

    // Set the orthographic and perspective projection matrices based on the image size
    m_pCamera = new CCamera(glm::vec3(0.0f, 60.0f, 0.0f),     // position
                            glm::vec3(0.0f, 1.0f, 0.0f),      // worldUp
                            PITCH,                            // pitch
                            YAW,                              // yaw
                            FOV,                              // fieldOfView
                            SCREEN_WIDTH,                     // width
                            SCREEN_HEIGHT,                    // height
                            ZNEAR,                            // zNear
                            ZFAR,                             // zFar
                            SPEED,                            // speed
                            SPEEDRATIO,                       // speed ratio
                            SENSITIVTY                        // sensitivity
                            );

}

void Game::CreateShaderPrograms(const std::string &path) {

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert"); // 0
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert"); // 1
	sShaderFileNames.push_back("textShader.frag");
    sShaderFileNames.push_back("fontShader.vert"); // 2
    sShaderFileNames.push_back("fontShader.frag");
    sShaderFileNames.push_back("SkyBoxShader.vert"); // 3
    sShaderFileNames.push_back("SkyBoxShader.frag");

	for (int i = 0; i < (int) sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int) sShaderFileNames[i].size()-4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
        shader.LoadShader(path+"/shaders/"+sShaderFileNames[i], iShaderType);
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

    CShaderProgram *pFontProgram2 = new CShaderProgram;
    pFontProgram2->CreateProgram();
    pFontProgram2->AddShaderToProgram(&shShaders[4]);
    pFontProgram2->AddShaderToProgram(&shShaders[5]);
    pFontProgram2->LinkProgram();
    m_pShaderPrograms->push_back(pFontProgram2);

    // Create a shader program for skybox
    CShaderProgram *pSkyboxProgram = new CShaderProgram;
    pSkyboxProgram->CreateProgram();
    pSkyboxProgram->AddShaderToProgram(&shShaders[6]);
    pSkyboxProgram->AddShaderToProgram(&shShaders[7]);
    pSkyboxProgram->LinkProgram();
    m_pShaderPrograms->push_back(pSkyboxProgram);

}


void Game::LoadFromResources(const std::string &path)
{
	// You can follow this pattern to load additional shaders

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
    m_pSkybox->Create(3000.0f, path, 0);
	
	// Create the planar terrain
	m_pPlanarTerrain->Create(path+"/textures/", "grassfloor01.jpg", 2000.0f, 2000.0f, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

    //m_pFtFont->LoadSystemFont(path+"/fonts/Candy Script.ttf", 48);
    m_pFtFont->LoadFont(path+"/fonts/Candy Script.ttf", 48);

    m_pHelicopter->Load(path+"/models/Mi-28N_Havoc_BF3/havoc.obj");

}

void Game::LoadDSPFromResources(const std::string &path) {

    m_audioFiles.push_back("cw_amen12_137.wav");
    m_audioFiles.push_back("Boing.wav"); // Royalty free sound from //freesound.org // Custom DSP
    m_audioFiles.push_back("DST-Garote.mp3"); // Royalty free music from http://www.nosoapradio.us/
    m_audioFiles.push_back("Horse.wav"); // 3D Audio
    m_audioFiles.push_back("drone.wav");
    m_audioFiles.push_back("Helicopter.wav");

    //// Initialise audio and play background music
    //m_pFIR->InitialiseWithDSPEffect();
    //m_pFIR->LoadEventSound((path+"/audio/"+m_audioFiles[1]).c_str());
    //m_pFIR->LoadMusicStream((path+"/audio/"+m_audioFiles[0]).c_str());
    //m_pFIR->PlayMusicStreamUsingDSP();

    //m_pOscillator->Initialise();
    //m_pOscillator->LoadMusicStream((path+"/audio/"+m_audioFiles[5]).c_str());
    //m_pOscillator->PlayMusicStream();

    m_pFilter->Initialise();
    m_pFilter->LoadEventSound((path+"/audio/"+m_audioFiles[3]).c_str());
    m_pFilter->LoadMusicStream((path+"/audio/"+m_audioFiles[5]).c_str());
    m_pFilter->PlayMusicStream();

}

void Game::DisplayFrameRate() {

    CShaderProgram *pFontProgram = (*m_pShaderPrograms)[1];

    int width = m_gameWindow.Width();
    int height = m_gameWindow.Height();

    // Increase the elapsed time and frame counter
    m_elapsedTime += m_deltaTime;
    ++m_frameCount;

    m_time += (float) (0.01f * m_deltaTime);

    // Now we want to subtract the current time by the last time that was stored
    // to see if the time elapsed has been over a second, which means we found our FPS.
    if (m_elapsedTime > 1000)
    {
        m_elapsedTime = 0;
        m_framesPerSecond = m_frameCount;

        // Reset the frames per second
        m_frameCount = 0;
    }

    /*
     std::cout << std::endl;
    std::cout << "width: " <<width << std::endl;
    std::cout << "height: " <<height << std::endl;
     std::cout << "deltatime: " <<m_deltaTime << std::endl;
     std::cout << "elapsedTime: " << m_elapsedTime<< std::endl;
     std::cout << "time: " << m_time << std::endl;
     std::cout << "glfw getTime: " << glfwGetTime() << std::endl;

     float time = (float)m_elapsedTime / 1000.0f * 2.0f * 3.14159f * 0.75f;
     std::cout << "timeFromElapsed: " << time << std::endl;
     */

    if (m_framesPerSecond > 0) {
        // Use the font shader program and render the text
        glDisable(GL_DEPTH_TEST);
        pFontProgram->UseProgram();
        pFontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
        pFontProgram->SetUniform("textColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_pFtFont->Render(pFontProgram, 20, height - 40, 20, "FPS: %d", m_framesPerSecond);
        //m_pFtFont->Render(pFontProgram, "FPS: " + std::to_string(m_framesPerSecond), 20, height - 40, 0.5f);


        glEnable(GL_DEPTH_TEST);
    }
}


// Render method runs repeatedly in a loop
void Game::Render() 
{

    // draw skybox as last
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    CShaderProgram *pSkyBoxProgram = (*m_pShaderPrograms)[3];
    pSkyBoxProgram->UseProgram();
    pSkyBoxProgram->SetUniform("cubeMapTex", CUBEMAPTEXTUREUNIT);
    pSkyBoxProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
    pSkyBoxProgram->SetUniform("matrices.viewMatrix", m_pCamera->GetViewMatrix());

    glm::vec3 vEye = m_pCamera->GetPosition();

    m_pSkybox->transform.SetIdentity();
    m_pSkybox->transform.Translate(vEye);

    glm::mat4 skyBoxModel = m_pSkybox->transform.GetModel();
    pSkyBoxProgram->SetUniform("matrices.modelMatrix", skyBoxModel);
    pSkyBoxProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix
                               (skyBoxModel));
    m_pSkybox->Render(CUBEMAPTEXTUREUNIT);
    glDepthFunc(GL_LESS);


	// Use the main shader program
	CShaderProgram *pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);

	// Set the projection matrix
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
    pMainProgram->SetUniform("matrices.viewMatrix", m_pCamera->GetViewMatrix());

	
	// Set light and materials in main shader program
	glm::vec4 lightPosition1 = glm::vec4(-100, 100, -100, 1); // Position of light source *in world coordinates*
	pMainProgram->SetUniform("light1.position", m_pCamera->GetViewMatrix() * lightPosition1); // Position of light source *in eye coordinates*
	pMainProgram->SetUniform("light1.La", glm::vec3(1.0f));		// Ambient colour of light
	pMainProgram->SetUniform("light1.Ld", glm::vec3(1.0f));		// Diffuse colour of light
	pMainProgram->SetUniform("light1.Ls", glm::vec3(1.0f));		// Specular colour of light
	pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.0f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.0f));	// Specular material reflectance
	pMainProgram->SetUniform("material1.shininess", 15.0f);		// Shininess material property

	// Render the planar terrain
    m_pPlanarTerrain->transform.SetIdentity();
    m_pPlanarTerrain->transform.Translate(glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 terrainModel = m_pPlanarTerrain->transform.GetModel();
    pMainProgram->SetUniform("matrices.modelMatrix", terrainModel);
    pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix
                             (terrainModel));
    m_pPlanarTerrain->Render();


	// Turn on diffuse + specular materials
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance	

    m_helicopteRotorRotation += 30.0f;
    if (m_helicopteRotorRotation > 360 ) {
        m_helicopteRotorRotation = 0.0f;
    }
    // Render the helicopter
    m_pHelicopter->Render(pMainProgram, m_pCamera, m_helicopteRotorRotation, m_helicopteRotor);

    m_gameWindow.SetViewport();

	// Draw the 2D graphics after the 3D graphics
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	m_gameWindow.SwapBuffers();

}

// Update method runs repeatedly with the Render method
void Game::Update() 
{

    // Update the camera using the amount of time that has elapsed to avoid framerate dependent motion
    m_pCamera->Update(m_gameWindow.Window(), m_deltaTime, keyPressedCode, true, m_enableMouseMovement);
    MouseControls(mouseButton, mouseAction);
    KeyBoardControls(keyPressedCode, keyReleasedCode, keyPressedAction);

	//m_pFIR->Update(m_pCamera);
    //m_pOscillator->Update();
    m_pFilter->Update();
}


// The game loop runs repeatedly until game over
void Game::GameLoop()
{

     /*
     // Fixed timer (updating based on system frame rate )
     m_deltaTime = m_pGameTimer->Elapsed();
     if (m_deltaTime > 1.0 / (double) Game::FPS) {
     m_pGameTimer->Start();
     Update();
     Render();
     }
     */

	
	// Variable timer
	m_pGameTimer->Start();
	Update();
	Render();
	m_deltaTime = m_pGameTimer->Elapsed();

}

static void OnMouseDown_callback(GLFWwindow* window, int button, int action, int mods){

    //std::cout << "Mouse Down with button: " << button << " and with action: " << action << std::endl;
    mouseButton = button;
    mouseAction = action;

}


static void OnKeyDown_callback( GLFWwindow* window, int key, int scancode, int action, int mods)
{
    keyPressedAction = action;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    switch (action) {
        case GLFW_PRESS:
            keyPressedCode = key;
            break;
        case GLFW_RELEASE:
            keyReleasedCode = key;
            break;
        default:
            break;
    }
}


void Game::MouseControls(const int &button, const int &action){

    // https://stackoverflow.com/questions/37194845/using-glfw-to-capture-mouse-dragging-c
    // https://stackoverflow.com/questions/45130391/opengl-get-cursor-coordinate-on-mouse-click-in-c

    // Mouse Move
    if (m_mouseMouseMoveClickSwitch == true) {
        m_enableMouseMovement = false;
        //std::cout << "Mouse x: " << m_mouseX << " and mouse y: " << m_mouseY << std::endl;
    }
    // Mouse click
    if ( m_mouseMouseMoveClickSwitch == false){
        m_enableMouseMovement = true;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {

        if(GLFW_PRESS == action){
            m_mouseButtonDown = true;

        }else if(GLFW_RELEASE == action){
            // Mouse click
            if ( m_mouseMouseMoveClickSwitch == false && m_mouseButtonDown == true){
                m_isKeyPressRestriction = !m_isKeyPressRestriction;
                //glfwGetCursorPos(gameWindow.GetWindow(), &m_mouseX, &m_mouseY);
                //std::cout << "Mouse x: " << m_mouseX << " and mouse y: " << m_mouseY << std::endl;
            }

            m_mouseButtonDown = false;
        }

    }

    if(m_mouseButtonDown) {
        // do your drag here
    }

    glfwGetCursorPos(m_gameWindow.Window(), &m_mouseX, &m_mouseY);
    m_gameWindow.SetCursorVisible(m_isMouseCursorVisible);

}

void Game::KeyBoardControls(int &keyPressed, int &keyReleased, int &keyAction){

    if (keyAction == GLFW_RELEASE){
        keyPressed = -1;
    }

    if (keyPressed == -1){
        m_keyPressTime = 0.0;
        m_lastKeyPressTime = -1;
        keyReleased = -1;
    }

    if (keyPressed != -1){

        m_keyPressTime += 0.06;

        //m_lastKeyPress == keyPressedCode ||
        if ((int)m_lastKeyPressTime  == (int)m_keyPressTime && m_isKeyPressRestriction == true)
        {
            return;
        }

        switch (keyPressed)
        {

            case GLFW_KEY_SPACE:
                break;
            case GLFW_KEY_F1:
                m_pFilter->PlayEventSound();
                break;
            case GLFW_KEY_1 :
                m_pFilter->PlayEventSound();
                break;
            case GLFW_KEY_2:
                m_pFilter->ToggleMusicFilter();
                break;
            case GLFW_KEY_3:
                break;
            case GLFW_KEY_4:
                break;
            case GLFW_KEY_5:
                break;
            case GLFW_KEY_6:
                break;
            case GLFW_KEY_7:
                break;
            case GLFW_KEY_8:
                break;
            case GLFW_KEY_9:
                break;
            case GLFW_KEY_0:
                break;
            case GLFW_KEY_COMMA:
                break;
            case GLFW_KEY_PERIOD:
                break;
            case GLFW_KEY_MINUS:
                break;
            case GLFW_KEY_EQUAL:
                break;
            case GLFW_KEY_GRAVE_ACCENT:
                break;
            case GLFW_KEY_Z:
                break;
            case GLFW_KEY_X:
                break;
            case GLFW_KEY_C:
                break;
            case GLFW_KEY_V:
                break;
            case GLFW_KEY_B:
                break;
            case GLFW_KEY_N:
                break;
            case GLFW_KEY_M:
                break;
            case GLFW_KEY_R:
                break;
            case GLFW_KEY_T:
                break;
            case GLFW_KEY_Y:
                break;
            case GLFW_KEY_U:
                break;
            case GLFW_KEY_O:
            m_pFilter->DecreaseMusicVolume();
                break;
            case GLFW_KEY_P:
            m_pFilter->IncreaseMusicVolume();
                break;
            case GLFW_KEY_I:
                break;
            case GLFW_KEY_K:
                break;
            case GLFW_KEY_J:
                break;
            case GLFW_KEY_L:
                break;
            case GLFW_KEY_G:
                break;
            case GLFW_KEY_H:
                break;
            case GLFW_KEY_Q:
                break;
            case GLFW_KEY_BACKSLASH:
                break;
            case GLFW_KEY_SLASH:
                break;
            case GLFW_KEY_APOSTROPHE:
                break;
            case GLFW_KEY_UP:
                break;
            case GLFW_KEY_DOWN:
                break;
            case GLFW_KEY_LEFT:
                break;
            case GLFW_KEY_RIGHT:
                break;
            default:
                break;
        }

        //std::cout << " keypresstime: " << (int)m_keyPressTime << ", lastkeypresstime: " << (int)m_lastKeyPressTime << std::endl;
        //std::cout << " keypress: " << keyPressedCode << ", lastkeypress: " << m_lastKeyPress << std::endl;

        m_lastKeyPressTime = m_keyPressTime;
        m_lastKeyPress = keyPressed;
    }
}

void Game::Execute(const std::string &filepath)
{

    m_gameWindow.Init("OpenGL Window", SCREEN_WIDTH, SCREEN_HEIGHT, false);
    m_gameManager.SetResourcePath(filepath);

    Initialise();
    CreateShaderPrograms(filepath);
    LoadFromResources(filepath);
    LoadDSPFromResources(filepath);

    m_gameManager.SetLoaded(true); // everything has loaded

    m_gameWindow.SetInputs(OnKeyDown_callback, OnMouseDown_callback);

    m_gameWindow.PreRendering();

    m_gameManager.SetActive(true); // game is now going to be active, or activate application

    while ( !m_gameWindow.ShouldClose() ){

        m_gameWindow.ClearBuffers();

        if (m_gameManager.IsActive()) {
            GameLoop();
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Do not consume processor power if application isn't active
        }

        m_gameWindow.PostRendering();
    }

    m_gameWindow.DestroyWindow();

    return;
}
