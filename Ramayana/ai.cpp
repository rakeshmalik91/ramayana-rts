#include "stdafx.h"

#include "common.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "team.h"

namespace ramayana {

	TeamAI::TeamAI()
		: active(false) {
	}
	TeamAI::TeamAI(int team, UnitTypeInfo* unitTypeInfo, Game* game) 
		: active(false), team(team), unitTypeInfo(unitTypeInfo), game(game) {
	}
	void TeamAI::init() {
		if(game->teams[team].hasAI) {
			loadKnowledge(toLower(factionName[game->teams[team].faction])+".xml");
			calculateBuildingLocationBias();
			//forget();
		}

		statistics.territorry=allocate<bool>(game->getWidth(), game->getHeight());
		statistics.enemyTerritorry=allocate<bool>(game->getWidth(), game->getHeight());

		thread=SDL_CreateThread(commandThreadFunc, this);
		//thread=SDL_CreateThread(trainThreadFunc, this);
		active=true;
		counter=team;
	}
	void TeamAI::close() {
		if(active) {
			active=false;
			SDL_WaitThread(thread, NULL);
		}
	}

	int _SDL_THREAD TeamAI::commandThreadFunc(void *param) {
		TeamAI &ai=*(TeamAI*)param;
		
		while (ai.game->isPaused()) {
			SDL_Delay(1000 / FRAME_RATE);
		}
		while(ai.active) {
			unsigned int startTime=SDL_GetTicks();

			try {
				if(ai.game->teams[ai.team].hasAI)
					ai.command();
				ai.intelligence();
			} catch(Exception &e) {
				showMessage(e.getMessage(), "Runtime Exception : ramayana::TeamAI::commandThreadFunc()", false);
			}
	
			int duration=SDL_GetTicks()-startTime;
			SDL_Delay(50);
			while (ai.game->isLoaded() && ai.game->isPaused()) {
				SDL_Delay(1000 / FRAME_RATE);
			}
		}
		return 1;
	}
	//Prime : 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997
	void TeamAI::command() {
		if(counter%127==0)	createWorker();
		if(counter%101==0)	engageIdleWorkersToFood(true);
		if(counter%103==0)	engageIdleWorkersToWood(true);
		if(counter%109==0)	engageIdleWorkersToMetal(true);
		if(counter%107==0)	engageIdleWorkersToStone();

		if(counter%19 ==0)	scout();

		if(attackinfo.attackStarted) {
			if(counter%1  ==0)	attackEnemy();
		} else {
			if(counter%503==0)	attackEnemy();
		}
		if(counter%71==0)	performSpecialAttack();

		if(counter%997==0)	createMilitaryBuildings();
		if(counter%41 ==0)	createSoldiers();
	}
	void TeamAI::intelligence() {
		if(counter%61 ==0)	calculateStatistics();

		if(counter%31 ==0)	autoBuild();
		if(counter%41 ==0)	autoGather();
		
		if(counter%3 ==0)	flee();
		if(counter%1 ==0)	attackOnSight();
		if(counter%5 ==0)	helpAlly();
		if(counter%17==0)	garrison();

		if(!game->teams[team].hasAI && team != game->playerTeam) {
			if(counter%71==0)	performSpecialAttack();
		}

		counter=(counter+1)%UINT_MAX;
	}
	
	int _SDL_THREAD TeamAI::trainThreadFunc(void *param) {
		TeamAI &ai=*(TeamAI*)param;

		while (ai.game->isPaused()) {
			SDL_Delay(1000 / FRAME_RATE);
		}
		while(ai.game->isLoaded()) {
			try {
				ai.train();
			} catch(Exception &e) {
				showMessage(e.getMessage(), "Runtime Exception : ramayana::TeamAI::trainThreadFunc()", false);
			}
			
			SDL_Delay(30000);
			while (ai.game->isLoaded() && ai.game->isPaused()) {
				SDL_Delay(1000 / FRAME_RATE);
			}
		}
		return 1;
	}
	void TeamAI::train() {
		calculateStatistics();
		addTargetStat(game->getGamePlayTime(), game->teams[team]);
		saveKnowledge("test.xml");
	}
	
	void TeamAI::createWorker() {
		if(getBuildableUnitType().worker.builderType.size()<=0)
			return;
		if(game->teams[team].nWorker>=getTargetStat(game->getGamePlayTime()).nWorker)
			return;
		UnitID buildingID=-1;
		int minCommandList=INT_MAX;
		for(UnitID u=0; u<game->getNumberOfUnits(); u++)
			if(isTeamUnit(u) && game->unit[u].isActive())
				if(contains(getBuildableUnitType().worker.builderType, game->unit[u].type)/* && game->unit[u].commandListLength()<minCommandList*/) {
				buildingID=u;
				minCommandList=game->unit[u].commandListLength();
			}
		if(buildingID>=0) {
			UnitID workerID=game->addUnit(-1, -1, getBuildableUnitType().worker.type, team, 0, 1, true);
			if(workerID!=-1)
				game->unit[buildingID].newAICommand(COMMAND_BUILD, workerID);
		}
	}

