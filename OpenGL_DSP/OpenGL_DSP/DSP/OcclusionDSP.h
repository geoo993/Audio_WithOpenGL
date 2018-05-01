//
//  OcclusionDSP.h
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 20/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#ifndef OcclusionDSP_h
#define OcclusionDSP_h

#pragma once
#include "Common.h"
#include "Camera.h"
#include "DSPHelper.h"
#include "CircularBuffer.h"

static CCircularBuffer * circularBuffer;

static int sampleCount = 0;
static GLfloat coefficient[] = { 0.25f, 0.25f, 0.25f, 0.25f };
static float *buffer = nullptr;
static const int bufferSize = 1024;
static const int delay = 512;

class COcclusion
{
public:
    COcclusion();
    ~COcclusion();
    bool Initialise(GLfloat &doppler, GLfloat &distFactor, GLfloat &distRolloff);

    bool LoadEventSound(const char *filename1, const char *filename2);
    bool PlayEventSound(glm::vec3 &position1, glm::vec3 &position2, glm::vec3 &velocity);

    bool LoadMusicStream(const char *filename);
    bool PlayMusicStream();

    void IncreaseMusicVolume();
    void DecreaseMusicVolume();

    void ToggleMusicFilter();

    void Update(CCamera *camera, glm::vec3 &position1, glm::vec3 &position2, glm::vec3 &velocity);
    void CreateTerrain(glm::vec3 &position, const float &size);
    void AddCube(glm::vec3 &position, GLfloat &width,  GLfloat &height, GLfloat &depth);

private:

    void FmodErrorCheck(FMOD_RESULT result);
    void ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec);

    FMOD_RESULT m_result;
    FMOD::System *m_FmodSystem;    // the global variable for talking to FMOD

    GLfloat m_distanceFactor;
    FMOD::DSP *m_dsp;

    FMOD::Sound *m_eventSound1;
    FMOD::Channel *m_eventChannel1;

    FMOD::Sound *m_eventSound2;
    FMOD::Channel *m_eventChannel2;

    FMOD::Sound *m_music;
    FMOD::Channel* m_musicChannel;
    GLfloat m_musicVolume;

    FMOD::DSP *m_musicFilter;
    bool m_musicFilterActive;

    FMOD::DSP *m_musicDSPHead;
    FMOD::DSP *m_musicDSPHeadInput;

    // helicopter attributes
    FMOD_VECTOR m_cameraPosition;     // the camera position in the world
    FMOD_VECTOR m_cameraForward;      // the camera local forward vector
    FMOD_VECTOR m_cameraUp;           // the camera local up vector in the world
    FMOD_VECTOR m_cameraVelocity;     // the camera velocity vector in the world

    FMOD_VECTOR m_helicopterPosition; // position of the helicopter
    FMOD_VECTOR m_helicopterVelocity; // velocity of the moving helicopter

    FMOD_VECTOR m_racingCarPosition; // position of the racing car

};


#endif /* OcclusionDSP_h */
