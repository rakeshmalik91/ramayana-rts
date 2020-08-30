#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"

namespace ramayana {	
	
	void Game::updateSelectionAbility() {
		selectionAbility.clear();
		if(!isEditable()) {
			if(nSelected>0 && selectedUnit(0).team==playerTeam) {
				if(isConstruction(unitTypeInfo[selectedUnit(0).type].category)) {
					selectionAbility.push_back(Ability(ABILITY_DESTROY, 0, "Destroy Selected Building (Delete)", &uiTextureSet.icon_kill));
					for(int o=0; o<MAX_OBJECT_TYPE; o++) {
						bool canBuild=true;
						for(int s=0; s<nSelected && canBuild; s++)
							canBuild&=unitTypeInfo[selectedUnit(s).type].canBuild[o];
						if(canBuild) {
							string name = "";
							if (unitTypeInfo[o].isHeroic) {
								name = "Summon ";
							} else {
								name = "Train ";
							}
							name += unitTypeInfo[o].name + " - ";
							Resource<> cost=teams[playerTeam].getUnitCost(o);
							if(cost.food>0) name+="Food:"+toString(cost.food)+" ";
							if(cost.wood>0) name+="Wood:"+toString(cost.wood)+" ";
							if(cost.stone>0) name+="Stone:"+toString(cost.stone)+" ";
							if(cost.metal>0) name+="Metal:"+toString(cost.metal)+" ";
							if(unitTypeInfo[o].population>0) name+="Population:"+toString(unitTypeInfo[o].population);
							if(unitTypeInfo[o].maxPopulationIncrease>0) name+="Population Increase:"+toString(unitTypeInfo[o].maxPopulationIncrease);
							selectionAbility.push_back(Ability(ABILITY_BUILD, o, name, &unitTypeInfo[o].iconSmall, false, 0, selectedUnit(0).getBuildPercentage(o), selectedUnit(0).getBuildCount(o)));
						}
					}
					bool unitsGarrisoned=false;
					for(int s=0; s<nSelected && !unitsGarrisoned; s++) 
						unitsGarrisoned|=!unit[selectList[s]-1].garrisonedUnits.empty();
					if(unitsGarrisoned)
						selectionAbility.push_back(Ability(ABILITY_DEPLOY_ALL, 0, "Deploy All Garrisoned Units", &uiTextureSet.icon_deploy));
					if(nSelected==1)
						for(int gu=0; gu<unit[selectList[0]-1].garrisonedUnits.size(); gu++)
							selectionAbility.push_back(Ability(ABILITY_DEPLOY, unit[selectList[0]-1].garrisonedUnits[gu], "Deploy a "+unitTypeInfo[unit[unit[selectList[0]-1].garrisonedUnits[gu]].type].name, &unitTypeInfo[unit[unit[selectList[0]-1].garrisonedUnits[gu]].type].iconSmall));
				} else {
					selectionAbility.push_back(Ability(ABILITY_DESTROY, 0, "Kill Selected Unit (Delete)", &uiTextureSet.icon_kill));
					bool haveCommand=false;
					for(int s=0; s<nSelected && !haveCommand; s++) 
						haveCommand|=!selectedUnit(s).isIdle();
					if(haveCommand)
						selectionAbility.push_back(Ability(ABILITY_STOP, 0, "Stop (S)", &uiTextureSet.icon_stop));
					bool stanceGeneral=false, stanceStandGround=false, stanceHoldFire=false;
					for(int s=0; s<nSelected; s++) {
						stanceGeneral|=(unit[selectList[s]-1].stance==STANCE_GENERAL);
						stanceStandGround|=(unit[selectList[s]-1].stance==STANCE_STANDGROUND);
						stanceHoldFire|=(unit[selectList[s]-1].stance==STANCE_HOLDFIRE);
					}
					selectionAbility.push_back(Ability(ABILTY_STANCE_GENERAL, 0, "General Stance", &uiTextureSet.icon_general_stance, stanceGeneral));
					selectionAbility.push_back(Ability(ABILITY_STANCE_STANDGROUND, 0, "Stand Ground", &uiTextureSet.icon_standground_stance, stanceStandGround));
					selectionAbility.push_back(Ability(ABILITY_STANCE_HOLD_FIRE, 0, "Hold Fire", &uiTextureSet.icon_holdfire_stance, stanceHoldFire));
					for(int o=0; o<MAX_OBJECT_TYPE; o++) {
						bool canBuild=true;
						for(int s=0; s<nSelected && canBuild; s++)
							canBuild&=unitTypeInfo[unit[selectList[s]-1].type].canBuild[o];
						if(canBuild) {
							string name="Build "+unitTypeInfo[o].name+" - ";
							Resource<> cost=teams[playerTeam].getUnitCost(o);
							if(cost.food>0) name+="Food:"+toString(cost.food)+" ";
							if(cost.wood>0) name+="Wood:"+toString(cost.wood)+" ";
							if(cost.stone>0) name+="Stone:"+toString(cost.stone)+" ";
							if(cost.metal>0) name+="Metal:"+toString(cost.metal)+" ";
							if(unitTypeInfo[o].population>0) name+="Population:"+toString(unitTypeInfo[o].population);
							if(unitTypeInfo[o].maxPopulationIncrease>0) name+="Population Increase:"+toString(unitTypeInfo[o].maxPopulationIncrease);
							selectionAbility.push_back(Ability(ABILITY_BUILD, o, name, &unitTypeInfo[o].iconSmall, false, 0, unit[selectList[0]-1].getBuildPercentage(o), unit[selectList[0]-1].getBuildCount(o)));
						}
					}
					if(nSelected==1) {
						for(int i=0; i<unitTypeInfo[unit[selectList[0]-1].type].transformation.size();i++) {
							if (!unitTypeInfo[unit[selectList[0] - 1].type].transformation[i].hide && unit[selectList[0] - 1].level >= unitTypeInfo[unit[selectList[0] - 1].type].transformation[i].neededLevel) {
								float timeRemaining=unit[selectList[0]-1].transformationRechargeTimeRemaining(i);
								selectionAbility.push_back(
									Ability(
										ABILITY_TRANSFORM, 
										unitTypeInfo[unit[selectList[0]-1].type].transformation[i].unitType, 
										unitTypeInfo[unit[selectList[0]-1].type].transformation[i].name+(timeRemaining>0?" ("+timeAsString(timeRemaining)+" remaining)":""), 
										unitTypeInfo[unit[selectList[0]-1].type].transformation[i].icon, 
										unit[selectList[0]-1].transformationRecharged(i)<100, 
										i, 
										100-unit[selectList[0]-1].transformationRecharged(i)));
							}
						}
						for(int i=0; i<unitTypeInfo[unit[selectList[0]-1].type].specialPower.size();i++) {
							if(unit[selectList[0]-1].level>=unitTypeInfo[unit[selectList[0]-1].type].specialPower[i].neededLevel) {
								float timeRemaining=unit[selectList[0]-1].specialAbilityRechargeTimeRemaining(i);
								selectionAbility.push_back(
									Ability(
										ABILITY_SPECIAL, 
										0, 
										unitTypeInfo[unit[selectList[0]-1].type].specialPower[i].name+(timeRemaining>0?" ("+timeAsString(timeRemaining)+" remaining)":""), 
										unitTypeInfo[unit[selectList[0]-1].type].specialPower[i].icon, 
										unit[selectList[0]-1].specialAbilityRecharged(i)<100,
										i, 
										100-unit[selectList[0]-1].specialAbilityRecharged(i)));
							}
						}
					}
				}
			}
		} else {
			if(nSelected>0) {
				selectionAbility.push_back(Ability(ABILITY_DESTROY, 0, "Remove Unit (Delete)", &uiTextureSet.icon_remove_unit));
				selectionAbility.push_back(Ability(ABILITY_INCREASE_LEVEL, -1, "Increase Level", &uiTextureSet.icon_increase_level));
				selectionAbility.push_back(Ability(ABILITY_DECREASE_LEVEL, -1, "Decrease Level", &uiTextureSet.icon_decrease_level));
				for(int t=0; t<=nTeams; t++)
					selectionAbility.push_back(Ability(ABILITY_SET_TEAM, 0, "Set Team \""+teams[t].name+"\"", &uiTextureSet.image_blank_button, false, t, 0, 0, availableTeamColors[teams[t].color]));
			} else {
				switch(editState) {
				case MAPEDITSTATE_ADD_UNIT:
					for(int o=500; o<600; o++)
						if(unitTypeInfo[o].loaded && !unitTypeInfo[o].hideInEditor)
							selectionAbility.push_back(Ability(ABILITY_BUILD, o, unitTypeInfo[o].name, &unitTypeInfo[o].iconSmall));
					break;
				case MAPEDITSTATE_ADD_BUILDING:
					for(int o=600; o<MAX_OBJECT_TYPE; o++)
						if(unitTypeInfo[o].loaded && !unitTypeInfo[o].hideInEditor)
							selectionAbility.push_back(Ability(ABILITY_BUILD, o, unitTypeInfo[o].name, &unitTypeInfo[o].iconSmall));
					break;
				case MAPEDITSTATE_ADD_OTHERS:
					for(int o=0; o<500; o++)
						if(unitTypeInfo[o].loaded && !unitTypeInfo[o].hideInEditor)
							selectionAbility.push_back(Ability(ABILITY_BUILD, o, unitTypeInfo[o].name, &unitTypeInfo[o].iconSmall));
					break;
				case MAPEDITSTATE_CHANGE_TEAM:
					for(int t=0; t<=nTeams; t++)
						selectionAbility.push_back(Ability(ABILITY_SET_TEAM, 0, "Set Team \""+teams[t].name+"\"", &uiTextureSet.image_blank_button, false, t, 0, 0, availableTeamColors[teams[t].color]));
					break;
				case MAPEDITSTATE_CHANGE_TEXTURE:
					for(int t=0; t<N_TERRAIN_TEXTURE; t++)
						selectionAbility.push_back(Ability(ABILITY_SET_TERRAIN_TEXTURE, 0, "Texture", &uiTextureSet.icon_terrain_texture[t], false, t));
					break;
				}
			}
		}
	}
	void Game::updateSelection() {
		for(int s=0; s<nSelected; )
			if(!unit[selectedUnitIndex(s)].isSelectable())													//If unit no more selectable
				deselect(selectList[s]-1);																//remove it from select list
			else
				s++;
		updateSelectionAbility();
	}

