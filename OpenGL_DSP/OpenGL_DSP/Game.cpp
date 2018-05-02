
#include "Game.h"

//keyboard and mouse controlls
static int keyPressedCode = -1;
static int keyReleasedCode = -1;
static int keyPressedAction = -1;
static int mouseButton = -1;
static int mouseAction = -1;

// Constructor, to set values of properties
Game::Game()
{
    m_pGameTimer = nullptr;
    m_pShaderPrograms = nullptr;

	m_pSkybox = nullptr;
	m_pCamera = nullptr;
	m_pFtFont = nullptr;

    // game timers
    m_framesPerSecond = 0;
    m_frameCount = 0;
    m_elapsedTime = 0.0f;
    m_deltaTime = 0.5f;
    m_timeInSeconds = 0.0f;
    m_timeInMilliSeconds = 0.0f;
    m_timePerSecond = 0.0f;

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

    // terrain
    m_pPlanarTerrain = nullptr;
    m_terrainSize = 4000.0f;
    m_terrainPosition = glm::vec3(0.0f, 0.0f, 0.0f);

    // Pent House
    m_pPenthouse = nullptr;
    m_penthousePosition = glm::vec3(0.0f, 1.0f, 0.0f);

    // Racing Car
    m_pRacingCar = nullptr;
    m_racingCarPosition = glm::vec3(-80.0f, 1.0f, -50.0f);

    // helicopter
    m_pHelicopter = nullptr;
    m_helicopterPosition = glm::vec3(0.0f, 1.0f, 0.0f);
    m_helicopterOrientation = glm::mat4(1);
    m_helicopterVelocity = glm::vec3(0.01f, 0.01f, 0.01f);
    m_helicopterSpeed = 0.0f;
    m_helicopterRotor = 6;
    m_helicopterRotorRotation = 0.0f;
    m_helicopterSpeedScalar = 1.0f;

    // helicopter path
    m_pPath = nullptr;

    // audio DSP
    m_pDSP = nullptr;
}

