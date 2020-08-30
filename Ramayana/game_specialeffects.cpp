#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"
#include "steamStatsAndAchievements.h"

using namespace rapidxml;
using namespace math;
using namespace graphics;


namespace ramayana {
	void Game::loadTextureList(vector<Texture2D>& tex, string dirname, bool gen, GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT) {
		vector<string> filenames = getFiles(dirname);
		tex.resize(filenames.size());
		for (int i = 0; i<filenames.size(); i++)
			tex[i].load((dirname + "/" + filenames[i]).data(), gen, magFilter, minFilter, wrapS, wrapT);
	}
	void Game::loadParticles() {
		particleTexture.blue.load("special/blue_particle.png", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		particleTexture.rainbow.load("special/random_particle.png", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		particleTexture.yellow.load("special/yellow_particle.png", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		particleTexture.white.load("special/white_particle.png", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		particleTexture.poison.load("special/poison_particle.png", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);

		loadTextureList(particleTexture.fire, "special/fire_particle", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		loadTextureList(particleTexture.smoke, "special/smoke_particle", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		loadTextureList(particleTexture.dustHeavy, "special/heavy_dust_particle", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		loadTextureList(particleTexture.dustLight, "special/light_dust_particle", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		loadTextureList(particleTexture.waterHeavy, "special/heavy_water_particle", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		loadTextureList(particleTexture.waterLight, "special/light_water_particle", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
		loadTextureList(particleTexture.rain, "special/rain_particle", true, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP);
	}

	void Game::initFire() {
		fireIntensity = allocate<int>(width, height, 0);
		fireParticleEngine.bind(&particleRenderer);
		fireParticleEngine.set(
			Point3D(0, 0, 0.1),
			0,
			particleTexture.fire.data(), particleTexture.fire.size(),
			Range<float>(0, 359), Range<float>(88, 90),
			0.3, 0.01,
			0.02,
			0,
			0,
			Color(1, 1, 1, 1),
			0.05);
		smokeParticleEngine.bind(&particleRenderer);
		smokeParticleEngine.set(
			Point3D(0, 0, 0.2),
			200,
			particleTexture.smoke.data(), particleTexture.smoke.size(),
			Range<float>(0, 359), Range<float>(75, 90),
			0.25, 0.005,
			0.04,
			0,
			0,
			Color(1, 1, 1, 1),
			0.1);
	}
	void Game::updateFire() {
		for (int u = 0; u<nUnit; u++) {
			if (Point2D(unit[u].x, unit[u].y).in(0, 0, width - 1, height - 1)) {
				float ground = clampLow(getGroundHeight(unit[u].x, unit[u].y), currentWaterLevel) + 0.1;
				if (fireIntensity[roundInt(unit[u].y)][roundInt(unit[u].x)]>0 && frustum.pointInFrustum(Point3D(unit[u].x, unit[u].y, ground))) {
					unit[u].burn(fireIntensity[roundInt(unit[u].y)][roundInt(unit[u].x)]);
					smokeParticleEngine.setPosition(Point3D(unit[u].x, unit[u].y, ground));
					if (teams[playerTeam].visible[roundInt(unit[u].y)][roundInt(unit[u].x)])
						smokeParticleEngine.emit();
				}
			}
		}
		string sound[] = { "audio/fire0.ogg", "audio/fire1.ogg", "audio/fire2.ogg", "audio/fire3.ogg" };
		for (int r = 0; r<height; r++) {
			for (int c = 0; c<width; c++) {
				if (fireIntensity[r][c]>0) {
					float ground = clampLow(getGroundHeight(c, r), currentWaterLevel) + 0.1;
					if (teams[playerTeam].visible[r][c] && frustum.pointInFrustum(Point3D(c, r, ground))) {
						fireParticleEngine.setLifeSpan(clamp(fireIntensity[r][c] * 5, 10, 50));
						for (int i = 0; i<2; i++) {
							fireParticleEngine.setPosition(Point3D(c + choice(-1.0, 1.0), r + choice(-1.0, 1.0), ground));
							fireParticleEngine.emit();
						}
						if (fireIntensity[r][c] % 13 == 0) {
							playAudio(randomArrayElement(sound), CHANNEL_FIRE_SOUND, soundAngle(c, r), soundDistance(c, r));
						}
					}
					fireIntensity[r][c]--;
				}
			}
		}
	}
	void Game::addFire(Point3D p, int radius, int intensity) {
		int x0 = p.x, y0 = p.y;
		int f = 1 - radius;
		int ddF_x = 1;
		int ddF_y = -2 * radius;
		int x = 0;
		int y = radius;
		while (x <= y) {
			if (intensity>0) {
				if (x0 + x<width)	for (int r = (y0 - y<0) ? 0 : (y0 - y), c = x0 + x; r <= y0 + y && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) : (max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) / 2);
				if (x0 + y<width)	for (int r = (y0 - x<0) ? 0 : (y0 - x), c = x0 + y; r <= y0 + x && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) : (max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) / 2);
				if (x0 - y >= 0)     for (int r = (y0 - x<0) ? 0 : (y0 - x), c = x0 - y; r <= y0 + x && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) : (max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) / 2);
				if (x0 - x >= 0)     for (int r = (y0 - y<0) ? 0 : (y0 - y), c = x0 - x; r <= y0 + y && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) : (max(intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius)), (float)fireIntensity[r][c]) / 2);
			} else {
				if (x0 + x<width)	for (int r = (y0 - y<0) ? 0 : (y0 - y), c = x0 + x; r <= y0 + y && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) : (positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) / 2);
				if (x0 + y<width)	for (int r = (y0 - x<0) ? 0 : (y0 - x), c = x0 + y; r <= y0 + x && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) : (positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) / 2);
				if (x0 - y >= 0)     for (int r = (y0 - x<0) ? 0 : (y0 - x), c = x0 - y; r <= y0 + x && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) : (positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) / 2);
				if (x0 - x >= 0)     for (int r = (y0 - y<0) ? 0 : (y0 - y), c = x0 - x; r <= y0 + y && r<height; r++)	fireIntensity[r][c] = (z[r][c]>currentWaterLevel) ? positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) : (positiveSubtract(fireIntensity[r][c], -intensity*(positiveSubtract(1, dist(Point2D(x0, y0), Point2D(c, r)) / radius))) / 2);
			}
			if (f >= 0) {
				y--;
				ddF_y += 2;
				f += ddF_y;
			}
			x++;
			ddF_x += 2;
			f += ddF_x;
		}
	}
	void Game::removeFire(Point3D p, int radius) {
		addFire(p, radius * 2, -999999);
		for (int u = 0; u<nUnit; u++) {
			float d = dist(p, Point3D(unit[u].x, unit[u].y, unit[u].z));
			if (d<radius)
				unit[u].extinguish();
		}
	}

	void Game::updateTornedo() {
		float tornedoSpeed = 0.05;
		//update, emit & count tornedo
		for (int i = 0; i<MAX_TORNEDO; i++) {
			if (tornedo[i].time>0) {
				float waterLevel = clampLow(getGroundHeight(tornedo[i].pos.x, tornedo[i].pos.y), currentWaterLevel);
				//Extinguish fire
				if (waterLevel<currentWaterLevel)
					addFire(Point3D(tornedo[i].pos.x, tornedo[i].pos.y, waterLevel), 6, -500);
				else
					addFire(Point3D(tornedo[i].pos.x, tornedo[i].pos.y, waterLevel), 3, -100);
				for (int u = 0; u<nUnit; u++)
				if (dist(tornedo[i].pos, Point2D(unit[u].x, unit[u].y))<3)
					unit[u].hitTornedo(tornedo[i].pos);
				//update
				tornedo[i].time--;
				tornedo[i].pos.x += choice(0.0, tornedoSpeed)*tornedo[i].dir.x;
				tornedo[i].pos.y += choice(0.0, tornedoSpeed)*tornedo[i].dir.y;
				if (!tornedo[i].pos.in(0, 0, width - 1, height - 1)) {
					tornedo[i].time = 0;
					continue;
				}
				//emit
				tornedo[i].particleEngine.setPosition(Point3D(tornedo[i].pos.x, tornedo[i].pos.y, waterLevel));
				tornedo[i].particleEngineDust.setPosition(Point3D(tornedo[i].pos.x, tornedo[i].pos.y, waterLevel));
				tornedo[i].particleEngineWater.setPosition(Point3D(tornedo[i].pos.x, tornedo[i].pos.y, waterLevel));
				if (teams[playerTeam].visible[roundInt(tornedo[i].pos.y)][roundInt(tornedo[i].pos.x)]) {
					tornedo[i].particleEngine.emit();
					tornedo[i].particleEngineDust.emit();
					if (tornedo[i].time % 10 == 0) {
						string sound[] = { "audio/tornedo.ogg" };
						playAudio(randomArrayElement(sound), CHANNEL_WIND_SOUND, soundAngle(tornedo[i].pos.x, tornedo[i].pos.y), soundDistance(tornedo[i].pos.x, tornedo[i].pos.y));
					}
					setSoundPosition(CHANNEL_AMBIENCE_WATER, soundAngle(camX, camY), soundDistance(camX, camY));
					if (waterLevel <= currentWaterLevel) {
						tornedo[i].particleEngineWater.emit();
						if (tornedo[i].time % 10 == 0) {
							string sound[] = { "audio/waterfalls0.ogg" };
							playAudio(randomArrayElement(sound), CHANNEL_WATER_SOUND, soundAngle(tornedo[i].pos.x, tornedo[i].pos.y), soundDistance(tornedo[i].pos.x, tornedo[i].pos.y));
						}
					}
				}
				//Water Splash
				if (waterLevel<currentWaterLevel)
					addWaterSplash(tornedo[i].pos, 1, 4);
			}
		}
	}
	void Game::addTornedo(Point2D pos, Point2D dir, int time) {
		int index = 0;
		for (; index<MAX_TORNEDO; index++)
		if (tornedo[index].time <= 0)
			break;
		if (index<MAX_TORNEDO) {
			tornedo[index] = Tornedo(pos, dir, time);
			tornedo[index].particleEngine.set(pos, 100, particleTexture.dustHeavy.data(), particleTexture.dustHeavy.size(), Range<float>(0, 359), Range<float>(85, 90), 0.5, 0.015, 0.125, 0, 30, Color(1, 1, 1), 1);
			tornedo[index].particleEngine.bind(&particleRenderer);
			tornedo[index].particleEngineDust.set(pos, 50, particleTexture.dustLight.data(), particleTexture.dustLight.size(), Range<float>(0, 359), Range<float>(0, 75), 1.0, 0.05, 0.125, 0.1, 30, Color(1, 1, 1), 1);
			tornedo[index].particleEngineDust.bind(&particleRenderer);
			tornedo[index].particleEngineWater.set(pos, 50, particleTexture.waterLight.data(), particleTexture.waterLight.size(), Range<float>(0, 359), Range<float>(45, 85), 1.0, 0.05, Range<float>(0.25, 0.5), 1.0, 5, Color(1, 1, 1), 5);
			tornedo[index].particleEngineWater.bind(&particleRenderer);
		}
	}

	void Game::updateSpecialEffect() {
		//water spalshes
		for (int i = 0; i<MAX_COLLISION; i++) {
			if (waterSplash[i].time>0) {
				if (teams[playerTeam].visible[roundInt(waterSplash[i].pos.y)][roundInt(waterSplash[i].pos.x)]) {
					waterSplash[i].particleEngine.emit();
				}
				if (waterSplash[i].time % 10 == 9) {
					string sound[] = { "audio/waterfalls0.ogg" };
					playAudio(randomArrayElement(sound), CHANNEL_WATER_SOUND, soundAngle(waterSplash[i].pos.x, waterSplash[i].pos.y), soundDistance(waterSplash[i].pos.x, waterSplash[i].pos.y));
				}
				waterSplash[i].time--;
			}
		}
		//dust blows
		for (int i = 0; i<MAX_COLLISION; i++) {
			if (dustBlow[i].time>0) {
				if (teams[playerTeam].visible[roundInt(dustBlow[i].pos.y)][roundInt(dustBlow[i].pos.x)]) {
					dustBlow[i].particleEngine.emit();
				}
				dustBlow[i].time--;
			}
		}
		//blood splatters
		for (int i = 0; i<MAX_COLLISION; i++) {
			if (bloodSplatter[i].time>0) {
				if (teams[playerTeam].visible[roundInt(bloodSplatter[i].pos.y)][roundInt(bloodSplatter[i].pos.x)]) {
					bloodSplatter[i].particleEngine.emit();
				}
				bloodSplatter[i].time--;
			}
		}
		//poison
		for (int i = 0; i<MAX_POISON; i++) {
			if (poison[i].time>0) {
				if (teams[playerTeam].visible[roundInt(poison[i].pos.y)][roundInt(poison[i].pos.x)]) {
					poison[i].particleEngine.emit();
				}
				poison[i].time--;
			}
		}
		//shockwaves
		static const float shockWaveSpeed = 0.25;
		for (int i = 0; i<MAX_COLLISION; i++) {
			if (shockwave[i].radius < shockwave[i].maxRadius) {
				shockwave[i].radius += shockWaveSpeed;
			}
		}
	}
	void Game::addWaterSplash(Point2D pos, int time, float size) {
		if (!pos.in(0, 0, width - 1, height - 1))
			return;
		int index = 0;
		for (; index < MAX_COLLISION; index++) {
			if (waterSplash[index].time <= 0) {
				break;
			}
		}
		int rootSize = sqrt(size);
		if (index<MAX_COLLISION) {
			waterSplash[index] = ParticleEffect(pos, size, 0);
			waterSplash[index].particleEngine.set(
				Point3D(pos.x, pos.y, clampLow(getGroundHeight(pos.x, pos.y), currentWaterLevel)),
				10 * size,
				particleTexture.waterHeavy.data(), particleTexture.waterHeavy.size(),
				Range<float>(0, 359), Range<float>(15, 75),
				0.01*size, 0.0005,
				Range<float>(0.3*size, 0.5*size),
				2,
				0,
				Color(1, 1, 1, 1),
				5 * size*size);
			waterSplash[index].particleEngine.bind(&particleRenderer);
			waterSplash[index].time = time;
			if (size>5) {
				string sound[] = { "audio/water_splash_big0.ogg", "audio/water_splash_big1.ogg", "audio/water_splash_big2.ogg" };
				playAudio(randomArrayElement(sound), CHANNEL_WATER_SOUND, soundAngle(pos.x, pos.y), soundDistance(pos.x, pos.y));
			} else {
				string sound[] = { "audio/water_splash_small0.ogg", "audio/water_splash_small1.ogg" };
				playAudio(randomArrayElement(sound), CHANNEL_WATER_SOUND, soundAngle(pos.x, pos.y), soundDistance(pos.x, pos.y));
			}
		} else {
			getLogger().print("Water Splash Limit reached", LOG_SEVERE);
		}
	}
	void Game::addDustBlow(Point3D pos, int time, float size) {
		if (weather.weatherType == WEATHER_RAINY || weather.weatherType == WEATHER_STORMY) {
			addWaterSplash(pos, time, size);
		}

		if (!pos.in(0, 0, width - 1, height - 1)) {
			return;
		}
		int index = 0;
		for (; index < MAX_COLLISION; index++) {
			if (dustBlow[index].time <= 0) {
				break;
			}
		}
		if (index<MAX_COLLISION) {
			dustBlow[index] = ParticleEffect(pos, size, 0);
			dustBlow[index].particleEngine.set(
				pos,
				10 * size,
				particleTexture.dustLight.data(), particleTexture.dustLight.size(),
				Range<float>(0, 359), Range<float>(15, 60),
				0.5*size, 0.005,
				0.25,
				0.1,
				0,
				Color(1, 1, 1, 1),
				3.0*size);
			dustBlow[index].particleEngine.bind(&particleRenderer);
			dustBlow[index].time = time;
			if (size>5) {
				string sound[] = { "audio/dust_blow_small0.ogg", "audio/dust_blow_small1.ogg", "audio/dust_blow_small2.ogg" };
				playAudio(randomArrayElement(sound), CHANNEL_DUST_SOUND, soundAngle(pos.x, pos.y), soundDistance(pos.x, pos.y));
			} else {
				string sound[] = { "audio/dust_blow_small0.ogg", "audio/dust_blow_small1.ogg", "audio/dust_blow_small2.ogg" };
				playAudio(randomArrayElement(sound), CHANNEL_DUST_SOUND, soundAngle(pos.x, pos.y), soundDistance(pos.x, pos.y));
			}
		} else {
			getLogger().print("Dust blow Limit reached", LOG_SEVERE);
		}
	}
	void Game::addBloodSplatter(Point3D pos, float size) {
		if (!pos.in(0, 0, width - 1, height - 1)) {
			return;
		}
		int index = 0;
		for (; index < MAX_COLLISION; index++) {
			if (bloodSplatter[index].time <= 0) {
				break;
			}
		}
		if (index<MAX_COLLISION) {
			bloodSplatter[index] = ParticleEffect(pos, size, 0);
			bloodSplatter[index].particleEngine.set(
				pos,
				10 * size,
				&particleTexture.white, 1,
				Range<float>(0, 359), Range<float>(-15, 15),
				Range<float>(0.001*size, 0.005*size), 0.005,
				0.125,
				1.0,
				0,
				Color(0.8, 0, 0, 1),
				1.0*size);
			bloodSplatter[index].particleEngine.bind(&particleRenderer);
			bloodSplatter[index].time = 1;
		} else {
			getLogger().print("Blood Splatter Limit reached", LOG_SEVERE);
		}
	}
	void Game::addPoison(Point3D pos, int time, float size) {
		if (!pos.in(0, 0, width - 1, height - 1)) {
			return;
		}
		int index = 0;
		for (; index < MAX_POISON; index++) {
			if (poison[index].time <= 0) {
				break;
			}
		}
		if (index<MAX_POISON) {
			poison[index] = ParticleEffect(pos, size, 0);
			poison[index].particleEngine.set(
				pos,
				20 * size,
				&particleTexture.poison, 1,
				Range<float>(0, 359), Range<float>(0, 10),
				Range<float>(0.7, 1.0), 0.0005,
				0.1,
				0.0,
				0,
				Color(1, 1, 1, 1),
				size);
			poison[index].particleEngine.bind(&particleRenderer);
			poison[index].time = time;
		} else {
			getLogger().print("Poison Limit reached", LOG_SEVERE);
		}
	}
	void Game::addShockWave(Point3D pos, float radius) {
		if (!pos.in(0, 0, width - 1, height - 1)) {
			return;
		}
		int index = 0;
		for (; index < MAX_COLLISION; index++) {
			if (shockwave[index].radius >= shockwave[index].maxRadius) {
				break;
			}
		}
		if (index < MAX_COLLISION) {
			shockwave[index] = ShockWave(pos, radius);
		} else {
			getLogger().print("Poison Limit reached", LOG_SEVERE);
		}
	}

	void Game::_makeProjectileTrail() {
		for (int p = 0; p<nProjectile; p++) {
			if (!projectile[p].hit && !paused) {
				switch (projectile[p].weaponType) {
				case WEAPON_ARROW:
					if (projectile[p].areaDamageRadius > 0) {
						if (projectile[p].particleEngine == NULL) {
							projectile[p].particleEngine = new ParticleEngine();
							projectile[p].particleEngine->bind(&particleRenderer);
							projectile[p].particleEngine->set(Point3D(), 1, &particleTexture.white, 1, projectile[p].hAngle + 180, projectile[p].vAngle + 180, 0.05, -0.005, 0.05, 0.1, 0, Color(1, 1, 1, 1), 1);
						}
						projectile[p].particleEngine->setPosition(projectile[p].pos);
						projectile[p].particleEngine->emitTrail(projectile[p].lastpos, 50);
					}
					break;
				case WEAPON_SPECIAL_ARROW_1:
					if (projectile[p].particleEngine == NULL) {
						projectile[p].particleEngine = new ParticleEngine();
						projectile[p].particleEngine->bind(&particleRenderer);
						projectile[p].particleEngine->set(Point3D(), 30, &particleTexture.white, 1, Range<float>(0, 360), Range<float>(0, 360), 0.05, 0.005, 0.25, 0.5, 0, Color(1, 1, 1, 1), 5);
					}
					projectile[p].particleEngine->setPosition(projectile[p].pos);
					projectile[p].particleEngine->emitTrail(projectile[p].lastpos, 10);
					break;
				case WEAPON_SPECIAL_ARROW_2:
					if (projectile[p].particleEngine == NULL) {
						projectile[p].particleEngine = new ParticleEngine();
						projectile[p].particleEngine->bind(&particleRenderer);
						projectile[p].particleEngine->set(Point3D(), 20, &particleTexture.blue, 1, projectile[p].hAngle + 180, projectile[p].vAngle + 180, 0.25, 0.005, 0.05, 0.1, 0, Color(1, 1, 1, 1), 1);
					}
					projectile[p].particleEngine->setPosition(projectile[p].pos);
					projectile[p].particleEngine->emitTrail(projectile[p].lastpos, 3);
					break;
				case WEAPON_SPIKE:
				case WEAPON_POISON_ARROW:
					if (projectile[p].particleEngine == NULL) {
						projectile[p].particleEngine = new ParticleEngine();
						projectile[p].particleEngine->bind(&particleRenderer);
						projectile[p].particleEngine->set(Point3D(), 5, &particleTexture.poison, 1, Range<float>(0, 360), Range<float>(0, 360), 0.25, 0.005, 0.05, 0.5, 0, Color(1, 1, 1, 1), 1);
					}
					projectile[p].particleEngine->setPosition(projectile[p].pos);
					projectile[p].particleEngine->emitTrail(projectile[p].lastpos, 10);
					break;
				case WEAPON_DARK_ARROW:
					if (projectile[p].particleEngine == NULL) {
						projectile[p].particleEngine = new ParticleEngine();
						projectile[p].particleEngine->bind(&particleRenderer);
						projectile[p].particleEngine->set(Point3D(), 5, &particleTexture.white, 1, projectile[p].hAngle + 180, projectile[p].vAngle + 180, 0.25, -0.005, 0.05, 0.1, 0, Color(0, 0, 0, 1), 1);
					}
					projectile[p].particleEngine->setPosition(projectile[p].pos);
					projectile[p].particleEngine->emitTrail(projectile[p].lastpos, 10);
					break;
				case WEAPON_WIND_ARROW:
					if (projectile[p].particleEngine == NULL) {
						projectile[p].particleEngine = new ParticleEngine();
						projectile[p].particleEngine->bind(&particleRenderer);
						projectile[p].particleEngine->set(Point3D(), 2, &particleTexture.white, 1, Range<float>(0, 360), Range<float>(0, 360), 0.1, 0.00, 0.25, 0.0, 30, Color(1, 1, 1, 1), 1);
					}
					projectile[p].particleEngine->setPosition(projectile[p].pos);
					projectile[p].particleEngine->emit();
					break;
				case WEAPON_WATER_ARROW:
					if (projectile[p].particleEngine == NULL) {
						projectile[p].particleEngine = new ParticleEngine();
						projectile[p].particleEngine->bind(&particleRenderer);
						projectile[p].particleEngine->set(Point3D(), 20, particleTexture.waterHeavy.data(), particleTexture.waterHeavy.size(), Range<float>(0, 360), Range<float>(0, 360), 0.1, 0.005, 0.5, 1.0, 0, Color(1, 1, 1, 1), 5);
					}
					projectile[p].particleEngine->setPosition(projectile[p].pos);
					projectile[p].particleEngine->emitTrail(projectile[p].lastpos, 4);
					break;
				case WEAPON_FIRE_ARROW:
					if (projectile[p].particleEngine == NULL) {
						projectile[p].particleEngine = new ParticleEngine();
						projectile[p].particleEngine->bind(&particleRenderer);
						projectile[p].particleEngine->set(Point3D(), 1, particleTexture.fire.data(), particleTexture.fire.size(), projectile[p].hAngle + 180, projectile[p].vAngle + 180, 0.075, -0.005, 0.05, 0.1, 0, Color(1, 1, 1, 1), 1);
					}
					projectile[p].particleEngine->setPosition(projectile[p].pos);
					projectile[p].particleEngine->emitTrail(projectile[p].lastpos, 25);
					break;
				}
			}
		}
	}
	void Game::_hitProjectile(ProjectileWeapon& p, int unitIndex) {
		//do regular damage
		if (unitIndex >= 0) {
			p.pos = Point3D(unit[unitIndex].x, unit[unitIndex].y, unit[unitIndex].z);
			unit[unitIndex].hit(p);
		}
		//do area damage
		if (p.areaDamageRadius>0) {
			for (int u = 0; u<nUnit; u++) {
				if (u != unitIndex) {
					float d = dist(p.pos, Point3D(unit[u].x, unit[u].y, unit[u].z));
					if (d < p.areaDamageRadius) {
						unit[u].hit(p);
					}
				}
			}
		}
		//bind particle engine
		ParticleEngine particleEngine;
		particleEngine.bind(&particleRenderer);
		//Do special effects
		switch (p.weaponType) {
		case WEAPON_ARROW:
			if (p.areaDamageRadius>1 || unitIndex<0) {
				addDustBlow(p.pos, 1, p.areaDamageRadius);
				if (p.pos.z<currentWaterLevel)
					addWaterSplash(p.pos, 1, p.areaDamageRadius);
			}
			break;
		case WEAPON_FIRE_ARROW:
			addFire(p.pos, p.areaDamageRadius, 100 * p.areaDamageRadius * p.areaDamageRadius);
			if (p.areaDamageRadius>2) {
				particleEngine.set(p.pos, 15, &particleTexture.yellow, 1, Range<float>(0, 360), 0, 1.0, 0.1, 0.25, 0.1, 0, Color(1, 1, 1), 200);
				particleEngine.emit();
				if (p.pos.z < currentWaterLevel && p.team == playerTeam) {
					if (getSteamAchievements())
						getSteamAchievements()->SetAchievement("ACH_FIRE_IN_WATER");
				}
			}
			playAudio("audio/agnivaan_hit.ogg", CHANNEL_SOUND, soundAngle(p.pos.x, p.pos.y), soundDistance(p.pos.x, p.pos.y));
			break;
		case WEAPON_WIND_ARROW:
			addTornedo(p.pos, directionVector(p.lastpos, p.pos), 500);
			removeFire(p.pos, p.areaDamageRadius);
			particleEngine.set(p.pos, 15, particleTexture.dustLight.data(), particleTexture.dustLight.size(), Range<float>(0, 360), 0, 0.7, 0.2, 0.3, 0.1, 30, Color(1, 1, 1), 200);
			particleEngine.emit();
			break;
		case WEAPON_WATER_ARROW:
			addWaterSplash(p.pos, 3, p.areaDamageRadius);
			removeFire(p.pos, p.areaDamageRadius);
			particleEngine.set(p.pos, 15, particleTexture.waterLight.data(), particleTexture.waterLight.size(), Range<float>(0, 360), 0, 0.5, 0.1, 0.3, 0.1, 0, Color(1, 1, 1), 100);
			particleEngine.emit();
			playAudio("audio/varunvaan_hit.ogg", CHANNEL_SOUND, soundAngle(p.pos.x, p.pos.y), soundDistance(p.pos.x, p.pos.y));
			break;
		case WEAPON_POISON_ARROW:
			playAudio("audio/naagapasa_hit.ogg", CHANNEL_SOUND, soundAngle(p.pos.x, p.pos.y), soundDistance(p.pos.x, p.pos.y));
			break;
		}
		//Add unit if contains (eg: nagapasa conatins 100 snakes)
		for (int i = 0; i < p.addUnit_number; i++) {
			Point2D offset = rotatePoint(choice(0, 359), Point2D(choice(0, p.areaDamageRadius), 0));
			addUnit(p.pos.x + offset.x, p.pos.y + offset.y, p.addUnit_id, 0, choice(0, 359));
		}
		//Steam Achievements
		if (p.weaponType == WEAPON_SPECIAL_ARROW_2) {
			if (p.pos.z < currentWaterLevel && p.team == playerTeam) {
				if (getSteamAchievements())
					getSteamAchievements()->SetAchievement("ACH_USE_BRAHMASTRA");
			}
		}
	}
	void Game::_addProjectile(ProjectileWeapon p) {

		if (nProjectile<MAX_PROJECTILE) {
			projectile[nProjectile++] = p;
		} else {
			int i = 0;
			for (; i<nProjectile; i++)
			if (projectile[i].hit)
				break;
			if (i<MAX_PROJECTILE) {
				projectile[i].~ProjectileWeapon();
				projectile[i] = p;
				if (i == nProjectile)
					nProjectile++;
			}
		}
	}
	void Game::addProjectile(ProjectileWeapon p) {
		Synchronizer sync(projectileMutex);
		_addProjectile(p);
	}
	void Game::updateProjectile() {
		Synchronizer sync(projectileMutex);

		for (int p = 0; p<nProjectile; p++) {																	//Update ProjectileWeapon
			bool erased = false;
			if (!projectile[p].hit) {
				//Special effects
				switch (projectile[p].weaponType) {
				case WEAPON_REPLICATE_ARROW:
					_addProjectile(ProjectileWeapon(projectile[p].team, projectile[p].pos, projectile[p].vAngle, projectile[p].hAngle, projectile[p].v, projectile[p].weight, projectile[p].attack, projectile[p].siegeAttack, projectile[p].areaDamageRadius, WEAPON_ARROW));
					for (int i = 1; i<projectile[p].s; i++) {
						_addProjectile(ProjectileWeapon(projectile[p].team, projectile[p].pos, projectile[p].vAngle, projectile[p].hAngle + 5, projectile[p].v, projectile[p].weight, projectile[p].attack, projectile[p].siegeAttack, projectile[p].areaDamageRadius, WEAPON_ARROW));
						_addProjectile(ProjectileWeapon(projectile[p].team, projectile[p].pos, projectile[p].vAngle, projectile[p].hAngle - 5, projectile[p].v, projectile[p].weight, projectile[p].attack, projectile[p].siegeAttack, projectile[p].areaDamageRadius, WEAPON_ARROW));
					}
					break;
				}
				//Hitting ProjectileWeapon
				if (!projectile[p].pos.in(0, 0, width - 1, height - 1) || projectile[p].pos.z<getGroundHeight(projectile[p].pos.x, projectile[p].pos.y)) {
					//Hit ground
					projectile[p].hit = true;
					projectile[p].pos.z = getGroundHeight(projectile[p].pos.x, projectile[p].pos.y);
					_hitProjectile(projectile[p], -1);
				} else {
					projectile[p].move();
					for (int u = 0; u<nUnit; u++) {
						if (unit[u].state != STATE_DEAD
							&& unit[u].team != 0
							&& diplomacy(projectile[p].team, unit[u].team) != DIPLOMACY_ALLY
							&& unit[u].inRangeOf(projectile[p])) {
							//Hit Unit
							_hitProjectile(projectile[p], u);
							projectile[p].hit = true;
							projectile[p].pos.z = -100;
						}
					}
				}
			}
		}
	}

	void Game::initWeather() {
		if (isEditable()) {
			weather.weatherType = WEATHER_CLEAR;
		} else {
			WeatherType w[] = { WEATHER_CLEAR, WEATHER_WINDY, WEATHER_RAINY, WEATHER_STORMY };
			float probability[] = { 0.80f, 0.05f, 0.10f, 0.05f };
			setWeather(choice(w, probability, 4));
		}
		if (paused)
			pauseGameAudio();
	}
	void Game::updateWeather() {
		static unsigned int counter = 0;
		counter = (counter + 1) % UINT_MAX;

		//Weather change
		if (counter % 1000 == 0 && satisfiesInProbability(0.02))
			initWeather();

		//Wind
		wind_flow_phase = (wind_flow_phase + 10) % 360;
		int nTornedo = 0;
		for (int i = 0; i<MAX_TORNEDO; i++)
		if (tornedo[i].time>0)
			nTornedo++;
		if (nTornedo>0) {
			if (weather.weatherType == WEATHER_CLEAR)
				weather.weatherType = WEATHER_WINDY;
			else if (weather.weatherType == WEATHER_RAINY)
				weather.weatherType = WEATHER_STORMY;
		}
		if (weather.weatherType == WEATHER_WINDY || weather.weatherType == WEATHER_STORMY) {
			wind_flow = _sin[wind_flow_phase] * 0.05;
			weather.brightness = -0.1;
			weather.contrast = 0.05;
		} else {
			wind_flow = _sin[wind_flow_phase] * 0.01;
			weather.brightness = 0;
			weather.contrast = 0;
		}

		//Rain
		if (counter % 500 == 0 && weather.weatherType == WEATHER_RAINY || weather.weatherType == WEATHER_STORMY)
			weather.rainParticleEngine.setDensity(choice(0.01, 0.05));

		//Thunder
		if (weather.weatherType == WEATHER_RAINY || weather.weatherType == WEATHER_STORMY) {
			if (weather.thunderTimeRemaining>0) {
				if (satisfiesInProbability(0.4)) {
					weather.brightness = choice(0.5, 1.0);
					weather.contrast = choice(0.1, 0.2);
				}
				weather.thunderTimeRemaining--;
			} else {
				if (weather.weatherType == WEATHER_RAINY && satisfiesInProbability(0.01)
					|| weather.weatherType == WEATHER_STORMY && satisfiesInProbability(0.03)) {
					weather.thunderTimeRemaining = choice(5, 20);
					string sound[] = { "audio/thunder0.ogg", "audio/thunder1.ogg", "audio/thunder2.ogg", "audio/thunder3.ogg" };
					playAudio(randomArrayElement(sound), CHANNEL_THUNDER_SOUND);
				}
			}
		}
	}
	void Game::setWeather(WeatherType newtype) {
		weather.weatherType = newtype;
		if (weather.weatherType == WEATHER_WINDY || weather.weatherType == WEATHER_STORMY)
			playAudio("audio/ambience/wind.ogg", CHANNEL_AMBIENCE_WIND, 0, 0, -1);
		else
			stopAudio(CHANNEL_AMBIENCE_WIND);
		if (weather.weatherType == WEATHER_RAINY || weather.weatherType == WEATHER_STORMY)
			playAudio("audio/ambience/rain.ogg", CHANNEL_AMBIENCE_RAIN, 0, 0, -1);
		else
			stopAudio(CHANNEL_AMBIENCE_RAIN);
		if (weather.weatherType == WEATHER_STORMY)
			playAudio("audio/thunder0.ogg", CHANNEL_THUNDER_SOUND);
	}

	void Game::initRain() {
		weather.rainParticleEngine.bind(&particleRenderer);
		weather.rainParticleEngine.set(
			Point3D(),
			20,
			particleTexture.rain.data(), particleTexture.rain.size(),
			Range<float>(0, 359), Range<float>(260, 280),
			Range<float>(0.05, 0.15), 0.0,
			Range<float>(1.0, 2.0),
			0.1,
			0,
			Color(1, 1, 1, 1),
			choice(0.005, 0.025));
		weather.rainParticleEngine.setFade(false);
	}
	void Game::updateRain() {
		if (weather.weatherType == WEATHER_RAINY || weather.weatherType == WEATHER_STORMY) {
			for (int r = camY - 25; r<camY + 25; r++)
			for (int c = camX - 25; c<camX + 25; c++) {
				weather.rainParticleEngine.setPosition(Point3D(c, r, 20));
				weather.rainParticleEngine.emit();
			}
			for (int i = 0; i<height*width / 1000; i++)
				addWaterSplash(Point2Di(choice(0, width - 1), choice(0, height - 1)), 1, 1);
		}
	}
}
