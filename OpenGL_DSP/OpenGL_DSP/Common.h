//
//  Common.h
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 27/02/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//
#pragma once


#include <iostream>
#include <ctime>
#include <assert.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <map>
#include <string>
#include "sys/time.h"
#include <stack>
#include <thread>
#include <chrono>
#include <complex>
#include <memory.h>
#include <unistd.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>

#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#define GLEW_STATIC
#include <GL/glew.h>  
#include <GL/gl.h>

//GLFW
#define GLFW_DLL
#include <GLFW/glfw3.h>// GLFW helper library

//Free Type Font
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>

//Free Image
#include <FreeImage.h>

//ASSIMP
#include <assimp/Importer.hpp>      // C++ importer interface  //assimp
#include <assimp/scene.h>       // Output data structure  //assimp
#include <assimp/PostProcess.h> // Post processing flags  //assimp

//FMod
#include <fmod.h>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <fmod_studio_common.h>

//STB Image
#include <stb/stb_image.h> 

#define _USE_MATH_DEFINES
#include <math.h>

#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 720
#define FPS 60

// Settings
#define FOV 45.0
#define ZNEAR 0.1
#define ZFAR 5000.0
#define CUBEMAPTEXTUREUNIT 10 //cube map texture of the skyBox is at 10 texture unit

// Default camera values
#define YAW -90.0f
#define PITCH 0.1
#define SPEED 40.0
#define SPEEDRATIO 0.025
#define SENSITIVTY 0.25
#define ZOOM 45.0f


using namespace std;
