#include "DSPAudio.h"

/*
    list coefficients generated from http://t-filter.engineerjs.com/
 */
#define FILTER_NUM 25 // maximum number of filter coefficients

// averaging filter coefficients
static double averaging_filter[FILTER_NUM] = {
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
    0.04,
};

// low pass filter coefficients
static double low_pass_filter[FILTER_NUM] = {
    -0.02010411882885732,
    -0.05842798004352509,
    -0.061178403647821976,
    -0.010939393385338943,
    0.05125096443534972,
    0.033220867678947885,
    -0.05655276971833928,
    -0.08565500737264514,
    0.0633795996605449,
    0.310854403656636,
    0.4344309124179415,
    0.310854403656636,
    0.0633795996605449,
    -0.08565500737264514,
    -0.05655276971833928,
    0.033220867678947885,
    0.05125096443534972,
    -0.010939393385338943,
    -0.061178403647821976,
    -0.05842798004352509,
    -0.02010411882885732,
    -0.0144104118828832,
    -0.0111840364782196,
    -0.0100084204352509,
    -0.0003084204352509,
};

// high pass filter coefficients
static double high_pass_filter[FILTER_NUM] = {
    0.02857983994169657,
    -0.07328836181028245,
    0.04512928732568175,
    0.03422632401030237,
    -0.034724262386629436,
    -0.05343090761376418,
    0.032914528649623416,
    0.09880818246272206,
    -0.034135422078843417,
    -0.3160339484471911,
    0.5341936566511765,
    -0.3160339484471911,
    -0.034135422078843417,
    0.09880818246272206,
    0.032914528649623416,
    -0.05343090761376418,
    -0.034724262386629436,
    0.03422632401030237,
    0.04512928732568175,
    -0.07328836181028245,
    0.02857983994169657,
    0.03857983994169,
    0.050579839941,
    0.018579839941,
    0.0084798399459,
};

// band pass filter coefficients
static double band_pass_filter[FILTER_NUM] = {
    0.008315515510919604,
    0.013703008819203135,
    -0.008125257257844711,
    -0.01649214060817737,
    -0.0016884593471476288,
    -0.006913035271285468,
    -0.03139161346522045,
    0.022740863526439097,
    0.1198490872411674,
    0.05186355035523461,
    -0.17137740316854042,
    -0.20124347467075893,
    0.08441813048666601,
    0.2816314309336389,
    0.08441813048666601,
    -0.20124347467075893,
    -0.17137740316854042,
    0.05186355035523461,
    0.1198490872411674,
    0.022740863526439097,
    -0.03139161346522045,
    -0.006913035271285468,
    -0.0016884593471476288,
    -0.01649214060817737,
    -0.008125257257844711,
    //    0.013703008819203135,
    //    0.008315515510919604
};

// band stop filter coefficients
static double band_stop_filter[FILTER_NUM] = {
    0.037391727827352596,
    -0.03299884552335979,
    0.044230583967321345,
    0.0023050970833628304,
    -0.06768087195950104,
    -0.046347105409124706,
    -0.011717387509232432,
    -0.0707342284185183,
    -0.049766517282999544,
    0.16086413543836361,
    0.21561058688743148,
    -0.10159456907827959,
    0.6638637561392535,
    -0.10159456907827959,
    0.21561058688743148,
    0.16086413543836361,
    -0.049766517282999544,
    -0.0707342284185183,
    -0.011717387509232432,
    -0.046347105409124706,
    -0.06768087195950104,
    0.0023050970833628304,
    0.044230583967321345,
    -0.03299884552335979,
    0.037391727827352596
};

static float *buffer = nullptr;         // circular buffer array
static const int bufferSize = 1024;     // circular buffer filter size
static const int delay = 512;           // filter delay
static float multiplier = 0.8f;         // FIR filter miltiplier to change the coeficients
static int sampleCount = 0;             // sample count for circular buffer
static unsigned int filter_type = 0;    // FIR filter type, can be low_pass, high_pass, average, band_pass, band_stop
static const unsigned int number_of_filter = 5; // number of FIR filter created


