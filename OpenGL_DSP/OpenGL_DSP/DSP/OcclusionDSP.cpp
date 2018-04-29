//
//  OcclusionDSP.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 20/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#include "OcclusionDSP.h"


COcclusion::COcclusion()
{
    m_musicVolume = 1.0f;
}

COcclusion::~COcclusion()
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
void COcclusion::FmodErrorCheck(FMOD_RESULT result)
{
    if (m_result != FMOD_OK) {
        const char *errorString = FMOD_ErrorString(result);
        // MessageBox(NULL, errorString, "FMOD Error", MB_OK);
        // Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
    }
}

bool COcclusion::Initialise(GLfloat &doppler, GLfloat &distFactor, GLfloat &distRolloff)
{
    // Create an FMOD system
    m_result = FMOD::System_Create(&m_FmodSystem);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    // Initialise the system
    // dopple inntesity
    // dist factor chnages the boundary between the sound and camera
    // roll off scales the sound from high when very low, and low when it is very high
    m_result = m_FmodSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, 0);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    // 3) Set the FMOD 3D settings to some sensible values.
    m_result = m_FmodSystem->set3DSettings(doppler, distFactor, distRolloff);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    // Set geometry settings
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool COcclusion::LoadEventSound(const char *filename)
{
    m_result = m_FmodSystem->createSound(filename, FMOD_LOOP_NORMAL, 0, &m_eventSound);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool COcclusion::PlayEventSound()
{
    m_result = m_FmodSystem->playSound(m_eventSound, NULL, false, &m_eventChannel);
    //FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    //4) Refactor the “event sound” (triggered with '1') to play through a 3D channel.
    m_eventChannel->setMode(FMOD_3D);

    // Set the volume
    m_result = m_musicChannel->setVolume(m_musicVolume);
    //FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Load a music stream
bool COcclusion::LoadMusicStream(const char *filename)
{
    m_result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // create a low-pass filter DSP object
    m_result = m_FmodSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &m_musicFilter);

    // Set the volume
    m_result = m_musicChannel->setVolume(m_musicVolume);
    //FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    // you can start the DSP in an inactive state
    m_musicFilter->setActive(false);

    return true;
}

// Play a music stream
bool COcclusion::PlayMusicStream()
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

void COcclusion::Update(CCamera *camera, glm::vec3 &position, glm::vec3 &velocity)
{
    // 5) update the listener's position with the helicopter position
    ToFMODVector(position, &m_helicopterPosition);

    ToFMODVector(velocity, &m_helicopterVelocity);

    // 6) update the listener's position with the camera position
    glm::vec3 cameraForward = camera->GetForward();
    ToFMODVector(cameraForward, &m_cameraForward);

    glm::vec3 cameraUp = camera->GetUp();
    ToFMODVector(cameraUp, &m_cameraUp);

    glm::vec3 camPosition = camera->GetPosition();
    ToFMODVector(camPosition, &m_cameraPosition);

    glm::vec3 camVelocity = camera->GetVelocity();
    ToFMODVector(camVelocity, &m_cameraVelocity);

    /*
    cout << "position: " << position.x << ", " << position.y << ", " << position.z << endl;
    cout << "velocity: " << velocity.x << ", " << velocity.y << ", " << velocity.z << endl;
    cout << "cam position: " << camPosition.x << ", " << camPosition.y << ", " << camPosition.z << endl;
    cout << "cam velocity: " << camVelocity.x << ", " << camVelocity.y << ", " << camVelocity.z << endl;
    cout << "cam forward: " << cameraForward.x << ", " << cameraForward.y << ", " << cameraForward.z << endl;
    cout << "cam up: " << cameraUp.x << ", " << cameraUp.y << ", " << cameraUp.z << endl << endl;
    */

    m_result = m_eventChannel->set3DAttributes(&m_helicopterPosition, &m_helicopterVelocity);
    m_result = m_FmodSystem->set3DListenerAttributes(0, &m_cameraPosition, &m_cameraVelocity, &m_cameraForward, &m_cameraUp);
    //    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK) {
        return;
    }

    m_FmodSystem->update();

}

void COcclusion::ToggleMusicFilter()
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

void COcclusion::IncreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // increment the volume
    m_musicVolume += 0.05f;
    if (m_musicVolume > 1)
        m_musicVolume = 1.0f;

    m_musicChannel->setVolume(m_musicVolume);
}

void COcclusion::DecreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // deccrement the volume
    m_musicVolume -= 0.05f;
    if (m_musicVolume < 0)
        m_musicVolume = 0.0f;

    m_musicChannel->setVolume(m_musicVolume);
}

void COcclusion::ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec)
{
    fmodVec->x = glVec3.x;
    fmodVec->y = glVec3.y;
    fmodVec->z = glVec3.z;
}

/*
 This method creates the occlusion wall. Note that the poly must be *convex*!
 */
void COcclusion::CreateWall(glm::vec3 &position, float &width, float &height)
{
    FMOD::Geometry *geometry;
    m_result = m_FmodSystem->createGeometry(1, 4, &geometry);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK) {
        return;
    }

    float halfWidth = width / 2;
    FMOD_VECTOR wallPoly[4];
    wallPoly[0] = { -halfWidth, 0.0f, 0.0f };
    wallPoly[1] = { -halfWidth, height, 0.0f };
    wallPoly[2] = { halfWidth, height, 0.0f };
    wallPoly[3] = { halfWidth, 0.0f, 0.0f };
    int polyIndex = 0;
    m_result = geometry->addPolygon(1.0f, 1.0f, TRUE, 4, wallPoly, &polyIndex);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK) {
        return;
    }

    FMOD_VECTOR wallPosition;
    DSPHelper::ToFMODVector(position, &wallPosition);
    geometry->setPosition(&wallPosition);
    geometry->setActive(TRUE);
}
