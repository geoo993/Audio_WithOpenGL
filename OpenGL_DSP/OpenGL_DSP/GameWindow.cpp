#include "GameWindow.h"

#define SIMPLE_OPENGL_CLASS_NAME "simple_openGL_class_name"

CGameWindow::CGameWindow(const CGameWindow &other) {
    this -> m_appName = other.m_appName;
    this -> m_width = other.m_width;
    this -> m_height = other.m_height;
    this -> m_fullscreen = other.m_fullscreen;
}

CGameWindow &CGameWindow::operator=(const CGameWindow &other){
    this -> m_appName = other.m_appName;
    this -> m_width = other.m_width;
    this -> m_height = other.m_height;
    this -> m_fullscreen = other.m_fullscreen;
    return *this;
}

CGameWindow::CGameWindow() : m_appName(""), m_fullscreen(false), m_width(0), m_height(0) {}

void CGameWindow::Set(const std::string &appName, const int &w, const int &h, const bool &fullscreen){
    m_appName = appName;
    m_width = w;
    m_height = h;
    m_fullscreen = fullscreen;
}

// Create a dummy window, intialise GLEW, and then delete the dummy window
bool CGameWindow::InitGLEW()
{
	static bool bGlewInitialized = false;
	if(bGlewInitialized) return true;

    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        exit(EXIT_FAILURE);
        return false;
    }

    GLFWwindow* fakeWindow = glfwCreateWindow(m_width, m_height, m_appName.c_str(), nullptr, nullptr); // Windowed

    if (fakeWindow == nullptr) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(fakeWindow);

    bool bResult = true;

    if(!bGlewInitialized)
    {
        if(glewInit() != GLEW_OK){
            throw std::runtime_error("Couldn't initialize GLEW!");
            bResult = false;
        }
        bGlewInitialized = true;
    }

    glfwMakeContextCurrent(nullptr);
    glfwDestroyWindow(fakeWindow);
    glfwTerminate();

	return bResult;
}


// Initialise GLEW and create the real game window
void CGameWindow::Init(const std::string &appName,
                       const int &w,
                       const int &h,
                       const bool &fullscreen)
{
    Set(appName, w, h, fullscreen);

    if(!InitGLEW()){
        return;
    }

    CreateGameWindow(m_appName);

    return;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// Create the game window
void CGameWindow::CreateGameWindow(const string &appName)
{
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        exit(EXIT_FAILURE);
        return;
    }

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);  // Multi Sample Anti-Aliasing  for smooth edges.
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    //glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//We don't want the old OpenGL and want the core profile
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    if (m_fullscreen){
        m_window = glfwCreateWindow(m_width, m_height, appName.c_str(), glfwGetPrimaryMonitor(), nullptr);//// Windowed
    }else{
        m_window = glfwCreateWindow(m_width, m_height, appName.c_str(), nullptr, nullptr); // Fullscreen
    }

    glfwGetWindowSize(m_window, &m_width, &m_height);

    if (m_window == nullptr) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return ;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSwapInterval(1); ////<-- force interval (not guaranteed to work with all graphics drivers)

    //start GLEW extension handler
    if(InitOpenGL()){
        SetCursorVisible(false);
        CenterTheWindow();
        return;
    }

    return;

}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// Initialise OpenGL, including the pixel format descriptor and the OpenGL version
bool CGameWindow::InitOpenGL()
{

    bool bError = false;

    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        throw std::runtime_error("glewInit failed");
        bError = true;
    }

    if(!GLEW_VERSION_3_2){
        throw std::runtime_error("OpenGL 3.2 API is not available.");
        bError = true;
    }

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer as a string
    const GLubyte* version = glGetString(GL_VERSION); // "graphics griver version as a string
    const GLubyte* vendor = glGetString(GL_VENDOR); // vendor as a string
    const GLubyte* shaderLanguage = glGetString(GL_SHADING_LANGUAGE_VERSION); // shader lang as a string
    // print out some info about the graphics drivers
    std::cout << "OpenGL or Graphics Driver version supported: " << version << std::endl;
    std::cout << "GLSL version: " << shaderLanguage << std::endl;
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "Vendor: " << vendor << std::endl;

    //3. Check for specific functionality
    if (GLEW_ARB_vertex_array_object){
        printf("genVertexArrays supported\n");
    }
    if (GLEW_APPLE_vertex_array_object){
        printf("genVertexArrayAPPLE supported\n");
    }

    if(bError)
    {
        glfwSetErrorCallback(error_callback);
        return bError;
    }

    GLint MaxTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureUnits);
    std::cout << "There are "<< MaxTextureUnits << " texture units supported by GPU. " << std::endl;

	return bError;
}

