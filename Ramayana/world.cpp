#include "stdafx.h"

#include "world.h"

namespace ramayana {

	const float World::BASE_WATER_LEVEL = 0.1;

	int World::getWidth() const {
		return width;
	}
	int World::getHeight() const {
		return height;
	}

	void World::initLight() {
		float ambient[] = { 0.6, 0.6, 0.6, 1.0 };
		float diffuse[] = { 0.9, 0.9, 0.9, 1.0 };
		float specular[] = { 1.0, 1.0, 1.0, 1.0 };
		float position[4] = { 0, 0, 0, 1 };
		float direction[] = { 0, 0, -1, 1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 60);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0f);
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		glEnable(GL_LIGHT0);
	}
	void World::initFog() {
		glFogi(GL_FOG_MODE, GL_LINEAR);
		float fogcolorArray[4];
		glFogfv(GL_FOG_COLOR, fogColor.rgba().toArray(fogcolorArray));
		glFogf(GL_FOG_DENSITY, 0.005f);
		glHint(GL_FOG_HINT, GL_NICEST);
		glFogf(GL_FOG_START, 20.0f);
		glFogf(GL_FOG_END, WORLD_RADIUS);
		glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
		glEnable(GL_FOG);
	}

	void World::init() {
		initLight();
		initFog();
		getLogger().print("World (light, fog) initialized.");
	}

	void World::moveUp(float v) {
		float newCamX = camX + v*_sin[(int)rotation];
		float newCamY = camY + v*_cos[(int)rotation];
		camX = clamp(newCamX, -10, width - 1 + 10);
		camY = clamp(newCamY, -10, height - 1 + 10);
		setAmbienceSound();
	}
	void World::moveRight(float v) {
		float newCamX = camX + v*_sin[(int)(rotation + 90) % 360];
		float newCamY = camY + v*_cos[(int)(rotation + 90) % 360];
		camX = clamp(newCamX, -10, width - 1 + 10);
		camY = clamp(newCamY, -10, height - 1 + 10);
		setAmbienceSound();
	}
	void World::zoomOut(float v) {
		camZ = clamp(camZ - v, isEditable() ? -50 : -15, 0);
	}
	void World::setZoom(float v) {
		camZ = clamp(-v, isEditable() ? -50 : -25, 0);
	}
	void World::tiltBack(float v) {
		tilt = clamp(tilt + v, 0, 90);
	}
	void World::rotateLeft(float v) {
		rotation = modulo(rotation + v + 360, 360);
	}
	void World::goTo(float x, float y) {
		camX = clamp(x, 0, width - 1);
		camY = clamp(y, 0, height - 1);
		setAmbienceSound();
	}
	void World::resetCamera() {
		tilt = 55;
		rotation = 0;
		if (isEditable())
			camZ = -20;
		else
			camZ = -8;
	}
	void World::setCamera() {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, camZ);
		glRotatef(-tilt, 1, 0, 0);
		glRotatef(rotation, 0, 0, 1);
		float ground = clamp(getGroundHeight(camX, camY), BASE_WATER_LEVEL, 15);
		glTranslatef(-camX, -camY, -ground - 0.1);
	}

	Point3D World::getCameraPosition() const {
		return Point3D(camX, camY, -camZ);
	}

	float World::getCurrentWaterLevel() const {
		return currentWaterLevel;
	}
}