DSPAudio::DSPAudio()
{
    m_musicVolume = 0.8f;
    m_bypass = false;
    m_dopplerLevel = 1.0f;
    m_distanceFactor = 1.0f;
    m_pauseChannels = false;
    m_musicFilterActive = false;
    m_switchFrequency = false;
    m_changeChannelFrequency = true;
}

DSPAudio::~DSPAudio()
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
void DSPAudio::FmodErrorCheck(FMOD_RESULT result)
{
    if (m_result != FMOD_OK) {
        const char *errorString = FMOD_ErrorString(result);
        //MessageBox(NULL, errorString, "FMOD Error", MB_OK);
        // Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
    }
}


// DSP callback function for circular buffer
FMOD_RESULT F_CALLBACK FIRConvolutionDSPCallback(FMOD_DSP_STATE *dsp_state,
                                                 float *inbuffer,
                                                 float *outbuffer,
                                                 unsigned int length,
                                                 int inchannels,
                                                 int *outchannels)
{
    FMOD::DSP *thisdsp = (FMOD::DSP *)dsp_state->instance;

    //set circular buffer
    if (buffer == NULL) {
        buffer = (float*)malloc(bufferSize * sizeof(float) * inchannels);
    }

    unsigned int samp, chan;

    // change the current FIR Filter coeficients using filter_type to get different coefficients
    double *fir_filter;
    switch (filter_type) {
        case 0:
            fir_filter = averaging_filter;
            break;
        case 1:
            fir_filter = low_pass_filter;
            break;
        case 2:
            fir_filter = high_pass_filter;
            break;
        case 3:
            fir_filter = band_pass_filter;
            break;
        case 4:
            fir_filter = band_stop_filter;
            break;
        default:
            break;
    }

    for (samp = 0; samp < length; samp++)
    {
        for (chan = 0; chan < *outchannels; chan++)
        {

            // type 1, using circualar buffer to apply an FIR filter
            buffer[(sampleCount * inchannels) % bufferSize + chan] = inbuffer[(samp * inchannels) + chan];

            if (sampleCount < FILTER_NUM) // wait for the buffer to have enough samples (FILTER_NUM)
                outbuffer[(samp * inchannels) + chan] = 0;
            else {
                // applying filter after we have enough samples, this includes filter coeficients
                float tmp = 0;
                for (int i=0; i <FILTER_NUM; i++){
                    tmp += fir_filter[i] * buffer[(sampleCount * inchannels) % bufferSize + chan];
                }
                outbuffer[(samp * inchannels) + chan] = multiplier * tmp;
            }


            /*
             // type 2,  using circualar buffer to apply an FIR filter
             buffer[((samp % FILTER_NUM) * *outchannels) +chan] = inbuffer[(samp * inchannels) + chan];

             // convolution, where the filtering happens
             float tmp = 0;
             for (int i=0; i <FILTER_NUM; i++){
                tmp += buffer[(samp * FILTER_NUM) % FILTER_NUM + chan] * fir_filter[i];
             }
             // add the result together
             outbuffer[(samp * *outchannels) + chan] = multiplier * tmp;
             */

        }

        sampleCount++;
    }

    return FMOD_OK;
}

/*
    Initialise the DSP object, passing in the doppler, distanceFactor and distanceRolloff values
 */
