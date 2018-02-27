#pragma once
#include "Common.h"

class CAudio
{
public:
	CAudio();
	~CAudio();
	bool Initialise();
	bool LoadEventSound(char *filename);
	bool PlayEventSound();
	bool LoadMusicStream(char *filename);
	bool PlayMusicStream();
	void Update();

private:
		

	void FmodErrorCheck(FMOD_RESULT result);


	FMOD_RESULT result;
	FMOD::System *m_FmodSystem;	// the global variable for talking to FMOD
	FMOD::Sound *m_eventSound;

	
	FMOD::Sound *m_music;
	FMOD::Channel* m_musicChannel;

};
