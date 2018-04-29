//
//  FilterDSP.hpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 20/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#ifndef FilterDSP_h
#define FilterDSP_h

#pragma once
#include "Common.h"
#include "Camera.h"
#include "DSPHelper.h"

class CFilterDSP
{
public:
    CFilterDSP();
    ~CFilterDSP();
    bool Initialise();

    bool LoadEventSound(const char *filename);
    bool PlayEventSound();

    bool LoadMusicStream(const char *filename);
    bool PlayMusicStream();

    void IncreaseMusicVolume();
    void DecreaseMusicVolume();

    void ToggleMusicFilter();

    void Update(CCamera *camera);

private:

    void FmodErrorCheck(FMOD_RESULT result);

    FMOD_RESULT m_result;
    FMOD::System *m_FmodSystem;    // the global variable for talking to FMOD

    FMOD::Sound *m_eventSound;
    FMOD::Channel *m_eventChannel;

    FMOD::Sound *m_music;
    FMOD::Channel* m_musicChannel;
    GLfloat m_musicVolume;

    FMOD::DSP *m_musicFilter;
    bool m_musicFilterActive;

    FMOD::DSP *m_musicDSPHead;
    FMOD::DSP *m_musicDSPHeadInput;

    // helicopter attributes
    FMOD_VECTOR m_cameraPosition;     // the camera position in the world

};
#endif /* FilterDSP_hpp */
