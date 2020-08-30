/*****************************************************************************************************
* Subject                   : OpenGL premitives                                                     *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include <cstdio>

#include "glpremitive.h"

namespace gltools {

	void glDrawRectangle(float x1, float y1, float x2, float y2, float z) {
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);	glVertex3f(x1, y1, z);
		glTexCoord2f(0, 0);	glVertex3f(x1, y2, z);
		glTexCoord2f(1, 0);	glVertex3f(x2, y2, z);
		glTexCoord2f(1, 1);	glVertex3f(x2, y1, z);
		glEnd();
	}
	void glDrawWireRectangle(float x1, float y1, float x2, float y2, float z) {
		glBegin(GL_LINE_STRIP);
		glVertex3f(x1, y1, z);
		glVertex3f(x1, y2, z);
		glVertex3f(x2, y2, z);
		glVertex3f(x2, y1, z);
		glVertex3f(x1, y1, z);
		glEnd();
	}
	void glDrawFullScreenRectangle() {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-1, 1, 1, -1, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDrawRectangle(-1, -1, 1, 1);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
}