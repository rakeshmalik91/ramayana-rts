#ifndef __RAMAYANA_COMMON_H
#define __RAMAYANA_COMMON_H

#include "stdafx.h"

#include "enums.h"

using namespace rapidxml;
using namespace graphics;
using namespace gltools;
using namespace algorithm;
using namespace physics;

namespace ramayana {

	static const int FRAME_RATE							=40;
	static const int UPDATE_THREAD_FRAME_RATE			=20;
		
	static const int MAX_OBJECT_TYPE					=800;
	
	static const int  NATURE_COLOR 						=0;
	static Color availableTeamColors[]={
		COLOR_WHITE, COLOR_TURQUOISE, COLOR_SCARLET, COLOR_SAP_GREEN, 
		COLOR_YELLOW, COLOR_LIGHT_TURQUOISE, COLOR_ROSE, COLOR_VIOLET,
		COLOR_ORANGE, COLOR_LIME, COLOR_MAGENTA, COLOR_BLACK};
	
	static const string factionName[]={"Nature", "Vaanara", "Rakshasa"};
	
	template<class DT=int> struct Resource {
		DT food, wood, stone, metal;
		Resource() {}
		Resource(DT f, DT w, DT s, DT m) : food(f), wood(w), stone(s), metal(m) {}
	};

	struct HUDMessage {
		string msg;
		HUDMessage(string msg) : msg(msg) {}
	};

	class Game;

	typedef int TeamID, TeamGroupID, UnitID, UnitType;

#define TEXTURE_DEFAULT			GL_TEXTURE0
#define TEXTURE_NORMAL_MAP		GL_TEXTURE1
#define TEXTURE_ENV_MAP			GL_TEXTURE5
#define TEXTURE_FOG_OF_WAR		GL_TEXTURE6
#define TEXTURE_SHADOW_MAP		GL_TEXTURE7

#define SAMPLER_DEFAULT			0
#define SAMPLER_NORMAL_MAP		1
#define SAMPLER_ENV_MAP			5
#define SAMPLER_FOG_OF_WAR		6
#define SAMPLER_SHADOW_MAP		7

#define ARROW_SPEED				3.0
#define ARROW_WEIGHT			0.5
};

#endif