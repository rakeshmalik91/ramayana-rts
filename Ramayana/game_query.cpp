#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"

namespace ramayana {

	WaveFrontObjSequence& Unit::currentAnimation() const {
		return unitTypeInfo[type].getObject(state, hitPoint);
	}
	WaveFrontObj& Unit::currentFrame() const {
		return currentAnimation().getFrame(frame_number);
	}
	
	int Game::getNumberOfUnits() {
		return nUnit;
	}
	int Game::getNumberOfTeams() {
		return nTeams;
	}

	int Game::getMaxPopulation(TeamID team) {
		return clampHigh(teams[team].maxPopulation, MAX_TOTAL_POPULATION / nTeams);
	}
	
	bool Game::hasEnoughResourceToBuild(int unitType, int team) const {
		return unitTypeInfo[unitType].cost.food<=teams[team].resource.food &&
			unitTypeInfo[unitType].cost.wood<=teams[team].resource.wood &&
			unitTypeInfo[unitType].cost.stone<=teams[team].resource.stone &&
			unitTypeInfo[unitType].cost.metal<=teams[team].resource.metal &&
			(unitTypeInfo[unitType].population==0 || teams[team].population+unitTypeInfo[unitType].population<=teams[team].maxPopulation);
	}
	bool Game::isTerrainSuitableToBuild(Point2D position, float angle, int unitType, bool suppressMessage) const {
		UnitTypeInfo &o=unitTypeInfo[unitType];
		int x=roundInt(position.x), y=roundInt(position.y);
		if(!Point2Di(x, y).in(0, 0, width-1, height-1))
			return false;
		bool suitableTerrain=true;
		float h=z[y][x];
		UnitID nearestTree;
		vector<Point2Di> points;
		switch(o.category) {
		case UNIT_MINE:
			suitableTerrain=isUneven(position.x, position.y, 0.5);
			if(!suppressMessage && !suitableTerrain) {
				throw HUDMessage("Mines must be built on uneven land...");
			}
		case UNIT_BUILDING:
			points=o.getOccupiedPoints(position, angle);
			for(int p=0; p<points.size() && suitableTerrain; p++) {
				if(points[p].in(0, 0, width-1, height-1)) {
					suitableTerrain&=(abs(z[points[p].y][points[p].x]-h)<=Game::MAX_SUITABLE_UNEVEN_BULDING_LAND) && z[points[p].y][points[p].x]>=currentWaterLevel;
				}
			}
			if(!suppressMessage && !suitableTerrain) {
				throw HUDMessage("Terrain is not suitable for this building...");
			}
			break;
		case UNIT_TREE_HOUSE:
			nearestTree=nearestUnitIndex(position.x, position.y, UNIT_TREE);
			if(nearestTree<0 || dist(position, (Point2D)unit[nearestTree].position())>=1) {
				suitableTerrain=false;
				if(!suppressMessage) {
					throw HUDMessage("Tree house must be built on trees...");
				}
			} else {
				suitableTerrain=(roundInt(position.x)==roundInt(unit[nearestTree].x) && roundInt(position.y)==roundInt(unit[nearestTree].y));
			}
			break;
		case UNIT_WATER_CONSTRUCTION:
			points=o.getOccupiedPoints(position, angle);
			for(int p=0; p<points.size() && suitableTerrain; p++) {
				if(points[p].in(0, 0, width-1, height-1)) {
					suitableTerrain &= (z[points[p].y][points[p].x]<currentWaterLevel);
				}
			}
			if(!suppressMessage && !suitableTerrain) {
				throw HUDMessage("This construction must be built on water...");
			}
			break;
		}
		return suitableTerrain;
	}
	bool Game::isTerrainFreeToBuild(Point2D position, float angle, int unitType, int team, bool suppressMessage) const {
		UnitTypeInfo &o=unitTypeInfo[unitType];
		if(roundInt(position.y)<0 || roundInt(position.y)>=height || roundInt(position.x)<0 || roundInt(position.x)>=width) {
			return false;
		}
		bool freeTerrain=true;
		freeTerrain=true;
		vector<Point2Di> points=o.getOccupiedPoints(position, angle);
		for(int p=0; p<points.size() && freeTerrain; p++) {
			if(points[p].in(0, 0, width-1, height-1)) {
				if(teams[team].explored[points[p].y][points[p].x]) {
					freeTerrain&=(minimap[points[p].y][points[p].x].landUnit==-1);
				} else {
					freeTerrain=false;
					if(!suppressMessage)
						throw HUDMessage("Terrain must be explored first...");
				}
			}
		}
		if(!suppressMessage && !freeTerrain) {
			throw HUDMessage("Terrain is blocked by other unit...");
		}
		return freeTerrain;
	}
	UnitID Game::getNearestFreeTree(Point2Di position) {
		int index=-1;
		int dmin=INT_MAX;
		for(int u=0; u<nUnit; u++) {
			if(unit[u].isAlive() && unit[u].getCategory()==UNIT_TREE && unit[u].capturedByUnit==-1) {
				int d=manhattanDist(position, (Point2Di)unit[u].position());
				if(d<dmin) {
					dmin=d;
					index=u;
				}
			}
		}
		return index;
	}
	Tuple<Point2D, int> Game::getBuildablePlace(Point2Di start, int buildingType, int team, bool** mask) {
		int dx[8]={-1, 1, 0, 0,-1,-1, 1, 1};
		int dy[8]={ 0, 0,-1, 1,-1, 1,-1, 1};
		queue<Point2Di> points;
		bool **visited=allocate<bool>(getWidth(), getHeight());
		unsigned int startTime=SDL_GetTicks();

		if(unitTypeInfo[buildingType].category==UNIT_TREE_HOUSE) {
			UnitID treeID=getNearestFreeTree(start);
			if(treeID>=0) {
				return Tuple<Point2D, int>(unit[treeID].position(), 0);
			}
		}

		setAll(visited, getWidth(), getHeight(), false);
		points.push(start);
		while(!points.empty()) {
			Point2Di p=points.front();
			points.pop();
			for(int angle=315; angle>=0; angle-=45) {
				bool suitableTerrain=isTerrainSuitableToBuild(p, angle, buildingType);
				bool freeTerrain=suitableTerrain?isTerrainFreeToBuild(p, angle, buildingType, team):false;
				if(freeTerrain) {
					deallocate(visited, getWidth(), getHeight());
					return Tuple<Point2D, int>(p, angle);
				}
			}
			for(int i=0; i<8; i++) {
				Point2Di neighbour(p.x+dx[i], p.y+dy[i]);
				if(neighbour.in(0, 0, getWidth()-1, getHeight()-1) && !visited[neighbour.y][neighbour.x] && mask[neighbour.y][neighbour.x]) {
					points.push(neighbour);
					visited[neighbour.y][neighbour.x]=true;
				}
			}
			if(SDL_GetTicks()-startTime<1000) {
				break;
			}
		}
		deallocate(visited, getWidth(), getHeight());
		return Tuple<Point2D, int>(Point2Di(-1, -1), 0);
	}
	
