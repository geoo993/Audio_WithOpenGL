#pragma once
#include "Common.h"
#include "Camera.h"

class CAudio
{
public:
	CAudio();
	~CAudio();
	bool Initialise();
    bool InitialiseWithDSPEffect();
    bool InitialiseWith3DSettings();
	bool LoadEventSound(const char *filename);
	bool PlayEventSound();
    bool PlayEventSoundUsingEventChannel();
	bool LoadMusicStream(const char *filename);
    bool LoadMusicStreamUsingLowPassFilter(const char *filename);
	bool PlayMusicStreamUsingFilter();
    bool PlayMusicStreamUsingDSP();
    void ToggleMusicFilter();

    void IncreaseMusicVolume();
    void DecreaseMusicVolume();
    void Update(const CCamera *camera );

private:
    
	void FmodErrorCheck(FMOD_RESULT result);
    void ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec);

	FMOD_RESULT result;
	FMOD::System *m_FmodSystem;	// the global variable for talking to FMOD

    FMOD::Sound *m_eventSound;
    FMOD::Channel *m_eventChannel;

    FMOD::DSP *m_dsp;
	
	FMOD::Sound *m_music;

    FMOD::DSP *m_musicFilter;
    bool m_musicFilterActive;

    FMOD::Channel* m_musicChannel;

    FMOD::DSP *m_musicDSPHead;
    FMOD::DSP *m_musicDSPHeadInput;

    GLfloat m_musicVolume = 0.2f;

    FMOD_VECTOR camPos;
    FMOD_VECTOR eventPos;
    FMOD_VECTOR eventVel;

};