bool DSPAudio::Initialise(GLfloat &doppler, GLfloat &distFactor, GLfloat &distRolloff)
{
    // 1) Create an FMOD system
    m_result = FMOD::System_Create(&m_FmodSystem);

    // 2) Initialise the system
    m_result = m_FmodSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, 0);


    // 3) Set the FMOD 3D settings using the given doppler, distFactor, distRolloff.
    // doppler inntesity
    // dist factor changes the boundary between the sound and camera
    // roll off scales the sound from high when very low, and low when it is very high
    m_dopplerLevel = doppler;
    m_distanceFactor = distFactor;
    m_result = m_FmodSystem->set3DSettings(doppler, distFactor, distRolloff);


    // 4) Create the DSP effect
    {
        FMOD_DSP_DESCRIPTION dspdesc;
        memset(&dspdesc, 0, sizeof(dspdesc));

        strncpy(dspdesc.name, "My first DSP unit", sizeof(dspdesc.name));
        dspdesc.numinputbuffers = 1;
        dspdesc.numoutputbuffers = 1;
        dspdesc.read = FIRConvolutionDSPCallback; // using the circular buffer callback function

        m_result = m_FmodSystem->createDSP(&dspdesc, &m_dsp);
        return false;
    }


    return true;
}
/*

*/
// Load our event sounds for 3D sound
bool DSPAudio::LoadEventSound(const char *filename1, const char *filename2)
{
    //5) Load event sound
    m_result = m_FmodSystem->createSound(filename1, FMOD_LOOP_NORMAL, 0, &m_eventSound1);
    m_result = m_FmodSystem->createSound(filename2, FMOD_LOOP_NORMAL, 0, &m_eventSound2);

    return true;
}

/*
    Play event sounds for 3D sound, pasing in the position of the helicopter and car, including the velocity vectors
*/
bool DSPAudio::PlayEventSound(glm::vec3 &position1, glm::vec3 &position2, glm::vec3 &velocity)
{
    //5) Load event sound
    m_result = m_FmodSystem->playSound(m_eventSound1, 0, false, &m_eventChannel1);
    m_result = m_FmodSystem->playSound(m_eventSound2, 0, false, &m_eventChannel2);

    //6) play through a 3D channel.
    m_eventChannel1->setMode(FMOD_3D);
    m_eventChannel2->setMode(FMOD_3D);


    //7) set the position of the car and the helicopter as well as the velocity of the helicopter
    ToFMODVector(position1, &m_helicopterPosition);
    ToFMODVector(velocity, &m_helicopterVelocity);
    ToFMODVector(position2, &m_racingCarPosition);
    m_result = m_eventChannel1->set3DAttributes(&m_helicopterPosition, &m_helicopterVelocity);
    m_result = m_eventChannel2->set3DAttributes(&m_racingCarPosition, 0, 0);

    return true;
}

// Load a music stream for custom dsp
bool DSPAudio::LoadMusicStream(const char *filename)
{
    m_result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);

    // create a low-pass filter DSP object
    m_result = m_FmodSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &m_musicFilter);

    // you can start the DSP in an inactive state
    m_musicFilter->setActive(false);

    return true;
}

// Play a music stream for custom dsp
bool DSPAudio::PlayMusicStream()
{
    m_result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);

    // Set the volume
    m_result = m_musicChannel->setVolume(m_musicVolume);

    // create dsp
    m_musicChannel->addDSP(0, m_dsp);

    // enables or disable the bypass which the DSP unit so that it does or doesn't process the data coming into it.
    // the sound continues to run regardless, but the effect on the sound is applied when the bypass is disable
    m_result = m_dsp->setBypass(m_bypass);

    // connecting the music filter to the music stream
    // 1) Get the DSP head and it's input
    m_musicChannel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &m_musicDSPHead);
    m_musicDSPHead->getInput(0, &m_musicDSPHeadInput, NULL);
    // 2) Disconnect them
    m_musicDSPHead->disconnectFrom(m_musicDSPHeadInput);
    // 3) Add input to the music head from the filter
    m_result = m_musicDSPHead->addInput(m_musicFilter);

    // 4) Add input to the filter head music DSP head input
    m_result = m_musicFilter->addInput(m_musicDSPHeadInput);

    // set the DSP object to be active
    m_musicFilter->setActive(m_musicFilterActive);
    // initially set the cutoff to a high value
    m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
    // this state is used for toggling
    m_switchFrequency = false;

    return true;
}

/*
    Update the position and veliocity of the helicopter, car and the camera
 */
