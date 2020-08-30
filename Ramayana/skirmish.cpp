#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "skirmish.h"
#include "audio.h"

namespace ramayana {
	
	void Skirmish::initTeams(Team teams[], int nTeams, int playerTeam, int food, int wood, int stone, int metal) {
		this->nTeams=nTeams;
		this->playerTeam=playerTeam;
		this->teams[0] = Team(NATURE_COLOR, false, 0, 0);
		this->teams[0].name = "Nature";
		this->teams[0].init(this, unitTypeInfo, 0);
		for(int t = 1; t <= this->nTeams; t++) {
			this->teams[t] = teams[t-1];
			this->teams[t].name = "Team-"+t;
			this->teams[t].resource = Resource<>(food, wood, stone, metal);
			this->teams[t].init(this, unitTypeInfo, t);
			this->teams[t].hasAI = !this->teams[t].human;
			if (this->playerTeam == 0 && this->teams[t].human)
				this->playerTeam = t;
		}
	}
	void Skirmish::load(string fname, UnitTypeInfo* unitTypeInfo, Team teams[], int nTeams, int playerTeam, int food, int wood, int stone, int metal) {
		Game::load(fname, unitTypeInfo, teams, nTeams, playerTeam, food, wood, stone, metal, false);
	
		for(int t=1; t<=this->nTeams; t++) {
			switch(this->teams[t].faction) {
			case 1:
				loadUnit(606, 1);
				addUnit(startPos[this->teams[t].startPositionIndex].x+0, height-startPos[this->teams[t].startPositionIndex].y+0, 606, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+2, height-startPos[this->teams[t].startPositionIndex].y+0, 500, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+2, height-startPos[this->teams[t].startPositionIndex].y+1, 500, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+3, height-startPos[this->teams[t].startPositionIndex].y+0, 500, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+3, height-startPos[this->teams[t].startPositionIndex].y+1, 500, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+4, height-startPos[this->teams[t].startPositionIndex].y+0, 500, t);
				break;
			case 2:
				loadUnit(609, 1);
				addUnit(startPos[this->teams[t].startPositionIndex].x+0, height-startPos[this->teams[t].startPositionIndex].y+0, 609, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+2, height-startPos[this->teams[t].startPositionIndex].y+0, 550, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+2, height-startPos[this->teams[t].startPositionIndex].y+1, 550, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+3, height-startPos[this->teams[t].startPositionIndex].y+0, 550, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+3, height-startPos[this->teams[t].startPositionIndex].y+1, 550, t);
				addUnit(startPos[this->teams[t].startPositionIndex].x+4, height-startPos[this->teams[t].startPositionIndex].y+0, 550, t);
				break;
			case 3:
				break;
			}
		}
	
		if (this->playerTeam > 0) {
			goTo(this->startPos[this->teams[this->playerTeam].startPositionIndex].x, 
				this->startPos[this->teams[this->playerTeam].startPositionIndex].y);
		}
	}
	bool Skirmish::isSkirmish() {
		return true;
	}
}

#include "stdafx.h"