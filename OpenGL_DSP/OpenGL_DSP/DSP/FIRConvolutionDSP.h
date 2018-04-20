//
//  FIRConvolutionDSP.hpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 21/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#ifndef FIRConvolutionDSP_h
#define FIRConvolutionDSP_h

#pragma once
#include "Common.h"
#include "CircularBuffer.h"

class CFIRConvolutionDSP
{
public:
    CFIRConvolutionDSP();
    ~CFIRConvolutionDSP();
    bool Initialise();

    bool LoadEventSound(const char *filename);
    bool PlayEventSound();

    bool LoadMusicStream(const char *filename);
    bool PlayMusicStream();

    void IncreaseMusicVolume();
    void DecreaseMusicVolume();

    void Update();

private:

    void FmodErrorCheck(FMOD_RESULT result);
    void ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec);

    FMOD_RESULT m_result;
    FMOD::System *m_FmodSystem;    // the global variable for talking to FMOD

    FMOD::Sound *m_eventSound;
    FMOD::Channel *m_eventChannel;

    FMOD::DSP *m_dsp;

    FMOD::Sound *m_music;
    FMOD::Channel* m_musicChannel;
    GLfloat m_musicVolume;

};
#endif /* FIRConvolutionDSP_h */
