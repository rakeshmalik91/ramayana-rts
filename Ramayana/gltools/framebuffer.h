#ifndef __FRAMEBUFFER_H
#define __FRAMEBUFFER_H

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
	class FrameBuffer {
		GLuint id;
		size_t width, height;
		GLuint colorTexture[GL_MAX_COLOR_ATTACHMENTS_EXT], colorRenderBuffer[GL_MAX_COLOR_ATTACHMENTS_EXT];
		GLuint depthTexture, depthRenderBuffer;
	public:
		FrameBuffer();
		~FrameBuffer();
		void checkStatus();
		void create(size_t, size_t);
		void dontDraw();
		void attachDepthTexture();
		void attachDepthTextureCube();
		void attachColorTexture(GLuint i = 0);
		void attachColorTextureCube(GLuint i = 0);
		void setColorAttachmentTarget(GLenum textarget, GLuint i = 0);
		void bindDepthTexture() const;
		void bindColorTexture(GLuint i = 0) const;
		void bindColorTextureCube(GLuint i = 0) const;
		void bind();
		void unbind() const;
		size_t getWidth() const;
		size_t getHeight() const;
		GLuint getID() const;
		void copy(const FrameBuffer&);
	};
}

#endif