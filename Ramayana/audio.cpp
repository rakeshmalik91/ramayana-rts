#include "stdafx.h"

#include "common.h"
#include "game.h"
#include "audio.h"

using namespace std;

namespace ramayana {

#define N_CHANNELS		100

	int getFirstChannel(ChannelTag grp) {
		switch (grp) {
		case CHANNEL_MUSIC:				return  0;
		case CHANNEL_AMBIENCE_WATER:	return  2;
		case CHANNEL_AMBIENCE_FOREST:	return  3;
		case CHANNEL_AMBIENCE_WIND:		return  4;
		case CHANNEL_AMBIENCE_RAIN:		return  5;
		case CHANNEL_DUST_SOUND:		return  6;
		case CHANNEL_WIND_SOUND:		return 20;
		case CHANNEL_FIRE_SOUND:		return 25;
		case CHANNEL_WATER_SOUND:		return 30;
		case CHANNEL_THUNDER_SOUND:		return 50;
		case CHANNEL_SPEECH:			return 60;
		default:						return 70; //CHANNEL_SOUND
		}
	}
	int getLastChannel(ChannelTag grp) {
		switch (grp) {
		case CHANNEL_MUSIC:				return  1;
		case CHANNEL_AMBIENCE_WATER:	return  2;
		case CHANNEL_AMBIENCE_FOREST:	return  3;
		case CHANNEL_AMBIENCE_WIND:		return  4;
		case CHANNEL_AMBIENCE_RAIN:		return  5;
		case CHANNEL_DUST_SOUND:		return 19;
		case CHANNEL_WIND_SOUND:		return 24;
		case CHANNEL_FIRE_SOUND:		return 29;
		case CHANNEL_WATER_SOUND:		return 49;
		case CHANNEL_THUNDER_SOUND:		return 59;
		case CHANNEL_SPEECH:			return 69;
		default:						return 99; //CHANNEL_SOUND
		}
	}

