#ifndef __RAMAYANA_TERRAIN_H
#define __RAMAYANA_TERRAIN_H

#include "common.h"
#include "world.h"

namespace ramayana {
		
	enum TerrainType {
		TERRAIN_DEEP_WATER,
		TERRAIN_SHALLOW_WATER, 
		TERRAIN_MOVABLE_LAND, 
		TERRAIN_UNMOVABLE_LAND
	};

	class Terrain : public Surface, public virtual World {
	public:
		const static int  MIN_TERRAIN_HEIGHT = -32;
		const static int  MAX_TERRAIN_HEIGHT = 32;
		const static int  N_TERRAIN_TEXTURE = 8;
	private:
		bool loaded;																							//
		Texture3D terrainTexture, terrainBumpTexture;															//
		GLuint terrainVBName, terrainFogVBName;																	//
		Texture2D *fogOfWarMap;																					//
		ShaderProgram terrainShader;																			//
	protected:
		float **z;																								//height of all locaton on map
		GLuint selectBuf[SELECT_BUF_SIZE];																		//
		int **textureIndex;																						//
	private:
		Point3D getVertexPosition(unsigned int x, unsigned int y);
		TexCoord3D getTexCoord0(unsigned int x, unsigned int y);
		TexCoord2D getTexCoord1(unsigned int x, unsigned int y);
		void initFBOs(int, int);																				//
		void initTerrain();																						//
		void updateFogOfWarSampler();
	public:
		Terrain();																								//
		~Terrain();																								//
		void init(int, int);																					//
		void loadTerrain(string, string);																		//
		void saveTerrain(string, string);																		//
		void updateTerrain(bool init=false);																	//
		void drawTerrain(bool, bool, bool);																		//
		void drawTerrainForShadow();																			//
		void increaseHeight(int screen_width, int screen_height, int x, int y, float dh, bool plain=false, float level=0.0);											//EDIT : Increase height of location in map
		void setTexture(int, int, int, int, int);																//EDIT : set texture of a location in map
		float getGroundHeight(float x, float y) const;															//
		float getGroundHeight(int x, int y) const;																//
		bool isWater(int x, int y) const;																		//
		bool isUneven(int x, int y, float threshold) const;														//
		virtual float getWaterLevel(int x, int y) const;
		virtual bool isTerrainEditMode() const = 0;
		virtual float getFog(unsigned int x, unsigned int y) = 0;
	};
	
};

#endif