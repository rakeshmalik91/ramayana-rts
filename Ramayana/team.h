#ifndef __RAMAYANA_TEAM_H
#define __RAMAYANA_TEAM_H

#include <string>
#include "common.h"
#include "unit.h"

namespace ramayana {

	class Game;

	class Team {
		static const int INITIAL_FOOD_GATHER_TIME				=20;
		static const int INITIAL_WOOD_GATHER_TIME				=20;
		static const int INITIAL_STONE_GATHER_TIME				=30;
		static const int INITIAL_METAL_GATHER_TIME				=40;
	private:
		Game *game;
		UnitTypeInfo* unitTypeInfo;
	public:
		TeamID teamIndex;
		string name;
		int color;
		int faction;
		TeamGroupID teamGroup;
		Resource<> resource;
		int population, maxPopulation;
		int startPositionIndex;
		int nWorker, nBuilder, nIdleWorker;
		int nHero, nMilitary, nBuilding;
		int nUnits[MAX_OBJECT_TYPE];
		Resource<> nGatherer, gatherTime;
		bool **explored, **visible;
		unsigned int human : 1, hasAI : 1, exploreAll : 1, revealAll : 1, defeated : 1;
	private:
		void calculateStatistics();
		void calculategetLOS();
		void checkStatus();
	public:
		Team(int color=NATURE_COLOR, bool human=false, int faction=0, int teamGroup=0) : 
			defeated(false),
			visible(NULL), explored(NULL), exploreAll(false), revealAll(false),
			color(color), human(human), faction(faction), teamGroup(teamGroup),
			resource(0, 0, 0, 0), population(0), maxPopulation(0),
			gatherTime(INITIAL_FOOD_GATHER_TIME, INITIAL_WOOD_GATHER_TIME, INITIAL_STONE_GATHER_TIME, INITIAL_METAL_GATHER_TIME),
			nGatherer(0, 0, 0, 0), nWorker(0), nBuilder(0), nIdleWorker(0),
			startPositionIndex(-1), hasAI(false) {}
		~Team();
		void init(Game*, UnitTypeInfo*, int);
		void update();
		Resource<> getUnitCost(UnitType);
		void setRevealAll(bool);
		void setExploreAll(bool);
		xml_node<char>* toXMLNode(xml_document<char>&) const;
		void loadFromXMLNode(xml_node<char>*, UnitTypeInfo*, Game*);
	};
}

#endif