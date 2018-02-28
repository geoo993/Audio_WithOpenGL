#pragma once

#include "Common.h"

class CGameWindow {
public:

	CGameWindow();
    ~CGameWindow();

    void Init(const string &appName = "OpenGL Window",
              const int &w = SCREEN_WIDTH,
              const int &h = SCREEN_HEIGHT,
              const bool &fullscreen = false);

    void CreateGameWindow(const string &appName);
    bool InitGLEW();
    bool InitOpenGL();

    void Set(const string &appName, const int &w, const int &h, const bool &fullscreen);

	bool IsFullscreen() const { return m_fullscreen; }
    float Ratio() { return (float)m_width / m_height; }
    int Width() const { return m_width; }
    int Height()  const { return m_height; }
    GLFWwindow * Window() const { return m_window; }

    void SetInputs(const GLFWkeyfun &cbfunKey, const GLFWmousebuttonfun &cbfunMouse);
    void SetCursorVisible( const bool &isVisible );
    void CenterTheWindow();
    void PreRendering();
    void PostRendering();
    void SetViewport();
    void SetViewport(const int & width, const int & height);
    bool ShouldClose();
    void ClearBuffers();
    void SwapBuffers();
    void DestroyWindow();
    
private:
	
    CGameWindow(const CGameWindow &other);
    CGameWindow &operator=(const CGameWindow &other);

	bool  m_fullscreen;

    int m_width, m_height;

    GLFWwindow * m_window = nullptr;

	string m_appName;

};

