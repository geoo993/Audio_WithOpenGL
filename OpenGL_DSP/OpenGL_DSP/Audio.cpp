#include "Audio.h"

static CCircularBuffer * cb;
static GLfloat FIRcoefficient[] = { 0.3f, 0.2f, 0.4f, 1.0f };


CAudio::CAudio()
{

}

CAudio::~CAudio()
{
    delete m_FmodSystem;
    delete m_eventSound;
    delete m_eventChannel;
    delete m_dsp;
    delete m_dspOSC;
    delete m_music;
    delete m_musicFilter;
    delete m_musicChannel;
    delete m_musicDSPHead;
    delete m_musicDSPHeadInput;

}

// Caluclate FIR filter
GLfloat CAudio::CalculateFIR(GLuint *inputs, GLfloat * coefficients) {
    GLfloat f;

    return f;
}

// Check for error
void CAudio::FmodErrorCheck(FMOD_RESULT result)
{
    if (result != FMOD_OK) {
        const char *errorString = FMOD_ErrorString(result);
        // MessageBox(NULL, errorString, "FMOD Error", MB_OK);
        // Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
    }
}

// DSP callback
FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state,
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

    // buffer, all x[n] are in this circular buffer
    if (cb == nullptr) cb = new CCircularBuffer(1024);

    GLfloat scale = 0.2f;

    for (unsigned int samp = 0; samp < length; samp++)
    {
        for (int chan = 0; chan < *outchannels; chan++)
        {

            /*
             This DSP filter just halves the volume!
             Input is modified, and sent to output.
             */
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

            //cout << "n is " << n << endl;

            //outbuffer[(samp * *outchannels) + chan] = f * scale;
            outbuffer[(samp * *outchannels) + chan] = inbuffer[(samp * inchannels) + chan];// * scale;

        }
    }

    return FMOD_OK;
}

bool CAudio::Initialise()
{
	// Create an FMOD system
	result = FMOD::System_Create(&m_FmodSystem);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	// Initialise the system
	result = m_FmodSystem->init(32, FMOD_INIT_NORMAL, 0);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

    return true;
}

bool CAudio::InitialiseWithDSPEffect() {

    bool initialise = Initialise();

    // Create the DSP effect
    {
        FMOD_DSP_DESCRIPTION dspdesc;
        memset(&dspdesc, 0, sizeof(dspdesc));

        strncpy(dspdesc.name, "My first DSP unit", sizeof(dspdesc.name));
        dspdesc.numinputbuffers = 1;
        dspdesc.numoutputbuffers = 1;
        dspdesc.read = DSPCallback;

        result = m_FmodSystem->createDSP(&dspdesc, &m_dsp);
        FmodErrorCheck(result);

        if (result != FMOD_OK)
            initialise = false;
    }

	return initialise;
}

