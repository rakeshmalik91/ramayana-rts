#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"

using namespace rapidxml;
using namespace math;
using namespace graphics;


namespace ramayana {

	float Game::getFog(unsigned int x, unsigned int y) {
		if (x <= 0 || y <= 0 || x >= (width - 1) || y >= (height - 1)) {
			return 0.90f;
		} else if (teams[playerTeam].visible != NULL) {
			return teams[playerTeam].visible[y][x] ? 0.0f : teams[playerTeam].explored[y][x] ? 0.50f : 0.90f;
		} else {
			return 0.90f;
		}
	}

	void Game::drawBlendMarker(float blend) {
		float glow = abs(float(frameCounter % 80) / 40.0 - 1.0);
		float glowStart = clampLow(glow - 0.1, 0.0);
		float glowEnd = clampHigh(glow + 0.1, 1.0);
		Point2D p(1, 0);
		glBegin(GL_LINES);
		for (int i = 0; i < 24; i++) {
			p = rotatePoint(15, p);
			glVertex3f(p.x*glowStart, p.y*glowStart, -0.25*glowStart);
			glColor4f(1, 1, 1, blend);
			glVertex3f(p.x*glow, p.y*glow, -0.25*glow);
			glVertex3f(p.x*glow, p.y*glow, -0.25*glow);
			glColor4f(1, 1, 1, 0);
			glVertex3f(p.x*glowEnd, p.y*glowEnd, -0.25*glowEnd);
		}
		glEnd();
	}
	void Game::drawMarkers() {
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Texture2D::bindNone();
		//Path
		if (Game::Settings::showPath) {
			if (nSelected>0) {
				for (int s = 0; s<nSelected; s++) {
					Unit &u = selectedUnit(s);
					vector<Point2Di> path = u.getPath();
					glColor4f(1.0, 1.0, 1.0, 0.8);
					glBegin(GL_LINE_STRIP);
					glVertex3f(u.x, u.y, z[roundInt(u.y)][roundInt(u.x)]);
					for (int i = 0; i<path.size(); i++)
						glVertex3f(path[i].x, path[i].y, clampLow(getGroundHeight(path[i].y, path[i].x), currentWaterLevel) + 0.1);
					vector<Unit::Command> command = u.getCommandList();
					if (command.size() > 1) {
						glColor4f(1.0, 1.0, 1.0, 0.5);
						for (int i = 0; i < command.size(); i++) {
							float ground = clampLow(getGroundHeight((int)command[i].dst.y, (int)command[i].dst.x), currentWaterLevel) + 0.1;
							glVertex3f(command[i].dst.x, command[i].dst.y, ground);
						}
					}
					glEnd();
				}
			}
		}

		//Command
		if (commandPosition.z>0) {
			glPushMatrix();
			glTranslatef(commandPosition.x, commandPosition.y, clampLow(getGroundHeight(commandPosition.x, commandPosition.y), currentWaterLevel));
			glColor4f(1.0, 1.0, 1.0, 0.8);
			glutWireCylinder(float(commandPosition.z) / FRAME_RATE, 0, 32, 1);
			glColor4f(1.0, 1.0, 1.0, 0.6);
			glutWireCylinder(0.8 - 0.8 * float(commandPosition.z) / FRAME_RATE, 0, 32, 1); 
			glColor4f(1.0, 1.0, 1.0, 0.4);
			glutWireCylinder(0.3 * float(commandPosition.z) / FRAME_RATE, 0, 32, 1);
			commandPosition.z--;
			glPopMatrix();
		}

		//Rally Point
		glColor4f(1.0, 1.0, 1.0, 0.8);
		if (!isEditable() && nSelected>0) {
			for (int s = 0; s<nSelected; s++) {
				Unit &u = selectedUnit(s);
				if (isConstruction(u.getCategory())) {
					glBegin(GL_LINES);
					glVertex3f(u.x, u.y, u.z);
					float flag_z = clampLow(getGroundHeight(u.rallyPoint.x, u.rallyPoint.y), currentWaterLevel);
					glVertex3f(u.rallyPoint.x, u.rallyPoint.y, flag_z);
					glEnd();
					glEnable(GL_LIGHTING);
					glPushMatrix();
					glTranslatef(u.rallyPoint.x, u.rallyPoint.y, flag_z);
					glScalef(OBJECT_SCALING, OBJECT_SCALING, OBJECT_SCALING);
					model.flag.render();
					glPopMatrix();
					glDisable(GL_LIGHTING);
				}
			}
		}

		//Select
		if (nSelected>0) {
			for (int s = 0; s<nSelected; s++) {
				Texture2D::bindNone();

				Unit &u = selectedUnit(s);
				Color color = availableTeamColors[teams[u.team].color];
				float groundHeight = getGroundHeight(u.x, u.y);
				if (groundHeight<currentWaterLevel) groundHeight = currentWaterLevel;

				//Selection
				glPushMatrix();
				glTranslatef(u.x, u.y, groundHeight + 0.01);
				float d = u.getRadiusAcrossZPlane();
				if (unitTypeInfo[u.type].isHeroic) {
					glColor4f(1.0, 1.0, 1.0, 0.6);
					glutWireCone(d + 0.05, 0, 32, 1);
				}
				glColor4f(color.r(), color.g(), color.b(), 0.6);
				glutWireCone(d, 0, 32, 1);

				//LOS
				if (Game::Settings::showLOS) {
					float los = unitTypeInfo[u.type].los;
					glColor4f(1.0, 1.0, 1.0, 0.3);
					glutWireSphere(los, 64, 64);
				}

				//Ground distance
				glColor4f(color.r(), color.g(), color.b(), 0.6);
				glBegin(GL_LINES);
				glVertex3f(0, 0, u.z - groundHeight);
				glVertex3f(0, 0, 0);
				glEnd();
				glPopMatrix();

				//Health-bar
				const float healthBarLength = 0.3, helathBarRadius = 0.03;
				glPushMatrix();
				float health = (float)u.hitPoint / u.getMaxHitPoint();
				glTranslatef(u.x - healthBarLength / 2, u.y, u.z + u.getMaxZ() + 0.1);
				glRotatef(90, 0, 1, 0);
				glColor4f((health<0.5) ? 1 : (1 - health), (health >= 0.5) ? 1 : health, 0, 0.5);
				glutSolidCylinder(helathBarRadius, health*healthBarLength, 4, 1);
				glColor4f(0, 0, 0, 0.5);
				glutWireCylinder(helathBarRadius, healthBarLength, 4, 1);
				glPopMatrix();

				//Auto transform time
				int att = u.getAutoTransformTimeRemaining();
				if (att>0) {
					int nMinute = ceil(float(att) / 60000);
					const float timeBarLength = 0.3*nMinute, timeBarRadius = 0.03;
					glPushMatrix();
					float time = (float)att / (nMinute * 60000);
					glTranslatef(u.x - timeBarLength / 2, u.y, u.z + u.getMaxZ() + 0.2);
					glRotatef(90, 0, 1, 0);
					glColor4f(0, 0, 255, 0.5);
					glutSolidCylinder(timeBarRadius, time*timeBarLength, 4, 1);
					glColor4f(0, 0, 0, 0.5);
					glutWireCylinder(timeBarRadius, timeBarLength, 4, 1);
					glPopMatrix();
				}
			}
		}

		for (UnitID u = 0; u<nUnit; u++) {
			if (unit[u].isAlive() && unit[u].team == playerTeam) {
				//Blend
				if (unit[u].blended>0) {
					glPushMatrix();
					glTranslatef(unit[u].x, unit[u].y, unit[u].z + unit[u].getMaxZ() + 0.1);
					drawBlendMarker(unit[u].blended);
					glPopMatrix();
				}
			}
		}

		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	}
	void Game::drawCursor() {
		glEnable(GL_BLEND);
		bool suitableTerrain, freeTerrain;
		switch (cursor.type) {
		case Game::Cursor::BUILD:
			suitableTerrain = isTerrainSuitableToBuild(Point2D(cursor.pos.x, cursor.pos.y), cursor.angle, cursor.buildUnitType);
			freeTerrain = suitableTerrain ? isTerrainFreeToBuild(Point2D(cursor.pos.x, cursor.pos.y), cursor.angle, cursor.buildUnitType, playerTeam) : false;
			glPushMatrix();
			glTranslatef(cursor.pos.x, cursor.pos.y, z[roundInt(cursor.pos.y)][roundInt(cursor.pos.x)]<currentWaterLevel ? currentWaterLevel : z[roundInt(cursor.pos.y)][roundInt(cursor.pos.x)]);
			glRotatef(cursor.angle, 0, 0, 1);
			if (freeTerrain) {
				glEnable(GL_LIGHTING);
				glEnable(GL_TEXTURE_2D);
				glPushMatrix();
				glScalef(OBJECT_SCALING, OBJECT_SCALING, OBJECT_SCALING);
				unitTypeInfo[cursor.buildUnitType].getObject(STATE_GENERAL).getFrame(0).render();
				glPopMatrix();
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LIGHTING);
				glColor4f(0, 1, 0, 0.3);
			} else {
				glColor4f(1, 0, 0, 0.3);
			}
			for (int i = 0; i<unitTypeInfo[cursor.buildUnitType].occupiedPoint.size(); i++) {
				glPushMatrix();
				glTranslatef(unitTypeInfo[cursor.buildUnitType].occupiedPoint[i].x, unitTypeInfo[cursor.buildUnitType].occupiedPoint[i].y, 0);
				glutSolidCube(1);
				glPopMatrix();
			}
			glPopMatrix();
			break;
		case Game::Cursor::TARGET:
			if (nSelected>0) {
				int radius = selectedUnit(0).getTypeInfo().specialPower[cursor.specialPowerIndex].areaDamageRadius;
				if (radius>0) {
					glColor4f(1, 0.8, 0.8, 0.8);
					glPushMatrix();
					glTranslatef(cursor.pos.x, cursor.pos.y, z[roundInt(cursor.pos.y)][roundInt(cursor.pos.x)]<currentWaterLevel ? currentWaterLevel : z[roundInt(cursor.pos.y)][roundInt(cursor.pos.x)]);
					glutWireCylinder(radius, 0, 32, 1);
					glPopMatrix();
				}
			}
			break;
		}
		glDisable(GL_BLEND);
	}

	void Game::renderObjectsForReflection() {
		Frustum localFrustum;
		localFrustum.updateFrustum();

		glEnable(GL_TEXTURE_2D);

		objectShader.use();
		glUniform1i(objectShader.getUniformLocation("colorMap"), 0);
		glUniform1f(objectShader.getUniformLocation("lightRadius"), WORLD_RADIUS);

		glEnable(GL_LIGHTING);
		glEnable(GL_FOG);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (int i = 0; i<nUnit; i++) {
			if (unit[i].isRenderable()
				&& localFrustum.sphereInFrustum(unit[i].position(), unit[i].getRadius())
				&& dist(unit[i].position(), getCameraPosition())<WORLD_RADIUS
				) {
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glTranslatef(unit[i].x, unit[i].y, unit[i].z);
				glRotatef(unit[i].angle, 0, 0, 1);
				glRotatef(unit[i].tilt, 0, 1, 0);
				if ((unitTypeInfo[unit[i].type].category == UNIT_TREE || isDecoration(unitTypeInfo[unit[i].type].category)) && unit[i].state == STATE_GENERAL)
					glShearf(0, 0, 0, 0, wind_flow, wind_flow);
				glScalef(unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING);

				glMatrixMode(GL_TEXTURE);
				glActiveTextureARB(GL_TEXTURE5);
				glPushMatrix();
				glTranslatef(unit[i].x, unit[i].y, unit[i].z);
				glRotatef(unit[i].angle, 0, 0, 1);
				glRotatef(unit[i].tilt, 0, 1, 0);
				if ((unitTypeInfo[unit[i].type].category == UNIT_TREE || isDecoration(unitTypeInfo[unit[i].type].category)) && unit[i].state == STATE_GENERAL)
					glShearf(0, 0, 0, 0, wind_flow, wind_flow);
				glScalef(unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING);

				unit[i].render();

				glMatrixMode(GL_TEXTURE);
				glActiveTextureARB(GL_TEXTURE5);
				glPopMatrix();

				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
			}
		}
		glDisable(GL_BLEND);
		glDisable(GL_FOG);
		glDisable(GL_LIGHTING);

		ShaderProgram::useNone();

		glDisable(GL_TEXTURE_2D);
	}
	void Game::makeReflection() {
		reflectionFrameBuffer.bind();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 0.0);

		initMatrix();

		//Clip everything over water
		glEnable(GL_CLIP_PLANE0);
		double water_plane[] = { 0.0f, 0.0f, -1.0f, currentWaterLevel };
		glClipPlane(GL_CLIP_PLANE0, water_plane);

		//Transform
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(0, 0, 2 * currentWaterLevel);
		glScalef(1, 1, -1.33);

		drawSky();
		drawTerrain(false, false, false);
		renderObjectsForReflection();

		glDisable(GL_CLIP_PLANE0);

		//Transform Texture Matrix
		static double modelView[16];
		static double projection[16];
		const GLdouble bias[16] = {
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0 };
		glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glMatrixMode(GL_TEXTURE);
		glActiveTextureARB(GL_TEXTURE5);
		glLoadIdentity();
		glLoadMatrixd(bias);
		glMultMatrixd(projection);
		glMultMatrixd(modelView);
		glMatrixMode(GL_MODELVIEW);
		glActiveTextureARB(GL_TEXTURE0);

		closeMatrix();

		reflectionFrameBuffer.unbind();
	}

	void Game::renderObjectsForShadow() {
		Frustum localFrustum;
		localFrustum.updateFrustum();

		glEnable(GL_TEXTURE_2D);

		alphaMaskShader.use();
		glUniform1i(alphaMaskShader.getUniformLocation("colorMap"), 0);
		glUniform1f(alphaMaskShader.getUniformLocation("threshold"), 0.5);

		for (int i = 0; i<nUnit; i++) {
			if (unit[i].isRenderable()
				&& localFrustum.sphereInFrustum(unit[i].position(), unit[i].getRadius())
				&& dist(unit[i].position(), getCameraPosition())<WORLD_RADIUS
				) {
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glTranslatef(unit[i].x, unit[i].y, unit[i].z);
				glRotatef(unit[i].angle, 0, 0, 1);
				glRotatef(unit[i].tilt, 0, 1, 0);
				if ((unitTypeInfo[unit[i].type].category == UNIT_TREE || isDecoration(unitTypeInfo[unit[i].type].category)) && unit[i].state == STATE_GENERAL)
					glShearf(0, 0, 0, 0, wind_flow, wind_flow);
				glScalef(unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING);

				glMatrixMode(GL_TEXTURE);
				glActiveTextureARB(GL_TEXTURE7);
				glPushMatrix();
				glTranslatef(unit[i].x, unit[i].y, unit[i].z);
				glRotatef(unit[i].angle, 0, 0, 1);
				glRotatef(unit[i].tilt, 0, 1, 0);
				if ((unitTypeInfo[unit[i].type].category == UNIT_TREE || isDecoration(unitTypeInfo[unit[i].type].category)) && unit[i].state == STATE_GENERAL)
					glShearf(0, 0, 0, 0, wind_flow, wind_flow);
				glScalef(unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING);

				unit[i].render();

				glMatrixMode(GL_TEXTURE);
				glActiveTextureARB(GL_TEXTURE7);
				glPopMatrix();

				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
			}

		}

		glDisable(GL_TEXTURE_2D);

		ShaderProgram::useNone();
	}
	void Game::makeShadow() {
		shadowMapFrameBuffer.bind();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glClearColor(0, 0, 0, 0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		//Set moveUp Viewport
		glViewport(0, 0, shadowMapFrameBuffer.getWidth(), shadowMapFrameBuffer.getHeight());

		float xmin = 0, xmax = 0, ymin = 0, ymax = 0;
		for (int y = 0; y<height; y++) {
			for (int x = 0; x<width; x++) {
				if (frustum.pointInFrustum(Point3D(x, y, z[y][x]<currentWaterLevel ? currentWaterLevel : z[y][x]))) {
					if (x - 1<camX + xmin) xmin = x - 1 - camX;
					if (x + 1>camX + xmax) xmax = x + 1 - camX;
					if (y - 1<camY + ymin) ymin = y - 1 - camY;
					if (y + 1>camY + ymax) ymax = y + 1 - camY;
				}
			}
		}
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(xmin - 5, xmax + 5, ymin - 5, ymax + 5, -100000, 100000);

		//Set moveUp Modelview
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, 100);
		glRotatef(45, SQRT2, SQRT2, 0);
		glTranslatef(-camX, -camY, 0);

		//Render
		drawTerrainForShadow();
		renderObjectsForShadow();

		//Transform Texture Matrix
		static double modelView[16];
		static double projection[16];
		const GLdouble bias[16] = {
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0 };
		glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glMatrixMode(GL_TEXTURE);
		glActiveTextureARB(GL_TEXTURE7);
		glLoadIdentity();
		glLoadMatrixd(bias);
		glMultMatrixd(projection);
		glMultMatrixd(modelView);
		glMatrixMode(GL_MODELVIEW);
		glActiveTextureARB(GL_TEXTURE0);

		//Revert to Normal
		glViewport(0, 0, Game::Settings::screenWidth, Game::Settings::screenHeight);
		shadowMapFrameBuffer.unbind();
	}

	void Game::drawObjectsUnderwater(bool noShader, bool objectBumpmapOn) {
		glEnable(GL_LIGHTING);
		glEnable(GL_FOG);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_FRONT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);

		glEnable(GL_CLIP_PLANE0);
		double water_plane[] = { 0.0f, 0.0f, -1.0f, currentWaterLevel };
		glClipPlane(GL_CLIP_PLANE0, water_plane);

		if (!isEditable() && !noShader) {
			objectShader.use();
			glUniform1i(objectShader.getUniformLocation("colorMap"), 0);
			glUniform1i(objectShader.getUniformLocation("normalMapEnabled"), objectBumpmapOn ? GL_TRUE : GL_FALSE);
			glUniform1i(objectShader.getUniformLocation("normalMap"), 1);
			glUniform1f(objectShader.getUniformLocation("lightRadius"), WORLD_RADIUS);
			if (objectBumpmapOn) {
				glActiveTextureARB(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				glActiveTextureARB(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
			} else {
				glEnable(GL_TEXTURE_2D);
			}
		} else {
			glEnable(GL_TEXTURE_2D);
		}

		for (int i = 0; i<nUnit; i++) {
			if (unit[i].isRenderable() && unit[i].bottom().z<currentWaterLevel && frustum.sphereInFrustum(unit[i].position(), unit[i].getRadius())) {
				glPushMatrix();
				glTranslatef(unit[i].x, unit[i].y, unit[i].z);
				glRotatef(unit[i].angle, 0, 0, 1);
				glRotatef(unit[i].tilt, 0, 1, 0);
				glScalef(unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING, unit[i].scaling*OBJECT_SCALING);
				unit[i].render();
				glPopMatrix();
			}
		}

		if (!isEditable() && !noShader) {
			ShaderProgram::useNone();

			if (objectBumpmapOn) {
				glActiveTextureARB(GL_TEXTURE1);
				glDisable(GL_TEXTURE_2D);
				glActiveTextureARB(GL_TEXTURE0);
				glDisable(GL_TEXTURE_2D);
			} else {
				glDisable(GL_TEXTURE_2D);
			}
		} else {
			glDisable(GL_TEXTURE_2D);
		}

		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_FOG);
		glDisable(GL_LIGHTING);
	}
	int Game::unitindexComparator(const int &i1, const int &i2, void *param) {
		if (((float*)param)[i1]<((float*)param)[i2])
			return -1;
		else if (((float*)param)[i1]<((float*)param)[i2])
			return 1;
		else
			return 0;
	}
	void Game::drawObjects(bool noShader, bool objectBumpmapOn) {
		//Sorting objects according to distance from screen
		float distance_from_screen[MAX_UNIT];
		vector<int> renderableUnit;
		for (int i = 0; i<nUnit; i++)
		if (unit[i].isRenderable()
			&& frustum.sphereInFrustum(unit[i].position(), unit[i].getRadius())
			&& dist(unit[i].position(), getCameraPosition())<WORLD_RADIUS
			) {
			renderableUnit.push_back(i);
			glPushMatrix();
			glTranslatef(unit[i].x, unit[i].y, unit[i].z);
			glRotatef(unit[i].angle, 0, 0, 1);
			glRotatef(unit[i].tilt, 0, 1, 0);
			glScalef(unit[i].scaling*0.5, unit[i].scaling*0.5, unit[i].scaling*0.5);
			GLfloat trans_mat[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, trans_mat);
			float pArr[4] = { unit[i].getMidX(), unit[i].getMidY(), unit[i].getMidZ(), 1 };
			vect4Mat16Mult(trans_mat, pArr);
			distance_from_screen[i] = pArr[2];
			glPopMatrix();
		}

		quicksort(renderableUnit.data(), renderableUnit.size(), unitindexComparator, (void*)distance_from_screen);

		glEnable(GL_LIGHTING);
		glEnable(GL_FOG);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CLIP_PLANE0);
		double water_plane[] = { 0.0f, 0.0f, 1.0f, currentWaterLevel };
		glClipPlane(GL_CLIP_PLANE0, water_plane);

		if (!isEditable() && !noShader) {
			objectShader.use();
			glUniform1i(objectShader.getUniformLocation("colorMap"), 0);
			glUniform1i(objectShader.getUniformLocation("normalMapEnabled"), objectBumpmapOn ? GL_TRUE : GL_FALSE);
			glUniform1i(objectShader.getUniformLocation("normalMap"), 1);
			glUniform1f(objectShader.getUniformLocation("lightRadius"), WORLD_RADIUS);
			if (objectBumpmapOn) {
				glActiveTextureARB(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				glActiveTextureARB(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
			} else {
				glEnable(GL_TEXTURE_2D);
			}
		} else {
			glEnable(GL_TEXTURE_2D);
		}

		nUnitsRendered = 0;
		//Rendering non-Tree objects
		for (int i = 0; i<renderableUnit.size(); i++) {
			if (unitTypeInfo[unit[renderableUnit[i]].type].category != UNIT_TREE && !isDecoration(unitTypeInfo[unit[renderableUnit[i]].type].category)
				) {
				glPushMatrix();
				glTranslatef(unit[renderableUnit[i]].x, unit[renderableUnit[i]].y, unit[renderableUnit[i]].z);
				glRotatef(unit[renderableUnit[i]].angle, 0, 0, 1);
				glRotatef(unit[renderableUnit[i]].tilt, 0, 1, 0);
				glScalef(unit[renderableUnit[i]].scaling*OBJECT_SCALING, unit[renderableUnit[i]].scaling*OBJECT_SCALING, unit[renderableUnit[i]].scaling*OBJECT_SCALING);
				unit[renderableUnit[i]].render();
				if (unitTypeInfo[unit[renderableUnit[i]].type].isWorker && unit[renderableUnit[i]].has.wood>0) {
					model.treelog.render();
				}
				glPopMatrix();
				nUnitsRendered++;
			}
		}
		//Rendering Decoration objects
		for (int i = 0; i<renderableUnit.size(); i++) {
			if (isDecoration(unitTypeInfo[unit[renderableUnit[i]].type].category)) {
				glPushMatrix();
				glTranslatef(unit[renderableUnit[i]].x, unit[renderableUnit[i]].y, unit[renderableUnit[i]].z);
				glRotatef(unit[renderableUnit[i]].angle, 0, 0, 1);
				glRotatef(unit[renderableUnit[i]].tilt, 0, 1, 0);
				if (unit[renderableUnit[i]].state == STATE_GENERAL)
					glShearf(0, 0, 0, 0, wind_flow, wind_flow);
				glScalef(unit[renderableUnit[i]].scaling*OBJECT_SCALING, unit[renderableUnit[i]].scaling*OBJECT_SCALING, unit[renderableUnit[i]].scaling*OBJECT_SCALING);
				unit[renderableUnit[i]].render();
				glPopMatrix();
				nUnitsRendered++;
			}
		}
		//Rendering Tree objects
		for (int i = 0; i<renderableUnit.size(); i++) {
			if (unitTypeInfo[unit[renderableUnit[i]].type].category == UNIT_TREE) {
				glPushMatrix();
				glTranslatef(unit[renderableUnit[i]].x, unit[renderableUnit[i]].y, unit[renderableUnit[i]].z);
				glRotatef(unit[renderableUnit[i]].angle, 0, 0, 1);
				glRotatef(unit[renderableUnit[i]].tilt, 0, 1, 0);
				if (unit[renderableUnit[i]].state == STATE_GENERAL)
					glShearf(0, 0, 0, 0, wind_flow, wind_flow);
				glScalef(unit[renderableUnit[i]].scaling*OBJECT_SCALING, unit[renderableUnit[i]].scaling*OBJECT_SCALING, unit[renderableUnit[i]].scaling*OBJECT_SCALING);
				unit[renderableUnit[i]].render();
				glPopMatrix();
				nUnitsRendered++;
			}
		}

		if (!isEditable() && !noShader) {
			ShaderProgram::useNone();

			if (objectBumpmapOn) {
				glActiveTextureARB(GL_TEXTURE1);
				glDisable(GL_TEXTURE_2D);
				glActiveTextureARB(GL_TEXTURE0);
				glDisable(GL_TEXTURE_2D);
			} else {
				glDisable(GL_TEXTURE_2D);
			}
		} else {
			glDisable(GL_TEXTURE_2D);
		}

		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_BLEND);
		glDisable(GL_FOG);
		glDisable(GL_LIGHTING);
	}

	void Game::drawProjectiles() {
		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (int p = 0; p<nProjectile; p++) {
			int r = roundInt(projectile[p].pos.y), c = roundInt(projectile[p].pos.x), ground = getGroundHeight(projectile[p].pos.x, projectile[p].pos.y);
			if (projectile[p].pos.z < ground - 1 || r < 0 || c < 0 || r >= height || c >= width || !teams[playerTeam].visible[r][c]) {
				continue;
			}

			//render
			glPushMatrix();
			glTranslatef(projectile[p].pos.x, projectile[p].pos.y, projectile[p].pos.z);
			glRotatef(projectile[p].hAngle, 0, 0, 1);
			glRotatef(projectile[p].angle, 0, 1, 0);
			glEnable(GL_LIGHTING);
			glScalef(OBJECT_SCALING, OBJECT_SCALING, OBJECT_SCALING);
			if (projectile[p].weaponType == WEAPON_SPIKE) {
				model.spike.render();
			} else {
				model.arrow.render();
			}
			glDisable(GL_LIGHTING);
			glPopMatrix();
		}

		glEnable(GL_BLEND);
		_makeProjectileTrail();
		glDisable(GL_BLEND);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	void Game::drawSpecialEffects() {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//shockwaves
		Point2D p[] = { 
			Point2D(-1.0, 0.0), Point2D(-0.923880, -0.382684), Point2D(-0.707107, -0.707107), Point2D(-0.382683, -0.923880),
			Point2D(0.0, -1.0), Point2D(0.382684, -0.923880), Point2D(0.707107, -0.707107), Point2D(0.923880, -0.382683),
			Point2D(1.0, 0.0), Point2D(0.923880, 0.382684), Point2D(0.707107, 0.707107), Point2D(0.382683, 0.923880),
			Point2D(0.0, 1.0), Point2D(-0.382684, 0.923880), Point2D(-0.707107, 0.707107), Point2D(-0.923880, 0.382683)
		};
		for (int index = 0; index<MAX_COLLISION; index++) {
			if (shockwave[index].radius < shockwave[index].maxRadius) {
				glPushMatrix();
				float ground = getWaterLevel(shockwave[index].pos.x, shockwave[index].pos.y);
				glTranslatef(shockwave[index].pos.x, shockwave[index].pos.y, ground - 0.5);
				float alpha = 1.0 - shockwave[index].radius / shockwave[index].maxRadius;
				glScalef(shockwave[index].radius, shockwave[index].radius, 1.0);
				glBegin(GL_QUAD_STRIP);
				for (int i = 0; i < arrayLength(p); i++) {
					if (i % 2 == 1) {
						glColor4f(1.0, 1.0, 1.0, 0.0);
						glVertex3f(p[i].x, p[i].y, ground + choice(-1.0, 1.0));
					}
					glColor4f(1.0, 1.0, 1.0, alpha);
					glVertex3f(p[i].x, p[i].y, ground);
					if (i % 2 == 0) {
						glColor4f(1.0, 1.0, 1.0, 0.0);
						glVertex3f(p[i].x, p[i].y, ground + choice(-1.0, 1.0));
					}
				}
				glColor4f(1.0, 1.0, 1.0, alpha);
				glVertex3f(p[0].x, p[0].y, ground);
				glColor4f(1.0, 1.0, 1.0, 0.0);
				glVertex3f(p[0].x, p[0].y, ground + choice(-1.0, 1.0));
				glEnd();
				glPopMatrix();
			}
		}
		glDisable(GL_BLEND);
	}
	void Game::drawParticles() {
		if (!isEditable() && !Settings::noShader) {
			alphaMaskShader.use();
			glUniform1i(alphaMaskShader.getUniformLocation("colorMap"), 0);
			glUniform1f(alphaMaskShader.getUniformLocation("threshold"), 0.0000);
		}

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		particleRenderer.render(frustum);
		glDisable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		if (!isEditable() && !Settings::noShader)
			ShaderProgram::useNone();
	}

	void Game::initMatrix() {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPerspective(45.0, (float)Game::Settings::screenWidth / Game::Settings::screenHeight, 0.1, 1000.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		setCamera();
	}
	void Game::closeMatrix() {
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	void Game::applyDepthOfFieldEffect() {
		dofShader.use();
		glUniform1i(dofShader.getUniformLocation("colorMap"), 0);
		glUniform1i(dofShader.getUniformLocation("depthMap"), 1);
		glUniform1f(dofShader.getUniformLocation("aspectRatio"), (float)Settings::screenWidth / Settings::screenHeight);
		glUniform1f(dofShader.getUniformLocation("aperture"), 0.1);
		glUniform1f(dofShader.getUniformLocation("blurSize"), 1.0 / (float)tempFBOSet0[1 - tempFBOSet0Index].getWidth());
		glActiveTextureARB(GL_TEXTURE1);
		tempFBOSet0[1 - tempFBOSet0Index].bindDepthTexture();
		glActiveTextureARB(GL_TEXTURE0);
		tempFBOSet0[tempFBOSet0Index].copy(tempFBOSet0[1 - tempFBOSet0Index]);
		glActiveTextureARB(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTextureARB(GL_TEXTURE0);
		ShaderProgram::useNone();

		tempFBOSet0Index = 1 - tempFBOSet0Index;
	}
	void Game::applyBloom() {
		int blurKernelSize[] = { 5, 11, 21, 41 };
		for (int i = 0; i<4; i++) {
			brightPassShader.use();
			glUniform1i(brightPassShader.getUniformLocation("colorMap"), 0);
			glUniform1f(brightPassShader.getUniformLocation("threshold"), 0.6);
			tempFBOSet1[i].copy(tempFBOSet0[1 - tempFBOSet0Index]);

			blurShader.use();
			glUniform1i(blurShader.getUniformLocation("blurSampler"), 0);
			glUniform1i(blurShader.getUniformLocation("kernelSize"), blurKernelSize[i]);
			glUniform1i(blurShader.getUniformLocation("horizontal"), GL_TRUE);
			tempFBOSet2[i].copy(tempFBOSet1[i]);
			glUniform1i(blurShader.getUniformLocation("horizontal"), GL_FALSE);
			tempFBOSet1[i].copy(tempFBOSet2[i]);
		}

		addShader.use();
		glUniform1i(addShader.getUniformLocation("n"), 5);
		for (int i = 0; i<4; i++) {
			glActiveTextureARB(GL_TEXTURE1 + i);
			glEnable(GL_TEXTURE_2D);
			tempFBOSet1[i].bindColorTexture();
			glUniform1i(addShader.getUniformLocation("sampler" + toString(i + 1)), i + 1);
		}
		glUniform1i(addShader.getUniformLocation("sampler0"), 0);
		glActiveTextureARB(GL_TEXTURE0);
		tempFBOSet0[tempFBOSet0Index].copy(tempFBOSet0[1 - tempFBOSet0Index]);
		for (int i = 0; i<4; i++) {
			glActiveTextureARB(GL_TEXTURE1 + i);
			glDisable(GL_TEXTURE_2D);
			Texture2D::bindNone();
		}
		glActiveTextureARB(GL_TEXTURE0);
		ShaderProgram::useNone();

		tempFBOSet0Index = 1 - tempFBOSet0Index;
	}
	void Game::applyAntialias() {
		antialiasShader.use();
		glUniform1i(antialiasShader.getUniformLocation("tex"), 0);
		glUniform1f(antialiasShader.getUniformLocation("rt_w"), (float)Settings::screenWidth);
		glUniform1f(antialiasShader.getUniformLocation("rt_h"), (float)Settings::screenHeight);
		tempFBOSet0[tempFBOSet0Index].copy(tempFBOSet0[1 - tempFBOSet0Index]);
		ShaderProgram::useNone();

		tempFBOSet0Index = 1 - tempFBOSet0Index;
	}

	void Game::drawMeshes() {
		try {
			initMatrix();
			frustum.updateFrustum();

			try {
				if (!isEditable()) {
					if (Settings::renderSkyOn) {
						drawSky();
					}
				}
				if (Settings::renderTerrainOn) {
					drawTerrain(Settings::noShader, Settings::terrainBumpmapOn, Settings::shadowOn);
				}
				if (Settings::renderObjectsOn) {
					drawObjectsUnderwater(Settings::noShader, Settings::objectBumpmapOn);
				}
				if (Settings::renderWaterOn) {
					drawWater(Settings::noShader, Settings::reflectionOn, Settings::shadowOn);
				}
				drawCursor();
				if (Settings::renderObjectsOn) {
					drawObjects(Settings::noShader, Settings::objectBumpmapOn);
				}
				drawMarkers();
				if (!isEditable()) {
					drawProjectiles();
					drawSpecialEffects();
					drawParticles();
				}
			} catch (...) {}

			closeMatrix();
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Game::draw()", true);
		}
	}

	void Game::render() {
		try {
#if !NO_SHADER
			if (!isEditable() && !Settings::noShader) {
				if (Settings::reflectionOn)			makeReflection();
				if (Settings::shadowOn)				makeShadow();

				tempFBOSet0[tempFBOSet0Index].bind();
			}
#endif

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			if (Game::Settings::stereoscopicOn) {
				glColorMask(false, true, true, true);
				moveRight(-Game::Settings::stereoSeperation / 2);
				drawMeshes();

				glClear(GL_DEPTH_BUFFER_BIT);
				glColorMask(true, false, false, true);
				moveRight(Game::Settings::stereoSeperation / 2);
				drawMeshes();

				glColorMask(true, true, true, true);
				glEnable(GL_BLEND);
				glDisable(GL_BLEND);
			} else {
				drawMeshes();
			}

#if !NO_SHADER
			if (!isEditable() && !Settings::noShader) {
				tempFBOSet0[tempFBOSet0Index].unbind();
				tempFBOSet0Index = 1 - tempFBOSet0Index;

				if (Settings::depthOfFieldOn)		applyDepthOfFieldEffect();
				if (Settings::bloomOn)				applyBloom();
				if (Settings::antialiasingOn)		applyAntialias();

				tempFBOSet0[1 - tempFBOSet0Index].bindColorTexture();
				colorShader.use();
				glUniform1i(colorShader.getUniformLocation("colorMap"), 0);
				glUniform1f(colorShader.getUniformLocation("brightness"), Settings::brightness + weather.brightness);
				glUniform1f(colorShader.getUniformLocation("contrast"), Settings::contrast + weather.contrast);
				glColor4f(1.0, 1.0, 1.0, 1.0);
				glDrawFullScreenRectangle();
				ShaderProgram::useNone();
				Texture2D::bindNone();

				if (Settings::motionBlurOn)			motionBlur(0.2);
			}
#endif
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Game::render()", true);
		}

		frameCounter++;
	}
}
