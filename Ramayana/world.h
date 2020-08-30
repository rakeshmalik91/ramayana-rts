#ifndef __RAMAYANA_WORLD_H
#define __RAMAYANA_WORLD_H

#include "common.h"
#include "interface.h"

namespace ramayana {

	class World {
	public:
		const static float BASE_WATER_LEVEL;
		const static int  WORLD_RADIUS = 500;
		const static size_t  TERRAIN_TEXTURE_SIZE = 512;
		static const int SELECT_BUF_SIZE = 0x0000FFFF;
		static const GLuint POS_NAME_START = 0x80000000;
		static const int SHADOWMAP_RATIO = 2;
	protected:
		int width, height;																						//Map size
		float camX, camY, camZ;																					//Camera position
		float currentWaterLevel;																				//
		Color fogColor;																							//
		Frustum frustum;																						//
		FrameBuffer shadowMapFrameBuffer, reflectionFrameBuffer;
	public:
		float tilt;																								//Camera tilt
		float rotation;																							//Camera rotation
	private:
		void initLight();																						//Initialization of GL light
		void initFog();																							//Initialization of GL Fog
	protected:
		void init();
		void setCamera();																						//
	public:
		int getWidth() const;																					//
		int getHeight() const;																					//
		float getCurrentWaterLevel() const;																		//
		void moveUp(float);																						//Moves camera moveUp
		void moveRight(float);																					//Moves camera moveRight
		void zoomOut(float);																					//Moves camera towards/away from map
		void setZoom(float);																					//
		void tiltBack(float);																					//
		void rotateLeft(float);																					//
		void goTo(float, float);																				//Moves camera to given location on map
		Point3D getCameraPosition() const;																		//
		void resetCamera();																						//
		virtual float getWaterLevel(int x, int y) const = 0;													//
		virtual float getGroundHeight(float x, float y) const = 0;												//
		virtual float getGroundHeight(int x, int y) const = 0;													//
		virtual void setAmbienceSound() = 0;																	//
		virtual bool isEditable() const = 0;																	//
		virtual bool isPaused() const = 0;
	};
}

#endif
