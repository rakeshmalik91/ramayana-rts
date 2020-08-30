#ifndef __GLTRANSFORM_H
#define __GLTRANSFORM_H

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

	void glShearf(float sXY, float sXZ, float sYX, float sYZ, float sZX, float sZY);
}

#endif