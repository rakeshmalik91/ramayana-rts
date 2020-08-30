/*****************************************************************************************************
* Subject                   : OpenGL transform                                                          *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include <cstdio>

#include "gltransform.h"

namespace gltools {

	void glShearf(float sXY, float sXZ, float sYX, float sYZ, float sZX, float sZY) {
		float mat[] = {
			1, sXY, sXZ, 0,
			sYX, 1, sYZ, 0,
			sZX, sZY, 1, 0,
			0, 0, 0, 1
		};
		glMultMatrixf(mat);
	}
}