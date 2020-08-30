#ifndef __GLPREMITIVE_H
#define __GLPREMITIVE_H

namespace gltools {

	void glDrawRectangle(float x1, float y1, float x2, float y2, float z = 0);
	void glDrawWireRectangle(float x1, float y1, float x2, float y2, float z = 0);
	void glDrawFullScreenRectangle();
}

#endif