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
    
    void Update();

private:

    void FmodErrorCheck(FMOD_RESULT result);
    void ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec);

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

};
#endif /* FilterDSP_hpp */
