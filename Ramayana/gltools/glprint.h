#ifndef __GLPRINT_H
#define __GLPRINT_H

#include <string>
#include <iostream>
#include <cstdarg>
#include <cstring>
#include <map>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "graphics.h"

using namespace std;

namespace gltools {
	
	enum TextAlignment {CENTRE_ALIGNMENT, LEFT_ALIGNMENT, RIGHT_ALIGNMENT};

	struct glPrintfModiier {
		enum Type{DOWN, RIGHT} type;
		int value;
		glPrintfModiier(Type type, int value):type(type),value(value){}
	};
	struct glPrint {
		int x, y, w, h, startX;
		TextAlignment alignment;
		void *font;
		glPrint(int x, int y, void* font);
		glPrint(int x, int y, int w, int h, void* font);
		glPrint& operator<<(string);
		glPrint& operator<<(const char*);
		glPrint& operator<<(int);
		glPrint& operator<<(float);
		glPrint& operator<<(bool);
		glPrint& operator<<(Point2D);
		glPrint& operator<<(Point3D);
		glPrint& operator<<(glPrintfModiier);
		glPrint& operator<<(TextAlignment);
		static glPrintfModiier down(int);
	};

	int glPrintWidth(void *font, string s);
	int glPrintHeight(void *font, string s);

}

#endif