	void Game::select(int screen_width, int screen_height, int x1, int y1, int x2, int y2, bool add=false, bool multiple=false, bool group=false) {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(SELECT_BUF_SIZE, selectBuf);
		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(0);
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		int select_xmin=min(x1, x2), select_xmax=max(x1, x2), select_ymin=min(y1, y2), select_ymax=max(y1, y2);
		int select_width=select_xmax-select_xmin, select_height=select_ymax-select_ymin;
		if(select_width*select_height>0)
			gluPickMatrix((GLdouble)select_xmin+select_width/2, (GLdouble)(viewport[3]-select_ymin-select_height/2), select_width, select_height, viewport);
		else
			gluPickMatrix((GLdouble)x1, (GLdouble)(viewport[3]-y1), 1, 1, viewport);
		gluPerspective(45.0, (float)screen_width/screen_height, 0.1, 1000.0);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		setCamera();
		for(int i=0; i<nUnit; i++) {
			if (unit[i].isSelectable() && unit[i].isRenderable() && frustum.sphereInFrustum(unit[i].position(), unit[i].getRadius())) {
				glLoadName(i+1);
				glPushMatrix();
				glTranslatef(unit[i].x, unit[i].y, unit[i].z);
				glRotatef(unit[i].angle, 0, 0, 1);
				glRotatef(unit[i].tilt, 0, 1, 0);
				glScalef(unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING);
				unit[i].render();
				glPopMatrix();
			}
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glFlush();
		
		GLuint hits=glRenderMode(GL_RENDER);
		moveTo=0;
		GLuint names, *ptr=(GLuint*)selectBuf;
	
		if(!add)
			nSelected=0;
		if(!group) {
			//Add/remove isSelected unit
			for(unsigned int i=0; i<hits; i++) {
				names=*ptr;
				ptr+=3;
				for(unsigned int j=0; j<names; j++) {
					if(*ptr!=0 && *ptr<POS_NAME_START) {
						if(add && !multiple && isSelected(*ptr-1))
							deselect(*ptr-1);
						else if(!isSelected(*ptr-1))
							selectList[nSelected++]=*ptr;
					}
					ptr++;
				}
			}
			//choose building/unit selection
			if(!isEditable()) {
				if(nSelected>1) {
					for(int i=0; i<nSelected; i++)
						if(unit[selectList[i]-1].team!=playerTeam || isConstruction(unitTypeInfo[unit[selectList[i]-1].type].category))
							deselect(selectList[i--]-1);
					if(!multiple && !add)
						nSelected=1;
				}
			}
		} else {
			int selectedUnit=-1;
			//add isSelected unit
			for(unsigned int i=0; i<hits; i++) {
				names=*ptr;
				ptr+=3;
				for(unsigned int j=0; j<names; j++) {
					if(*ptr!=0 && *ptr<POS_NAME_START) {
						selectedUnit=*ptr-1;
						break;
					}
					ptr++;
				}
				if(selectedUnit>=0) break;
			}
			//add group
			if(selectedUnit>=0)
				selectGroup(selectedUnit, add);
		}
		for(int i=0; i<nSelected && i<10; i++) {
			Unit &u=selectedUnit(i);
			if(u.getTypeInfo().selectSound.size()>0 && u.team==playerTeam && satisfiesInProbability(0.5/nSelected))
				playAudio(randomVectorElement(u.getTypeInfo().selectSound), CHANNEL_SPEECH, soundAngle(u.x, u.y), soundDistance(u.x, u.y));
		}
		if(isEditable() && nSelected>0)
			playerTeam = selectedUnit(0).team;
		glutPostRedisplay();
	}
	void Game::selectUnit(int u, bool add=false) {
		if(isSelected(u))
			return;
		if(!add || unit[u].team!=playerTeam || (nSelected>0 && selectedUnit(0).team!=playerTeam))
			nSelected=0;
		selectList[nSelected]=u+1;
		nSelected++;
	}
	void Game::selectGroup(int u0, bool add=false) {
		if(!add)
			nSelected=0;
		if(isEditable()) {
			//select select 1st selection team unit
			for(int u=0; u<nUnit; u++)
				if(unit[u].team==unit[u0].team && unit[u].type==unit[u0].type)
					selectUnit(u, true);
		} else {
			//select only player unit
			float r=0;
			Point2D mid=unit[u0].position();
			selectUnit(u0, true);
			int added=1;
			while(added>0) {
				added=0;
				for(int u=0; u<nUnit; u++)
					if(unit[u].team==playerTeam && unit[u].type==unit[u0].type && !isSelected(u))
						for(int i=0; i<nSelected; i++)
							if(dist(unit[u].position(), selectedUnit(i).position())<=10+selectedUnit(i).getRadius()) {
								selectUnit(u, true);
								added++;
								break;
							}
			}
		}
	}
	void Game::clearSelection() {
		nSelected=0;
	}
	void Game::goToSelected() {
		if(nSelected>0) {
			goTo(unit[selectList[0]-1].x, unit[selectList[0]-1].y);
			setAmbienceSound();
		}
	}
	UnitID Game::selectedUnitIndex(int selIndex) {
		if(selIndex<nSelected)
			return selectList[selIndex]-1;
		else if(nSelected>0)
			return selectList[0]-1;
		else 
			return 0;
	}
	Unit& Game::selectedUnit(int selIndex) {
		return unit[selectedUnitIndex(selIndex)];
	}
	void Game::filterSelectionRemoveAll(int unitType) {
		for(int s=0; s<nSelected; )
			if(unit[selectList[s]-1].type==unitType)
				deselect(selectList[s]-1);
			else
				s++;
	}
	void Game::filterSelectionRemoveFirst(int unitType) {
		for(int s=0; s<nSelected; )
			if(unit[selectList[s]-1].type==unitType) {
				deselect(selectList[s]-1);
				break;
			} else
				s++;
	}
	void Game::filterSelectionKeep(int unitType) {
		for(int s=0; s<nSelected; )
			if(unit[selectList[s]-1].type!=unitType)
				deselect(selectList[s]-1);
			else
				s++;
	}
	bool Game::isSelected(int unitID) {
		for(int i=0; i<nSelected; i++)
			if(selectList[i]-1==unitID)
				return true;
		return false;
	}
	void Game::deselect(int unitID) {
		for(int i=0; i<nSelected; ) {
			if(selectList[i]-1==unitID) {
				for(int j=i; j<nSelected-1; j++)
					selectList[j]=selectList[j+1];
				nSelected--;
			} else i++;
		}
	}
	int Game::numberOfSelectedUnit() {
		return nSelected;
	}
	
	void Game::selectIdleWorker(bool all) {
		int findStart=0;
		if(nSelected>0)
			findStart=selectedUnitIndex(nSelected-1)+1;
		nSelected=0;
		for(int i=0; i<(all?teams[playerTeam].nIdleWorker:1); i++) {
			int idleWorkerIndex=-1;
			for(int u=findStart; u<nUnit && idleWorkerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isIdle() && !isSelected(u))
					idleWorkerIndex=u;
			for(int u=0; u<findStart && idleWorkerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isIdle() && !isSelected(u))
					idleWorkerIndex=u;
			if(idleWorkerIndex>=0)
				selectUnit(idleWorkerIndex, true);
		}
	}
	void Game::selectFoodGatherer(bool all) {
		int findStart=0;
		if(nSelected>0)
			findStart=selectedUnitIndex(nSelected-1)+1;
		nSelected=0;
		for(int i=0; i<(all?teams[playerTeam].nGatherer.food:1); i++) {
			int workerIndex=-1;
			for(int u=findStart; u<nUnit && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isFoodGatherer() && !isSelected(u))
					workerIndex=u;
			for(int u=0; u<findStart && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isFoodGatherer() && !isSelected(u))
					workerIndex=u;
			if(workerIndex>=0)
				selectUnit(workerIndex, true);
		}
	}
	void Game::selectWoodGatherer(bool all) {
		int findStart=0;
		if(nSelected>0)
			findStart=selectedUnitIndex(nSelected-1)+1;
		nSelected=0;
		for(int i=0; i<(all?teams[playerTeam].nGatherer.wood:1); i++) {
			int workerIndex=-1;
			for(int u=findStart; u<nUnit && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isWoodGatherer() && !isSelected(u))
					workerIndex=u;
			for(int u=0; u<findStart && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isWoodGatherer() && !isSelected(u))
					workerIndex=u;
			if(workerIndex>=0)
				selectUnit(workerIndex, true);
		}
	}
	void Game::selectStoneGatherer(bool all) {
		int findStart=0;
		if(nSelected>0)
			findStart=selectedUnitIndex(nSelected-1)+1;
		nSelected=0;
		for(int i=0; i<(all?teams[playerTeam].nGatherer.stone:1); i++) {
			int workerIndex=-1;
			for(int u=findStart; u<nUnit && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isStoneGatherer() && !isSelected(u))
					workerIndex=u;
			for(int u=0; u<findStart && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isStoneGatherer() && !isSelected(u))
					workerIndex=u;
			if(workerIndex>=0)
				selectUnit(workerIndex, true);
		}
	}
	void Game::selectMetalGatherer(bool all) {
		int findStart=0;
		if(nSelected>0)
			findStart=selectedUnitIndex(nSelected-1)+1;
		nSelected=0;
		for(int i=0; i<(all?teams[playerTeam].nGatherer.metal:1); i++) {
			int workerIndex=-1;
			for(int u=findStart; u<nUnit && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isMetalGatherer() && !isSelected(u))
					workerIndex=u;
			for(int u=0; u<findStart && workerIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isWorker && unit[u].isMetalGatherer() && !isSelected(u))
					workerIndex=u;
			if(workerIndex>=0)
				selectUnit(workerIndex, true);
		}
	}
	void Game::selectHero(bool all) {
		int findStart=0;
		if(nSelected>0)
			findStart=selectedUnitIndex(nSelected-1)+1;
		nSelected=0;
		for(int i=0; i<(all?teams[playerTeam].nHero:1); i++) {
			int heroIndex=-1;
			for(int u=findStart; u<nUnit && heroIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isHeroic && !isSelected(u))
					heroIndex=u;
			for(int u=0; u<findStart && heroIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isHeroic && !isSelected(u))
					heroIndex=u;
			if(heroIndex>=0)
				selectUnit(heroIndex, true);
		}
	}
	void Game::selectMilitary(bool all) {
		int findStart=0;
		if(nSelected>0)
			findStart=selectedUnitIndex(nSelected-1)+1;
		nSelected=0;
		for(int i=0; i<(all?teams[playerTeam].nMilitary:1); i++) {
			int heroIndex=-1;
			for(int u=findStart; u<nUnit && heroIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isMilitaryUnit() && !isSelected(u))
					heroIndex=u;
			for(int u=0; u<findStart && heroIndex==-1; u++)
				if(unit[u].team==playerTeam && unit[u].isAlive() && unitTypeInfo[unit[u].type].isMilitaryUnit() && !isSelected(u))
					heroIndex=u;
			if(heroIndex>=0)
				selectUnit(heroIndex, true);
		}
	}

	void Game::setSelectedUnitTeam(TeamID t) {
		for(int s=0; s<nSelected; s++)
			if(!isNaturalUnit(selectedUnit(s).getCategory()))
				selectedUnit(s).team=t;
	}

	void Game::increaseSelectedUnitLevel(int val) {
		for(int s = 0; s < nSelected; s++) {
			selectedUnit(s).level = clamp(selectedUnit(s).level+val, 1, 10);
		}
	}
}
