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
    m_result = m_eventSound1->release();
    m_result = m_eventSound2->release();
    m_result = m_FmodSystem->close();
    m_result = m_FmodSystem->release();

    delete m_FmodSystem;
    delete m_eventSound1;
    delete m_eventSound2;
    delete m_eventChannel1;
    delete m_eventChannel2;
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
        //MessageBox(NULL, errorString, "FMOD Error", MB_OK);
        // Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
    }
}

// DSP callback
FMOD_RESULT F_CALLBACK ControlFilterDSPCallback(FMOD_DSP_STATE *dsp_state,
                                                float *inbuffer,
                                                float *outbuffer,
                                                unsigned int length,
                                                int inchannels,
                                                int *outchannels)
{
    FMOD::DSP *thisdsp = (FMOD::DSP *)dsp_state->instance;

    //    std::cout << "Length of array = " << (sizeof(inbuffer)/sizeof(inbuffer[0])) << std::endl;
    //    std::cout << "Length " << length << std::endl;
    //    std::cout << "Length of Coefficients = " << (sizeof(FIRcoefficient)/sizeof(FIRcoefficient[0])) << std::endl;

    // TODO: use circular buffer and refer to  vladamir code
    //if (circularBuffer == nullptr) circularBuffer = new CCircularBuffer(bufferSize * sizeof(float) * inchannels);
    if (buffer == NULL)
        buffer = (float*)malloc(bufferSize * sizeof(float) * inchannels);

    unsigned int samp, chan;
    GLfloat scale = 0.2f;

    for (samp = 0; samp < length; samp++)
    {
        for (chan = 0; chan < *outchannels; chan++)
        {

            /*
             This DSP filter just halves the volume!
             Input is modified, and sent to output.
             */

            // TODO: implement fir convolution
            /*
             GLuint n = ((samp * inchannels) + chan);
             GLfloat f;
             if (n < 3) {
             f = inbuffer[n];
             } else {
             for (int i = 0; i < n; ++i)
             {
             GLfloat currentCoeficient = FIRcoefficient[i % 4];
             f += currentCoeficient * inbuffer[n - i];
             }
             }
             //outbuffer[(samp * *outchannels) + chan] = f * scale;
             */
            //outbuffer[(samp * *outchannels) + chan] = inbuffer[(samp * inchannels) + chan] * scale;

            buffer[(sampleCount * inchannels) % bufferSize + chan] = inbuffer[(samp * inchannels) + chan];

            if (sampleCount < 4) // don't reach before the the start of the buffer with sample_count-3 below
                outbuffer[(samp * inchannels) + chan] = 0;
            else {
                // this is a simple averaging filter with 4 coefficients
                // TODO: Implemnet delay with the delay variable
                outbuffer[(samp * inchannels) + chan] = coefficient[0] * buffer[(sampleCount * inchannels) % bufferSize + chan];
                outbuffer[(samp * inchannels) + chan] += coefficient[1] * buffer[((sampleCount - 1) * inchannels) % bufferSize + chan];
                outbuffer[(samp * inchannels) + chan] += coefficient[2] * buffer[((sampleCount - 2) * inchannels) % bufferSize + chan];
                outbuffer[(samp * inchannels) + chan] += coefficient[3] * buffer[((sampleCount - 3) * inchannels) % bufferSize + chan];
            }
        }

        sampleCount++;
    }

    return FMOD_OK;
}


bool COcclusion::Initialise(GLfloat &doppler, GLfloat &distFactor, GLfloat &distRolloff)
{
    // 1) Create an FMOD system
    m_result = FMOD::System_Create(&m_FmodSystem);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }

    // 2) Initialise the system
    m_result = m_FmodSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, 0);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }

    // 3) Set the FMOD 3D settings to some sensible values.
    // dopple inntesity
    // dist factor chnages the boundary between the sound and camera
    // roll off scales the sound from high when very low, and low when it is very high
    m_distanceFactor = distFactor;
    m_result = m_FmodSystem->set3DSettings(doppler, distFactor, distRolloff);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }

    // Set geometry settings
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }


    // Create the DSP effect
    {
        FMOD_DSP_DESCRIPTION dspdesc;
        memset(&dspdesc, 0, sizeof(dspdesc));

        strncpy(dspdesc.name, "My first DSP unit", sizeof(dspdesc.name));
        dspdesc.numinputbuffers = 1;
        dspdesc.numoutputbuffers = 1;
        dspdesc.read = ControlFilterDSPCallback;

        m_result = m_FmodSystem->createDSP(&dspdesc, &m_dsp);
        //FmodErrorCheck(m_result);
        //if (m_result != FMOD_OK)
        //    return false;
    }

    return true;
}