	void TeamAI::calculateTerritorry() {
		setAll(statistics.territorry, game->getWidth(), game->getHeight(), false);
		setAll(statistics.enemyTerritorry, game->getWidth(), game->getHeight(), false);
		for(int u=0; u<game->getNumberOfUnits(); u++) {
			if(game->unit[u].isAlive() && isConstruction(game->unit[u].getCategory())) {
				int r=game->unit[u].x, c=game->unit[u].y;
				if(Point2Di(r, c).in(0, 0, game->getWidth()-1, game->getHeight()-1)) {
					if(isEnemyUnit(u))
						fillCircle(statistics.enemyTerritorry, game->getWidth(), game->getHeight(), true, Point2Di(r, c), game->unit[u].getLOS());
					if(isAllyUnit(u)) 
						fillCircle(statistics.territorry, game->getWidth(), game->getHeight(), true, Point2Di(r, c), game->unit[u].getLOS());
				}
			}
		}
	}
	void TeamAI::calculateStatistics() {
		calculateTerritorry();

		int mapsize=game->getWidth()*game->getHeight();
		statistics.percentageExplored=100.0*(float)count(game->teams[team].explored, game->getWidth(), game->getHeight(), true)/mapsize;
		statistics.percentageVisible=100.0*(float)count(game->teams[team].visible, game->getWidth(), game->getHeight(), true)/mapsize;
		statistics.percentageTerritory=100.0*(float)count(statistics.territorry, game->getWidth(), game->getHeight(), true)/mapsize;
		statistics.percentageEnemyTerritory=100.0*(float)count(statistics.enemyTerritorry, game->getWidth(), game->getHeight(), true)/mapsize;
	}

	void TeamAI::scout() {
		//Command
		for(vector<UnitID>::iterator s=scoutUnits.begin(); s!=scoutUnits.end(); ) {
			if(!game->unit[*s].isAlive()) {
				s=scoutUnits.erase(s);
			} else {
				if(game->unit[*s].isIdle()) {
					Point2Di pos(choice(0, game->getWidth()-1), choice(0, game->getHeight()-1));
					for(int nTry=0; nTry<10 && (game->teams[team].visible[pos.y][pos.x] || !game->teams[team].explored[pos.y][pos.x] || game->isWater(pos.x, pos.y)); nTry++)
						pos=Point2Di(choice(0, game->getWidth()-1), choice(0, game->getHeight()-1));
					for(int nTry=0; nTry<50 && (!game->teams[team].explored[pos.y][pos.x] || game->isWater(pos.x, pos.y)); nTry++)
						pos=Point2Di(choice(0, game->getWidth()-1), choice(0, game->getHeight()-1));
					game->unit[*s].newAICommand(COMMAND_MOVE, pos);
				}
				s++;
			}
		}
		
		//Find new scout
		if(scoutUnits.size()<getTargetStat(game->getGamePlayTime()).nScout) {
			float maxSpeed=0.0;
			int scoutID=-1;
			for(int u=0; u<game->getNumberOfUnits(); u++) {
				if(isTeamUnit(u) && !game->unit[u].getTypeInfo().isWorker && !game->unit[u].getTypeInfo().isHeroic && game->unit[u].isIdle()) {
					if(isTeamUnit(u) && unitTypeInfo[game->unit[u].type].speed>maxSpeed) {
						scoutID=u;
						maxSpeed=unitTypeInfo[game->unit[u].type].speed;
					}
				}
			}
			if(scoutID>=0)
				scoutUnits.push_back(scoutID);
		}
	}