void DSPAudio::Update(CCamera *camera, glm::vec3 &position1, glm::vec3 &position2, glm::vec3 &velocity, GLfloat &speed)
{
    // 7) update the helicopter position and velocity using 3d attributes
    ToFMODVector(position1, &m_helicopterPosition);
    ToFMODVector(velocity, &m_helicopterVelocity);
    //m_result = m_eventSound1->set3DMinMaxDistance(0.5f * m_distanceFactor, 100.0f * m_distanceFactor);
    m_result = m_eventChannel1->set3DAttributes(&m_helicopterPosition, &m_helicopterVelocity);
    m_result = m_eventChannel1->setPaused(m_pauseChannels);

    // 7) update the helicopter eventChannel, changign the frequency of the helicopter sound to a dynamic filter control effect.
    // 44100 is the original frequency of the helicopter sound
    float frequency = m_changeChannelFrequency ? glm::clamp(254 * speed, 11000.0f, 88000.0f) : 44100;
    m_result = m_eventChannel1->setFrequency(frequency);

    // 7) update the car position using 3d attributes
    ToFMODVector(position2, &m_racingCarPosition);
    //m_result = m_eventSound2->set3DMinMaxDistance(0.5f * m_distanceFactor, 100.0f * m_distanceFactor);
    m_result = m_eventChannel2->set3DAttributes(&m_racingCarPosition, 0, 0);
    m_result = m_eventChannel2->setPaused(m_pauseChannels);

    // 8) update the camera position using the 3d listener for the occlusion
    glm::vec3 cameraForward = camera->GetForward();
    ToFMODVector(cameraForward, &m_cameraForward);

    glm::vec3 cameraUp = camera->GetUp();
    ToFMODVector(cameraUp, &m_cameraUp);

    glm::vec3 camPosition = camera->GetPosition();
    ToFMODVector(camPosition, &m_cameraPosition);

    glm::vec3 camVelocity = camera->GetVelocity();
    ToFMODVector(camVelocity, &m_cameraVelocity);

    m_result = m_FmodSystem->set3DListenerAttributes(0,
                                                     &m_cameraPosition,
                                                     &m_cameraVelocity,
                                                     &m_cameraForward,
                                                     &m_cameraUp);

    m_FmodSystem->update();

}

/*
 Toggle the music Filter Frequency value
 */
void DSPAudio::ToggleMusicFilterFrequency()
{
    // called externally from Game::ProcessEvents
    // toggle the effect on/off
    m_switchFrequency = !m_switchFrequency;
    if (m_switchFrequency) {
        // set the parameter to a low value
        m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 700);
    }
    else {
        // set the parameter to a high value
        // you could also use m_musicFilter->setBypass(true) instead...
        m_musicFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
    }
}

/*
    increase the volume of the music channel
*/
void DSPAudio::IncreaseMusicVolume()
{
    // called externally from Game::KeyBoardControls
    // increment the volume
    m_musicVolume += 0.05f;
    if (m_musicVolume > 1)
        m_musicVolume = 1.0f;

    m_musicChannel->setVolume(m_musicVolume);
}

/*
 decrease the volume of the music channel
*/
void DSPAudio::DecreaseMusicVolume()
{
    // called externally from Game::KeyBoardControls
    // deccrement the volume
    m_musicVolume -= 0.05f;
    if (m_musicVolume < 0)
        m_musicVolume = 0.0f;

    m_musicChannel->setVolume(m_musicVolume);
}


/*
    a scaler multiplier which increases the coeficient of the current FIR filter
*/
void DSPAudio::IncreaseCoefficients()
{
    // called externally from Game::KeyBoardControls
    // increment the filter coefficients by 0.05f
    multiplier += 0.05f;
    if (multiplier > 1.0f)
        multiplier = 1.0f;

}

/*
 a scaler multiplier which decreases the coeficient of the current FIR filter
*/
void DSPAudio::DecreaseCoefficients()
{
    // called externally from Game::ProcessEvents
    // deccrement the filter coefficients by 0.05f
    multiplier -= 0.05f;
    if (multiplier < 0)
        multiplier = 0.0f;
}

/*
    toggle the byPass to enable or disable the filtering on the sound channel
*/
void DSPAudio::ToggleByPass() {
    m_bypass = !m_bypass;
    m_result = m_dsp->setBypass(m_bypass);
}

/*
    switch between the filter coeficients
*/
void DSPAudio::ToggleFilterCoefficients() {
    filter_type = (filter_type + 1) % number_of_filter;
}