// Destructor
Game::~Game() 
{ 
	//game objects
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
    delete m_pPenthouse;
    delete m_pRacingCar;
    delete m_pHelicopter;
	delete m_pDSP;
    delete m_pPath;

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
    m_pRacingCar = new COpenAssetImportMesh;
    m_pPenthouse = new COpenAssetImportMesh;
    m_pPath = new CCatmullRom;
    m_pDSP = new DSPAudio;

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

// Load and Create every shader program file from resources
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

// Load every object from resources
void Game::LoadFromResources(const std::string &path)
{
	// You can follow this pattern to load additional shaders

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
    m_pSkybox->Create(3000.0f, path, 0);
	
	// Create the planar terrain
	m_pPlanarTerrain->Create(path+"/textures/", "grassfloor01.jpg", m_terrainSize, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

    m_pFtFont->LoadFont(path+"/fonts/Arial.ttf", 48);

    m_pPenthouse->Load(path+"/models/Penthouse/fantasthouse 9.obj"); // https://www.turbosquid.com/FullPreview/Index.cfm/ID/1043651
    m_pRacingCar->Load(path+"/models/Racing_Car/free_car_1.obj"); // https://www.turbosquid.com/3d-models/free-car-3d-model/930099
    m_pHelicopter->Load(path+"/models/Mi-28N_Havoc_BF3/havoc.obj"); // https://free3d.com/3d-model/mi-28-havoc-51447.html

    //create catmullrom
    m_pPath->CreateCentreline(200.0f, true);
    m_pPath->CreateSpline(path+"/textures/", "tiles41a");

}

// Load and Initialise audio from resources and set he DSP properties
void Game::InitialiseAudioDSP(const std::string &path) {
    // "drag_car.wav"  from https://freesound.org/people/lonemonk/sounds/156634/
    // "Helicopter.wav" from https://freesound.org/people/Akc1231/sounds/340802/

    // doppler, occlusion and FIR filter demo
    glm::vec3 penthouse = glm::vec3(170.0f, 109, 206 ); // pent house properties technology
    GLfloat doppler = 1.0f;
    GLfloat distFactor = 40.0f;
    GLfloat rollOff = 1.0f;

    m_pDSP->Initialise(doppler, distFactor, rollOff);
    m_pDSP->LoadEventSound((path+"/audio/Helicopter.wav").c_str(), (path+"/audio/drag_car.wav").c_str());
    m_pDSP->PlayEventSound(m_helicopterPosition, m_terrainPosition, m_helicopterVelocity);
    m_pDSP->LoadMusicStream((path+"/audio/03 Brooks.mp3").c_str());
    m_pDSP->PlayMusicStream();
    m_pDSP->CreateTerrain(m_terrainPosition, m_terrainSize);
    m_pDSP->AddCube(m_terrainPosition, penthouse.x, penthouse.y, penthouse.z);

}

// Update dsp audio and passing through the position, speed and velocity og the car and helicopter
void Game::UpdateAudioDSP() {
    m_pDSP->Update(m_pCamera, m_helicopterPosition, m_racingCarPosition, m_helicopterVelocity, m_helicopterSpeed);
}

// shows the Heads Up Display with the frame rate and dsp properties
void Game::DisplayFrameRate() {

    CShaderProgram *pFontProgram = (*m_pShaderPrograms)[1];

    int width = m_gameWindow.Width();
    int height = m_gameWindow.Height();

    // Increase the elapsed time and frame counter
    m_elapsedTime += m_deltaTime;
    ++m_frameCount;

    m_timePerSecond = (float)(m_deltaTime / 1000.0f);
    m_timeInSeconds += m_timePerSecond;
    m_timeInMilliSeconds += (float) (m_deltaTime);

    // Now we want to subtract the current time by the last time that was stored
    // to see if the time elapsed has been over a second, which means we found our FPS.
    if (m_elapsedTime > 1000)
    {
        m_elapsedTime = 0;
        m_framesPerSecond = m_frameCount;

        // Reset the frames per second
        m_frameCount = 0;
    }


    if (m_framesPerSecond > 0) {
        // Use the font shader program and render the text on screen
        glDisable(GL_DEPTH_TEST);
        pFontProgram->UseProgram();
        pFontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
        pFontProgram->SetUniform("textColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_pFtFont->Render(pFontProgram, 20, height - 40, 20, "FPS: %d", m_framesPerSecond);
        m_pFtFont->Render(pFontProgram, 20, height - 60, 20, "Press - or +        Volume: %f" , m_pDSP->Volume());
        m_pFtFont->Render(pFontProgram, 20, height - 80, 20, "Press 1 or 2       Helicopter Speed: %f" , m_helicopterSpeed);
        m_pFtFont->Render(pFontProgram, 20, height - 100, 20, "Press 3               Reset Helicopter Frequency: %f" , m_pDSP->ChannelFrequency());
        m_pFtFont->Render(pFontProgram, 20, height - 120, 20, "Press 4               Pause Event Channels: %s" , BoolToString(m_pDSP->PauseChannels()));
        m_pFtFont->Render(pFontProgram, 20, height - 140, 20, "Press 5               Switch Music Filter Frequency: %s" , BoolToString(m_pDSP->MusicFilterFrequency()));
        m_pFtFont->Render(pFontProgram, 20, height - 160, 20, "Press 6               Music Filter Active: %s" , BoolToString(m_pDSP->MusicFilterActive()));
        m_pFtFont->Render(pFontProgram, 20, height - 180, 20, "Press 7               Switch FIR Filter: %s" , m_pDSP->FIRFilter());
        m_pFtFont->Render(pFontProgram, 20, height - 200, 20, "Press 8               Bypass FIR Filtering: %s" , BoolToString(m_pDSP->ByPassFIRFilters()));
        m_pFtFont->Render(pFontProgram, 20, height - 220, 20, "Press 9 or 0       FIR Filter Coefficients Mulitplier : %f" , m_pDSP->FIRFilterMultiplier());

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
    m_pPlanarTerrain->transform.Translate(m_terrainPosition);
    glm::mat4 terrainModel = m_pPlanarTerrain->transform.GetModel();
    pMainProgram->SetUniform("matrices.modelMatrix", terrainModel);
    pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix
                             (terrainModel));
    m_pPlanarTerrain->Render();

	// Turn on diffuse + specular materials
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance

    RenderPenthouse(pMainProgram);
    RenderRacingCar(pMainProgram);
    RenderHelicopter(pMainProgram);

    m_gameWindow.SetViewport();

	// Draw the 2D graphics after the 3D graphics
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	m_gameWindow.SwapBuffers();

}

// render the pent house at the center of the scene
void Game::RenderPenthouse(CShaderProgram * shaderProgram){
    m_pPenthouse->transform.SetIdentity();
    m_pPenthouse->transform.Translate(m_penthousePosition);
    glm::mat4 penthouseModel = m_pPenthouse->transform.GetModel();
    shaderProgram->SetUniform("matrices.modelMatrix", penthouseModel);
    shaderProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix
                              (penthouseModel));
    m_pPenthouse->Render();

}

// render the racing car
void Game::RenderRacingCar(CShaderProgram * shaderProgram){

    m_pRacingCar->transform.SetIdentity();
    m_pRacingCar->transform.Translate(m_racingCarPosition);
    m_pRacingCar->transform.Scale(10.0f);
    glm::mat4 racingModel = m_pRacingCar->transform.GetModel();
    shaderProgram->SetUniform("matrices.modelMatrix", racingModel);
    shaderProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix
                             (racingModel));
    m_pRacingCar->Render();
}

// render the helicopter and calculating its speed, velocity, and position on the catmullrom spline
void Game::RenderHelicopter(CShaderProgram * shaderProgram) {

    /*
     in this section we calculate the position of the helicopter on the spline.
     the spline is the pathway or route of the helicoper.
     then when we have the position of the helicopter on the spline we can then use then calculate the speed,
     and velocity of the helicopter using the game timer counter.

     */

    // update the helicopter previous position
    m_helicopterPreviousPosition = m_helicopterPosition;

    // rotate the helicopter in a 360 degree motion
    m_helicopterRotorRotation += 30.0f;
    if (m_helicopterRotorRotation > 360 ) {
        m_helicopterRotorRotation = 0.0f;
    }

    // increase or slow down the helicopter using a scalar value
    GLfloat time = m_timeInSeconds * m_helicopterSpeedScalar;

    // calculate the next position of the helicopter
    glm::vec3 currentPosition = m_pPath->GetCentralLinePositionAtIndex(time); // the current position
    glm::vec3 nextPosition = m_pPath->GetCentralLinePositionAtIndex(time + 1); // the next position on the spline
    glm::vec3 interpolatedPosition = Extensions::interpolate(currentPosition, nextPosition, time - glm::floor(time));

    // calculate the speed and velocity of the helicopter
    glm::vec3 displacement = interpolatedPosition - m_helicopterPreviousPosition; // how much the helicopter has moved
    glm::vec3 front = glm::normalize(displacement); // this is the front vector of the helicopter in the scene
    glm::vec3 right = glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)); // the right vector of the helicopter in scene
    glm::vec3 up = glm::cross(right, front); // the up unit vector of the helicopter
    GLfloat distance = glm::distance(m_helicopterPreviousPosition, interpolatedPosition); // calculate the distance moved

    m_helicopterSpeed = distance / m_timePerSecond; // the speed is measured in metres per second (m/s) and not meters per frame.
    m_helicopterVelocity = displacement * (m_helicopterSpeed / distance); // calculate the velocity, which is the same as (displacement / time)

    // set the position and orientation of the helicopter
    m_helicopterPosition = interpolatedPosition;
    m_helicopterOrientation = glm::mat4(glm::mat3(front, up, right));

    // render the helicopter
    m_pHelicopter->Render(
                          shaderProgram,
                          m_pCamera,
                          m_helicopterRotorRotation,
                          m_helicopterPosition,
                          m_helicopterOrientation,
                          m_helicopterRotor);
}