	void TeamAI::createMilitaryBuildings() {
		BuildableUnit* buildingCategory[]={
			&getBuildableUnitType().barracks, 
			&getBuildableUnitType().archery, 
			&getBuildableUnitType().beastiary};
		int* nTargetBuildings[]={
			&getTargetStat(game->getGamePlayTime()).nBarracks, 
			&getTargetStat(game->getGamePlayTime()).nArchery, 
			&getTargetStat(game->getGamePlayTime()).nBeastiary};

		for(int b=0; b<arrayLength(buildingCategory); b++) {
			int nBuildings=0;
			for(UnitID u=0; u<game->getNumberOfUnits(); u++)
				if(isTeamUnit(u) && game->unit[u].type==buildingCategory[b]->type)
					nBuildings++;

			if(nBuildings>=*nTargetBuildings[b])
				return;

			UnitID builderID=-1;
			for(UnitID u=0; u<game->getNumberOfUnits() && builderID==-1; u++)
				if(isTeamUnit(u) && game->unit[u].isIdle() && contains(buildingCategory[b]->builderType, game->unit[u].type))
					builderID=u;
			for(UnitID u=0; u<game->getNumberOfUnits() && builderID==-1; u++)
				if(isTeamUnit(u) && contains(buildingCategory[b]->builderType, game->unit[u].type))
					builderID=u;
			if(builderID==-1)
				return;

			Point2Di posApprox;
			int nTry=0;
			do {
				posApprox=(Point2Di(choice(0, game->getWidth()-1), choice(0, game->getHeight()-1))+buildingLocationBias)/2;
				nTry++;
				if(nTry>=100) return;
			} while(!statistics.territorry[posApprox.y][posApprox.x] || game->isWater(posApprox.x, posApprox.y));
			Tuple<Point2D, UnitID> pos=getBuildablePlace(posApprox, buildingCategory[b]->type);

			if(pos.e1.in(0, 0, game->getWidth()-1, game->getHeight()-1)) {
				UnitID barracksID=game->addUnit(pos.e1.x, pos.e1.y, buildingCategory[b]->type, team, pos.e2, 1, true);
				if(barracksID!=-1)
					game->unit[builderID].newAICommand(COMMAND_BUILD, pos.e1, barracksID);
			}
		}
	}
	void TeamAI::createSoldiers() {
		vector<BuildableUnit>* soldierCategory[]={
			&getBuildableUnitType().lightMelee, 
			&getBuildableUnitType().heavyMelee, 
			&getBuildableUnitType().lightRanged, 
			&getBuildableUnitType().heavyRanged, 
			&getBuildableUnitType().lightBeast, 
			&getBuildableUnitType().heavyBeast};
		int* nTargetSoldier[]={
			&getTargetStat(game->getGamePlayTime()).nLightMelee, 
			&getTargetStat(game->getGamePlayTime()).nHeavyMelee, 
			&getTargetStat(game->getGamePlayTime()).nLightRanged, 
			&getTargetStat(game->getGamePlayTime()).nHeavyRanged, 
			&getTargetStat(game->getGamePlayTime()).nLightBeast, 
			&getTargetStat(game->getGamePlayTime()).nHeavyBeast};

		for(int s=0; s<arrayLength(soldierCategory); s++) {
			int nSoldiers=0;
			for(UnitID u=0; u<game->getNumberOfUnits(); u++)
				if(isTeamUnit(u) && contains(*soldierCategory[s], BuildableUnit(game->unit[u].type)))
					nSoldiers++;

			if(nSoldiers>=*nTargetSoldier[s])
				return;

			for(int i=0; i<soldierCategory[s]->size(); i++) {
				UnitID buildingID=-1;
				UnitType soldierType=-1;
				for(UnitID u=0; buildingID==-1 && u<game->getNumberOfUnits(); u++)
					if(isTeamUnit(u) && game->unit[u].isActive() && contains(soldierCategory[s]->at(i).builderType, game->unit[u].type)) {
						buildingID=u;
						soldierType=soldierCategory[s]->at(i).type;
					}
				if(buildingID!=-1) {
					UnitID soldierID=game->addUnit(-1, -1, soldierType, team, 0, 1, true);
					if(soldierID!=-1) {
						//Point2Di rally=game->unit[buildingID].front();
						//game->unit[buildingID].newAICommand(COMMAND_MOVE, rally);
						game->unit[buildingID].newAICommand(COMMAND_BUILD, soldierID);
						break;
					}
				}
			}
		}
	}

