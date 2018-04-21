//
//  FIRConvolutionDSP.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 21/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#include "FIRConvolutionDSP.h"

static CCircularBuffer * circularBuffer;

static int sampleCount = 0;
static GLfloat coefficient[] = { 0.25f, 0.25f, 0.25f, 0.25f };
static float *buffer = nullptr;
static const int bufferSize = 1024;
static const int delay = 512;

CFIRConvolutionDSP::CFIRConvolutionDSP()
{
    m_musicVolume = 0.2f;
}

CFIRConvolutionDSP::~CFIRConvolutionDSP()
{
    delete m_FmodSystem;
    delete m_eventSound;
    delete m_eventChannel;
    delete m_dsp;
    delete m_music;
    delete m_musicChannel;

}

// Check for error
void CFIRConvolutionDSP::FmodErrorCheck(FMOD_RESULT result)
{
    if (m_result != FMOD_OK) {
        const char *errorString = FMOD_ErrorString(result);
        // MessageBox(NULL, errorString, "FMOD Error", MB_OK);
        // Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
    }
}

// DSP callback
FMOD_RESULT F_CALLBACK FIRConvolutionDSPCallback(FMOD_DSP_STATE *dsp_state,
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

bool CFIRConvolutionDSP::Initialise()
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

     // Create the DSP effect
     {
         FMOD_DSP_DESCRIPTION dspdesc;
         memset(&dspdesc, 0, sizeof(dspdesc));

         strncpy(dspdesc.name, "My first DSP unit", sizeof(dspdesc.name));
         dspdesc.numinputbuffers = 1;
         dspdesc.numoutputbuffers = 1;
         dspdesc.read = FIRConvolutionDSPCallback;

         m_result = m_FmodSystem->createDSP(&dspdesc, &m_dsp);
         FmodErrorCheck(m_result);

         if (m_result != FMOD_OK)
         return false;
     }

    return true;
}

// Play an event sound
bool CFIRConvolutionDSP::LoadEventSound(const char *filename)
{
    m_result = m_FmodSystem->createSound(filename, FMOD_LOOP_OFF, 0, &m_eventSound);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool CFIRConvolutionDSP::PlayEventSound()
{
    m_result = m_FmodSystem->playSound(m_eventSound, NULL, false, NULL);
    FmodErrorCheck(m_result);
    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Load a music stream
bool CFIRConvolutionDSP::LoadMusicStream(const char *filename)
{
    m_result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
    FmodErrorCheck(m_result);

    if (m_result != FMOD_OK)
        return false;

    return true;
}

// Play a music stream
bool CFIRConvolutionDSP::PlayMusicStream()
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

    return true;
}

void CFIRConvolutionDSP::Update(CCamera *camera)
{
    glm::vec3 position = camera->GetPosition();
    // 5) update the listener's position with the camera position
    DSPHelper::ToFMODVector(position, &m_cameraPosition);
    m_result = m_FmodSystem->set3DListenerAttributes(0, &m_cameraPosition, NULL, NULL, NULL);
    FmodErrorCheck(m_result);

    m_FmodSystem->update();
}

void CFIRConvolutionDSP::IncreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // increment the volume
    m_musicVolume += 0.05f;
    if (m_musicVolume > 1)
        m_musicVolume = 1.0f;

    m_musicChannel->setVolume(m_musicVolume);
}

void CFIRConvolutionDSP::DecreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // deccrement the volume
    m_musicVolume -= 0.05f;
    if (m_musicVolume < 0)
        m_musicVolume = 0.0f;

    m_musicChannel->setVolume(m_musicVolume);
}
