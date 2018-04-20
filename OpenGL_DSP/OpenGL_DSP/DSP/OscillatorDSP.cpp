//
//  OscillatorDSP.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 20/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#include "OscillatorDSP.h"

COscillator::COscillator()
{
    m_musicVolume = 0.1f;
}

COscillator::~COscillator()
{
    delete m_FmodSystem;
    delete m_eventSound;
    delete m_eventChannel;
    delete m_dspOSC;
    delete m_music;
    delete m_musicChannel;
}

// Check for error
void COscillator::FmodErrorCheck(FMOD_RESULT result)
{
    if (m_result != FMOD_OK) {
        const char *errorString = FMOD_ErrorString(result);
        // MessageBox(NULL, errorString, "FMOD Error", MB_OK);
        // Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
    }
}

bool COscillator::Initialise()
{
    // Create an FMOD system
    m_result = FMOD::System_Create(&m_FmodSystem);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    // Initialise the system
    m_result = m_FmodSystem->init(32, FMOD_INIT_NORMAL, 0);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool COscillator::LoadEventSound(const char *filename)
{
    m_result = m_FmodSystem->createSound(filename, FMOD_LOOP_OFF, 0, &m_eventSound);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool COscillator::PlayEventSound()
{
    m_result = m_FmodSystem->playSound(m_eventSound, NULL, false, nullptr);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Load a music stream
bool COscillator::LoadMusicStream(const char *filename)
{
    m_result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // create a oscillator DSP Object
    m_result = m_FmodSystem->createDSPByType(FMOD_DSP_TYPE_OSCILLATOR, &m_dspOSC);

    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play a music stream
bool COscillator::PlayMusicStream()
{
    m_result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // Set the volume lower
    m_result = m_musicChannel->setVolume(m_musicVolume);
    FmodErrorCheck(m_result);

    // create oscillator dsp
    m_dspOSC->setParameterFloat(FMOD_DSP_OSCILLATOR_RATE, 440);
    // 0 = sine. 1 = square. 2 = sawup.
    // 3 = sawdown. 4 = triangle. 5 = noise.
    m_dspOSC->setParameterInt(FMOD_DSP_OSCILLATOR_TYPE, 4);
    m_FmodSystem->playDSP(m_dspOSC, NULL, false, &m_musicChannel);

    if (m_result != FMOD_OK)
        return false;

    return true;
}

void COscillator::Update()
{
    m_FmodSystem->update();
}