	int TeamAI::getNearestUnit(Point2D p, int tTeam, UnitCategory tCategory) {
		int index=-1;
		int dmin=INT_MAX;
		for(int u=0; u<game->getNumberOfUnits(); u++) {
			if(isUnitVisible(u) && game->unit[u].team==tTeam && game->unit[u].state!=STATE_DEAD && game->unit[u].getCategory()==tCategory) {
				int d=manhattanDist(p, game->unit[u].position());
				if(d<dmin) {
					dmin=d;
					index=u;
				}
			}
		}
		return index;
	}
	Tuple<Point2D, int> TeamAI::getBuildablePlace(Point2Di start, int buildingType) {
		return game->getBuildablePlace(start, buildingType, team, statistics.territorry);
	}
	Point2Di TeamAI::getForest() {
		int** woods=allocate<int>(game->getWidth(), game->getHeight());
		for (int r = 0; r < game->getHeight(); r++) {
			for (int c = 0; c < game->getWidth(); c++) {
				woods[r][c] = game->teams[team].explored[r][c] ? 0 : INT_MIN;
			}
		}
		for(int u=0; u<game->getNumberOfUnits(); u++) {
			Point2Di unitPos(roundInt(game->unit[u].x), roundInt(game->unit[u].y));
			if (game->unit[u].isAlive()) {
				if ((game->unit[u].getCategory() == UNIT_TREE && game->teams[team].explored[unitPos.y][unitPos.x])) {
					fillCircle(woods, game->getWidth(), game->getHeight(), game->unit[u].has.wood, unitPos, 10);
				} else if (game->unit[u].type == getBuildableUnitType().lumbercamp.type && game->unit[u].team == team) {
					fillCircle(woods, game->getWidth(), game->getHeight(), -10000, unitPos, 10);
				}
			}
		}
		int maxWood=INT_MIN;
		Point2Di pos(-1, -1);
		for(int r=0; r<game->getHeight(); r++) {
			for (int c = 0; c<game->getWidth(); c++) {
				if (woods[r][c]>maxWood) {
					maxWood = woods[r][c];
					pos = Point2Di(c, r);
				}
			}
		}
		deallocate(woods, game->getWidth(), game->getHeight());
		return pos;
	}
	void TeamAI::engageWorkerToFood(int workerID, bool build) {
		Point2D workerPos=game->unit[workerID].position();
		int farmID=-1;
		int dmin=INT_MAX;
		for(int u=0; u<game->getNumberOfUnits(); u++) {
			if(isTeamUnit(u) && game->unit[u].type==getBuildableUnitType().farm.type && game->unit[u].hasVacancy()) {
				int d=manhattanDist(workerPos, game->unit[u].position());
				if(d<dmin) {
					dmin=d;
					farmID=u;
				}
			}
		}
		if(farmID==-1) {
			if(!build)
				return;
			if(game->hasEnoughResourceToBuild(getBuildableUnitType().farm.type, team)) {
				Point2Di posApprox=game->unit[workerID].position();
				if(unitTypeInfo[getBuildableUnitType().farm.type].category==UNIT_BUILDING) {
					int nTry=0;
					do {
						posApprox=(Point2Di(choice(0, game->getWidth()-1), choice(0, game->getHeight()-1))+buildingLocationBias)/2;
						nTry++;
						if(nTry>=10000) return;
					} while(!statistics.territorry[posApprox.y][posApprox.x] || game->isWater(posApprox.x, posApprox.y));
				}
				Tuple<Point2D, UnitID> pos=getBuildablePlace(posApprox, getBuildableUnitType().farm.type);
				if(pos.e1.in(0, 0, game->getWidth()-1, game->getHeight()-1)) {
					farmID=game->addUnit(pos.e1.x, pos.e1.y, getBuildableUnitType().farm.type, team, pos.e2, 1, true);
					if(farmID!=-1)
						game->unit[workerID].newAICommand(COMMAND_BUILD, pos.e1, farmID);
				}
			}
		} else {
			if(game->unit[farmID].isActive())
				game->unit[workerID].newAICommand(COMMAND_GARRISON, game->unit[farmID].position(), farmID);
			else
				game->unit[workerID].newAICommand(COMMAND_BUILD, game->unit[farmID].position(), farmID);
		}
	}
	void TeamAI::engageWorkerToWood(int workerID, bool build) {
		Point2D workerPos=game->unit[workerID].position();
		int lumbercampID=-1;
		int dmin=INT_MAX;
		for(int u=0; u<game->getNumberOfUnits(); u++) {
			if(isTeamUnit(u) && game->unit[u].type==getBuildableUnitType().lumbercamp.type && game->unit[u].hasVacancy()) {
				int d=manhattanDist(workerPos, game->unit[u].position());
				if(d<dmin) {
					dmin=d;
					lumbercampID=u;
				}
			}
		}
		if(lumbercampID==-1) {
			if(!build)
				return;
			if(game->hasEnoughResourceToBuild(getBuildableUnitType().lumbercamp.type, team)) {
				Point2Di forestPos=(getForest()+buildingLocationBias)/2;
				Tuple<Point2D, int> p=getBuildablePlace(forestPos, getBuildableUnitType().lumbercamp.type);
				if(!p.e1.in(0, 0, game->getWidth()-1, game->getHeight()-1)) {
					int nTry=0;
					do {
						forestPos=(Point2Di(choice(0, game->getWidth()-1), choice(0, game->getHeight()-1))+buildingLocationBias)/2;
						nTry++;
						if(nTry>=10000) return;
					} while(!game->teams[team].explored[forestPos.y][forestPos.x] || game->isWater(forestPos.x, forestPos.y));
					p=getBuildablePlace(forestPos, getBuildableUnitType().lumbercamp.type);
				}
				if(p.e1.in(0, 0, game->getWidth()-1, game->getHeight()-1)) {
					lumbercampID=game->addUnit(p.e1.x, p.e1.y, getBuildableUnitType().lumbercamp.type, team, p.e2, 1, true);
					if(lumbercampID!=-1)
						game->unit[workerID].newAICommand(COMMAND_BUILD, p.e1, lumbercampID);
				}
			}
		} else {
			if(game->unit[lumbercampID].isActive())
				game->unit[workerID].newAICommand(COMMAND_GARRISON, game->unit[lumbercampID].position(), lumbercampID);
			else
				game->unit[workerID].newAICommand(COMMAND_BUILD, game->unit[lumbercampID].position(), lumbercampID);
		}
	}
	void TeamAI::engageWorkerToStone(int workerID) {
		int stoneID=getNearestUnit(game->unit[workerID].position(), 0, UNIT_STONE);
		if(stoneID!=-1) {
			game->unit[workerID].newAICommand(COMMAND_GATHER_STONE, game->unit[stoneID].position(), stoneID);
		}
	}
	void TeamAI::engageWorkerToMetal(int workerID, bool build) {
		Point2D workerPos=game->unit[workerID].position();
		int mineID=-1;
		int dmin=INT_MAX;
		for(int u=0; u<game->getNumberOfUnits(); u++) {
			if(isTeamUnit(u) && game->unit[u].type==getBuildableUnitType().mine.type && game->unit[u].hasVacancy()) {
				int d=manhattanDist(workerPos, game->unit[u].position());
				if(d<dmin) {
					dmin=d;
					mineID=u;
				}
			}
		}
		if(mineID==-1) {
			if(!build)
				return;
			if(game->hasEnoughResourceToBuild(getBuildableUnitType().mine.type, team)) {
				Point2Di posApprox;
				int nTry=0;
				do {
					posApprox=(Point2Di(choice(0, game->getWidth()-1), choice(0, game->getHeight()-1))+buildingLocationBias)/2;
					nTry++;
					if(nTry>=10000) return;
				} while(!game->teams[team].visible[posApprox.y][posApprox.x] || game->isWater(posApprox.x, posApprox.y) || !game->isUneven(posApprox.x, posApprox.y, 0.5));
				Tuple<Point2D, int> pos=getBuildablePlace(posApprox, getBuildableUnitType().mine.type);
				if(pos.e1.in(0, 0, game->getWidth()-1, game->getHeight()-1)) {
					mineID=game->addUnit(pos.e1.x, pos.e1.y, getBuildableUnitType().mine.type, team, pos.e2, 1, true);
					if(mineID!=-1)
						game->unit[workerID].newAICommand(COMMAND_BUILD, pos.e1, mineID);
				}
			}
		} else {
			if(game->unit[mineID].isActive())
				game->unit[workerID].newAICommand(COMMAND_GARRISON, game->unit[mineID].position(), mineID);
			else
				game->unit[workerID].newAICommand(COMMAND_BUILD, game->unit[mineID].position(), mineID);
		}
	}
	void TeamAI::engageIdleWorkersToFood(bool build) {
		if(game->teams[team].nGatherer.food<getTargetStat(game->getGamePlayTime()).gatherer.food)
			for(int u=0; u<game->getNumberOfUnits(); u++)
				if(isTeamUnit(u) && unitTypeInfo[game->unit[u].type].isWorker && game->unit[u].isIdle()) {
					engageWorkerToFood(u, build);
					break;
				}
	}
	void TeamAI::engageIdleWorkersToWood(bool build) {
		if(game->teams[team].nGatherer.wood<getTargetStat(game->getGamePlayTime()).gatherer.wood)
			for(int u=0; u<game->getNumberOfUnits(); u++)
				if(isTeamUnit(u) && unitTypeInfo[game->unit[u].type].isWorker && game->unit[u].isIdle()) {
					engageWorkerToWood(u, build);
					break;
				}
	}
	void TeamAI::engageIdleWorkersToStone() {
		if(game->teams[team].nGatherer.stone<getTargetStat(game->getGamePlayTime()).gatherer.stone)
			for(int u=0; u<game->getNumberOfUnits(); u++)
				if(isTeamUnit(u) && unitTypeInfo[game->unit[u].type].isWorker && game->unit[u].isIdle()) {
					engageWorkerToStone(u);
					break;
				}
	}
	void TeamAI::engageIdleWorkersToMetal(bool build) {
		if(game->teams[team].nGatherer.metal<getTargetStat(game->getGamePlayTime()).gatherer.metal)
			for(int u=0; u<game->getNumberOfUnits(); u++)
				if(isTeamUnit(u) && unitTypeInfo[game->unit[u].type].isWorker && game->unit[u].isIdle()) {
					engageWorkerToMetal(u, build);
					break;
				}
	}
	
