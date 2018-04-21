//
//  FilterDSP.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 20/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#include "FilterDSP.h"

CFilterDSP::CFilterDSP()
{
    m_musicVolume = 0.2f;
    m_doppler = 1.0f;
    m_distFactor = 1.0f;
    m_distRolloff = 1.0f;
}

CFilterDSP::~CFilterDSP()
{
    delete m_FmodSystem;
    delete m_eventSound;
    delete m_eventChannel;
    delete m_music;
    delete m_musicChannel;

    delete m_musicFilter;
    delete m_musicDSPHead;
    delete m_musicDSPHeadInput;
}

// Check for error
void CFilterDSP::FmodErrorCheck(FMOD_RESULT result)
{
    if (m_result != FMOD_OK) {
        const char *errorString = FMOD_ErrorString(result);
        // MessageBox(NULL, errorString, "FMOD Error", MB_OK);
        // Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
    }
}

bool CFilterDSP::Initialise()
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

    // 3) Set the FMOD 3D settings to some sensible values.
    m_result = m_FmodSystem->set3DSettings(m_doppler, m_distFactor, m_distRolloff);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool CFilterDSP::LoadEventSound(const char *filename)
{
    m_result = m_FmodSystem->createSound(filename, FMOD_LOOP_OFF, 0, &m_eventSound);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool CFilterDSP::PlayEventSound()
{
    m_result = m_FmodSystem->playSound(m_eventSound, NULL, false, NULL);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    //4) Refactor the “event sound” (triggered with '1') to play through a 3D channel.
    m_eventChannel->setMode(FMOD_3D);

    // 6) set the position to be the helicopter position
    //m_result = m_eventChannel->set3DAttributes(0, 0, 0);
    m_result = m_eventChannel->set3DAttributes(&m_helicopterPosition, &m_helicopterVelocity);

    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Load a music stream
bool CFilterDSP::LoadMusicStream(const char *filename)
{
    m_result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // create a low-pass filter DSP object
    m_result = m_FmodSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &m_musicFilter);

    if (m_result != FMOD_OK)
        return false;

    // you can start the DSP in an inactive state
    m_musicFilter->setActive(false);

    return true;
}

// Play a music stream
bool CFilterDSP::PlayMusicStream()
{
    m_result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // Set the volume lower
    m_result = m_musicChannel->setVolume(m_musicVolume);
    FmodErrorCheck(m_result);

    // connecting the music filter to the music stream
    // 1) Get the DSP head and it's input
    m_musicChannel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &m_musicDSPHead);
    m_musicDSPHead->getInput(0, &m_musicDSPHeadInput, NULL);
    // 2) Disconnect them
    m_musicDSPHead->disconnectFrom(m_musicDSPHeadInput);
    // 3) Add input to the music head from the filter
    m_result = m_musicDSPHead->addInput(m_musicFilter);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // 4) Add input to the filter head music DSP head input
    m_result = m_musicFilter->addInput(m_musicDSPHeadInput);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // set the DSP object to be active
    m_musicFilter->setActive(true);
    // initially set the cutoff to a high value
    m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
    // this state is used for toggling
    m_musicFilterActive = false;

    return true;
}

void CFilterDSP::Update(CCamera *camera, glm::vec3 &helicopterPosition, glm::vec3 &helicopterVelocity)
{
    DSPHelper::ToFMODVector(helicopterPosition, &m_helicopterPosition);
    DSPHelper::ToFMODVector(helicopterVelocity, &m_helicopterVelocity);
    m_result = m_eventChannel->set3DAttributes(&m_helicopterPosition, &m_helicopterVelocity);

    // 5) update the listener's position with the camera position
    glm::vec3 position = camera->GetPosition();
    DSPHelper::ToFMODVector(position, &m_cameraPosition);
    m_result = m_FmodSystem->set3DListenerAttributes(0, &m_cameraPosition, NULL, NULL, NULL);
    FmodErrorCheck(m_result);

    m_FmodSystem->update();
}

void CFilterDSP::ToggleMusicFilter()
{
    // called externally from Game::ProcessEvents
    // toggle the effect on/off
    m_musicFilterActive = !m_musicFilterActive;
    if (m_musicFilterActive) {
        // set the parameter to a low value
        m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 700);
    }
    else {
        // set the parameter to a high value
        // you could also use m_musicFilter->setBypass(true) instead...
        m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
    }
}

void CFilterDSP::IncreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // increment the volume
    m_musicVolume += 0.05f;
    if (m_musicVolume > 1)
        m_musicVolume = 1.0f;

    m_musicChannel->setVolume(m_musicVolume);
}

void CFilterDSP::DecreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // deccrement the volume
    m_musicVolume -= 0.05f;
    if (m_musicVolume < 0)
        m_musicVolume = 0.0f;

    m_musicChannel->setVolume(m_musicVolume);
}
