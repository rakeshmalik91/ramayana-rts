#ifndef __RAMAYANA_WATER_H
#define __RAMAYANA_WATER_H

#include "common.h"
#include "world.h"
#include "interface.h"

namespace ramayana {

	class Water : public virtual World {
		int waterWavePhase;
		Texture2DAnimated waterTexture, waterBumpTexture;														//
		int waterTextureFrameNumber, waterBumpTextureFrameNumber;												//
		struct Vertex {
			Point3D pos;
			TexCoord texcoord;
			Vector3D normal;
			Vector4D tangent;
		} waterVB[4];																							//
		GLuint waterVBName;																						//
		ShaderProgram waterShader;																				//
	public:
		float waterWaveAmplitude;																				//
		Color waterColor;																						//
	public:	
		Water();
		~Water();
		void init();																							//
		void updateWater(bool init = false);																	//
		void drawWater(bool, bool, bool);																		//
		int getWaterWavePhase() const;
		float getWaterWaveAmplitude() const;
	};

}

#endif