	void TeamAI::autoBuild() {
		UnitID buildingID=-1;
		for(UnitID u=0; u<game->getNumberOfUnits(); u++)
			if(isTeamUnit(u) && isConstruction(game->unit[u].getCategory()) && (game->unit[u].isIncomplete() || game->unit[u].isDamaged()))
				buildingID=u;

		if(buildingID==-1)
			return;

		for(int u=0; u<game->getNumberOfUnits(); u++)
			if(isTeamUnit(u) && unitTypeInfo[game->unit[u].type].canRepair && game->unit[u].isIdle()) {
				game->unit[u].newAICommand(COMMAND_BUILD, game->unit[buildingID].position(), buildingID);
				break;
			}
	}
	void TeamAI::autoGather() {
		for(UnitID unitID=0; unitID<game->getNumberOfUnits(); unitID++) {
			if(isTeamUnit(unitID) && game->unit[unitID].getTypeInfo().isWorker && game->unit[unitID].isIdle()) {
				for(UnitID targetID=0; targetID<game->getNumberOfUnits(); targetID++)
					if(game->unit[targetID].isAlive() && game->unit[targetID].isVisibleToUnit(unitID)) {
						if(isTeamUnit(targetID) && game->unit[targetID].isActive() && game->unit[targetID].hasVacancy() && (game->unit[targetID].getTypeInfo().canGather.food || game->unit[targetID].getTypeInfo().canGather.wood || game->unit[targetID].getTypeInfo().canGather.stone || game->unit[targetID].getTypeInfo().canGather.metal)) {
							game->unit[unitID].newAICommand(COMMAND_GARRISON, game->unit[targetID].position(), targetID);
							return;
						}
					}
			}
		}
		for(UnitID unitID=0; unitID<game->getNumberOfUnits(); unitID++) {
			if(isTeamUnit(unitID) && game->unit[unitID].getTypeInfo().isWorker && game->unit[unitID].isIdle()) {
				for(UnitID targetID=0; targetID<game->getNumberOfUnits(); targetID++)
					if(game->unit[targetID].isAlive() && game->unit[targetID].isVisibleToUnit(unitID)) {
						 if(game->unit[targetID].getCategory()==UNIT_STONE) {
							game->unit[unitID].newAICommand(COMMAND_GATHER_STONE, game->unit[targetID].position(), targetID);
							return;
						}
					}
			}
		}
	}

