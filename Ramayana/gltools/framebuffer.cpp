/*****************************************************************************************************
 * Subject                   : Framebuffer management class                                          *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
 #include "framebuffer.h"
 #include "graphics.h"

namespace gltools {

	FrameBuffer::FrameBuffer() : id(0), width(0), height(0), depthTexture(0), depthRenderBuffer(0) {
		for(int i=0; i<GL_MAX_COLOR_ATTACHMENTS; i++) {
			colorTexture[i]=0;
			colorRenderBuffer[i]=0;
		}
	}
	FrameBuffer::~FrameBuffer() {
		for(int i=0; i<GL_MAX_COLOR_ATTACHMENTS; i++)
			if(colorTexture[i]!=0)
				glDeleteTextures(1, &colorTexture[i]);
		if(depthTexture!=0)	
			glDeleteTextures(1, &depthTexture);
		if(depthRenderBuffer!=0)	
			glDeleteRenderbuffersEXT(1, &depthRenderBuffer);
		width=height=0;
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		if(id!=0) 
			glDeleteFramebuffersEXT(1, &id);
	}
	void FrameBuffer::checkStatus() {
		switch(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)) {
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			throw Exception("FBO Create error : GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT");
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			throw Exception("FBO Create error : GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			throw Exception("FBO Create error : GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			throw Exception("FBO Create error : GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			throw Exception("FBO Create error : GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			throw Exception("FBO Create error : GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			throw Exception("FBO Create error : GL_FRAMEBUFFER_UNSUPPORTED_EXT");
		}
	}
	void FrameBuffer::create(size_t width, size_t height) {
		this->width=width;
		this->height=height;

		glGenFramebuffersEXT(1, &id);
		
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	void FrameBuffer::dontDraw() {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	void FrameBuffer::attachDepthTexture() {
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY); 

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthTexture, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	void FrameBuffer::attachColorTexture(GLuint i) {
		i=clamp(i, 0, GL_MAX_COLOR_ATTACHMENTS_EXT-1);
		
		glGenTextures(1, &colorTexture[i]);
		glBindTexture(GL_TEXTURE_2D, colorTexture[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_2D, colorTexture[i], 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	void FrameBuffer::attachDepthTextureCube() {
		glGenTextures(1, &depthTexture);
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthTexture);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (int face=0; face<6; face++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
		for(int face=0; face<6; face++)
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, depthTexture, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	void FrameBuffer::attachColorTextureCube(GLuint i) {
		i=clamp(i, 0, GL_MAX_COLOR_ATTACHMENTS_EXT-1);
		
		glGenTextures(1, &colorTexture[i]);
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, colorTexture[i]);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for(int face=0; face<6; face++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
		//for(int face=0; face<6; face++)
			//glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, colorTexture[i], 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, colorTexture[i], 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	void FrameBuffer::setColorAttachmentTarget(GLenum textarget, GLuint i) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textarget, colorTexture[i], 0);
	}
	void FrameBuffer::bindDepthTexture() const {
		glBindTexture(GL_TEXTURE_2D, depthTexture);
	}
	void FrameBuffer::bindColorTexture(GLuint i) const {
		i=clamp(i, 0, GL_MAX_COLOR_ATTACHMENTS_EXT-1);
		glBindTexture(GL_TEXTURE_2D, colorTexture[i]);
	}
	void FrameBuffer::bindColorTextureCube(GLuint i) const {
		i=clamp(i, 0, GL_MAX_COLOR_ATTACHMENTS_EXT-1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, colorTexture[i]);
	}
	void FrameBuffer::bind() {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
	}
	void FrameBuffer::unbind() const {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	size_t FrameBuffer::getWidth() const {
		return width;
	}
	size_t FrameBuffer::getHeight() const {
		return height;
	}
	GLuint FrameBuffer::getID() const {
		return id;
	}
	void FrameBuffer::copy(const FrameBuffer &fb) {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		bind();
		glViewport(0, 0, width, height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		
		fb.bindColorTexture();
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glDrawFullScreenRectangle();
		glBindTexture(GL_TEXTURE_2D, 0);

		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		unbind();
	}
}