/*
    enable or disable the event sound from m_eventChannel1
*/
void DSPAudio::TogglePauseChannels() {
    m_pauseChannels = !m_pauseChannels;
}

/*
    enable or disable the musicFilter which temporarily deactivates the music stream
*/
void DSPAudio::ToggleMusicFilter() {
    m_musicFilterActive = !m_musicFilterActive;
    m_musicFilter->setActive(m_musicFilterActive);
}

/*
    switch between the musicFilter frequency value from 700 to 22000
*/
void DSPAudio::ToggleChannelFrequency() {
    m_changeChannelFrequency = !m_changeChannelFrequency;
}

/*
    convert vector to FMOD_Vector
*/
void DSPAudio::ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec)
{
    fmodVec->x = glVec3.x;
    fmodVec->y = glVec3.y;
    fmodVec->z = glVec3.z;
}

/*
    creating a cube which with the bounds of the penthouse in the scene,
    this is to create the occlusion effect for the penthouse when going in and out of the penthouse.
*/
void DSPAudio::AddCube(glm::vec3 &position, GLfloat &width,  GLfloat &height, GLfloat &depth)
{

    FMOD::Geometry *geometry;
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

    m_result = geometry->addPolygon(1.0f, 1.0f, true, 8, cubePoly, &polyIndex);

    FMOD_VECTOR cubePosition;
    ToFMODVector(position, &cubePosition);
    geometry->setPosition(&cubePosition);
    geometry->setActive(true);

}

/*
    creating a terran or plane which with the bounds of the terrain in the scene,
    this is to create the occlusion effect for the ground bellow the ground.
 */
void DSPAudio::CreateTerrain(glm::vec3 &position, const float &size)
{
    FMOD::Geometry *geometry;

    m_result = m_FmodSystem->createGeometry(1, 4, &geometry);

    float halfSize = size / 2;
    FMOD_VECTOR groundPoly[4];
    groundPoly[0] = { -halfSize, position.y, -halfSize }; // left back
    groundPoly[1] = { -halfSize, position.y, halfSize }; // left front
    groundPoly[2] = { halfSize, position.y, -halfSize };  // right back
    groundPoly[3] = { halfSize, position.y, -halfSize };    // right front
    int polyIndex = 0;

    m_result = geometry->addPolygon(1.0f, 1.0f, true, 4, groundPoly, &polyIndex);

    FMOD_VECTOR terrainPosition;
    ToFMODVector(position, &terrainPosition);
    geometry->setPosition(&terrainPosition);
    geometry->setActive(true);
}

/*
    get the value of the volume
 */
GLfloat DSPAudio::Volume() const {
    return  m_musicVolume;
}

/*
    get the value of the FIR filter coefficient multiplier
 */
GLfloat DSPAudio::FIRFilterMultiplier() const {
    return multiplier;
};

/*
    get the value that tells you whether the event sound is enabled or disabled
*/
GLboolean DSPAudio::PauseChannels() const {
    return  m_pauseChannels;
}

/*
    get the value that tells you whether the music filter for music channel is enabled or disabled
*/
GLboolean DSPAudio::MusicFilterActive() const {
    return  m_musicFilterActive;
}

/*
    get the value that tells you what frequency the music filter has
*/
GLboolean DSPAudio::MusicFilterFrequency() const {
    return  m_switchFrequency;
}

/*
    get the value that tells you whether the bypass for the FIR filter is enable or disable
 */
GLboolean DSPAudio::ByPassFIRFilters() const {
    return  m_bypass;
}

/*
    get the frequency value the eventChannel1
*/
GLfloat DSPAudio::ChannelFrequency() const {
    float frequency;
    m_eventChannel1->getFrequency(&frequency);
    return frequency;
}

/*
    get the type of FIR filter that is currently active
*/
const char * DSPAudio::FIRFilter() const {
    switch (filter_type) {
        case 0:
            return "Averaging";
        case 1:
            return "Low Pass";
        case 2:
            return "High Pass";
        case 3:
            return "Band Pass";
        case 4:
            return "Band Stop";
        default:
            return "";
    }
}