	string Game::getMessage() {
		return message;
	}
	string Game::getObjectives() {
		return objective;
	}
	float Game::getMessageTransparency() {
		return (float)messageTimer/MAX_MESSAGE_TIME;
	}
	int Game::getGamePlayTime() const {
		return gamePlayTime;
	}
	
	bool Game::isCampaign() {
		return false;
	}
	bool Game::isSkirmish() {
		return false;
	}
	
	bool Game::isLoaded() const {
		return loaded;
	}
	bool Game::isPaused() const {
		return paused;
	}

	float Game::getBlend(Point2Di p) const {
		if(p.in(0, 0, width-1, height-1))
			return blend[p.y][p.x];
		return 0;
	}
	void Game::setBlend(Point2Di p, float f) {
		if(p.in(0, 0, width-1, height-1))
			blend[p.y][p.x]=f;
	}

	AbilityCategory toAbilityCategory(AbilityType a) {
		switch(a) {
		case ABILITY_BUILD:
			return ABILITY_CATEGORY_BUILD;
		case ABILTY_STANCE_GENERAL:
		case ABILITY_STANCE_STANDGROUND:
		case ABILITY_STANCE_HOLD_FIRE:
			return ABILITY_CATEGORY_STANCE;
		case ABILITY_DEPLOY:
		case ABILITY_DEPLOY_ALL:
			return ABILITY_CATEGORY_DEPLOY;
		case ABILITY_SPECIAL:
			return ABILITY_CATEGORY_SPECIAL;
		case ABILITY_PASSIVE:
			return ABILITY_CATEGORY_PASSIVE;
		case ABILITY_TRANSFORM:
			return ABILITY_CATEGORY_TRANSFORM;
		case ABILITY_SET_TEAM:
			return ABILITY_CATEGORY_SET_TEAM;
		case ABILITY_SET_TERRAIN_TEXTURE:
		case ABILITY_INCREASE_LEVEL:
		case ABILITY_DECREASE_LEVEL:
			return ABILITY_CATEGORY_EDIT;
		default:
			return ABILITY_CATEGORY_GENERAL;
		}
	}

	string Game::getFileName() const {
		return filename;
	}

	PathfinderAStar* Game::getPathFinder(UnitID id) const {
		return updateUnitThreadResource[id % N_UNIT_UPDATE_THRAD].pathfinder;
	}
}