// Play an event sound
bool COcclusion::LoadEventSound(const char *filename1, const char *filename2)
{
    //4) Load event sound2
    m_result = m_FmodSystem->createSound(filename1, FMOD_LOOP_NORMAL, 0, &m_eventSound1);
    m_result = m_eventSound1->set3DMinMaxDistance(0.5f * m_distanceFactor, 1000.0f * m_distanceFactor);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }

    m_result = m_FmodSystem->createSound(filename2, FMOD_LOOP_NORMAL, 0, &m_eventSound2);
    m_result = m_eventSound2->set3DMinMaxDistance(0.5f * m_distanceFactor, 1000.0f * m_distanceFactor);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }

    return true;
}

// Play an event sound
bool COcclusion::PlayEventSound(glm::vec3 &position1, glm::vec3 &position2, glm::vec3 &velocity)
{
    //5) Load event sound
    m_result = m_FmodSystem->playSound(m_eventSound1, 0, true, &m_eventChannel1);
    //FmodErrorCheck(m_result);

    m_result = m_FmodSystem->playSound(m_eventSound2, 0, true, &m_eventChannel2);

//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }

    //6) Refactor the “event sound” (triggered with '1') to play through a 3D channel.
    m_eventChannel1->setMode(FMOD_3D);
    m_eventChannel2->setMode(FMOD_3D);


    // set the position of the car and the helicopter
    ToFMODVector(position1, &m_helicopterPosition);
    ToFMODVector(velocity, &m_helicopterVelocity);
    ToFMODVector(position2, &m_racingCarPosition);
    m_result = m_eventChannel1->set3DAttributes(&m_helicopterPosition, &m_helicopterVelocity);
    m_result = m_eventChannel2->set3DAttributes(&m_racingCarPosition, 0, 0);

//    FmodErrorCheck(result);
//    if (m_result != FMOD_OK) {
//        return false;
//    }

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
    FmodErrorCheck(m_result);

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

    // create dsp
    m_musicChannel->addDSP(0, m_dsp);
    

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

void COcclusion::Update(CCamera *camera, glm::vec3 &position1, glm::vec3 &position2, glm::vec3 &velocity)
{
    // 7) update the helicopter position using 3d attributes
    ToFMODVector(position1, &m_helicopterPosition);
    ToFMODVector(velocity, &m_helicopterVelocity);
    ToFMODVector(position2, &m_racingCarPosition);

    m_result = m_eventChannel1->set3DAttributes(&m_helicopterPosition, &m_helicopterVelocity);
    m_result = m_eventChannel1->setPaused(false);

    m_result = m_eventChannel2->set3DAttributes(&m_racingCarPosition, 0, 0);
    m_result = m_eventChannel2->setPaused(false);

//
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return;
//    }

    // 8) update the camera position using the 3d listener for the occlusion
    glm::vec3 cameraForward = camera->GetForward();
    ToFMODVector(cameraForward, &m_cameraForward);

    glm::vec3 cameraUp = camera->GetUp();
    ToFMODVector(cameraUp, &m_cameraUp);

    glm::vec3 camPosition = camera->GetPosition();
    ToFMODVector(camPosition, &m_cameraPosition);

    glm::vec3 camVelocity = camera->GetVelocity();
    ToFMODVector(camVelocity, &m_cameraVelocity);

    //cout << endl;
    //cout << "position: " << position.x << ", " << position.y << ", " << position.z << endl;
    //cout << "velocity: " << velocity.x << ", " << velocity.y << ", " << velocity.z << endl;
    cout << "cam position: " << camPosition.x << ", " << camPosition.y << ", " << camPosition.z << endl;
    cout << "cam velocity: " << camVelocity.x << ", " << camVelocity.y << ", " << camVelocity.z << endl;
    //cout << "cam forward: " << cameraForward.x << ", " << cameraForward.y << ", " << cameraForward.z << endl;
    //cout << "cam up: " << cameraUp.x << ", " << cameraUp.y << ", " << cameraUp.z << endl;

    m_result = m_FmodSystem->set3DListenerAttributes(0,
                                                     &m_cameraPosition,
                                                     &m_cameraVelocity,
                                                     &m_cameraForward,
                                                     &m_cameraUp);
    
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return;
//    }

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
void COcclusion::AddCube(glm::vec3 &position, GLfloat &width,  GLfloat &height, GLfloat &depth)
{

    FMOD::Geometry *geometry;
    //
    // FMOD_RESULT System::createGeometry( int maxpolygons, int maxvertices, FMOD::Geometry **geometry);
    // m_result = m_FmodSystem->createGeometry(numTriangles, numVertices, &geometry);
    m_result = m_FmodSystem->createGeometry(6, 8, &geometry);

    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;
    FMOD_VECTOR cubePoly[8];
    cubePoly[0] = { -halfWidth, position.y, -halfDepth }; // bottom left back
    cubePoly[1] = { -halfWidth, position.y, halfDepth }; // bottom left front
    cubePoly[2] = { halfWidth, position.y, -halfDepth };  // bottom right back
    cubePoly[3] = { halfWidth, position.y, -halfDepth };    // bottom right front
    cubePoly[4] = { -halfWidth, position.y + height, -halfDepth }; // top left back
    cubePoly[5] = { -halfWidth, position.y + height, halfDepth }; // top left front
    cubePoly[6] = { halfWidth, position.y + height, -halfDepth };  // top right back
    cubePoly[7] = { halfWidth, position.y + height, -halfDepth };    // top right front
    int polyIndex = 0;

//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return;
//    }


    m_result = geometry->addPolygon(1.0f, 1.0f, true, 8, cubePoly, &polyIndex);
//    FMOD_VECTOR polygons[numVertices];
//    int polyIndex = 0;
//
//    for (unsigned int j = 0 ; j < numVertices ; j++) {
//        glm::vec3 vertex = polygon.m_vertices[j].m_pos;
//        ToFMODVector(vertex, &polygons[j]);
//        //std::cout << "v" << j << ": " << polygons[i].x << ", " << polygons[i].y << ", " <<  polygons[i].z << std::endl;
//    }

    /*
    FMOD_RESULT Geometry::addPolygon(
                                     float directocclusion,
                                     float reverbocclusion,
                                     bool doublesided,
                                     int numvertices,
                                     const FMOD_VECTOR *vertices,
                                     int *polygonindex
                                     );
    */
    //m_result = geometry->addPolygon(1.0f, 1.0f, true, 3, polygons, &polyIndex);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return;
//    }

    FMOD_VECTOR cubePosition;
    ToFMODVector(position, &cubePosition);
    geometry->setPosition(&cubePosition);
    geometry->setActive(true);

}

void COcclusion::CreateTerrain(glm::vec3 &position, const float &size)
{
    FMOD::Geometry *geometry;
    //                                      // number of triangles, vertices, geometry
    // FMOD_RESULT System::createGeometry( int maxpolygons, int maxvertices, FMOD::Geometry **geometry);
    m_result = m_FmodSystem->createGeometry(1, 4, &geometry);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return;
//    }

    float halfSize = size / 2;
    FMOD_VECTOR groundPoly[4];
    groundPoly[0] = { -halfSize, position.y, -halfSize }; // left back
    groundPoly[1] = { -halfSize, position.y, halfSize }; // left front
    groundPoly[2] = { halfSize, position.y, -halfSize };  // right back
    groundPoly[3] = { halfSize, position.y, -halfSize };    // right front
    int polyIndex = 0;

    m_result = geometry->addPolygon(1.0f, 1.0f, true, 4, groundPoly, &polyIndex);
//    FmodErrorCheck(m_result);
//    if (m_result != FMOD_OK) {
//        return;
//    }

    FMOD_VECTOR terrainPosition;
    ToFMODVector(position, &terrainPosition);
    geometry->setPosition(&terrainPosition);
    geometry->setActive(true);
}