	void closeSDLAudio() {
		if(Game::Settings::noSound)
			return;
		for(map<string, Mix_Chunk*>::iterator i=sounds.begin(); i!=sounds.end(); i++)
			Mix_FreeChunk(i->second);
		sounds.clear();
		Mix_CloseAudio();
		Mix_Quit();
		SDL_Quit();
	}
	void initSDLAudio() {
		if(SDL_Init(SDL_INIT_AUDIO)) {
			Game::Settings::noSound=true;
			showMessage(SDL_GetError(), "SDL ERROR");
			return;
		}
		int audio_rate=22050;
		Uint16 audio_format=AUDIO_S16SYS;
		int audio_channels=2;
		int audio_buffers=4096;
		if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
			Game::Settings::noSound=true;
			showMessage(Mix_GetError(), "SDL ERROR");
			return;
		}
		Mix_AllocateChannels(100);
		Mix_GroupChannels(getFirstChannel(CHANNEL_MUSIC), getLastChannel(CHANNEL_MUSIC), CHANNEL_MUSIC);
		Mix_GroupChannels(getFirstChannel(CHANNEL_AMBIENCE_WATER), getLastChannel(CHANNEL_AMBIENCE_WATER), CHANNEL_AMBIENCE_WATER);
		Mix_GroupChannels(getFirstChannel(CHANNEL_AMBIENCE_FOREST), getLastChannel(CHANNEL_AMBIENCE_FOREST), CHANNEL_AMBIENCE_FOREST);
		Mix_GroupChannels(getFirstChannel(CHANNEL_AMBIENCE_WIND), getLastChannel(CHANNEL_AMBIENCE_WIND), CHANNEL_AMBIENCE_WIND);
		Mix_GroupChannels(getFirstChannel(CHANNEL_AMBIENCE_RAIN), getLastChannel(CHANNEL_AMBIENCE_RAIN), CHANNEL_AMBIENCE_RAIN);
		Mix_GroupChannels(getFirstChannel(CHANNEL_DUST_SOUND), getLastChannel(CHANNEL_DUST_SOUND), CHANNEL_DUST_SOUND);
		Mix_GroupChannels(getFirstChannel(CHANNEL_WIND_SOUND), getLastChannel(CHANNEL_WIND_SOUND), CHANNEL_WIND_SOUND);
		Mix_GroupChannels(getFirstChannel(CHANNEL_FIRE_SOUND), getLastChannel(CHANNEL_FIRE_SOUND), CHANNEL_FIRE_SOUND);
		Mix_GroupChannels(getFirstChannel(CHANNEL_WATER_SOUND), getLastChannel(CHANNEL_WATER_SOUND), CHANNEL_WATER_SOUND);
		Mix_GroupChannels(getFirstChannel(CHANNEL_THUNDER_SOUND), getLastChannel(CHANNEL_THUNDER_SOUND), CHANNEL_THUNDER_SOUND);
		Mix_GroupChannels(getFirstChannel(CHANNEL_SPEECH), getLastChannel(CHANNEL_SPEECH), CHANNEL_SPEECH);
		Mix_GroupChannels(getFirstChannel(CHANNEL_SOUND), getLastChannel(CHANNEL_SOUND), CHANNEL_SOUND);
	}
	int playAudio(string fname, ChannelTag grp, int angle, float distance, int loops) {
		if(Game::Settings::noSound)
			return -1;
		int channel=-1;
		Mix_Chunk *sound;
		if(sounds[fname]==NULL)
			sound=Mix_LoadWAV(fname.data());
		else
			sound=sounds[fname];
		if(sound==NULL) {
			showMessage("\""+string(Mix_GetError())+"\" playing "+fname, "SDL ERROR");
			return -1;
		}
		sounds[fname]=sound;
		channel = Mix_GroupAvailable(grp);
		if(channel<0)
			channel = Mix_GroupOldest(grp);
		if(channel>=0) {
			channel=Mix_PlayChannel(channel, sound, loops);
			Mix_SetPosition(channel, angle%360, clamp(distance, 0.0, 1.0)*255);
		}
		return channel;
	}
	void stopAudio(ChannelTag grp) {
		Mix_FadeOutGroup(grp, 3000);
	}

	void setSoundPosition(ChannelTag grp, int angle, float distance) {
		if(Game::Settings::noSound)
			return;
		for (int channel = getFirstChannel(grp); channel <= getLastChannel(grp); channel++) {
			Mix_SetPosition(channel, angle, clamp(distance, 0.0, 1.0) * 255);
		}
	}

	void setMusicVolume(float vol) {
		for(int channel=0; channel<=1; channel++)
			Mix_Volume(channel, clamp(vol, 0.0, 1.0)*128);
	}
	void setAmbientVolume(float vol) {
		for(int channel=2; channel<=59; channel++)
			Mix_Volume(channel, clamp(vol, 0.0, 1.0)*128);
	}
	void setSpeechVolume(float vol) {
		for(int channel=60; channel<79; channel++)
			Mix_Volume(channel, clamp(vol, 0.0, 1.0)*128);
	}
	void setSoundVolume(float vol) {
		for(int channel=70; channel<N_CHANNELS; channel++)
			Mix_Volume(channel, clamp(vol, 0.0, 1.0)*128);
	}
	
	void pauseGameAudio() {
		if(Game::Settings::noSound)
			return;
		for(int channel=2; channel<N_CHANNELS; channel++)
			Mix_Pause(channel);
	}
	void resumeGameAudio() {
		if(Game::Settings::noSound)
			return;
		for(int channel=2; channel<N_CHANNELS; channel++)
			Mix_Resume(channel);
	}
	
	bool isPlaying(ChannelTag grp) {
		for (int channel = getFirstChannel(grp); channel < getLastChannel(grp); channel++) {
			if (Mix_Playing(channel)) {
				return true;
			}
		}
		return false;
	}
};