bool CAudio::InitialiseWith3DSettings() {
    // Create an FMOD system
    result = FMOD::System_Create(&m_FmodSystem);
    FmodErrorCheck(result);
    if (result != FMOD_OK)
        return false;

    // Initialise the system
    result = m_FmodSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, 0);
    FmodErrorCheck(result);
    if (result != FMOD_OK)
        return false;

    // Set 3D settings
    result = m_FmodSystem->set3DSettings(1.0f, 1.0f, 1.0f);
    FmodErrorCheck(result);
    if (result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool CAudio::LoadEventSound(const char *filename)
{
    result = m_FmodSystem->createSound(filename, FMOD_LOOP_OFF, 0, &m_eventSound);
    FmodErrorCheck(result);
    if (result != FMOD_OK)
        return false;

    return true;
}

// Play an event sound
bool CAudio::PlayEventSound()
{
    result = m_FmodSystem->playSound(m_eventSound, NULL, false, nullptr);
    FmodErrorCheck(result);
    if (result != FMOD_OK)
        return false;

    return true;
}

/*
// Play an event sound
bool CAudio::PlayEventSoundUsingEventChannel()
{
    result = m_FmodSystem->playSound(m_eventSound, NULL, false, &m_eventChannel);
    FmodErrorCheck(result);
    if (result != FMOD_OK)
        return false;
    // play through 3D channel
    m_eventChannel->setMode(FMOD_3D);
    // set the position to be the horse's position
    result = m_eventChannel->set3DAttributes(0, 0, 0);
    FmodErrorCheck(result);
    if (result != FMOD_OK)
        return false;

    return true;
}
 */


// Load a music stream
bool CAudio::LoadMusicStream(const char *filename)
{
    result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
    FmodErrorCheck(result);

    if (result != FMOD_OK)
        return false;

    return true;
}

/*
bool CAudio::LoadMusicStreamUsingLowPassFilter(const char *filename) {

    bool load = LoadMusicStream(filename);

    // create a low-pass filter DSP object
    result = m_FmodSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &m_musicFilter);

    if (result != FMOD_OK){
        load = false;
        return load;
    }

    // you can start the DSP in an inactive state
    m_musicFilter->setActive(false);

    return load;
}
*/

// Play a music stream
bool CAudio::PlayMusicStreamUsingDSP()
{
    result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
    FmodErrorCheck(result);

    if (result != FMOD_OK)
        return false;

    m_musicChannel->addDSP(0, m_dsp);

    // Set the volume lower
    result = m_musicChannel->setVolume(m_musicVolume);
    FmodErrorCheck(result);


    m_FmodSystem->createDSPByType(FMOD_DSP_TYPE_OSCILLATOR, &m_dspOSC);
    m_dspOSC->setParameterFloat(FMOD_DSP_OSCILLATOR_RATE, 440);
    // 0 = sine. 1 = square. 2 = sawup.
    // 3 = sawdown. 4 = triangle. 5 = noise.
    m_dspOSC->setParameterInt(FMOD_DSP_OSCILLATOR_TYPE, 4);
    m_FmodSystem->playDSP(m_dspOSC, NULL, false, &m_musicChannel);

    if (result != FMOD_OK)
        return false;

    return true;
}

/*
// Play a music stream
bool CAudio::PlayMusicStreamUsingFilter()
{
    result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
    FmodErrorCheck(result);

    if (result != FMOD_OK)
        return false;

    // Set the volume lower
    result = m_musicChannel->setVolume(m_musicVolume);
    FmodErrorCheck(result);

    if (result != FMOD_OK)
        return false;

    // connecting the music filter to the music stream
    // 1) Get the DSP head and it's input
    m_musicChannel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &m_musicDSPHead);
    m_musicDSPHead->getInput(0, &m_musicDSPHeadInput, NULL);
    // 2) Disconnect them
    m_musicDSPHead->disconnectFrom(m_musicDSPHeadInput);
    // 3) Add input to the music head from the filter
    result = m_musicDSPHead->addInput(m_musicFilter);
    FmodErrorCheck(result);

    if (result != FMOD_OK)
        return false;

    // 4) Add input to the filter head music DSP head input
    result = m_musicFilter->addInput(m_musicDSPHeadInput);
    FmodErrorCheck(result);

    if (result != FMOD_OK)
        return false;

    // set the DSP object to be active
    m_musicFilter->setActive(true);
    // initially set the cutoff to a high value
    m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
    // this state is used for toggling
    m_musicFilterActive = false;

    return true;
}
 */

void CAudio::Update(const CCamera *camera)
{
    glm::vec3 cameraPosition = camera->GetPosition();
    // 5) update the listener's position with the camera position
    ToFMODVector(cameraPosition, &camPos);
    result = m_FmodSystem->set3DListenerAttributes(0, &camPos, NULL, NULL, NULL);
    FmodErrorCheck(result);
    
    m_FmodSystem->update();
}

/*
void CAudio::ToggleMusicFilter()
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
*/
void CAudio::IncreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // increment the volume
    m_musicVolume += 0.05f;
    if (m_musicVolume > 1)
        m_musicVolume = 1.0f;

    m_musicChannel->setVolume(m_musicVolume);
}

void CAudio::DecreaseMusicVolume()
{
    // called externally from Game::ProcessEvents
    // deccrement the volume
    m_musicVolume -= 0.05f;
    if (m_musicVolume < 0)
        m_musicVolume = 0.0f;

    m_musicChannel->setVolume(m_musicVolume);
}

void CAudio::ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec)
{
    fmodVec->x = glVec3.x;
    fmodVec->y = glVec3.y;
    fmodVec->z = glVec3.z;
}
