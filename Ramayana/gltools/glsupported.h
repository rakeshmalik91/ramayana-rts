#ifndef __GLSUPPORTED_H
#define __GLSUPPORTED_H

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
	bool isEXTSupported(const char *ext);
	bool setUpVertexBuffer();
	bool setUp_EXT_vertex_array();
	bool setUp_ARB_vertex_buffer_object();
	bool setUp_ARB_vertex_program();
	bool setUp_EXT_vertex_attrib_64bit();
	bool setUpFogCoord();
	bool setUp_EXT_fog_coord();
	bool setUp_EXT_texture3D();
	bool setUp_EXT_texture_object();
	bool setUpMultiTexture();
	bool setUp_ARB_multitexture();
	bool setUp_ARB_multisample();
	void setUpGLShader();
	bool setUp_ARB_shader_objects();
	bool setUp_EXT_framebuffer_object();
	bool setUp_EXT_geometry_shader4();
	bool setUp_EXT_texture_array();
}

#endif