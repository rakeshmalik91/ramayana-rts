#ifndef __TEXTURE_H
#define __TEXTURE_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <cmath>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <dirent.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include "math.h"
#include "array.h"

using namespace std;
using namespace math;

namespace graphics {

	class TextureNotLoaded : public Exception {
	public:
		TextureNotLoaded(string fname) : Exception("Texture \"" + fname + "\" not loaded"){}
	};

	class TextureCopiedException : public Exception {
	public:
		TextureCopiedException(string fname) : Exception("Texture \"" + fname + "\" copied"){}
	};

	class Texture2D {
		int width, height;
		int actualHeight, actualWidth;
		GLuint texture;
		unsigned char *data;
		int nChannels;
		bool loaded, generated;
	public:
		GLenum getColorFormat() const;
		GLenum getColorComponent() const;
	public:
		string path;
		Texture2D();
		Texture2D(const Texture2D&);
		Texture2D& operator=(const Texture2D&);
		void make(IplImage* image_original, string name = "unnamed", bool gen = true, GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, int width = -1, int height = -1);
		void load(const char* path, bool gen = true, GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, int width = -1, int height = -1);
		Texture2D(const char* path, bool gen = true, GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, int width = -1, int height = -1);
		void bind();
		void bind(GLint, GLint, GLint, GLint);
		static void bindNone();
		void generate(GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);
		bool isLoaded();
		GLuint getID();
		Color getColorAt(float x, float y) const;
		int getWidth() const;
		int getHeight() const;
		int getActualWidth() const;
		int getActualHeight() const;
		void unload();
		~Texture2D();
	};

	class Texture3D {
		string dir;
		int width, height, depth;
		unsigned char *data;
		GLuint texture;
		bool loaded, generated;
	public:
		Texture3D();
		void load(string dir, int width, int height, int depth, bool gen = true, GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, GLint wrapR = GL_CLAMP);
		void generate(GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, GLint wrapR = GL_CLAMP);
		void bind();
		void bind(GLint, GLint, GLint, GLint, GLint);
		static void bindNone();
		void unload();
		~Texture3D();
	};

	class Texture2DAnimated {
		bool loaded, generated;
		Texture2D *frame;
		int nFrame;
	public:
		Texture2DAnimated();
		Texture2DAnimated(const char* dirname, bool gen = true, GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);
		void load(const char* dirname, bool gen = true, GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);
		void generate(GLint magFilter = GL_LINEAR, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);
		void bind(int);
		void bind(int, GLint, GLint, GLint, GLint);
		int length();
		void unload();
		~Texture2DAnimated();
	};
}

#endif