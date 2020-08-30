#include "stdafx.h"

#include "world.h"
#include "sky.h"

namespace ramayana {

	void Sky::init() {
		string time[] = { "dawn", "morning", "noon", "afternoon", "evening", "dusk", "sunset", "night" };
		string currentTime = randomArrayElement(time);
		skyTexture.load(
			("special/sky/" + currentTime + ".jpg").data(),
			true,
			GL_LINEAR, GL_LINEAR,
			GL_REPEAT, GL_REPEAT);
		fogColor = skyTexture.getColorAt(0.5, 0.5);
	}

	void Sky::drawSky() {
		float ground = getWaterLevel(camX, camY);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0, 0, camZ - ground);
		glRotatef(-tilt, 1, 0, 0);
		glRotatef(rotation - 90, 0, 0, 1);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glEnable(GL_TEXTURE_2D);
		skyTexture.bind();

		GLUquadricObj *q = gluNewQuadric();
		gluQuadricNormals(q, GL_SMOOTH);
		gluQuadricTexture(q, true);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glColor4f(1, 1, 1, 1);
		gluSphere(q, WORLD_RADIUS, 32, 16);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}