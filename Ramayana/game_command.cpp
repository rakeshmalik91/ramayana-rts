#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"

namespace ramayana {	

	void Game::lockCommand() {
		commandLocked = true;
	}
	void Game::unlockCommand() {
		commandLocked = false;
	}

	GLuint Game::getMouseHits(int screen_width, int screen_height, int x, int y, bool renderObject) {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(SELECT_BUF_SIZE, selectBuf);
		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(0);
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3]-y), 1, 1, viewport);
		gluPerspective(45.0, (float)screen_width/screen_height, 0.1, 1000.0);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		setCamera();
		int groupSize=10;
		for(int gr=0; gr<height/groupSize; gr++)
			for(int gc=0; gc<width/groupSize; gc++)
				if(frustum.sphereInFrustum(Point3D(gc*groupSize+groupSize/2, gr*groupSize+groupSize/2, 0), groupSize*SQRT2))
					for(int r=gr*groupSize; r<gr*groupSize+groupSize && r<height-1; r++)
						for(int c=gc*groupSize; c<gc*groupSize+groupSize && c<width-1; c++) {
							glLoadName(POS_NAME_START+(height-1)*c+r);
							glBegin(GL_QUADS);
							glVertex3f(c,	r,		clampLow(z[(int)(r  )][(int)(c  )], currentWaterLevel));
							glVertex3f(c+1,	r,		clampLow(z[(int)(r  )][(int)(c+1)], currentWaterLevel));
							glVertex3f(c+1,	r+1,	clampLow(z[(int)(r+1)][(int)(c+1)], currentWaterLevel));
							glVertex3f(c,	r+1,	clampLow(z[(int)(r+1)][(int)(c  )], currentWaterLevel));
							glEnd();
						}
		if(renderObject)
			for(int i=0; i<nUnit; i++) {
				if(unit[i].isSelectable() && unit[i].isRenderable() && (isEditable() || unitTypeInfo[unit[i].type].category!=UNIT_DECORATION) && frustum.sphereInFrustum(unit[i].position(), unit[i].getRadius())) {
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
		return glRenderMode(GL_RENDER);
	}
	void Game::processMouseHits(int screen_width, int screen_height, int x, int y, bool renderObject) {
		GLuint hits=getMouseHits(screen_width, screen_height, x, y, renderObject);
		GLuint names, *ptr=(GLuint*)selectBuf;
		GLuint z1Min=0xffffffff, z2Min=0xffffffff;
		moveTo=targetUnit=-1;
		for(unsigned int i=0; i<hits; i++) {
			names=*ptr;
			ptr++;
			GLuint z1=*ptr;
			ptr++;
			GLuint z2=*ptr;
			ptr++;
			for(unsigned int j=0; j<names; j++) {
				if(*ptr>=POS_NAME_START) {
					if(z1<z1Min) {
						moveTo=*ptr;
						z1Min=z1;
					}
				} else if(*ptr>0) {
					if(targetUnit==-1)
						targetUnit=*ptr-1;
				}
				ptr++;
			}
		}
	}
	void Game::setCursor(int screen_width, int screen_height, int x, int y) {	
		bool renderObject=(cursor.type==Cursor::TARGET || (cursor.type==Cursor::BUILD && unitTypeInfo[cursor.buildUnitType].category==UNIT_TREE_HOUSE));
		GLuint hits=getMouseHits(screen_width, screen_height, x, y, renderObject);
		GLuint names, *ptr=(GLuint*)selectBuf;
		GLuint pos=0;
		cursor.targetID=-1;
		for(unsigned int i=0; i<hits && pos==0; i++) {
			names=*ptr;
			ptr+=3;
			for(unsigned int j=0; j<names; j++) {
				if(*ptr>=POS_NAME_START)
					pos=*ptr;
				else if(*ptr>0)
					cursor.targetID=*ptr-1;
			}
		}
		if(pos>=POS_NAME_START) {
			cursor.pos.x=clamp(((pos-POS_NAME_START)/(int)(height-1)), 0, width);
			cursor.pos.y=clamp(((pos-POS_NAME_START)%(int)(height-1)), 0, height);
		}
		glutPostRedisplay();

		if(cursor.type==Cursor::BUILD) {
			if(unitTypeInfo[cursor.buildUnitType].category==UNIT_TREE_HOUSE || unitTypeInfo[cursor.buildUnitType].category==UNIT_MINE) {
				Tuple<Point2D, int> newpos=getBuildablePlace(cursor.pos, cursor.buildUnitType, playerTeam, teams[playerTeam].visible);
				if(newpos.e1.x>=0) {
					cursor.pos=newpos.e1;
					cursor.angle=newpos.e2;
					if(!frustum.pointInFrustum(Point3D(cursor.pos.x, cursor.pos.y, getGroundHeight(cursor.pos.x, cursor.pos.y))))
						goTo(cursor.pos.x, cursor.pos.y);
				}
			}
		}
	}
		
	void Game::stop() {
		if(commandLocked)
			return;
		for(int i=0; i<nSelected; i++) {
			Unit &u=unit[selectList[i]-1];
			if(u.team==playerTeam) {
				unit[selectList[i]-1].stop();
			}
		}
	}
	void Game::_kill() {
		if(commandLocked)
			return;
		if(!isEditable()) {
			for(int i=0; i<nSelected; i++) {
				Unit &u=unit[selectList[i]-1];
				if(u.team==playerTeam) {
					selectedUnit(i).suicide();
				}
			}
		} else {
			for(int i=0; i<nSelected; i++) {
				selectedUnit(i).remove();
			}
		}
	}
	void Game::transform(UnitID index)  {
		if(commandLocked)
			return;
		if(nSelected==1) {
			unit[selectList[0]-1].commandTransform(index);
		}
	}
	void Game::setStance(UnitStance stance) {
		if(commandLocked)
			return;
		for(int i=0; i<nSelected; i++)
			if(unit[selectList[i]-1].team==playerTeam)
				unit[selectList[i]-1].stance=stance;
	}
	
	void Game::command(int screen_width, int screen_height, int x, int y, bool add, bool attackMove, CommandType commandtype, int targetBuilding, int specialPowerIndex) {
		if(commandLocked)
			return;
		
		processMouseHits(screen_width, screen_height, x, y, true);
		if(moveTo==-1 && targetUnit==-1)
			return;
		float dstx=((moveTo-POS_NAME_START)/(int)(height-1));
		float dsty=((moveTo-POS_NAME_START)%(int)(height-1));
		if(!isEditable()) {
			switch(commandtype) {
			case COMMAND_MOVE:
			case COMMAND_ATTACK:
			case COMMAND_CUT_TREE:
			case COMMAND_GATHER_STONE:
				if(targetUnit>=0 && unit[targetUnit].team!=0 && diplomacy(playerTeam, unit[targetUnit].team)!=DIPLOMACY_ALLY) {
					attack(unit[targetUnit].x, unit[targetUnit].y, add);
				} else if(targetUnit>=0 && diplomacy(playerTeam, unit[targetUnit].team)!=DIPLOMACY_ENEMY && isConstruction(unitTypeInfo[unit[targetUnit].type].category) && unit[targetUnit].hitPoint<unitTypeInfo[unit[targetUnit].type].hitPoint) {
					repair(unit[targetUnit].x, unit[targetUnit].y, add);
				} else if(targetUnit>=0 && unitTypeInfo[unit[targetUnit].type].category==UNIT_STONE) {
					cutStone(dstx, dsty, add);
				} else if(targetUnit>=0 &&
					((diplomacy(playerTeam, unit[targetUnit].team)==DIPLOMACY_ALLY && isConstruction(unitTypeInfo[unit[targetUnit].type].category))
					|| (unitTypeInfo[selectedUnit(0).type].canJump && isJumpableUnit(unitTypeInfo[unit[targetUnit].type].category)))
					&& unit[targetUnit].isActive() && unit[targetUnit].hasVacancy()) {
					if(unit[targetUnit].capturedByUnit!=INVALID_UNIT_INDEX && unitTypeInfo[unit[unit[targetUnit].capturedByUnit].type].category==UNIT_TREE_HOUSE) {
						targetUnit=unit[targetUnit].capturedByUnit;
						garrison(unit[targetUnit].x, unit[targetUnit].y, add);
					} else {
						garrison(unit[targetUnit].x, unit[targetUnit].y, add);
					}
				} else {
					move(dstx, dsty, add, attackMove);
				}
				break;
			case COMMAND_SPECIAL_ATTACK:
				if(targetUnit==-1)
					unit[selectList[0]-1].newUserCommand(COMMAND_SPECIAL_ATTACK, Point2D(dstx, dsty), targetUnit, false, false, false, specialPowerIndex);
				else
					unit[selectList[0]-1].newUserCommand(COMMAND_SPECIAL_ATTACK, unit[targetUnit].position(), targetUnit, false, false, false, specialPowerIndex);
				break;
			case COMMAND_BUILD:
				build(targetBuilding, add);
				break;
			}

			for(int i=0; i<nSelected && i<10; i++) {
				Unit &u=selectedUnit(i);
				if(u.getTypeInfo().commandSound.size()>0 && u.team==playerTeam && !u.isIdle() && satisfiesInProbability(0.5/nSelected))
					playAudio(randomVectorElement(u.getTypeInfo().commandSound), CHANNEL_SPEECH, soundAngle(u.x, u.y), soundDistance(u.x, u.y));
			}
			
			if(nSelected>0) {
				if(targetUnit>=0)
					commandPosition = Point3Di(unit[targetUnit].x, unit[targetUnit].y, FRAME_RATE);
				else
					commandPosition = Point3Di(dstx, dsty, FRAME_RATE);
			}
		} else {
			if(targetUnit>=0)
				move(unit[targetUnit].x, unit[targetUnit].y, add, false);
			else
				move(dstx, dsty, add, false);
		}
		glutPostRedisplay();
	}
	void Game::move(float x, float y, bool add, bool attackMove) {
		if(commandLocked)
			return;
		int dstWidth=sqrt(nSelected);
		for(int k=0; k<nSelected; k++) {
			int dstCol=k/dstWidth-dstWidth/2, dstRow=k%dstWidth-dstWidth/2;
			float xnew=clamp(x+dstCol, 0, width-1);
			float ynew=clamp(y+dstRow, 0, height-1);
			if(!isEditable()) {
				if(selectedUnit(k).team==playerTeam)
					selectedUnit(k).newUserCommand(COMMAND_MOVE, Point2D(xnew, ynew), -1, add, attackMove);
			} else {
				selectedUnit(k).x=xnew;
				selectedUnit(k).y=ynew;
			}
		}
	}
	void Game::attack(float x, float y, bool add) {
		if(commandLocked)
			return;
		vector<int> group;
		for(int k=0; k<nSelected; k++) 
			group.push_back(selectedUnitIndex(k));
		for(int k=0; k<nSelected; k++)
			if(selectedUnit(k).team==playerTeam)
				selectedUnit(k).newUserCommand(COMMAND_ATTACK, Point2D(x, y), targetUnit, add);
	}
	void Game::cutTree(float x, float y, bool add) {
		if(commandLocked)
			return;
		for(int k=0; k<nSelected; k++) {
			if(unit[selectList[k]-1].team==playerTeam)
				if(unitTypeInfo[unit[selectList[k]-1].type].isWorker)
					unit[selectList[k]-1].newUserCommand(COMMAND_CUT_TREE, unit[targetUnit].position(), targetUnit, add);
				else
					unit[selectList[k]-1].newUserCommand(COMMAND_MOVE, Point2D(x, y), targetUnit, add);
		}
	}
	void Game::cutStone(float x, float y, bool add) {
		if(commandLocked)
			return;
		for(int k=0; k<nSelected; k++) {
			if(unit[selectList[k]-1].team==playerTeam)
				if(unitTypeInfo[unit[selectList[k]-1].type].isWorker)
					unit[selectList[k]-1].newUserCommand(COMMAND_GATHER_STONE, unit[targetUnit].position(), targetUnit, add);
				else
					unit[selectList[k]-1].newUserCommand(COMMAND_MOVE, Point2D(x, y), targetUnit, add);
		}
	}
	void Game::build(UnitType targetBuilding, bool add) {
		if(commandLocked)
			return;
		if(roundInt(cursor.pos.y)<0 || roundInt(cursor.pos.y)>=height || roundInt(cursor.pos.x)<0 || roundInt(cursor.pos.x)>=width)
			return;
		try {
			int h=z[roundInt(cursor.pos.y)][roundInt(cursor.pos.x)];
			isTerrainSuitableToBuild(Point2D(cursor.pos.x, cursor.pos.y), cursor.angle, cursor.buildUnitType, false);
			isTerrainFreeToBuild(Point2D(cursor.pos.x, cursor.pos.y), cursor.angle, cursor.buildUnitType, playerTeam, false);
			targetUnit=addUnit(cursor.pos.x, cursor.pos.y, targetBuilding, playerTeam, cursor.angle, 1, true);
			if(targetUnit!=INVALID_UNIT_INDEX)
				repair(cursor.pos.x, cursor.pos.y, add);
		} catch(HUDMessage &m) {
			setMessage(m.msg);
		}
	}
	void Game::train(UnitType targetUnitType) {
		if(commandLocked)
			return;
		for(int k=0; k<nSelected; k++) {
			if(unit[selectList[k]-1].team==playerTeam) {
				targetUnit=addUnit(-1, -1, targetUnitType, playerTeam, unit[selectList[k]-1].angle, 1, true);
				if(targetUnit!=INVALID_UNIT_INDEX)
					unit[selectList[k]-1].newUserCommand(COMMAND_BUILD, targetUnit, true);
			}
		}
	}
	void Game::cancelBuild(UnitType tartoUnitCategory) {
		if(commandLocked)
			return;
		UnitTypeInfo &o=unitTypeInfo[tartoUnitCategory];
		for(int k=0; k<nSelected; k++) {
			if(unit[selectList[k]-1].team==playerTeam) {
				unit[selectList[k]-1].cancelBuild(tartoUnitCategory);
			}
		}
	}
	void Game::repair(float x, float y, bool add) {
		for(int k=0; k<nSelected; k++) {
			if(unit[selectList[k]-1].team==playerTeam && unitTypeInfo[unit[selectList[k]-1].type].canRepair) {
				unit[selectList[k]-1].newUserCommand(COMMAND_BUILD, Point2D(x, y), targetUnit, add);
			}
		}
	}
	void Game::garrison(float x, float y, bool add) {
		int n=unitTypeInfo[unit[targetUnit].type].garrisonedUnitPosition.size()-unit[targetUnit].garrisonedUnits.size();
		for(int k=0; k<nSelected && k<n; k++) {
			if(unit[selectList[k]-1].team==playerTeam && unitTypeInfo[unit[selectList[k]-1].type].category==UNIT_INFANTRY)
				unit[selectList[k]-1].newUserCommand(COMMAND_GARRISON, Point2D(x, y), targetUnit, add);
		}
	}
	
	void Game::deploy(UnitID unitIndex) {
		if(commandLocked)
			return;
		if(nSelected==1) {
			int index=-1;
			for(int i=0; i<unit[selectList[0]-1].garrisonedUnits.size(); i++)
				if(unit[selectList[0]-1].garrisonedUnits[i]==unitIndex) {
					index=i;
					break;
				}
			if(index>=0)
				unit[selectList[0]-1].deploy(index);
		}
	}
	void Game::deployAll() {
		if(commandLocked)
			return;
		for(int i=0; i<nSelected; i++) {
			Unit &u=unit[selectList[i]-1];
			if(u.team==playerTeam) {
				unit[selectList[i]-1].deployAll();
			}
		}
	}
}