	void TeamAI::flee() {
		for(UnitID u = 0; u < game->getNumberOfUnits(); u++) {
			if(isTeamUnit(u) 
			   && game->unit[u].getTypeInfo().speed>0
			   && game->unit[u].stance!=STANCE_STANDGROUND 
			   && game->unit[u].state!=STATE_MOVING
			   && !game->unit[u].garrisoned) {
				//Non-military/Hold-fire unit flee from enemy
				if(game->unit[u].getTypeInfo().attack<10 || game->unit[u].stance==STANCE_HOLDFIRE) {
					UnitID attacker=-1;
					int dMin=5;
					for(UnitID probableAttacker=0; probableAttacker<game->getNumberOfUnits(); probableAttacker++)
						if(isEnemyUnit(probableAttacker) 
						   && game->unit[probableAttacker].getTypeInfo().attack>10 
						   && !game->unit[probableAttacker].isIdle() 
						   && game->unit[probableAttacker].isVisibleToUnit(u)) {
							int d=manhattanDist((Point2Di)game->unit[probableAttacker].position(), (Point2Di)game->unit[u].position());
							if(d<dMin) {
								attacker=probableAttacker;
								dMin=d;
								break;
							}
						}
					if(attacker!=-1)
						game->unit[u].flee(unitVector(game->unit[u].position2D(), game->unit[attacker].position2D()), 5);
				}
				//Ranged military flee from near melee
				if(isRangedWeapon(game->unit[u].getTypeInfo().weaponType) && game->unit[u].isIdle()) {
					UnitID attacker=-1;
					int dMin=3;
					for(UnitID probableAttacker=0; probableAttacker<game->getNumberOfUnits(); probableAttacker++)
						if(isEnemyUnit(probableAttacker) 
						   && game->unit[probableAttacker].getTypeInfo().attack>0 
						   && !isRangedWeapon(game->unit[probableAttacker].getTypeInfo().weaponType) 
						   && !game->unit[probableAttacker].isIdle()) {
							int d=manhattanDist((Point2Di)game->unit[probableAttacker].position(), (Point2Di)game->unit[u].position());
							if(d<dMin) {
								attacker=probableAttacker;
								dMin=d;
							}
						}
					if(attacker!=-1)
						game->unit[u].flee(unitVector(game->unit[u].position2D(), game->unit[attacker].position2D()), 2);
				}
			}
		}
	}
	void TeamAI::attackOnSight() {
		for(UnitID u = 0; u<game->getNumberOfUnits(); u++) {
			if(isTeamUnit(u) 
			   && game->unit[u].getTypeInfo().attack>0 
			   && game->unit[u].stance!=STANCE_HOLDFIRE 
			   && (game->unit[u].isIdle() || game->unit[u].isOnAttackMove() || game->unit[u].garrisoned)
			   && !game->unit[u].isFleeing()) {
				UnitID target=-1;
				int dMin=game->unit[u].getLOS();
				for(UnitID t=0; t<game->getNumberOfUnits(); t++)
					if(isEnemyUnit(t) 
					   && game->unit[t].getTypeInfo().isMilitaryUnit() 
					   && game->unit[t].isAttacking() 
					   && game->unit[t].isVisibleToUnit(u)) {
						int d=manhattanDist((Point2Di)game->unit[t].position(), (Point2Di)game->unit[u].position());
						if(d<dMin) {
							target=t;
							dMin=d;
						}
					}
				if(target==-1)
					for(UnitID t=0; t<game->getNumberOfUnits(); t++)
						if(isEnemyUnit(t) 
						   && game->unit[t].getTypeInfo().isMilitaryUnit() 
						   && game->unit[t].isVisibleToUnit(u)) {
							int d=manhattanDist((Point2Di)game->unit[t].position(), (Point2Di)game->unit[u].position());
							if(d<dMin) {
								target=t;
								dMin=d;
							}
						}
				if(target==-1)
					for(UnitID t=0; t<game->getNumberOfUnits(); t++)
						if(isEnemyUnit(t) 
						   && game->unit[t].isVisibleToUnit(u)) {
							int d=manhattanDist((Point2Di)game->unit[t].position(), (Point2Di)game->unit[u].position());
							if(d<dMin) {
								target=t;
								dMin=d;
							}
						}
				if(target!=-1) {
					if (game->unit[u].isIdle()) {
						game->unit[u].newAICommand(COMMAND_MOVE, game->unit[u].position(), true, true, true);
					}
					game->unit[u].newAICommand(COMMAND_ATTACK, target, true, true, true);
				}
			}
		}
	}
	void TeamAI::helpAlly() {
		for(UnitID u=0; u<game->getNumberOfUnits(); u++)
			if(isTeamUnit(u) && game->unit[u].getAttack()>0 && game->unit[u].isIdle()) {
				for(UnitID f=0; f<game->getNumberOfUnits(); f++)
					if(isAllyUnit(f) && game->unit[f].isDying()) {
						game->unit[u].newAICommand(COMMAND_MOVE, game->unit[u].position(), true, true, true);
						game->unit[u].newAICommand(COMMAND_MOVE, game->unit[f].position(), true, true, true);
						break;
					}
			}
	}
	void TeamAI::garrison() {
		//Fleeing Unit
		for(UnitID u = 0; u<game->getNumberOfUnits(); u++) {
			if(isTeamUnit(u) 
			   && game->unit[u].getCategory()==UNIT_INFANTRY
			   && game->unit[u].isFleeing()) {
				UnitID building=-1;
				int dMin=game->unit[u].getTypeInfo().los;
				for(UnitID b=0; b<game->getNumberOfUnits(); b++) {
					if(isTeamUnit(b) && game->unit[b].isVacantDefensiveUnit()) {
						int d=manhattanDist((Point2Di)game->unit[b].position(), (Point2Di)game->unit[u].position());
						if(d<dMin) {
							building=b;
							dMin=d;
						}
					}
				}
				if(building != -1)
					game->unit[u].newAICommand(COMMAND_GARRISON, game->unit[building].position(), building);
			}
		}
		//Ranged Military
		for(UnitID u = 0; u<game->getNumberOfUnits(); u++) {
			if (isTeamUnit(u)
			   && game->unit[u].getCategory() == UNIT_INFANTRY
			   && game->unit[u].getTypeInfo().attack>0 
			   && isRangedWeapon(game->unit[u].getTypeInfo().weaponType) 
			   && game->unit[u].stance==STANCE_GENERAL 
			   && (game->unit[u].isIdle() || game->unit[u].isFleeing())) {
				UnitID building=-1;
				int dMin=game->unit[u].getTypeInfo().los;
				for(UnitID b=0; b<game->getNumberOfUnits(); b++) {
					if(isTeamUnit(b) && game->unit[b].isVacantDefensiveUnit()) {
						int d=manhattanDist((Point2Di)game->unit[b].position(), (Point2Di)game->unit[u].position());
						if(d<dMin) {
							building=b;
							dMin=d;
						}
					}
				}
				if(building != -1) {
					game->unit[u].newAICommand(COMMAND_GARRISON, game->unit[building].position(), building);
					break;
				}
			}
		}
	}

