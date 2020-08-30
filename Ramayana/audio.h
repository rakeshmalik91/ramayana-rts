#ifndef __RAMAYANA_AUDIO_H
#define __RAMAYANA_AUDIO_H

#include "enums.h"
#include "unit.h"

#define MAX_AUDIBLE_SOUND_DIST		100

namespace ramayana {
	
	static map<string, Mix_Chunk*> sounds;

	void closeSDLAudio();
	void initSDLAudio();
	int playAudio(string fname, ChannelTag grp, int angle=0, float distance=0.0, int loops=0);
	void stopAudio(ChannelTag);
	void playAmbienceSounds();
	void setSoundPosition(ChannelTag grp, int angle, float distance);
	void pauseGameAudio();
	void resumeGameAudio();

	void setMusicVolume(float);
	void setAmbientVolume(float);
	void setSoundVolume(float);
	void setSpeechVolume(float);

	bool isPlaying(ChannelTag grp);
};

#endif