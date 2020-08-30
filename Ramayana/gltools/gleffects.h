#ifndef __GLEFFECTS_H
#define __GLEFFECTS_H

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

using namespace std;

namespace gltools {

	void motionBlur(float blur);

}

#endif