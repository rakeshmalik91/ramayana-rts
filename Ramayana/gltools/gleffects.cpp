/*****************************************************************************************************
* Subject                   : OpenGL effects                                                        *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include <cstdio>

#include "gleffects.h"

namespace gltools {

	void motionBlur(float blur) {
		glAccum(GL_ACCUM, 1.0 - blur);
		glAccum(GL_RETURN, 1.0);
		glAccum(GL_MULT, blur);
	}
}