void CGameWindow::SetCursorVisible( const bool &isVisible )
{
    if( m_window == nullptr )
        return;

    if( isVisible ){
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        assert(glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL);
    }else{
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        assert(glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN);
    }

    //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

}

void CGameWindow::SetInputs(const GLFWkeyfun &cbfunKey, const GLFWmousebuttonfun &cbfunMouse){

    //glfwSetWindowSizeCallback(m_window, ReshapeWindow);
    //glfwSetWindowShouldClose(m_window, GLUS_TRUE);

    glfwSetKeyCallback(m_window, cbfunKey);
    //glfwSetCursorPosCallback(m_window, cbfunMove);
    glfwSetMouseButtonCallback(m_window, cbfunMouse );
    //glfwSetInputMode(m_window, GLFW_STICKY_KEYS, 1);

    glfwMakeContextCurrent(m_window);

}

//Check whether window should close
bool CGameWindow::ShouldClose(){
    return glfwWindowShouldClose(m_window);
}

// https://stackoverflow.com/questions/11335301/glfw-get-screen-height-width
void CGameWindow::CenterTheWindow(){
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(m_window, (mode->width - m_width) / 2, (mode->height - m_height) / 2);
}

//Get the Current framebuffer Size in pixels and Set the Viewport to it
void CGameWindow::SetViewport(){
    //glfwGetWindowSize(window, &windowWidth, &windowHeight);

    GLint width, height;

    // returns the framebuffer size, not the window size.
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport( 0, 0, width, height);

}

//Get the Current framebuffer Size in pixels and Set the Viewport to it
void CGameWindow::SetViewport(const int & width, const int & height){

    glViewport( 0, 0, width, height);
}

void CGameWindow::PreRendering(){

    // configure global opengl states

    // Clear the buffers and enable depth testing (z-buffering)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set background color to black
    glClear (GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_MULTISAMPLE);

    // https://www.ntu.edu.sg/home/ehchua/programming/opengl/CG_Examples.html
    //glClearDepth(1.0f);          // Set background depth to farthest
    //glShadeModel(GL_SMOOTH);   // Enable smooth shading

    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_CULL_FACE);
    //glPolygonOffset(1.0f, 0.0f);

    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);

}

void CGameWindow::PostRendering()
{
    glfwPollEvents();
}

void CGameWindow::ClearBuffers(){

    // Clear the buffers and enable depth testing (z-buffering) or stencil testing
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    //// CLEAR Buffers, The default clear value for the depth is 1.0f, which is equal to the depth of your far clipping plane
    glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);// | GL_STENCIL_BUFFER_BIT); ////<-- CLEAR WINDOW
    //glClearDepth(1.0f); // same as glClear, we are simply specificaly clearing the depthbuffer

}

//Swap front and back buffers
void CGameWindow::SwapBuffers(){
    //glfwSwapInterval(1);
    glfwSwapBuffers(m_window);
}

//Destroy the window, // Deinitialise the window and rendering context
void CGameWindow::DestroyWindow(){
    glfwMakeContextCurrent(nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

// Deinitialise the window and rendering context
CGameWindow::~CGameWindow(){

    DestroyWindow();
    SetCursorVisible(true);
}