// Update method runs repeatedly with the Render method
void Game::Update() 
{

    // Update the camera using the amount of time that has elapsed to avoid framerate dependent motion
    m_pCamera->Update(m_gameWindow.Window(), m_deltaTime, keyPressedCode, true, m_enableMouseMovement);
    MouseControls(mouseButton, mouseAction);
    KeyBoardControls(keyPressedCode, keyReleasedCode, keyPressedAction);
}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{

	// Variable timer
	m_pGameTimer->Start();
	Update();
	Render();
    UpdateAudioDSP();
	m_deltaTime = m_pGameTimer->Elapsed();

}

// mouse events callback function
static void OnMouseDown_callback(GLFWwindow* window, int button, int action, int mods){

    //std::cout << "Mouse Down with button: " << button << " and with action: " << action << std::endl;
    mouseButton = button;
    mouseAction = action;
}

// keyboard events callback function
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

// mouse inputs controls
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

// keybaord inputs controls and updating the DSP values with keys from 1 to 0
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

        if ((int)m_lastKeyPressTime  == (int)m_keyPressTime && m_isKeyPressRestriction == true)
        {
            return;
        }

        switch (keyPressed)
        {
            case GLFW_KEY_SPACE:
                break;
            case GLFW_KEY_F1:
                break;
            case GLFW_KEY_1 :
                m_helicopterSpeedScalar -= 0.2f;
                break;
            case GLFW_KEY_2:
                m_helicopterSpeedScalar += 0.2f;
                break;
            case GLFW_KEY_3:
                m_pDSP->ToggleChannelFrequency();
                break;
            case GLFW_KEY_4:
                m_pDSP->TogglePauseChannels();
                break;
            case GLFW_KEY_5:
                m_pDSP->ToggleMusicFilterFrequency();
                break;
            case GLFW_KEY_6:
                m_pDSP->ToggleMusicFilter();
                break;
            case GLFW_KEY_7:
                m_pDSP->ToggleFilterCoefficients();
                break;
            case GLFW_KEY_8:
                m_pDSP->ToggleByPass();
                break;
            case GLFW_KEY_9:
                m_pDSP->DecreaseCoefficients();
                break;
            case GLFW_KEY_0:
                m_pDSP->IncreaseCoefficients();
                break;
            case GLFW_KEY_COMMA:
                break;
            case GLFW_KEY_PERIOD:
                break;
            case GLFW_KEY_MINUS:
                m_pDSP->DecreaseMusicVolume();
                break;
            case GLFW_KEY_EQUAL:
                m_pDSP->IncreaseMusicVolume();
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
                break;
            case GLFW_KEY_P:
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

    InitialiseAudioDSP(filepath); // dsp

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
