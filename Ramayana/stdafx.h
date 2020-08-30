// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#if defined _MSC_VER
	#pragma once
	#include "targetver.h"
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

// C Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#if defined _MSC_VER
	#include <direct.h>
#elif defined __GNUC__
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

// C++ Header Files
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <map>

// OpenGL Header Files
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#ifndef _WINDOWS_
	#include "GL/glx.h"
#endif

// OpenCV Header Files
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv/cv.hpp>
#include <opencv2/highgui/highgui.hpp>

// SDL Header Files
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_timer.h>
#include <SDL/SDL_mixer.h>
//#include <SDL/SDL_net.h>

// RapidXML Header Files
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
using namespace rapidxml;

// Steam SDK
#include <steam/steam_api.h>

// Local Header Files
#include "gltools/io.h"
#include "gltools/array.h"
#include "gltools/string.h"
#include "gltools/exception.h"
using namespace std;
#include "gltools/math.h"
#include "gltools/random.h"
using namespace math;
#include "gltools/graphics.h"
#include "gltools/texture.h"
#include "gltools/object.h"
#include "gltools/surface.h"
using namespace graphics;
#include "gltools/glprint.h"
#include "gltools/gltransform.h"
#include "gltools/glpremitive.h"
#include "gltools/glkey.h"
#include "gltools/gleffects.h"
#include "gltools/glsupported.h"
#include "gltools/framebuffer.h"
#include "gltools/shader.h"
using namespace gltools;
#include "gltools/particle.h"
using namespace physics;
#include "gltools/algorithm.h"
using namespace algorithm;
#include "gltools/thread.h"
using namespace thread;


#define _SDL_THREAD
#define _GLUT_CALLBACK
