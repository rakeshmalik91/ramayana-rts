#ifndef __RAMAYANA_AI_H
#define __RAMAYANA_AI_H

#include "common.h"
#include "team.h"

namespace ramayana {	

	class Game;
	
	class TeamKnowledgeBase {
	public:
		struct BuildableUnit {
			UnitType type;
			vector<UnitType> builderType;
			BuildableUnit(UnitType type=-1) : type(type) {}
			bool operator==(BuildableUnit &bu) {return type==bu.type;}
		};
	private:
		struct BuildableUnitType {
			BuildableUnit lumbercamp, mine, farm, barracks, archery, beastiary, citadel, outpost;
			vector<BuildableUnit> lightMelee, heavyMelee, lightRanged, heavyRanged, lightBeast, heavyBeast;
			BuildableUnit worker;
		} buildableUnit;
		struct TargetStat {
			unsigned int timeStamp;
			int nWorker;
			Resource<> resource, gatherer;
			int nScout;
			int nBarracks, nArchery, nBeastiary;
			int nLightMelee, nHeavyMelee, nLightRanged, nHeavyRanged, nLightBeast, nHeavyBeast;
			TargetStat(unsigned int timeStamp=0) : timeStamp(timeStamp) {}
		};
		vector<TargetStat> targetStat;
	private:
		BuildableUnit getUnitFromXML(xml_node<char>*, const char*);
		vector<BuildableUnit> getUnitsFromXML(xml_node<char>*, const char*);
	public:
		void loadKnowledge(string);
		void saveKnowledge(string);
		void forget();
		TargetStat& getTargetStat(unsigned int);
		void addTargetStat(unsigned int, Team&);
		BuildableUnitType& getBuildableUnitType();
	};

	class TeamAI : public TeamKnowledgeBase {
	private:
		unsigned int counter;
		int team;
		UnitTypeInfo *unitTypeInfo;
		Game *game;
		SDL_Thread *thread;
		bool active;
		struct Statistics {
			float percentageExplored, percentageVisible;
			float percentageTerritory, percentageEnemyTerritory;
			bool **territorry, **enemyTerritorry;
			Resource<int> resourceUsed, gatherRate;//unused
		} statistics;
		vector<UnitID> scoutUnits;
		Point2Di buildingLocationBias;
		struct AttackInfo {
			bool attackStarted;
			Point2D position;
			int nUnitsAttacking;
			TeamID attackingTeam;
			AttackInfo() : attackStarted(false), attackingTeam(-1), nUnitsAttacking(0) {}
		} attackinfo;
	private:
		void calculateBuildingLocationBias();
		void createWorker();
		void calculateTerritorry();
		void calculateStatistics();
		int getBuilderOf();
		int getNearestUnit(Point2D, int, UnitCategory);
		Tuple<Point2D, int> getBuildablePlace(Point2Di, int);
		Point2Di getForest();
		void engageWorkerToFood(int, bool);
		void engageWorkerToWood(int, bool);
		void engageWorkerToStone(int);
		void engageWorkerToMetal(int, bool);
		void autoBuild();
		void engageIdleWorkersToFood(bool);
		void engageIdleWorkersToWood(bool);
		void engageIdleWorkersToStone();
		void engageIdleWorkersToMetal(bool);
		void scout();
		void createMilitaryBuildings();
		void createSoldiers();
		void flee();
		void attackOnSight();
		void helpAlly();
		void garrison();
		void attackEnemy();
		void autoGather();
		void performSpecialAttack();
	public:
		TeamAI();
		TeamAI(int, UnitTypeInfo*, Game*);
		void close();
		void init();
		void command();
		void intelligence();
		void train();
		bool isTeamUnit(int) const;
		bool isAllyUnit(int) const;
		bool isUnitVisible(int) const;
		bool isEnemyUnit(int) const;
		bool isActive() const;
	private:
		static int commandThreadFunc(void*);																				//
		static int trainThreadFunc(void*);																					//
	};
}

#endif