	void TeamAI::attackEnemy() {
		vector<UnitID> soldiers;
		for(UnitID u=0; u<game->getNumberOfUnits(); u++)
			if(isTeamUnit(u) && !isConstruction(game->unit[u].getCategory()) && !game->unit[u].getTypeInfo().isWorker)
				soldiers.push_back(u);
		if(soldiers.size()<7)
			return;

		vector<TeamID> enemyTeamIndex;
		for(int t=1; t<=game->getNumberOfTeams(); t++)
			if(game->diplomacy(team, t)==DIPLOMACY_ENEMY)
				enemyTeamIndex+=t;
		if(enemyTeamIndex.size()<=0)
			return;
		attackinfo.attackingTeam=randomVectorElement(enemyTeamIndex);

		attackinfo.position=Point2Di(-1, -1);
		for(UnitID u=0; u<game->getNumberOfUnits() && attackinfo.position==Point2Di(-1, -1); u++)
			if(game->unit[u].team==attackinfo.attackingTeam && game->unit[u].isAlive() && isConstruction(game->unit[u].getCategory()))
				attackinfo.position=game->unit[u].position();
		for(UnitID u=0; u<game->getNumberOfUnits() && attackinfo.position==Point2Di(-1, -1); u++)
			if(game->unit[u].team==attackinfo.attackingTeam && game->unit[u].isAlive())
				attackinfo.position=game->unit[u].position();
		if(attackinfo.position==Point2Di(-1, -1))
			return;

		Point2Di centre=(game->getWidth()/2, game->getHeight()/2);
		for(int s=0; s<5; s++)
			if(game->unit[soldiers[s]].isIdle()) {
				game->unit[soldiers[s]].newAICommand(COMMAND_MOVE, centre, true, true);
				game->unit[soldiers[s]].newAICommand(COMMAND_MOVE, attackinfo.position, true, true);
				attackinfo.nUnitsAttacking++;
			}

		attackinfo.attackStarted=true;
	}
	void TeamAI::performSpecialAttack() {
		for(UnitID u = 0; u<game->getNumberOfUnits(); u++) {
			if(isTeamUnit(u) 
			   && game->unit[u].getTypeInfo().isHeroic
			   && game->unit[u].getTypeInfo().attack>0) {
				//Non-friendly Special power
				for(int specialPowerIndex = 0; specialPowerIndex < game->unit[u].getTypeInfo().specialPower.size(); specialPowerIndex++) {
					if(!game->unit[u].getTypeInfo().specialPower[specialPowerIndex].friendly 
					   && game->unit[u].specialAbilityRecharged(specialPowerIndex) == 100
					   && satisfiesInProbability(0.5)) {
						UnitID target=-1;
						int dMin=game->unit[u].getLOS();
						for(UnitID t = 0; t < game->getNumberOfUnits(); t++) {
							if(isEnemyUnit(t) 
							   && game->unit[t].getTypeInfo().isHeroic 
							   && game->unit[t].getTypeInfo().isMilitaryUnit()
							   && game->unit[t].isVisibleToUnit(u)
							   && game->unit[u].isValidSpecialAttack(Unit::Command(game->unit[t].position(), COMMAND_SPECIAL_ATTACK, t, false, false, specialPowerIndex))) {
								int d=manhattanDist((Point2Di)game->unit[t].position(), (Point2Di)game->unit[u].position());
								if(d<dMin) {
									target=t;
									dMin=d;
								}
							}
						}
						if(target != -1) {
							game->unit[u].newAICommand(COMMAND_SPECIAL_ATTACK, target, false, false, false, specialPowerIndex);
							break;
						}
					}
				}
				//Transformation
				if(game->unit[u].getAutoTransformTimeRemaining()<=0 && game->unit[u].state==STATE_ATTACKING) {
					int transformIndex = -1;
					for(int i = 0; i < game->unit[u].getTypeInfo().transformation.size(); i++) {
						if(game->unit[u].getTypeInfo().transformation[i].rechargeTime>500 
						   && game->unit[u].transformationRecharged(i)==100
						   && satisfiesInProbability(0.25))
							transformIndex = i;
					}
					if(transformIndex>=0)
						game->unit[u].commandTransform(transformIndex);
				}
			}
		}
	}

