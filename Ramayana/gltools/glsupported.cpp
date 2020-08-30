/*****************************************************************************************************
 * Subject                   : OpenGL supported                                                      *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
#include <cstdio>

#include "string.h"
#include "glsupported.h"

#ifdef _WINDOWS_
	#define glGetProcAddress wglGetProcAddress
	#define GLUT_STRING LPCSTR
#else
	#define glGetProcAddress glXGetProcAddress
	#define GLUT_STRING const GLubyte*
#endif

namespace gltools {

	bool isEXTSupported(const char *ext) {
		const char *extnames=(const char*)glGetString(GL_EXTENSIONS);
		return strstr(extnames, ext)!=NULL;
	}
	bool setUpVertexBuffer() {
		glGenBuffers				= (PFNGLGENBUFFERSPROC)					glGetProcAddress((GLUT_STRING)"glGenBuffers"); 
		glBindBuffer				= (PFNGLBINDBUFFERPROC)					glGetProcAddress((GLUT_STRING)"glBindBuffer"); 
		glBufferData				= (PFNGLBUFFERDATAPROC)					glGetProcAddress((GLUT_STRING)"glBufferData"); 
		glDeleteBuffers				= (PFNGLDELETEBUFFERSPROC)				glGetProcAddress((GLUT_STRING)"glDeleteBuffers");
		
		glBeginQuery				= (PFNGLBEGINQUERYPROC)					glGetProcAddress((GLUT_STRING)"glBeginQuery");
		glBindBuffer				= (PFNGLBINDBUFFERPROC)					glGetProcAddress((GLUT_STRING)"glBindBuffer");
		glBufferData				= (PFNGLBUFFERDATAPROC)					glGetProcAddress((GLUT_STRING)"glBufferData");
		glBufferSubData				= (PFNGLBUFFERSUBDATAPROC)				glGetProcAddress((GLUT_STRING)"glBufferSubData");
		glDeleteBuffers				= (PFNGLDELETEBUFFERSPROC)				glGetProcAddress((GLUT_STRING)"glDeleteBuffers");
		glDeleteQueries				= (PFNGLDELETEQUERIESPROC)				glGetProcAddress((GLUT_STRING)"glDeleteQueries");
		glEndQuery					= (PFNGLENDQUERYPROC)					glGetProcAddress((GLUT_STRING)"glEndQuery");
		glGenBuffers				= (PFNGLGENBUFFERSPROC)					glGetProcAddress((GLUT_STRING)"glGenBuffers");
		glGenQueries				= (PFNGLGENQUERIESPROC)					glGetProcAddress((GLUT_STRING)"glGenQueries");
		glGetBufferParameteriv		= (PFNGLGETBUFFERPARAMETERIVPROC)		glGetProcAddress((GLUT_STRING)"glGetBufferParameteriv");
		glGetBufferPointerv			= (PFNGLGETBUFFERPOINTERVPROC)			glGetProcAddress((GLUT_STRING)"glGetBufferPointerv");
		glGetBufferSubData			= (PFNGLGETBUFFERSUBDATAPROC)			glGetProcAddress((GLUT_STRING)"glGetBufferSubData");
		glGetQueryObjectiv			= (PFNGLGETQUERYOBJECTIVPROC)			glGetProcAddress((GLUT_STRING)"glGetQueryObjectiv");
		glGetQueryObjectuiv			= (PFNGLGETQUERYOBJECTUIVPROC)			glGetProcAddress((GLUT_STRING)"glGetQueryObjectuiv");
		glGetQueryiv				= (PFNGLGETQUERYIVPROC)					glGetProcAddress((GLUT_STRING)"glGetQueryiv");
		glIsBuffer					= (PFNGLISBUFFERPROC)					glGetProcAddress((GLUT_STRING)"glIsBuffer");
		glIsQuery					= (PFNGLISQUERYPROC)					glGetProcAddress((GLUT_STRING)"glIsQuery");
		glMapBuffer					= (PFNGLMAPBUFFERPROC)					glGetProcAddress((GLUT_STRING)"glMapBuffer");
		glUnmapBuffer				= (PFNGLUNMAPBUFFERPROC)				glGetProcAddress((GLUT_STRING)"glUnmapBuffer");
		
		glBindAttribLocation		= (PFNGLBINDATTRIBLOCATIONPROC)			glGetProcAddress((GLUT_STRING)"glBindAttribLocation");
		glGetAttribLocation			= (PFNGLGETATTRIBLOCATIONPROC)			glGetProcAddress((GLUT_STRING)"glGetAttribLocation");
		glDisableVertexAttribArray	= (PFNGLDISABLEVERTEXATTRIBARRAYPROC)	glGetProcAddress((GLUT_STRING)"glDisableVertexAttribArray");
		glEnableVertexAttribArray	= (PFNGLENABLEVERTEXATTRIBARRAYPROC)	glGetProcAddress((GLUT_STRING)"glEnableVertexAttribArray");
		glVertexAttribPointer		= (PFNGLVERTEXATTRIBPOINTERPROC)		glGetProcAddress((GLUT_STRING)"glVertexAttribPointer");
		return true;
	}
	bool setUp_EXT_vertex_array() {
		if (isEXTSupported("EXT_vertex_array")) {
			glVertexPointerEXT			= (PFNGLVERTEXPOINTEREXTPROC)			glGetProcAddress((GLUT_STRING)"glVertexPointerEXT");
			glColorPointerEXT			= (PFNGLCOLORPOINTEREXTPROC)			glGetProcAddress((GLUT_STRING)"glColorPointerEXT");
			glTexCoordPointerEXT		= (PFNGLTEXCOORDPOINTEREXTPROC)			glGetProcAddress((GLUT_STRING)"glTexCoordPointerEXT");
			glNormalPointerEXT			= (PFNGLNORMALPOINTEREXTPROC)			glGetProcAddress((GLUT_STRING)"glNormalPointerEXT");
			return true;
		}
		return false;
	}
	bool setUp_ARB_vertex_buffer_object() {
		if(isEXTSupported("ARB_vertex_buffer_object")) {
			glGenBuffersARB				= (PFNGLGENBUFFERSARBPROC)				glGetProcAddress((GLUT_STRING)"glGenBuffersARB"); 
			glBindBufferARB				= (PFNGLBINDBUFFERARBPROC)				glGetProcAddress((GLUT_STRING)"glBindBufferARB"); 
			glBufferDataARB				= (PFNGLBUFFERDATAARBPROC)				glGetProcAddress((GLUT_STRING)"glBufferDataARB"); 
			glDeleteBuffersARB			= (PFNGLDELETEBUFFERSARBPROC)			glGetProcAddress((GLUT_STRING)"glDeleteBuffersARB");
			return true;
		}
		return false;
	}
	bool setUp_ARB_vertex_program() {
		if (isEXTSupported("ARB_vertex_program")) {
			glBindProgramARB 				  = (PFNGLBINDPROGRAMARBPROC)					glGetProcAddress((GLUT_STRING)"glBindProgramARB");
			glDeleteProgramsARB 			  = (PFNGLDELETEPROGRAMSARBPROC)				glGetProcAddress((GLUT_STRING)"glDeleteProgramsARB");
			glDisableVertexAttribArrayARB 	  = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)		glGetProcAddress((GLUT_STRING)"glDisableVertexAttribArrayARB");
			glEnableVertexAttribArrayARB 	  = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)		glGetProcAddress((GLUT_STRING)"glEnableVertexAttribArrayARB");
			glGenProgramsARB 				  = (PFNGLGENPROGRAMSARBPROC)					glGetProcAddress((GLUT_STRING)"glGenProgramsARB");
			glGetProgramEnvParameterdvARB 	  = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC)		glGetProcAddress((GLUT_STRING)"glGetProgramEnvParameterdvARB");
			glGetProgramEnvParameterfvARB 	  = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)		glGetProcAddress((GLUT_STRING)"glGetProgramEnvParameterfvARB");
			glGetProgramLocalParameterdvARB   = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)	glGetProcAddress((GLUT_STRING)"glGetProgramLocalParameterdvARB");
			glGetProgramLocalParameterfvARB   = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)	glGetProcAddress((GLUT_STRING)"glGetProgramLocalParameterfvARB");
			glGetProgramStringARB 			  = (PFNGLGETPROGRAMSTRINGARBPROC)				glGetProcAddress((GLUT_STRING)"glGetProgramStringARB");
			glGetProgramivARB 				  = (PFNGLGETPROGRAMIVARBPROC)					glGetProcAddress((GLUT_STRING)"glGetProgramivARB");
			glGetVertexAttribPointervARB 	  = (PFNGLGETVERTEXATTRIBPOINTERVARBPROC)		glGetProcAddress((GLUT_STRING)"glGetVertexAttribPointervARB");
			glGetVertexAttribdvARB 			  = (PFNGLGETVERTEXATTRIBDVARBPROC)				glGetProcAddress((GLUT_STRING)"glGetVertexAttribdvARB");
			glGetVertexAttribfvARB 			  = (PFNGLGETVERTEXATTRIBFVARBPROC)				glGetProcAddress((GLUT_STRING)"glGetVertexAttribfvARB");
			glGetVertexAttribivARB 			  = (PFNGLGETVERTEXATTRIBIVARBPROC)				glGetProcAddress((GLUT_STRING)"glGetVertexAttribivARB");
			glIsProgramARB 					  = (PFNGLISPROGRAMARBPROC)						glGetProcAddress((GLUT_STRING)"glIsProgramARB");
			glProgramEnvParameter4dARB 		  = (PFNGLPROGRAMENVPARAMETER4DARBPROC)			glGetProcAddress((GLUT_STRING)"glProgramEnvParameter4dARB");
			glProgramEnvParameter4dvARB 	  = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)		glGetProcAddress((GLUT_STRING)"glProgramEnvParameter4dvARB");
			glProgramEnvParameter4fARB 		  = (PFNGLPROGRAMENVPARAMETER4FARBPROC)			glGetProcAddress((GLUT_STRING)"glProgramEnvParameter4fARB");
			glProgramEnvParameter4fvARB 	  = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)		glGetProcAddress((GLUT_STRING)"glProgramEnvParameter4fvARB");
			glProgramLocalParameter4dARB 	  = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC)		glGetProcAddress((GLUT_STRING)"glProgramLocalParameter4dARB");
			glProgramLocalParameter4dvARB 	  = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)		glGetProcAddress((GLUT_STRING)"glProgramLocalParameter4dvARB");
			glProgramLocalParameter4fARB 	  = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)		glGetProcAddress((GLUT_STRING)"glProgramLocalParameter4fARB");
			glProgramLocalParameter4fvARB 	  = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)		glGetProcAddress((GLUT_STRING)"glProgramLocalParameter4fvARB");
			glProgramStringARB				  = (PFNGLPROGRAMSTRINGARBPROC)					glGetProcAddress((GLUT_STRING)"glProgramStringARB");
			glVertexAttrib1dARB 			  = (PFNGLVERTEXATTRIB1DARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib1dARB");
			glVertexAttrib1dvARB 			  = (PFNGLVERTEXATTRIB1DVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib1dvARB");
			glVertexAttrib1fARB 			  = (PFNGLVERTEXATTRIB1FARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib1fARB");
			glVertexAttrib1fvARB 			  = (PFNGLVERTEXATTRIB1FVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib1fvARB");
			glVertexAttrib1sARB 			  = (PFNGLVERTEXATTRIB1SARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib1sARB");
			glVertexAttrib1svARB 			  = (PFNGLVERTEXATTRIB1SVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib1svARB");
			glVertexAttrib2dARB 			  = (PFNGLVERTEXATTRIB2DARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib2dARB");
			glVertexAttrib2dvARB 			  = (PFNGLVERTEXATTRIB2DVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib2dvARB");
			glVertexAttrib2fARB 			  = (PFNGLVERTEXATTRIB2FARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib2fARB");
			glVertexAttrib2fvARB 			  = (PFNGLVERTEXATTRIB2FVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib2fvARB");
			glVertexAttrib2sARB 			  = (PFNGLVERTEXATTRIB2SARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib2sARB");
			glVertexAttrib2svARB 			  = (PFNGLVERTEXATTRIB2SVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib2svARB");
			glVertexAttrib3dARB 			  = (PFNGLVERTEXATTRIB3DARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib3dARB");
			glVertexAttrib3dvARB 			  = (PFNGLVERTEXATTRIB3DVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib3dvARB");
			glVertexAttrib3fARB 			  = (PFNGLVERTEXATTRIB3FARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib3fARB");
			glVertexAttrib3fvARB 			  = (PFNGLVERTEXATTRIB3FVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib3fvARB");
			glVertexAttrib3sARB 			  = (PFNGLVERTEXATTRIB3SARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib3sARB");
			glVertexAttrib3svARB 			  = (PFNGLVERTEXATTRIB3SVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib3svARB");
			glVertexAttrib4NbvARB 			  = (PFNGLVERTEXATTRIB4NBVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4NbvARB");
			glVertexAttrib4NivARB 			  = (PFNGLVERTEXATTRIB4NIVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4NivARB");
			glVertexAttrib4NsvARB 			  = (PFNGLVERTEXATTRIB4NSVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4NsvARB");
			glVertexAttrib4NubARB 			  = (PFNGLVERTEXATTRIB4NUBARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4NubARB");
			glVertexAttrib4NubvARB 			  = (PFNGLVERTEXATTRIB4NUBVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4NubvARB");
			glVertexAttrib4NuivARB 			  = (PFNGLVERTEXATTRIB4NUIVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4NuivARB");
			glVertexAttrib4NusvARB 			  = (PFNGLVERTEXATTRIB4NUSVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4NusvARB");
			glVertexAttrib4bvARB 			  = (PFNGLVERTEXATTRIB4BVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4bvARB");
			glVertexAttrib4dARB 			  = (PFNGLVERTEXATTRIB4DARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4dARB");
			glVertexAttrib4dvARB 			  = (PFNGLVERTEXATTRIB4DVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4dvARB");
			glVertexAttrib4fARB 			  = (PFNGLVERTEXATTRIB4FARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4fARB");
			glVertexAttrib4fvARB 			  = (PFNGLVERTEXATTRIB4FVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4fvARB");
			glVertexAttrib4ivARB 			  = (PFNGLVERTEXATTRIB4IVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4ivARB");
			glVertexAttrib4sARB				  = (PFNGLVERTEXATTRIB4SARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4sARB");
			glVertexAttrib4svARB 			  = (PFNGLVERTEXATTRIB4SVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4svARB");
			glVertexAttrib4ubvARB 			  = (PFNGLVERTEXATTRIB4UBVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4ubvARB");
			glVertexAttrib4uivARB 			  = (PFNGLVERTEXATTRIB4UIVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4uivARB");
			glVertexAttrib4usvARB 			  = (PFNGLVERTEXATTRIB4USVARBPROC)				glGetProcAddress((GLUT_STRING)"glVertexAttrib4usvARB");
			glVertexAttribPointerARB 		  = (PFNGLVERTEXATTRIBPOINTERARBPROC)			glGetProcAddress((GLUT_STRING)"glVertexAttribPointerARB");
			return true;
		}
		return false;
	}
	bool setUp_EXT_vertex_attrib_64bit() {
		if (isEXTSupported("EXT_vertex_attrib_64bit")) {
			glVertexAttribLPointerEXT	= (PFNGLVERTEXATTRIBLPOINTEREXTPROC)	glGetProcAddress((GLUT_STRING)"glVertexAttribLPointerEXT");

			return true;
		}
		return false;
	}
	bool setUpFogCoord() {
		glFogCoordPointer	 = (PFNGLFOGCOORDPOINTERPROC)		glGetProcAddress((GLUT_STRING)"glFogCoordPointer");
		glFogCoordd			 = (PFNGLFOGCOORDDPROC)				glGetProcAddress((GLUT_STRING)"glFogCoordd");
		glFogCoorddv		 = (PFNGLFOGCOORDDVPROC)			glGetProcAddress((GLUT_STRING)"glFogCoorddv");
		glFogCoordf 		 = (PFNGLFOGCOORDFPROC)				glGetProcAddress((GLUT_STRING)"glFogCoordf");
		glFogCoordfv		 = (PFNGLFOGCOORDFVPROC)			glGetProcAddress((GLUT_STRING)"glFogCoordfv");
		return true;
	}
	bool setUp_EXT_fog_coord() {
		if (isEXTSupported("EXT_fog_coord")) {
			glFogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC)	glGetProcAddress((GLUT_STRING)"glFogCoordPointerEXT");
			glFogCoorddEXT		 = (PFNGLFOGCOORDDEXTPROC)			glGetProcAddress((GLUT_STRING)"glFogCoorddEXT");
			glFogCoorddvEXT		 = (PFNGLFOGCOORDDVEXTPROC)			glGetProcAddress((GLUT_STRING)"glFogCoorddvEXT");
			glFogCoordfEXT 		 = (PFNGLFOGCOORDFEXTPROC)			glGetProcAddress((GLUT_STRING)"glFogCoordfEXT");
			glFogCoordfvEXT		 = (PFNGLFOGCOORDFVEXTPROC)			glGetProcAddress((GLUT_STRING)"glFogCoordfvEXT");
			return true;
		}
		return false;
	}
	bool setUp_EXT_texture3D() {
		glTexImage3D		= (PFNGLTEXIMAGE3DPROC)				glGetProcAddress((GLUT_STRING)"glTexImage3D");
		glCopyTexSubImage3D	= (PFNGLCOPYTEXSUBIMAGE3DPROC)		glGetProcAddress((GLUT_STRING)"glCopyTexSubImage3D");
		glDrawRangeElements	= (PFNGLDRAWRANGEELEMENTSPROC)		glGetProcAddress((GLUT_STRING)"glDrawRangeElements");
		glTexSubImage3D		= (PFNGLTEXSUBIMAGE3DPROC)			glGetProcAddress((GLUT_STRING)"glTexSubImage3D");

		if (isEXTSupported("EXT_texture3D")) {
			glTexImage3DEXT		= (PFNGLTEXIMAGE3DEXTPROC)		glGetProcAddress((GLUT_STRING)"glTexImage3DEXT");
			return true;
		}
		return false;
	}
	bool setUp_EXT_texture_object() {
		if (isEXTSupported("EXT_texture_object")) {
			glAreTexturesResidentEXT	= (PFNGLARETEXTURESRESIDENTEXTPROC)	glGetProcAddress((GLUT_STRING)"glAreTexturesResidentEXT");
			glBindTextureEXT			= (PFNGLBINDTEXTUREEXTPROC)			glGetProcAddress((GLUT_STRING)"glBindTextureEXT");
			glDeleteTexturesEXT			= (PFNGLDELETETEXTURESEXTPROC)		glGetProcAddress((GLUT_STRING)"glDeleteTexturesEXT");
			glGenTexturesEXT			= (PFNGLGENTEXTURESEXTPROC)			glGetProcAddress((GLUT_STRING)"glGenTexturesEXT");
			glIsTextureEXT				= (PFNGLISTEXTUREEXTPROC)			glGetProcAddress((GLUT_STRING)"glIsTextureEXT");
			glPrioritizeTexturesEXT		= (PFNGLPRIORITIZETEXTURESEXTPROC)	glGetProcAddress((GLUT_STRING)"glPrioritizeTexturesEXT");
			return true;
		}
		return false;
	}
	bool setUpMultiTexture() {
		glActiveTexture			= (PFNGLACTIVETEXTUREARBPROC)		glGetProcAddress((GLUT_STRING)"glActiveTexture");
		glClientActiveTexture	= (PFNGLCLIENTACTIVETEXTUREPROC)	glGetProcAddress((GLUT_STRING)"glClientActiveTexture");
		return true;
	}
	bool setUp_ARB_multitexture() {
		if (isEXTSupported("ARB_multitexture")) {
			glActiveTextureARB			= (PFNGLACTIVETEXTUREARBPROC)		glGetProcAddress((GLUT_STRING)"glActiveTextureARB"); 
			glClientActiveTextureARB	= (PFNGLCLIENTACTIVETEXTUREARBPROC)	glGetProcAddress((GLUT_STRING)"glClientActiveTextureARB");
			glMultiTexCoord1fARB		= (PFNGLMULTITEXCOORD1FARBPROC)		glGetProcAddress((GLUT_STRING)"glMultiTexCoord1fARB"); 
			glMultiTexCoord2fARB		= (PFNGLMULTITEXCOORD2FARBPROC)		glGetProcAddress((GLUT_STRING)"glMultiTexCoord2fARB"); 
			glMultiTexCoord3fARB		= (PFNGLMULTITEXCOORD3FARBPROC)		glGetProcAddress((GLUT_STRING)"glMultiTexCoord3fARB"); 
			glMultiTexCoord4fARB		= (PFNGLMULTITEXCOORD4FARBPROC)		glGetProcAddress((GLUT_STRING)"glMultiTexCoord4fARB");
			return true;
		}
		return false;
	}
	bool setUp_ARB_multisample() {
		if (isEXTSupported("ARB_multisample")) {
			glSampleCoverageARB = (PFNGLSAMPLECOVERAGEARBPROC) glGetProcAddress((GLUT_STRING)"glSampleCoverageARB");
			return true;
		}
		return false;
	}
	void setUpGLShader() {
		glCreateShader				= (PFNGLCREATESHADERPROC)			glGetProcAddress((GLUT_STRING)"glCreateShader");
		glShaderSource				= (PFNGLSHADERSOURCEPROC)			glGetProcAddress((GLUT_STRING)"glShaderSource");
		glCompileShader				= (PFNGLCOMPILESHADERPROC)			glGetProcAddress((GLUT_STRING)"glCompileShader");
		glGetShaderiv				= (PFNGLGETSHADERIVPROC)			glGetProcAddress((GLUT_STRING)"glGetShaderiv");
		glGetShaderInfoLog			= (PFNGLGETSHADERINFOLOGPROC)		glGetProcAddress((GLUT_STRING)"glGetShaderInfoLog");
		glCreateProgram				= (PFNGLCREATEPROGRAMPROC)			glGetProcAddress((GLUT_STRING)"glCreateProgram");
		glAttachShader				= (PFNGLATTACHSHADERPROC)			glGetProcAddress((GLUT_STRING)"glAttachShader");
		glLinkProgram				= (PFNGLLINKPROGRAMPROC)			glGetProcAddress((GLUT_STRING)"glLinkProgram");
		glGetProgramiv				= (PFNGLGETPROGRAMIVPROC)			glGetProcAddress((GLUT_STRING)"glGetProgramiv");
		glGetProgramInfoLog			= (PFNGLGETPROGRAMINFOLOGPROC)		glGetProcAddress((GLUT_STRING)"glGetProgramInfoLog");
		glDeleteShader				= (PFNGLDELETESHADERPROC)			glGetProcAddress((GLUT_STRING)"glDeleteShader");
		glDeleteProgram				= (PFNGLDELETEPROGRAMPROC)			glGetProcAddress((GLUT_STRING)"glDeleteProgram");
		glUseProgram				= (PFNGLUSEPROGRAMPROC)				glGetProcAddress((GLUT_STRING)"glUseProgram");
		glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONPROC)		glGetProcAddress((GLUT_STRING)"glGetUniformLocation");
		glUniform1i					= (PFNGLUNIFORM1IPROC)				glGetProcAddress((GLUT_STRING)"glUniform1i");
		glUniform1ui				= (PFNGLUNIFORM1UIPROC)				glGetProcAddress((GLUT_STRING)"glUniform1ui");
		glUniform1f					= (PFNGLUNIFORM1FPROC)				glGetProcAddress((GLUT_STRING)"glUniform1f");
		glUniform1d					= (PFNGLUNIFORM1DPROC)				glGetProcAddress((GLUT_STRING)"glUniform1d");
		glUniform2i					= (PFNGLUNIFORM2IPROC)				glGetProcAddress((GLUT_STRING)"glUniform2i");
		glUniform2ui				= (PFNGLUNIFORM2UIPROC)				glGetProcAddress((GLUT_STRING)"glUniform2ui");
		glUniform2f					= (PFNGLUNIFORM2FPROC)				glGetProcAddress((GLUT_STRING)"glUniform2f");
		glUniform2d					= (PFNGLUNIFORM2DPROC)				glGetProcAddress((GLUT_STRING)"glUniform2d");
		glUniform3i					= (PFNGLUNIFORM3IPROC)				glGetProcAddress((GLUT_STRING)"glUniform3i");
		glUniform3ui				= (PFNGLUNIFORM3UIPROC)				glGetProcAddress((GLUT_STRING)"glUniform3ui");
		glUniform3f					= (PFNGLUNIFORM3FPROC)				glGetProcAddress((GLUT_STRING)"glUniform3f");
		glUniform3d					= (PFNGLUNIFORM3DPROC)				glGetProcAddress((GLUT_STRING)"glUniform3d");
		glUniform4i					= (PFNGLUNIFORM4IPROC)				glGetProcAddress((GLUT_STRING)"glUniform4i");
		glUniform4ui				= (PFNGLUNIFORM4UIPROC)				glGetProcAddress((GLUT_STRING)"glUniform4ui");
		glUniform4f					= (PFNGLUNIFORM4FPROC)				glGetProcAddress((GLUT_STRING)"glUniform4f");
		glUniform4d					= (PFNGLUNIFORM4DPROC)				glGetProcAddress((GLUT_STRING)"glUniform4d");
		glUniform1iv				= (PFNGLUNIFORM1IVPROC)				glGetProcAddress((GLUT_STRING)"glUniform1iv");
		glUniform1uiv				= (PFNGLUNIFORM1UIVPROC)			glGetProcAddress((GLUT_STRING)"glUniform1uiv");
		glUniform1fv				= (PFNGLUNIFORM1FVPROC)				glGetProcAddress((GLUT_STRING)"glUniform1fv");
		glUniform1dv				= (PFNGLUNIFORM1DVPROC)				glGetProcAddress((GLUT_STRING)"glUniform1dv");
		glUniform2iv				= (PFNGLUNIFORM2IVPROC)				glGetProcAddress((GLUT_STRING)"glUniform2iv");
		glUniform2uiv				= (PFNGLUNIFORM2UIVPROC)			glGetProcAddress((GLUT_STRING)"glUniform2uiv");
		glUniform2fv				= (PFNGLUNIFORM2FVPROC)				glGetProcAddress((GLUT_STRING)"glUniform2fv");
		glUniform2dv				= (PFNGLUNIFORM2DVPROC)				glGetProcAddress((GLUT_STRING)"glUniform2dv");
		glUniform3iv				= (PFNGLUNIFORM3IVPROC)				glGetProcAddress((GLUT_STRING)"glUniform3iv");
		glUniform3uiv				= (PFNGLUNIFORM3UIVPROC)			glGetProcAddress((GLUT_STRING)"glUniform3uiv");
		glUniform3fv				= (PFNGLUNIFORM3FVPROC)				glGetProcAddress((GLUT_STRING)"glUniform3fv");
		glUniform3dv				= (PFNGLUNIFORM3DVPROC)				glGetProcAddress((GLUT_STRING)"glUniform3dv");
		glUniform4iv				= (PFNGLUNIFORM4IVPROC)				glGetProcAddress((GLUT_STRING)"glUniform4iv");
		glUniform4uiv				= (PFNGLUNIFORM4UIVPROC)			glGetProcAddress((GLUT_STRING)"glUniform4uiv");
		glUniform4fv				= (PFNGLUNIFORM4FVPROC)				glGetProcAddress((GLUT_STRING)"glUniform4fv");
		glUniform4dv				= (PFNGLUNIFORM4DVPROC)				glGetProcAddress((GLUT_STRING)"glUniform4dv");
	}
	bool setUp_ARB_shader_objects() {
		if (isEXTSupported("ARB_shader_objects")) {
			glAttachObjectARB			= (PFNGLATTACHOBJECTARBPROC)		glGetProcAddress((GLUT_STRING)"glAttachObjectARB");
			glCompileShaderARB 		    = (PFNGLCOMPILESHADERARBPROC)		glGetProcAddress((GLUT_STRING)"glCompileShaderARB");
			glCreateProgramObjectARB    = (PFNGLCREATEPROGRAMOBJECTARBPROC)	glGetProcAddress((GLUT_STRING)"glCreateProgramObjectARB");
			glCreateShaderObjectARB     = (PFNGLCREATESHADEROBJECTARBPROC)	glGetProcAddress((GLUT_STRING)"glCreateShaderObjectARB");
			glDeleteObjectARB 		    = (PFNGLDELETEOBJECTARBPROC)		glGetProcAddress((GLUT_STRING)"glDeleteObjectARB");
			glDetachObjectARB 		    = (PFNGLDETACHOBJECTARBPROC)		glGetProcAddress((GLUT_STRING)"glDetachObjectARB");
			glGetActiveUniformARB 	    = (PFNGLGETACTIVEUNIFORMARBPROC)	glGetProcAddress((GLUT_STRING)"glGetActiveUniformARB");
			glGetAttachedObjectsARB     = (PFNGLGETATTACHEDOBJECTSARBPROC)	glGetProcAddress((GLUT_STRING)"glGetAttachedObjectsARB");
			glGetHandleARB 			    = (PFNGLGETHANDLEARBPROC)			glGetProcAddress((GLUT_STRING)"glGetHandleARB");
			glGetInfoLogARB 		    = (PFNGLGETINFOLOGARBPROC)			glGetProcAddress((GLUT_STRING)"glGetInfoLogARB");
			glGetObjectParameterfvARB   = (PFNGLGETOBJECTPARAMETERFVARBPROC)glGetProcAddress((GLUT_STRING)"glGetObjectParameterfvARB");
			glGetObjectParameterivARB   = (PFNGLGETOBJECTPARAMETERIVARBPROC)glGetProcAddress((GLUT_STRING)"glGetObjectParameterivARB");
			glGetShaderSourceARB 	    = (PFNGLGETSHADERSOURCEARBPROC)		glGetProcAddress((GLUT_STRING)"glGetShaderSourceARB");
			glGetUniformLocationARB     = (PFNGLGETUNIFORMLOCATIONARBPROC)	glGetProcAddress((GLUT_STRING)"glGetUniformLocationARB");
			glGetUniformfvARB 		    = (PFNGLGETUNIFORMFVARBPROC)		glGetProcAddress((GLUT_STRING)"glGetUniformfvARB");
			glGetUniformivARB 		    = (PFNGLGETUNIFORMIVARBPROC)		glGetProcAddress((GLUT_STRING)"glGetUniformivARB");
			glLinkProgramARB 		    = (PFNGLLINKPROGRAMARBPROC)			glGetProcAddress((GLUT_STRING)"glLinkProgramARB");
			glShaderSourceARB 		    = (PFNGLSHADERSOURCEARBPROC)		glGetProcAddress((GLUT_STRING)"glShaderSourceARB");
			glUniform1fARB 			    = (PFNGLUNIFORM1FARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform1fARB");
			glUniform1fvARB 		    = (PFNGLUNIFORM1FVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform1fvARB");
			glUniform1iARB 			    = (PFNGLUNIFORM1IARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform1iARB");
			glUniform1ivARB 		    = (PFNGLUNIFORM1IVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform1ivARB");
			glUniform2fARB 			    = (PFNGLUNIFORM2FARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform2fARB");
			glUniform2fvARB 		    = (PFNGLUNIFORM2FVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform2fvARB");
			glUniform2iARB 			    = (PFNGLUNIFORM2IARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform2iARB");
			glUniform2ivARB 		    = (PFNGLUNIFORM2IVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform2ivARB");
			glUniform3fARB 			    = (PFNGLUNIFORM3FARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform3fARB");
			glUniform3fvARB 		    = (PFNGLUNIFORM3FVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform3fvARB");
			glUniform3iARB 			    = (PFNGLUNIFORM3IARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform3iARB");
			glUniform3ivARB 		    = (PFNGLUNIFORM3IVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform3ivARB");
			glUniform4fARB 			    = (PFNGLUNIFORM4FARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform4fARB");
			glUniform4fvARB 		    = (PFNGLUNIFORM4FVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform4fvARB");
			glUniform4iARB 			    = (PFNGLUNIFORM4IARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform4iARB");
			glUniform4ivARB 		    = (PFNGLUNIFORM4IVARBPROC)			glGetProcAddress((GLUT_STRING)"glUniform4ivARB");
			glUniformMatrix2fvARB 	    = (PFNGLUNIFORMMATRIX2FVARBPROC)	glGetProcAddress((GLUT_STRING)"glUniformMatrix2fvARB");
			glUniformMatrix3fvARB 	    = (PFNGLUNIFORMMATRIX3FVARBPROC)	glGetProcAddress((GLUT_STRING)"glUniformMatrix3fvARB");
			glUniformMatrix4fvARB 	    = (PFNGLUNIFORMMATRIX4FVARBPROC)	glGetProcAddress((GLUT_STRING)"glUniformMatrix4fvARB");
			glUseProgramObjectARB 	    = (PFNGLUSEPROGRAMOBJECTARBPROC)	glGetProcAddress((GLUT_STRING)"glUseProgramObjectARB");
			glValidateProgramARB 	    = (PFNGLVALIDATEPROGRAMARBPROC)		glGetProcAddress((GLUT_STRING)"glValidateProgramARB");
			return true;
		}
		return false;
	}
	bool setUp_EXT_framebuffer_object() {
		if (isEXTSupported("EXT_framebuffer_object")) {
			glBindFramebufferEXT						= (PFNGLBINDFRAMEBUFFEREXTPROC)							glGetProcAddress((GLUT_STRING)"glBindFramebufferEXT");
			glBindRenderbufferEXT						= (PFNGLBINDRENDERBUFFEREXTPROC)						glGetProcAddress((GLUT_STRING)"glBindRenderbufferEXT");
			glCheckFramebufferStatusEXT					= (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)					glGetProcAddress((GLUT_STRING)"glCheckFramebufferStatusEXT");
			glDeleteFramebuffersEXT						= (PFNGLDELETEFRAMEBUFFERSEXTPROC)						glGetProcAddress((GLUT_STRING)"glDeleteFramebuffersEXT");
			glDeleteRenderbuffersEXT					= (PFNGLDELETERENDERBUFFERSEXTPROC)						glGetProcAddress((GLUT_STRING)"glDeleteRenderbuffersEXT");
			glFramebufferRenderbufferEXT				= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)					glGetProcAddress((GLUT_STRING)"glFramebufferRenderbufferEXT");
			glFramebufferTexture1DEXT					= (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)					glGetProcAddress((GLUT_STRING)"glFramebufferTexture1DEXT");
			glFramebufferTexture2DEXT					= (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)					glGetProcAddress((GLUT_STRING)"glFramebufferTexture2DEXT");
			glFramebufferTexture3DEXT					= (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)					glGetProcAddress((GLUT_STRING)"glFramebufferTexture3DEXT");
			glGenFramebuffersEXT						= (PFNGLGENFRAMEBUFFERSEXTPROC)							glGetProcAddress((GLUT_STRING)"glGenFramebuffersEXT");
			glGenRenderbuffersEXT						= (PFNGLGENRENDERBUFFERSEXTPROC)						glGetProcAddress((GLUT_STRING)"glGenRenderbuffersEXT");
			glGenerateMipmapEXT							= (PFNGLGENERATEMIPMAPEXTPROC)							glGetProcAddress((GLUT_STRING)"glGenerateMipmapEXT");
			glGetFramebufferAttachmentParameterivEXT	= (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)		glGetProcAddress((GLUT_STRING)"glGetFramebufferAttachmentParameterivEXT");
			glGetRenderbufferParameterivEXT				= (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)				glGetProcAddress((GLUT_STRING)"glGetRenderbufferParameterivEXT");
			glIsFramebufferEXT							= (PFNGLISFRAMEBUFFEREXTPROC)							glGetProcAddress((GLUT_STRING)"glIsFramebufferEXT");
			glIsRenderbufferEXT							= (PFNGLISRENDERBUFFEREXTPROC)							glGetProcAddress((GLUT_STRING)"glIsRenderbufferEXT");
			glRenderbufferStorageEXT					= (PFNGLRENDERBUFFERSTORAGEEXTPROC)						glGetProcAddress((GLUT_STRING)"glRenderbufferStorageEXT");
			return true;
		}
		return false;
	}
	bool setUp_EXT_geometry_shader4() {
		if (isEXTSupported("EXT_geometry_shader4")) {
			glFramebufferTextureEXT			= (PFNGLFRAMEBUFFERTEXTUREEXTPROC)		glGetProcAddress((GLUT_STRING)"glFramebufferTextureEXT");
			glFramebufferTextureFaceEXT		= (PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC)	glGetProcAddress((GLUT_STRING)"glFramebufferTextureFaceEXT");
			glProgramParameteriEXT			= (PFNGLPROGRAMPARAMETERIEXTPROC)		glGetProcAddress((GLUT_STRING)"glProgramParameteriEXT");
			return true;
		}
		return false;
	}
	bool setUp_EXT_texture_array() {
		if (isEXTSupported("EXT_texture_array")) {
			glFramebufferTextureLayerEXT	= (PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC)		glGetProcAddress((GLUT_STRING)"glFramebufferTextureLayerEXT");
			return true;
		}
		return false;
	}
}
