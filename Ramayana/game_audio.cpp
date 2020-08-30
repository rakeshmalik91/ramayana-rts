#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"

namespace ramayana {

#define MIN_MUSIC_CHANGE_DELAY			30000
#define MIN_COMBAT_RATING_FOR_COMBAT	15
		
	void Game::initSound() {
		playAudio("audio/ambience/water.ogg", CHANNEL_AMBIENCE_WATER, 0, MAX_AUDIBLE_SOUND_DIST, -1);
		playAudio("audio/ambience/forest.ogg", CHANNEL_AMBIENCE_FOREST, 0, MAX_AUDIBLE_SOUND_DIST, -1);
		
		if (paused) {
			pauseGameAudio();
		}

		musicType = MUSIC_COMBAT;
		lastMusicCahngeTime = LONG_MIN;

		setSoundSettings();
	}
	int Game::soundAngle(float x, float y) {
		int a=Line2D(Point2D(x, y), Point2D(camX, camY)).tangent();
		if(camX-x<0) a+=180;
		a+=rotation+270;
		return (a+360)%360;
	}
	float Game::soundDistance(float x, float y) {
		return dist(Point2D(x, y), Point2D(camX, camY))/MAX_AUDIBLE_SOUND_DIST;
	}
	void Game::setAmbienceSound() {
		// Water Ambience
		float waterDist = MAX_AUDIBLE_SOUND_DIST;
		Point2D waterPos;
		for (int r = (camY - MAX_AUDIBLE_SOUND_DIST / 2 > 0) ? (camY - MAX_AUDIBLE_SOUND_DIST / 2) : 0; r < height && r<camY + MAX_AUDIBLE_SOUND_DIST / 2; r++) {
			for (int c = (camX - MAX_AUDIBLE_SOUND_DIST / 2>0) ? (camX - MAX_AUDIBLE_SOUND_DIST / 2) : 0; c < width && c < camX + MAX_AUDIBLE_SOUND_DIST / 2; c++) {
				if (z[r][c] < currentWaterLevel) {
					float d = dist(Point2D(camX, camY), Point2D(c, r));
					if (d < waterDist) {
						waterDist = d;
						waterPos = Point2D(c, r);
					}
				}
			}
		}
		if (waterDist < MAX_AUDIBLE_SOUND_DIST) {
			setSoundPosition(CHANNEL_AMBIENCE_WATER, soundAngle(waterPos.x, waterPos.y), soundDistance(waterPos.x, waterPos.y));
			setSoundSettings();
		}
		//Forest Ambience
		float forestDist;
		Point2D forestPos;
		for(int u=0; u<nUnit; u++) {
			if(unit[u].isAlive() && unitTypeInfo[unit[u].type].category==UNIT_TREE) {
				float d=dist(Point2D(camX, camY), Point2D(unit[u].x, unit[u].y));
				if(d<waterDist) {
					waterDist=d;
					waterPos=Point2D(unit[u].x, unit[u].y);
				}
			}
		}
		if (waterDist<MAX_AUDIBLE_SOUND_DIST) {
			setSoundPosition(CHANNEL_AMBIENCE_FOREST, soundAngle(waterPos.x, waterPos.y), soundDistance(waterPos.x, waterPos.y));
			setSoundSettings();
		}
	}
	void Game::setMusic() {
		if (SDL_GetTicks() - lastMusicCahngeTime > MIN_MUSIC_CHANGE_DELAY || !isPlaying(CHANNEL_MUSIC)) {
			//decide combat rating
			int combatRating = 0;
			if (musicType == MUSIC_COMBAT) {
				combatRating += 14;
			}
			for (UnitID u = 0; u < nUnit; u++) {
				if (unit[u].state == STATE_ATTACKING) {
					int rating = (unit[u].getTypeInfo().isHeroic ? 5 : unit[u].getTypeInfo().isWorker ? 0 : 1);
					if (unit[u].isRenderable()) {
						rating *= 2;
					}
					combatRating += rating;
				}
			}
			//set music if needed
			vector<string> files;
			if (combatRating >= MIN_COMBAT_RATING_FOR_COMBAT) {
				if (musicType != MUSIC_COMBAT || !isPlaying(CHANNEL_MUSIC)) {
					files = getFilePaths("audio/music/combat");
					musicType = MUSIC_COMBAT;
					stopAudio(CHANNEL_MUSIC);
					playAudio(randomVectorElement(files), CHANNEL_MUSIC);
					setSoundSettings();
					lastMusicCahngeTime = SDL_GetTicks();
				}
			} else {
				if (musicType != MUSIC_GENERAL || !isPlaying(CHANNEL_MUSIC)) {
					files = getFilePaths("audio/music/general");
					musicType = MUSIC_GENERAL;
					stopAudio(CHANNEL_MUSIC);
					playAudio(randomVectorElement(files), CHANNEL_MUSIC);
					setSoundSettings();
					lastMusicCahngeTime = SDL_GetTicks();
				}
			}
		}
	}
}