	void TeamAI::calculateBuildingLocationBias() {
		UnitID biasUnitID=-1;
		for(UnitID u=0; u<game->getNumberOfUnits(); u++)
			if(isTeamUnit(u) && game->unit[u].getType()==getBuildableUnitType().citadel.type) {
				biasUnitID=u;
				break;
			}
		if(biasUnitID<0)
			for(UnitID u=0; u<game->getNumberOfUnits(); u++)
				if(isTeamUnit(u) && isConstruction(game->unit[u].getCategory())) {
					biasUnitID=u;
					break;
				}
		if(biasUnitID>=0)
			buildingLocationBias=game->unit[biasUnitID].position();
		else
			buildingLocationBias=Point2Di(0, 0);
	}

	bool TeamAI::isTeamUnit(int u) const {
		return game->unit[u].isAlive() && game->unit[u].team==team;
	}
	bool TeamAI::isAllyUnit(int u) const {
		return game->unit[u].isAlive() && game->diplomacy(game->unit[u].team, team)==DIPLOMACY_ALLY;
	}
	bool TeamAI::isUnitVisible(int u) const {
		Point2Di pos(game->unit[u].x, game->unit[u].y);
		return pos.in(0, 0, game->getWidth()-1, game->getHeight()-1) && game->teams[team].visible[pos.y][pos.x];
	}
	bool TeamAI::isEnemyUnit(int u) const {
		return game->unit[u].isAlive() && game->diplomacy(game->unit[u].team, team)==DIPLOMACY_ENEMY;
	}
	bool TeamAI::isActive() const {
		return active;
	}
};
