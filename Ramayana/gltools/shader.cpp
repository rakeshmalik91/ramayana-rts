/*****************************************************************************************************
 * Subject                   : Shader loading class                                                  *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
#include "string.h"
#include "shader.h"

using namespace std;

namespace gltools {

	GLuint ShaderProgram::compile(GLenum type, const GLchar *pszSource, GLint length) {
	    // Compiles the shader given it's source code. Returns the shader unitTypeInfo.
	    // A std::string unitTypeInfo containing the shader's info log is thrown if the
	    // shader failed to compile.
	    //
	    // 'type' is either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
	    // 'pszSource' is a C style string containing the shader's source code.
	    // 'length' is the length of 'pszSource'.
	
	    GLuint shader = glCreateShaderObjectARB(type);
	
	    if (shader) {
	        GLint compiled = 0;
	
	        glShaderSourceARB(shader, 1, &pszSource, &length);
	        glCompileShaderARB(shader);
	        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	
	        if (!compiled) {
	            GLsizei infoLogSize = 0;
	            string infoLog;
	
	            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
	            infoLog.resize(infoLogSize);
	            glGetShaderInfoLog(shader, infoLogSize, &infoLogSize, &infoLog[0]);
	
	            throw ShaderCompilationError(fname, type==GL_VERTEX_SHADER?"Vertex shader":type==GL_FRAGMENT_SHADER?"Fragment shader":"", infoLog);
	        }
	    }
	
	    return shader;
	}
	void ShaderProgram::link(GLuint vertShader, GLuint fragShader) {
	    // Links the compiled vertex and/or fragment shaders into an executable
	    // shader program. Returns the executable shader unitTypeInfo. If the shaders
	    // failed to link into an executable shader program, then a std::string
	    // unitTypeInfo is thrown containing the info log.
	
	    program = glCreateProgram();
	
	    if (program) {
	        GLint linked = 0;
	
	        if (vertShader)
	            glAttachShader(program, vertShader);
	
	        if (fragShader)
	            glAttachShader(program, fragShader);
	
			glLinkProgram(program);
			glGetProgramiv(program, GL_LINK_STATUS, &linked);
	
	        if (!linked) {
	            GLsizei infoLogSize = 0;
	            std::string infoLog;
	
	            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
	            infoLog.resize(infoLogSize);
	            glGetProgramInfoLog(program, infoLogSize, &infoLogSize, &infoLog[0]);
	
	            throw ShaderLinkError(fname, infoLog);
	        }
	
	        // Mark the two attached shaders for deletion. These two shaders aren't
	        // deleted right now because both are already attached to a shader
	        // program. When the shader program is deleted these two shaders will
	        // be automatically detached and deleted.
	
	        if (vertShader)
	            glDeleteShader(vertShader);
	
	        if (fragShader)
	            glDeleteShader(fragShader);
	    }
	}
	void ShaderProgram::loadFromString(string buffer, string &infoLog) {
	    infoLog.clear();
		
	    // Compile and link the vertex and fragment shaders.
	    if (buffer.length() > 0) {
	        const GLchar *pSource = 0;
	        GLint length = 0;
	        GLuint vertShader = 0;
	        GLuint fragShader = 0;
	
	        string::size_type vertOffset = buffer.find("[vert]");
	        string::size_type fragOffset = buffer.find("[frag]");
	
			// Get the vertex shader source and compile it.
	        // The source is between the [vert] and [frag] tags.
			if (vertOffset != std::string::npos) {
			    vertOffset += 6;        // skip over the [vert] tag
			    pSource = reinterpret_cast<const GLchar *>(&buffer[vertOffset]);
			    length = static_cast<GLint>(fragOffset - vertOffset);
			    vertShader = compile(GL_VERTEX_SHADER, pSource, length);
			}
			
			// Get the fragment shader source and compile it.
	        // The source is between the [frag] tag and the end of the file.
			if (fragOffset != std::string::npos) {
			    fragOffset += 6;        // skip over the [frag] tag
			    pSource = reinterpret_cast<const GLchar *>(&buffer[fragOffset]);
			    length = static_cast<GLint>(buffer.length() - fragOffset - 1);
			    fragShader = compile(GL_FRAGMENT_SHADER, pSource, length);
			}
			
			// Now link the vertex and fragment shaders into a shader program.
			link(vertShader, fragShader);
	    }
		
		if(program==0) {
			throw ProgramLoadError(fname);
		}
	}
	void ShaderProgram::load(string filename) {
		fname=filename;
	
	    //Read the text file containing the GLSL shader program.
	    //This file contains 1 vertex shader and 1 fragment shader.
	    string buffer=readTextFile(filename.data());
		
	    string infoLog;
		loadFromString(buffer, infoLog);
	}
	void ShaderProgram::use() const {
		glUseProgram(program);
	}
	void ShaderProgram::useNone() {
		glUseProgram(0);
	}
	GLuint ShaderProgram::getUniformLocation(string uniform) {
		GLint loc=uniformLocation[uniform];
		if(loc==0) {
			loc=glGetUniformLocation(program, uniform.data());
			uniformLocation[uniform]=loc;
		}
		return loc;
	}
	GLuint ShaderProgram::getAttribLocation(string attrib) {
		GLint loc=attribLocation[attrib];
		if(loc==0) {
			loc=glGetAttribLocation(program, attrib.data());
			attribLocation[attrib]=loc;
		}
		return loc;
	}
	void ShaderProgram::enableVertexAttribArray(string attrib) {
		glEnableVertexAttribArray(getAttribLocation(attrib));
	}
	void ShaderProgram::disableVertexAttribArray(string attrib) {
		glDisableVertexAttribArray(getAttribLocation(attrib));
	}
	ShaderProgram::~ShaderProgram() {
		if(program)
			glDeleteProgram(program);
	}
}
