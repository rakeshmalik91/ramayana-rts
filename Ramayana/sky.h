#ifndef __RAMAYANA_SKY_H
#define __RAMAYANA_SKY_H

#include "common.h"
#include "world.h"

namespace ramayana {

	class Sky : public virtual World {
		Texture2D skyTexture;																					//
	public:
		void init();																							//
		void drawSky();																							//
	};
}

#endif