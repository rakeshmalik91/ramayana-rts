#ifndef __SHADER_H
#define __SHADER_H

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
	class ShaderCompilationError : public Exception {
	public:
		ShaderCompilationError(string file, string shader, string infolog) : Exception("Shader compilation error : " + file + " (" + shader + ") : " + infolog) {}
	};
	class ShaderLinkError : public Exception {
	public:
		ShaderLinkError(string file, string infolog) : Exception("Shader link error : " + infolog) {}
	};
	class ProgramLoadError : public Exception {
	public:
		ProgramLoadError(string file) : Exception("Program load error : " + file) {}
	};

	class ShaderProgram {
		string fname;
		GLuint program;
		map<string, GLint> uniformLocation, attribLocation;
	private:
		GLuint compile(GLenum, const char*, GLint);
		void link(GLuint, GLuint);
	public:
		GLuint getID() const { return program; }
		void loadFromString(string, string&);
		void load(string);
		void use() const;
		static void useNone();
		GLuint getUniformLocation(string);
		GLuint getAttribLocation(string);
		void enableVertexAttribArray(string);
		void disableVertexAttribArray(string);
		~ShaderProgram();
	};

}

#endif