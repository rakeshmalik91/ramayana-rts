#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "unit.h"
#include "game.h"
#include "audio.h"
#include "steamStatsAndAchievements.h"

using namespace algorithm;
using namespace math;


namespace ramayana {

	//------------------------------------------------------------------Private Functions
	void Unit::_updatePosition() {
		if (state != STATE_DEAD) {
			_removeFromMinimap();
		}
		x = clamp(x, 0, game->getWidth() - 1);
		y = clamp(y, 0, game->getHeight() - 1);
		float groundLevel = game->getGroundHeight(x, y);

		if (!game->isEditable()
			&& unitTypeInfo[type].speed>0.0
			&& game->minimap[roundInt(y)][roundInt(x)].waterUnit >= 0
			&& game->unit[game->minimap[roundInt(y)][roundInt(x)].waterUnit].getTypeInfo().isBridge) {
			groundLevel = clampLow(groundLevel, game->unit[game->minimap[roundInt(y)][roundInt(x)].waterUnit].top().z);
		}

		float waterLevel = clampLow(groundLevel, game->getCurrentWaterLevel());
		if (projected) {
			x = clamp(projection.pos.x, 0, game->getWidth() - 1);
			y = clamp(projection.pos.y, 0, game->getHeight() - 1);
			z = projection.pos.z;
			projection.move();
			if (z<waterLevel) {
				projected = false;
				z = waterLevel;
			}
		} else {
			if (state == STATE_DEAD) {
				if (deathTimer == DEAD_UNIT_VANISH_TIMER && z>waterLevel) {
					projected = true;
					projection = Projectile(Point3D(x, y, z), 0, -90, 0, 1.0);
				} else {
					z = waterLevel - (getMaxZ() - getMinZ())*(1 - float(deathTimer) / DEAD_UNIT_VANISH_TIMER);
				}
			} else {
				if (garrisoned)
					return;
				if (unitTypeInfo[type].category == UNIT_WATER || unitTypeInfo[type].category == UNIT_WATER_CONSTRUCTION) {
					int phase = ((game->frameCounter + unitID*unitID) % 180) * 2;
					z = waterLevel + unitTypeInfo[type].flyLevel + _sin[phase] * 0.05;
				} else if (unitTypeInfo[type].category == UNIT_AMPHIBIAN) {
					if (groundLevel <= waterLevel)
						z = clampLow(waterLevel - getMaxZ()*0.9, groundLevel);
					else
						z = game->getGroundHeight(x, y);
				} else if (unitTypeInfo[type].category == UNIT_INSECT) {
					int phase = (game->frameCounter % 60) * 6;
					z = waterLevel + unitTypeInfo[type].flyLevel + _sin[phase] * 0.25;
				} else if (isWaterUnit(unitTypeInfo[type].category)) {
					z = waterLevel;
				} else if (isAirUnit(unitTypeInfo[type].category)) {
					int nFrame = unitTypeInfo[type].getObject(state).length();
					tilt = 0;
					if (state == STATE_ATTACKING && !isRangedWeapon(unitTypeInfo[type].weaponType)) {
						z = (2 * z + getTarget().z) / 3;
					} else if (state == STATE_TRANSFORM_IN || state == STATE_TRANSFORM_OUT) {
						if (!projected) {
							projected = true;
							projection = Projectile(Point3D(x, y, z), 0, -90, 0, 1.0);
						}
					} else if (state == STATE_MOVING) {
						if (z<waterLevel + unitTypeInfo[type].flyLevel)
							z = clamp(z + 0.02, waterLevel, waterLevel + unitTypeInfo[type].flyLevel);
						else
							z = clamp(z - 0.02, waterLevel, waterLevel + unitTypeInfo[type].flyLevel);
					} else {
						if (z<waterLevel + unitTypeInfo[type].flyLevel)
							z = clamp(z + 0.005, waterLevel, waterLevel + unitTypeInfo[type].flyLevel);
						else
							z = clamp(z - 0.005, waterLevel, waterLevel + unitTypeInfo[type].flyLevel);
					}
				} else {
					z = groundLevel;
				}

				//Garrisoned units
				for (int i = 0; i<garrisonedUnits.size(); i++) {
					if (game->unit[garrisonedUnits[i]].isAlive()) {
						Point2D p = rotatePoint(angle, unitTypeInfo[type].garrisonedUnitPosition[i]);
						game->unit[garrisonedUnits[i]].x = x + p.x;
						game->unit[garrisonedUnits[i]].y = y + p.y;
						game->unit[garrisonedUnits[i]].z = z + unitTypeInfo[type].garrisonedUnitPosition[i].z;
						if (!game->unit[garrisonedUnits[i]].isAttacking()) {
							game->unit[garrisonedUnits[i]].angle = angle + unitTypeInfo[type].garrisonedUnitPosition[i].w;
						}
					}
				}

				//Tilt
				if ((unitTypeInfo[type].category == UNIT_CHARIOT || unitTypeInfo[type].category == UNIT_AMPHIBIAN || unitTypeInfo[type].category == UNIT_BEAST) && currentFrame().getRadiusAcrossZPlane()>1) {
					float r = getRadiusAcrossZPlane() / 2;
					Point2D d(r*_cos[(int)angle], r*_sin[(int)angle]);
					Point2Di p1(x - d.x, y - d.y), p2(x + d.x, y + d.y);
					float z1 = clampLow(game->getGroundHeight(p1.x, p1.y), waterLevel), z2 = clampLow(game->getGroundHeight(p2.x, p2.y), waterLevel);
					Line2D base(Point2D(-r, z1), Point2D(r, z2));
					tilt = -base.tangent();
					if (z>waterLevel) {
						z += z2 - groundLevel;
					}
				}
			}

		}
		if (state != STATE_DEAD)
			_insertIntoMinimap();
	}
	void Unit::_setState(UnitState state) {
		if (state != this->state) {
			if (state == STATE_GENERAL) {
				frame_number = choice(0, unitTypeInfo[type].getObject(state).length() - 1);
			} else if (state == STATE_ATTACKING) {
				frame_number = choice(0, unitTypeInfo[type].getObject(state).length() / unitTypeInfo[type].attackDelay)*unitTypeInfo[type].attackDelay;
			} else {
				frame_number = 0;
			}
		}
		this->state = state;
		if (state != STATE_MOVING) {
			fleeing = false;
		}
	}
	bool Unit::_unitCanMoveTo(void* param, int x, int y) {
		Unit &u = *((Unit*)param);
		Game &game = *(u.game);
		if (!Point2Di(x, y).in(0, 0, game.getWidth() - 1, game.getHeight() - 1)) {
			return false;
		}
		if (squareDist(Point2D(x, y), u.targetUnitPosition)<1) {
			return true;
		}
		switch (u.unitTypeInfo[u.type].category) {
		case UNIT_AMPHIBIAN:
			return (game.minimap[y][x].type == TERRAIN_MOVABLE_LAND || game.minimap[y][x].type == TERRAIN_SHALLOW_WATER || game.minimap[y][x].type == TERRAIN_DEEP_WATER)
				? ((game.minimap[y][x].landUnit == -1 || game.minimap[y][x].landUnit == u.unitID) && (game.minimap[y][x].waterUnit == -1 || game.minimap[y][x].waterUnit == u.unitID))
				: false;
			break;
		case UNIT_GIANT:
			return (game.minimap[y][x].type == TERRAIN_MOVABLE_LAND || game.minimap[y][x].type == TERRAIN_SHALLOW_WATER)
				? ((game.minimap[y][x].landUnit == -1 || game.minimap[y][x].landUnit == u.unitID) && (game.minimap[y][x].waterUnit == -1 || game.minimap[y][x].waterUnit == u.unitID))
				: false;
			break;
		case UNIT_INSECT:
			return game.minimap[y][x].landUnit == -1 || game.minimap[y][x].landUnit == u.unitID;
			break;
		case UNIT_BIRD:
		case UNIT_AIR:
			return game.minimap[y][x].airUnit == -1 || game.minimap[y][x].airUnit == u.unitID;
			break;
		case UNIT_INFANTRY:
		case UNIT_BEAST:
		case UNIT_CHARIOT:
			return (game.minimap[y][x].type == TERRAIN_MOVABLE_LAND)
				? (game.minimap[y][x].landUnit == -1
				|| game.minimap[y][x].landUnit == u.unitID
				|| (u.garrisonedBuildingIndex >= 0 && game.minimap[y][x].landUnit == u.garrisonedBuildingIndex))
				: (game.minimap[y][x].waterUnit >= 0 && game.unit[game.minimap[y][x].waterUnit].getTypeInfo().isBridge);
			break;
		case UNIT_WATER:
		case UNIT_WATER_CONSTRUCTION:
			return (game.getGroundHeight(x, y) <= Game::BASE_WATER_LEVEL)
				? (game.minimap[y][x].waterUnit == -1 || game.minimap[y][x].waterUnit == u.unitID)
				: false;
			break;
		}
		return false;
	}
	bool Unit::_canMoveToIfNoUnit(int x, int y) {
		switch (unitTypeInfo[type].category) {
		case UNIT_AMPHIBIAN:
			return (game->minimap[y][x].type == TERRAIN_MOVABLE_LAND || game->minimap[y][x].type == TERRAIN_SHALLOW_WATER || game->minimap[y][x].type == TERRAIN_DEEP_WATER);
			break;
		case UNIT_GIANT:
			return (game->minimap[y][x].type == TERRAIN_MOVABLE_LAND || game->minimap[y][x].type == TERRAIN_SHALLOW_WATER);
			break;
		case UNIT_BIRD:
		case UNIT_AIR:
		case UNIT_INSECT:
			return true;
			break;
		case UNIT_INFANTRY:
		case UNIT_BEAST:
		case UNIT_CHARIOT:
			return (game->minimap[y][x].type == TERRAIN_MOVABLE_LAND);
			break;
		case UNIT_WATER:
			return (game->minimap[y][x].type == TERRAIN_SHALLOW_WATER || game->minimap[y][x].type == TERRAIN_DEEP_WATER);
			break;
		}
		return false;
	}
	void Unit::_calculatePath() {
		if (command.empty()) {
			return;
		}
		if (isAirUnit(unitTypeInfo[type].category)) {
			path.clear();
			path.push_back(command[0].dst);
		} else {
			Point2Di src = Point2Di(roundInt(x), roundInt(y));
			src.x = clamp(src.x, 0, game->getWidth() - 1);
			src.y = clamp(src.y, 0, game->getHeight() - 1);
			Point2Di dst = Point2Di(roundInt(command[0].dst.x), roundInt(command[0].dst.y));
			dst.x = clamp(dst.x, 0, game->getWidth() - 1);
			dst.y = clamp(dst.y, 0, game->getHeight() - 1);
			if (path.empty()) {
				path = game->getPathFinder(unitID)->findPath(src, dst, _unitCanMoveTo, this);
			} else {
				path = game->getPathFinder(unitID)->recalculatePath(path, src, dst, _unitCanMoveTo, this, 5.0);
			}
			if (_pathBlocked()) {
				waitTimer--;
				if (waitTimer == 0) {
					command[0].completed = true;
					waitTimer = maxWaitTimer;
				}
			} else {
				waitTimer = maxWaitTimer;
			}
		}
	}
	bool Unit::_rotate(float nextX, float nextY) {
		float theta;
		if (nextX == x) theta = (nextY>y) ? 90 : 270;
		else theta = atan((nextY - y) / (nextX - x)) * 180 / PI;
		theta = (nextX >= x) ? theta : (theta + 180);
		theta = (theta >= 360) ? (theta - 360) : (theta<0) ? (theta + 360) : theta;
		float diff1 = fabs(angle - theta), diff2 = 360 - diff1;
		float diff = min(diff1, diff2);
		if (fabs(diff)>unitTypeInfo[type].rotAngle) {
			if (diff == diff1) {
				angle += (angle>theta ? -1 : 1)*unitTypeInfo[type].rotAngle;
			} else {
				angle -= (angle>theta ? -1 : 1)*unitTypeInfo[type].rotAngle;
			}
			angle = (angle >= 360) ? (angle - 360) : (angle<0) ? (angle + 360) : angle;
		} else {
			angle = theta;
		}
		return angle == theta;
	}
	void Unit::_move(float nextX, float nextY) {
		_setState(STATE_MOVING);

		//Move
		float theta = atan((nextY - y) / (nextX - x)) * 180 / PI;
		theta = (nextX >= x) ? theta : (theta + 180);
		theta = (theta >= 360) ? (theta - 360) : (theta<0) ? (theta + 360) : theta;
		float d = sqrt((x - nextX)*(x - nextX) + (y - nextY)*(y - nextY));
		float speed = unitTypeInfo[type].speed;
		if (isGroundUnit(getCategory()) && z<game->getCurrentWaterLevel()) {
			speed /= 2;
		}
		if (d <= speed) {
			x = nextX;
			y = nextY;
		} else {
			float dx, dy;
			if (isAirUnit(unitTypeInfo[type].category) || abs(theta - angle) <= 45) {
				int a = angle;
				dx = speed*_cos[a];
				dy = speed*_sin[a];
				x = x + dx;
				y = y + dy;
			}
		}
		//Trample Damage
		for (int u = 0; u<game->getNumberOfUnits(); u++) {
			if (game->unit[u].state != STATE_DEAD && game->unit[u].team != 0 && game->diplomacy(team, game->unit[u].team) != DIPLOMACY_ALLY) {
				if (dist(Point3D(x, y, z), Point3D(game->unit[u].x, game->unit[u].y, game->unit[u].z)) <= 2) {
					int effective_attack = choice(0, unitTypeInfo[type].trampleDamage);
					game->unit[u]._hit(effective_attack, angle);
				}
			}
		}
		//Dust/Water
		if (state == STATE_MOVING) {
			switch (unitTypeInfo[type].category) {
			case UNIT_INFANTRY:
			case UNIT_BEAST:
				if (frame_number == 0 || frame_number == unitTypeInfo[type].getObject(state).length() / 2) {
					if (z <= game->getCurrentWaterLevel())	{
						game->addWaterSplash(front(), 1, unitTypeInfo[type].speed * 4);
					} else  {
						game->addDustBlow(front(), 1, unitTypeInfo[type].speed * 4);
					}
				}
				break;
			case UNIT_CHARIOT:
				if (z <= game->getCurrentWaterLevel())	{
					game->addWaterSplash(frontLeft(), 1, unitTypeInfo[type].speed * 4);
					game->addWaterSplash(frontRight(), 1, unitTypeInfo[type].speed * 4);
				} else {
					game->addDustBlow(frontLeft(), 1, unitTypeInfo[type].speed * 4);
					game->addDustBlow(frontRight(), 1, unitTypeInfo[type].speed * 4);
				}
				break;
			case UNIT_GIANT:
				if (frame_number == 0 || frame_number == unitTypeInfo[type].getObject(state).length() / 2) {
					if (z <= game->getCurrentWaterLevel()) {
						game->addWaterSplash(front(), 1, unitTypeInfo[type].speed * 20);
					} else {
						game->addDustBlow(front(), 1, unitTypeInfo[type].speed * 20);
					}
				}
				break;
			}
		}
	}
	void Unit::_removeFromMinimap() {
		game->setInMinimap(currentlyOccupiedPoints, -1, isGroundUnit(getCategory()), isWaterUnit(getCategory()), isAirUnit(getCategory()));
		currentlyOccupiedPoints.clear();
	}
	void Unit::_insertIntoMinimap() {
		_removeFromMinimap();
		currentlyOccupiedPoints = unitTypeInfo[type].getOccupiedPoints(Point2Di(roundInt(x), roundInt(y)), angle);
		game->setInMinimap(currentlyOccupiedPoints, unitID, isGroundUnit(getCategory()), isWaterUnit(getCategory()), isAirUnit(getCategory()));
	}
	bool Unit::_pathClearForRangedAttack() {
		if (!isRangedWeapon(getTypeInfo().weaponType)) {
			return true;
		}
		vector<Point2Di> line = lineBresenham(position(), game->unit[command[0].targetID].position());
		int len = line.size();
		float z1 = top().z, z2 = game->unit[command[0].targetID].centre().z;
		for (int i = 0; i<line.size(); i++) {
			float groundHeight = game->getGroundHeight(line[i].x, line[i].y);
			float lineHeight = (z1*i + z2*(len - i)) / len;
			if (groundHeight>lineHeight) {
				return false;
			}
		}
		return true;
	}
	bool Unit::_targetInRange() {
		Unit &target = game->unit[command[0].targetID];
		if (!target.isAlive())
			return false;
		float dz = z - target.z;
		float sqrRange = getRange()*getRange();
		if (isAirUnit(getCategory())) {
			if (floor(squareDist((Point2D)target.position(), (Point2D)position())) <= sqrRange) {
				return true;
			}
			vector<Point2Di> occupiedPoints = target.getTypeInfo().getOccupiedPoints(target.position(), target.angle);
			for (int i = 0; i<occupiedPoints.size(); i++)
			if (floor(squareDist((Point2D)occupiedPoints[i], (Point2D)position())) <= sqrRange) {
				return true;
			}
		} else {
			if (squareDist(target.position(), position()) <= sqrRange) {
				return true;
			}
			vector<Point2Di> occupiedPoints = target.getTypeInfo().getOccupiedPoints(target.position(), target.angle);
			for (int i = 0; i<occupiedPoints.size(); i++)
			if (squareDist(Point3D(occupiedPoints[i], target.z), position()) <= sqrRange) {
				return true;
			}
		}
		return false;
	}
	bool Unit::_targetOutOfgetLOS() {
		Unit &target = game->unit[command[0].targetID];
		return !path.empty() && dist(Point2D(x, y), Point2D(target.x, target.y))>getLOS();
	}
	bool Unit::_pathBlocked() {
		return path.empty() || !_unitCanMoveTo(this, roundInt(path[0].x), roundInt(path[0].y));
	}
	void Unit::_commandMove(float dx, float dy) {
		if (unitTypeInfo[type].speed == 0 || (roundInt(x) == roundInt(command[0].dst.x) && roundInt(y) == roundInt(command[0].dst.y))) {
			command[0].completed = true;
			return;
		}
		if (garrisoned) {
			_selfDeploy();
		}
		if (!_pathBlocked()) {
			_removeFromMinimap();
			float nextX = path[0].x, nextY = path[0].y;
			_rotate(nextX, nextY);
			_move(nextX, nextY);
			if (roundInt(path[0].x) == roundInt(x) && roundInt(path[0].y) == roundInt(y)) {
				path.erase(path.begin());
			}
			_insertIntoMinimap();
		} else {
			_setState(STATE_GENERAL);
			_calculatePath();
		}
	}
	void Unit::_attackRanged() {
		Point3D src = centre();
		Point3D dst;
		if (command[0].targetID >= 0) {
			Unit &target = game->unit[command[0].targetID];
			dst = target.centre();
		} else {
			dst = Point3D(command[0].dst.x, command[0].dst.y, game->getGroundHeight(command[0].dst.x, command[0].dst.y));
		}
		if (command[0].commandType == COMMAND_SPECIAL_ATTACK) {
			UnitTypeInfo::SpecialPower &sa = unitTypeInfo[type].specialPower[command[0].specialPowerIndex];
			float d = dist(position2D(), command[0].dst);
			game->addProjectile(
				ProjectileWeapon(
				team,
				src,
				dst,
				angle,
				ARROW_SPEED, 
				ARROW_WEIGHT,
				d>20,
				sa.attack, sa.siegeAttack, sa.areaDamageRadius,
				sa.weaponType,
				sa.addUnit_number,
				sa.addUnit_id
				));
		} else {
			float speed = ARROW_SPEED + choice(ARROW_SPEED * -0.1, ARROW_SPEED * 0.1);
			float max_angle_deviation = (1.0 - getTypeInfo().accuracy) * 5;
			float hAngle = angle + choice(-max_angle_deviation, max_angle_deviation);
			game->addProjectile(
				ProjectileWeapon(
				team,
				src,
				dst,
				hAngle,
				speed, 
				ARROW_WEIGHT,
				false,
				getAttack(), getSiegeAttack(), unitTypeInfo[type].areaDamageRadius,
				unitTypeInfo[type].weaponType
				)
				);
		}
	}
	void Unit::_attackMelee() {
		if (command.empty()) {
			return;
		}

		if (command[0].commandType == COMMAND_SPECIAL_ATTACK) {
			UnitTypeInfo::SpecialPower &sa = unitTypeInfo[type].specialPower[command[0].specialPowerIndex];
			if (sa.areaDamageRadius>0) {
				for (int u = 0; u<game->getNumberOfUnits(); u++) {
					float d = dist((Point2D)game->unit[u].position(), command[0].dst);
					if (d<sa.areaDamageRadius && game->unit[u].state != STATE_DEAD) {
						switch (game->diplomacy(team, game->unit[u].team)) {
						case DIPLOMACY_ENEMY:
							game->unit[u]._hit(isConstruction(unitTypeInfo[game->unit[u].type].category) ? sa.siegeAttack : sa.attack, angle);
							break;
						case DIPLOMACY_ALLY:
							game->unit[u]._heal(sa.heal);
							break;
						}
					}
				}
				if (sa.areaDamageRadius>1) {
					if (z>game->getCurrentWaterLevel()) {
						game->addDustBlow(position(), 1, unitTypeInfo[type].areaDamageRadius);
					} else {
						game->addWaterSplash(position2D(), 3, sa.areaDamageRadius);
					}
				}
				game->addShockWave(position2D(), sa.areaDamageRadius);
				for (int t = 0; t<sa.nTornedo; t++) {
					game->addTornedo(position2D(), rotatePoint(choice(0, 359), Point2D(0, 1)), sa.attack);
				}
			} else {
				game->unit[command[0].targetID]._hit(game->unit[command[0].targetID].getTypeInfo().isConstruction() ? sa.siegeAttack : sa.attack, angle);
				game->unit[command[0].targetID]._heal(sa.heal);
			}
		} else {
			int min_attack = getAttack()*getTypeInfo().accuracy;
			int effective_attack = choice(min_attack, getAttack());
			if (unitTypeInfo[type].areaDamageRadius>0) {
				for (int u = 0; u<game->getNumberOfUnits(); u++) {
					float d = dist(Point2D(game->unit[u].x, game->unit[u].y), command[0].dst);
					if (d<unitTypeInfo[type].areaDamageRadius && game->unit[u].state != STATE_DEAD && game->unit[u].team != 0 && game->unit[u].team != team) {
						game->unit[u]._hit(isConstruction(unitTypeInfo[game->unit[u].type].category) ? getSiegeAttack() : effective_attack, angle);
					}
				}
				if (unitTypeInfo[type].areaDamageRadius>1 && command[0].targetID >= 0) {
					if (game->unit[command[0].targetID].z>game->getCurrentWaterLevel()) {
						game->addDustBlow(game->unit[command[0].targetID].position(), 1, unitTypeInfo[type].areaDamageRadius);
					} else {
						game->addWaterSplash(game->unit[command[0].targetID].position(), 3, unitTypeInfo[type].areaDamageRadius);
					}
				}
			} else {
				game->unit[command[0].targetID]._hit(game->unit[command[0].targetID].getTypeInfo().isConstruction() ? getSiegeAttack() : effective_attack, angle);
			}
			//steam achievements
			if (unitTypeInfo[type].category == UNIT_GIANT && unitTypeInfo[game->unit[command[0].targetID].type].category == UNIT_GIANT) {
				if (getSteamAchievements())
					getSteamAchievements()->SetAchievement("ACH_GIANTS_DUEL");
			}
		}
	}
	void Unit::_commandAttack(float dx, float dy) {
		if (unitTypeInfo[type].speed == 0 || unitTypeInfo[type].attack == 0 || stance == STANCE_HOLDFIRE) {
			command[0].completed = true;
			return;
		}
		Unit &target = game->unit[command[0].targetID];
		if (target.state == STATE_DEAD || isNaturalUnit(target.getCategory()) || _targetOutOfgetLOS()) {
			command[0].completed = true;
			return;
		}
		if (game->diplomacy(team, target.team) == DIPLOMACY_ALLY) {
			command[0].commandType = COMMAND_MOVE;
			command[0].dst = target.position();
			return;
		}
		if (command[0].dst != Point2D(target.x, target.y)) {
			command[0].dst = Point2D(target.x, target.y);
			if (!path.empty() && dist(path.back(), Point2D(target.x, target.y))>5.0) {
				path.clear();
			}
		}
		if (_targetInRange() && _pathClearForRangedAttack()) {
			if (state != STATE_ATTACKING) {
				_setState(STATE_ATTACKING);
				attackTimer = unitTypeInfo[type].attackDelay / 2;
			}
			_rotate(dx, dy);
			if (game->unit[command[0].targetID].state != STATE_DEAD) {
				if (attackTimer == 0) {
					if (!isRangedWeapon(unitTypeInfo[type].weaponType)) {
						_attackMelee();
						if (garrisoned || target.garrisoned) {
							command[0].completed = true;
						}
					} else {
						_attackRanged();
						frame_number = 0;
					}
					attackTimer = unitTypeInfo[type].attackDelay;
				}
			} else {
				command[0].completed = true;
				_setState(STATE_GENERAL);
			}
			if (attackTimer>0) {
				attackTimer--;
			}
		} else {
			if (unitTypeInfo[type].speed == 0) {
				return;
			}
			if (stance == STANCE_STANDGROUND || garrisoned) {
				_setState(STATE_GENERAL);
				command[0].completed = true;
			} else {
				_removeFromMinimap();
				if (path.empty() || _pathBlocked()) {
					_calculatePath();
				}
				if (!path.empty() && !_pathBlocked()) {
					float nextX = path[0].x, nextY = path[0].y;
					_rotate(nextX, nextY);
					_move(nextX, nextY);
					if (roundInt(path[0].x) == roundInt(x) && roundInt(path[0].y) == roundInt(y)) {
						path.erase(path.begin());
					}
				} else {
					if (dist(Point2D(x, y), Point2D(target.x, target.y))<getRange()) {
						_setState(STATE_GENERAL);
					} else if (_canMoveToIfNoUnit(target.x, target.y)) {
						float nextX = target.x, nextY = target.y;
						_rotate(nextX, nextY);
						_move(nextX, nextY);
					}
				}
				_insertIntoMinimap();
			}
		}
	}
	void Unit::_commandSpecialAttack(float dx, float dy) {
		if (unitTypeInfo[type].speed == 0) {
			command[0].completed = true;
			return;
		}
		UnitTypeInfo::SpecialPower sa = unitTypeInfo[type].specialPower[command[0].specialPowerIndex];
		if (command[0].targetID >= 0) {
			Unit &target = game->unit[command[0].targetID];
			if (frame_number<sa.hitFrameNumber && (target.state == STATE_DEAD || target.garrisoned)) {
				command[0].completed = true;
				return;
			}
			if (command[0].dst != Point2D(target.x, target.y)) {
				command[0].dst = Point2D(target.x, target.y);
				if (!path.empty() && dist(path.back(), Point2D(target.x, target.y))>5.0) {
					path.clear();
				}
			}
		}
		if (dist(Point2D(x, y), command[0].dst)<sa.range) {
			if (state != sa.state) {
				_setState(sa.state);
			}
			_rotate(dx, dy);
			if (command[0].targetID<0 || game->unit[command[0].targetID].state != STATE_DEAD) {
				if (frame_number == sa.hitFrameNumber) {
					if (!isRangedWeapon(sa.weaponType)) {
						_attackMelee();
					} else {
						_attackRanged();
					}
				}
			}
			if (frame_number == unitTypeInfo[type].getObject(state).length() - 1)
				command[0].completed = true;
		} else {
			if (unitTypeInfo[type].speed == 0) {
				return;
			}
			_setState(STATE_MOVING);
			_removeFromMinimap();
			if (path.empty() || _pathBlocked()) {
				_calculatePath();
			}
			if (!path.empty() && !_pathBlocked()) {
				float nextX = path[0].x, nextY = path[0].y;
				_rotate(nextX, nextY);
				_move(nextX, nextY);
				if (roundInt(path[0].x) == roundInt(x) && roundInt(path[0].y) == roundInt(y)) {
					path.erase(path.begin());
				}
			} else {
				if (dist(Point2D(x, y), Point2D(command[0].dst.x, command[0].dst.y))<sa.range) {
					_setState(STATE_GENERAL);
				} else if (_canMoveToIfNoUnit(command[0].dst.x, command[0].dst.y)) {
					float nextX = command[0].dst.x, nextY = command[0].dst.y;
					_rotate(nextX, nextY);
					_move(nextX, nextY);
				}
			}
			_insertIntoMinimap();
		}
	}
	void Unit::_commandCutTree(float dx, float dy) {
		if (garrisoned) {
			_selfDeploy();
		}
		if (unitTypeInfo[type].speed == 0 || command[0].targetID <= 0) {
			command[0].completed = true;
			return;
		}
		Unit &target = game->unit[command[0].targetID];
		if (target.capturedByUnit >= 0) {
			command[0].completed = true;
			return;
		}
		if (_targetInRange()) {
			_rotate(dx, dy);
			if (state != STATE_CUTTING_TREE) {
				_setState(STATE_CUTTING_TREE);
				attackTimer = unitTypeInfo[type].attackDelay / 2;
			}
			if (attackTimer == 0) {
				target._hit(getAttack(), 0);
				if (target.hitPoint <= 0) {
					has.wood += target.has.wood;
					if (has.wood>MAX_STORED_WOOD) has.wood = MAX_STORED_WOOD;
					target.has.wood = 0;
					command[0].completed = true;
				}
				attackTimer = unitTypeInfo[type].attackDelay;
			}
			if (attackTimer>0) attackTimer--;
		} else {
			_setState(STATE_MOVING);
			_removeFromMinimap();
			if (path.empty() || _pathBlocked()) {
				_calculatePath();
			}
			if (!path.empty() && !_pathBlocked()) {
				float nextX = path[0].x, nextY = path[0].y;
				_rotate(nextX, nextY);
				_move(nextX, nextY);
				if (roundInt(path[0].x) == roundInt(x) && roundInt(path[0].y) == roundInt(y)) {
					path.erase(path.begin());
				}
			} else {
				if (dist(Point2D(x, y), Point2D(target.x, target.y))<getRange()) {
					command[0].targetID = target.nearestUnit(unitTypeInfo[target.type].category);
				} else if (_canMoveToIfNoUnit(target.x, target.y)) {
					float nextX = target.x, nextY = target.y;
					_rotate(nextX, nextY);
					_move(nextX, nextY);
				}
			}
			_insertIntoMinimap();
		}
	}
	void Unit::_commandGathertStone(float dx, float dy) {
		if (garrisoned) {
			_selfDeploy();
		}
		if (unitTypeInfo[type].speed == 0 || command[0].targetID <= 0) {
			command[0].completed = true;
			return;
		}
		Unit &target = game->unit[command[0].targetID];
		if (target.state == STATE_DEAD) {
			int nearestStoneID = nearestUnit(UNIT_STONE);
			if (nearestStoneID == INVALID_UNIT_INDEX || dist(game->unit[nearestStoneID].position(), position())>getLOS()) {
				command[0].completed = true;
			} else {
				command[0].targetID = nearestStoneID;
				command[0].dst.x = game->unit[nearestStoneID].x;
				command[0].dst.y = game->unit[nearestStoneID].y;
			}
		} else if (_targetInRange()) {
			path.clear();
			_rotate(dx, dy);
			_setState(STATE_GATHER_STONE);
			if (stoneGatherTimer == 0) {
				target.hitPoint--;
				target.has.stone--;
				game->teams[team].resource.stone++;
				if (target.hitPoint <= 0) {
					target._kill();
				}
				stoneGatherTimer = game->teams[team].gatherTime.stone;
				frame_number = 0;
			}
		} else {
			_setState(STATE_MOVING);
			_removeFromMinimap();
			if (path.empty() || _pathBlocked()) {
				_calculatePath();
			}
			if (!path.empty() && !_pathBlocked()) {
				float nextX = path[0].x, nextY = path[0].y;
				_rotate(nextX, nextY);
				_move(nextX, nextY);
				if (roundInt(path[0].x) == roundInt(x) && roundInt(path[0].y) == roundInt(y)) {
					path.erase(path.begin());
				}
			} else {
				if (dist(Point2D(x, y), Point2D(target.x, target.y)) < getRange()) {
					int nearestStoneID = game->unit[command[0].targetID].nearestUnit(UNIT_STONE);
					if (nearestStoneID == INVALID_UNIT_INDEX || dist(game->unit[nearestStoneID].position(), position())>getLOS()) {
						command[0].completed = true;
					} else {
						command[0].targetID = nearestStoneID;
						command[0].dst = game->unit[nearestStoneID].position();
					}
				} else if (_canMoveToIfNoUnit(target.x, target.y)) {
					float nextX = target.x, nextY = target.y;
					_rotate(nextX, nextY);
					_move(nextX, nextY);
				}
				path.clear();
			}
			_insertIntoMinimap();
		}
		if (stoneGatherTimer>0) stoneGatherTimer--;
	}
	void Unit::_commandBuild(float dx, float dy) {
		if (garrisoned) {
			_selfDeploy();
		}
		Unit &target = game->unit[command[0].targetID];
		if (target.state == STATE_DEAD) {
			command[0].completed = true;
			return;
		}
		if (command[0].targetID<0) {
			command[0].completed = true;
			return;
		}
		if (isConstruction(unitTypeInfo[type].category)) {
			Unit &target = game->unit[command[0].targetID];
			if (target.builtPercentage<100) {
				target.builtPercentage += 100.0 / unitTypeInfo[target.type].buildTime;
				if (target.builtPercentage<100) {
					target._setState(STATE_BUILDING);
				} else {
					target._setState(STATE_GENERAL);
				}
			} else {
				target._newCommand(COMMAND_MOVE, rallyPoint, -1, false, true, true, false, 0);
				command[0].completed = true;
			}
		} else {
			if (unitTypeInfo[type].speed == 0) {
				return;
			}
			if (_targetInRange()) {
				Unit &target = game->unit[command[0].targetID];
				_setState(STATE_BUILD);
				_rotate(dx, dy);
				if (target.hitPoint<target.getMaxHitPoint()) {
					target.builtPercentage += 100.0 / unitTypeInfo[target.type].buildTime;
					target.hitPoint = target.builtPercentage*target.getMaxHitPoint() / 100.0;
					if (target.builtPercentage<100) {
						target._setState(STATE_BUILDING);
					} else {
						target._setState(STATE_GENERAL);
						game->teams[team].maxPopulation += unitTypeInfo[target.type].maxPopulationIncrease;
					}
				} else {
					command[0].completed = true;
				}
			} else {
				_setState(STATE_MOVING);
				_removeFromMinimap();
				if (path.empty() || _pathBlocked()) {
					_calculatePath();
				}
				if (!path.empty() && !_pathBlocked()) {
					float nextX = path[0].x, nextY = path[0].y;
					_rotate(nextX, nextY);
					_move(nextX, nextY);
					if (roundInt(path[0].x) == roundInt(x) && roundInt(path[0].y) == roundInt(y)) {
						path.erase(path.begin());
					}
				} else {
					if (dist(Point2D(x, y), Point2D(target.x, target.y))<getRange()) {
						_setState(STATE_GENERAL);
					} else if (_canMoveToIfNoUnit(target.x, target.y)) {
						float nextX = target.x, nextY = target.y;
						_rotate(nextX, nextY);
						_move(nextX, nextY);
					}
				}
				_insertIntoMinimap();
			}
		}
	}
	void Unit::_startJump() {
		Unit &target = game->unit[command[0].targetID];
		if (target.garrisonedUnits.size()<unitTypeInfo[target.type].garrisonedUnitPosition.size()) {
			Point3D dst = target.position() + unitTypeInfo[target.type].garrisonedUnitPosition[target.garrisonedUnits.size()];
			if (!_rotate(dst.x, dst.y)) {
				return;
			}
			projection = Projectile(position(), dst, angle, unitTypeInfo[type].jumpSpeed, 0.5, false);
			if (projection.isPossible()) {
				if (garrisoned) {
					_selfDeploy();
				}
				projected = true;
				_setState(STATE_JUMP);
			} else {
				command[0].completed = true;
			}
		} else {
			command[0].completed = true;
		}
	}
	void Unit::_garrison() {
		Unit &target = game->unit[command[0].targetID];
		projected = false;
		if (target.isActive() && target.hasVacancy()) {
			_removeFromMinimap();
			garrisonedBuildingIndex = command[0].targetID;
			garrisoned = true;
			Point4D relpos = unitTypeInfo[target.type].garrisonedUnitPosition[target.garrisonedUnits.size()];
			angle = target.angle + relpos.w;
			Point3D pos = rotatePointAlongZ(target.angle, target.position() + relpos, target.position());
			x = pos.x;
			y = pos.y;
			z = pos.z;
			_setState(STATE_GENERAL);
			target.garrisonedUnits.push_back(unitID);
			if (unitTypeInfo[target.type].canGather.wood) {
				target.has.wood += has.wood;
				has.wood = 0;
			}
			command[0].completed = true;
		} else {
			command[0].completed = true;
		}
	}
	void Unit::_moveTowardsTarget() {
		Unit &target = game->unit[command[0].targetID];
		_setState(STATE_MOVING);
		_removeFromMinimap();
		if (path.empty() || _pathBlocked()) {
			_calculatePath();
		}
		if (!path.empty() && !_pathBlocked()) {
			float nextX = path[0].x, nextY = path[0].y;
			if (projected) {
				_rotate(projection.dst.x, projection.dst.y);
			} else {
				_rotate(nextX, nextY);
			}
			_move(nextX, nextY);
			if (roundInt(path[0].x) == roundInt(x) && roundInt(path[0].y) == roundInt(y)) {
				path.erase(path.begin());
			}
		} else {
			_setState(STATE_GENERAL);
		}
		if (projected)
			_setState(STATE_JUMP);
		_insertIntoMinimap();
	}
	void Unit::_commandGarrison(float dx, float dy) {
		if (unitTypeInfo[type].speed == 0) {
			command[0].completed = true;								//Cancel if unit can't _move
			return;
		}

		Unit &target = game->unit[command[0].targetID];

		if (target.state == STATE_DEAD) {
			command[0].completed = true;								//Cancel command if dead target
		} else if (!unitTypeInfo[type].canJump && (unitTypeInfo[type].category == UNIT_TREE_HOUSE || unitTypeInfo[type].category == UNIT_TREE)) {
			command[0].completed = true;								//Cancel command if invalid target
		} else if (garrisoned) {
			if (garrisonedBuildingIndex == target.unitID) {				//if already in building/tree cancel command
				command[0].completed = true;
			} else if (unitTypeInfo[type].canJump) {
				_startJump();										//Jump from one Tree/Tree-House to another
			} else {
				_selfDeploy();										//get out from other building
			}
		} else {
			if (manhattanDist(position2D(), target.position2D()) <= 1) {
				if (unitTypeInfo[type].canJump || !(unitTypeInfo[target.type].category == UNIT_TREE_HOUSE || unitTypeInfo[target.type].category == UNIT_TREE)) {
					_garrison();										//Garrison Building
				} else {
					command[0].completed = true;						//command complete
					projected = false;
				}
			} else {
				_moveTowardsTarget();								//Move towards target
			}
		}
	}
	void Unit::_whenGarrisoned() {
		if (!garrisoned) {
			return;
		}
		Unit &building = game->unit[garrisonedBuildingIndex];
		if (unitTypeInfo[building.type].canGather.wood) {
			if (unitTypeInfo[type].isWorker) {
				if (building.has.wood>0) {
					_setState(STATE_GATHER_WOOD);
					if (woodGatherTimer == 0) {
						building.has.wood--;
						game->teams[team].resource.wood++;
						woodGatherTimer = game->teams[team].gatherTime.wood;
					} else {
						woodGatherTimer--;
					}
				} else {
					if (!command.empty()) {
						command[0].completed = true;
					}
					int targetTree = nearestUnit(UNIT_TREE);
					_selfDeploy();
					_newCommand(COMMAND_CUT_TREE, game->unit[targetTree].position(), targetTree, true, false, true, false, 0);
					_newCommand(COMMAND_GARRISON, position(), command[0].targetID, true, false, true, false, 0);
				}
			}
		} else if (unitTypeInfo[building.type].canGather.metal) {
			if (unitTypeInfo[type].isWorker) {
				_setState(STATE_GATHER_METAL);
				if (metalGatherTimer == 0) {
					building.has.metal--;
					game->teams[team].resource.metal++;
					metalGatherTimer = game->teams[team].gatherTime.metal;
				} else {
					metalGatherTimer--;
				}
			}
		} else if (unitTypeInfo[building.type].canGather.food) {
			if (unitTypeInfo[type].isWorker) {
				_setState(STATE_GATHER_FOOD);
				if (foodGatherTimer == 0) {
					building.has.food--;
					game->teams[team].resource.food++;
					foodGatherTimer = game->teams[team].gatherTime.food;
				} else {
					foodGatherTimer--;
				}
			}
		}
	}
	void Unit::_nextFrame() {
		int n_frames = unitTypeInfo[type].getObject(state).length();
		int last_frame = n_frames - 1;
		if (state == STATE_GENERAL && hitPoint<getMaxHitPoint() / 2) {
			_setState(STATE_DAMAGED);
		}
		switch (state) {
		case STATE_GATHER_FOOD:
			frame_number = (game->teams[team].gatherTime.food - foodGatherTimer - 1>last_frame) ? last_frame : (game->teams[team].gatherTime.food - foodGatherTimer - 1);
			break;
		case STATE_GATHER_WOOD:
			frame_number = (game->teams[team].gatherTime.wood - woodGatherTimer - 1>last_frame) ? last_frame : (game->teams[team].gatherTime.wood - woodGatherTimer - 1);
			break;
		case STATE_CUTTING_TREE:
			frame_number = (getTypeInfo().attackDelay - attackTimer - 1>last_frame) ? last_frame : (getTypeInfo().attackDelay - attackTimer - 1);
			break;
		case STATE_GATHER_STONE:
			frame_number = (game->teams[team].gatherTime.stone - stoneGatherTimer - 1>last_frame) ? last_frame : (game->teams[team].gatherTime.stone - stoneGatherTimer - 1);
			break;
		case STATE_DEAD:
			if (unitTypeInfo[type].category == UNIT_TREE) {
				frame_number = (frame_number == last_frame) ? frame_number : (frame_number + 1);
			} else {
				frame_number = (frame_number<last_frame) ? (frame_number + 1) : last_frame;
			}
			break;
		case STATE_DAMAGED:
			if (getTypeInfo().isConstruction()) {
				frame_number = (frame_number<last_frame) ? (frame_number + 1) : last_frame;
			} else {
				frame_number = (frame_number + 1) % n_frames;
			}
			break;
		case STATE_BUILDING:
			frame_number = n_frames*builtPercentage / 100.0;
			break;
		case STATE_TRANSFORM_IN:
			frame_number++;
			if (frame_number >= last_frame) {
				_setState(STATE_GENERAL);
			}
			break;
		case STATE_TRANSFORM_OUT:
			frame_number++;
			if (frame_number >= last_frame) {
				_transform(0);
				_setState(STATE_GENERAL);
			}
			break;
		case STATE_JUMP:
			frame_number = clamp(frame_number + 1, 0, last_frame);
			break;
		case STATE_STANDBY:
			frame_number++;
			if (frame_number == last_frame) {
				_setState(STATE_GENERAL);
			}
			break;
		default:
			frame_number = (frame_number + 1) % n_frames;
		}
	}
	void Unit::_selfDeploy() {
		for (vector<int>::iterator u = game->unit[garrisonedBuildingIndex].garrisonedUnits.begin(); u != game->unit[garrisonedBuildingIndex].garrisonedUnits.end(); u++)
		if (*u == unitID) {
			Synchronizer sync(game->unit[garrisonedBuildingIndex].mutex);
			game->unit[garrisonedBuildingIndex].garrisonedUnits.erase(u);
			break;
		}
		garrisoned = false;
		garrisonedBuildingIndex = -1;
	}
	void Unit::_hit(int power, int attack_angle) {
		if (state == STATE_DEAD) {
			return;
		} else if (state == STATE_BUILDING) {
			if (getTypeInfo().isConstruction()) {
				hitPoint = 0;
				_kill();
			}
		} else {
			int bonus_power = 0;
			if (state == STATE_MOVING || state == STATE_JUMP) {
				bonus_power = 2 * power;
			}
			int effective_hit = (float)(power + bonus_power) / getArmour();
			effective_hit = (effective_hit <= 0) ? 0 : effective_hit;
			hitPoint -= effective_hit;
			if (hitPoint <= 0) {
				hitPoint = 0;
				_kill();
				if (unitTypeInfo[type].category == UNIT_INFANTRY) {
					projected = true;
					float projectile_speed = 0.005*effective_hit;
					projectile_speed = (projectile_speed>1) ? 1 : projectile_speed;
					projection = Projectile(Point3D(x, y), attack_angle, 30, projectile_speed, 1.0);
				}
			} else if (unitTypeInfo[type].standByTime>0 && effective_hit >= unitTypeInfo[type].standByDamage) {
				_setState(STATE_STANDBY);
				standByTimer = unitTypeInfo[type].standByTime;
			}
			if (isConstruction(unitTypeInfo[type].category)) {
				if (effective_hit>hitPoint / 2) {
					game->addDustBlow(centre(), unitTypeInfo[type].getObject(state).length(), currentFrame().getRadius() / 3);
				} else if (hitPoint == 0) {
					game->addDustBlow(centre(), unitTypeInfo[type].getObject(state).length(), currentFrame().getRadius() / 2);
				}
			} else if (unitTypeInfo[type].category != UNIT_TREE && !isDecoration(unitTypeInfo[type].category)) {
				if (effective_hit>0) {
					if (hitPoint == 0) {
						game->addBloodSplatter(top(), clamp(effective_hit / 10, 0, 10));
					} else {
						game->addBloodSplatter(top(), clamp(effective_hit / 20, 0, 5));
					}
				}
			}
		}
	}
	void Unit::_heal(int amount) {
		hitPoint = clampHigh(hitPoint + amount, getMaxHitPoint());
	}
	void Unit::_revive() {
		hitPoint = getMaxHitPoint();
		deathTimer = 0;
		_setState(STATE_GENERAL);
	}
	void Unit::_kill() {
		if (!isAlive()) {
			return;
		}

		try {
			//Deploy all garrisoned units
			while (!garrisonedUnits.empty()) {
				game->unit[garrisonedUnits[0]].garrisoned = false;
				game->unit[garrisonedUnits[0]]._newCommand(COMMAND_MOVE, rallyPoint, -1, true, true, true, false, 0);
				garrisonedUnits.erase(garrisonedUnits.begin());
			}

			//Clear build queue
			if (isConstruction(unitTypeInfo[type].category)) {
				for (int i = 0; i<command.size(); i++) {
					if (command[i].commandType == COMMAND_BUILD) {
						game->teams[team].resource.food += unitTypeInfo[game->unit[command[i].targetID].type].cost.food;
						game->teams[team].resource.wood += unitTypeInfo[game->unit[command[i].targetID].type].cost.wood;
						game->teams[team].resource.stone += unitTypeInfo[game->unit[command[i].targetID].type].cost.stone;
						game->teams[team].resource.metal += unitTypeInfo[game->unit[command[i].targetID].type].cost.metal;
						game->unit[command[i].targetID]._kill();
					}
				}
			}

			//Decrease population
			game->teams[team].population -= unitTypeInfo[type].population;
			if (state != STATE_BUILDING) {
				game->teams[team].maxPopulation -= unitTypeInfo[type].maxPopulationIncrease;
			}

			//clear command
			path.clear();
			command.clear();

			//Change state
			_setState(STATE_DEAD);
			frame_number = 0;

			//Remove from minimap
			_removeFromMinimap();

			//Free captured resource
			for (int u = 0; u<game->getNumberOfUnits(); u++) {
				if (game->unit[u].capturedByUnit == unitID) {
					game->unit[u].capturedByUnit = INVALID_UNIT_INDEX;
				}
			}
			if (capturedByUnit != INVALID_UNIT_INDEX) {
				game->unit[capturedByUnit]._kill();
				capturedByUnit = INVALID_UNIT_INDEX;
			}

			//Dealocate
			if (particleEngine != NULL) {
				delete particleEngine;
				particleEngine = NULL;
			}

			//message 
			if (getTypeInfo().isHeroic) {
				if (team == game->playerTeam) {
					game->setMessage(getTypeInfo().name + " has died.");
				} else {
					game->setMessage(getTypeInfo().name + " of " + game->teams[team].name + " has died.");
				}
			}
		} catch (...) {}
	}
	void Unit::_generateRandomBirdFly() {
		if (unitTypeInfo[type].category == UNIT_BIRD && isIdle()) {
			Point2Di newPos = Point2Di(clamp(x + choice(-5, 5), 0, game->getWidth() - 1), clamp(y + choice(-5, 5), 0, game->getHeight() - 1));
			_newCommand(COMMAND_MOVE, newPos, (UnitID)-1, false, true, true, false, -1);
		}
	}
	void Unit::_transform(int index) {
		UnitType newType = unitTypeInfo[type].transformation[index].unitType, oldType = type;

		//check if valid transformation
		type = newType;
		if (!_unitCanMoveTo(this, x, y)) {
			type = oldType;
			if (getTypeInfo().autoTransformTime>0 && getAutoTransformTimeRemaining() <= 0) {
				birthTime += getTypeInfo().autoTransformTime / 30;
				hitPoint -= getMaxHitPoint() / 3;
				if (hitPoint <= 0) {
					hitPoint = 0;
					_kill();
				}
			}
			return;
		}
		type = oldType;

		//set hitpoint as same percentage
		int newMaxHitPoint = unitTypeInfo[newType].getMaxHitPoint(level);
		hitPoint = newMaxHitPoint*((float)hitPoint / getMaxHitPoint());

		//swap pointer to recharge special ability
		rechargedTransformation[index] = 0;
		int temp[MAX_SPECIAL_ABILITY];
		memcpy(temp, swapRechargedSpecialAbility, sizeof(int)*MAX_SPECIAL_ABILITY);
		memcpy(swapRechargedSpecialAbility, rechargedSpecialAbility, sizeof(int)*MAX_SPECIAL_ABILITY);
		memcpy(rechargedSpecialAbility, temp, sizeof(int)*MAX_SPECIAL_ABILITY);
		memcpy(temp, swapRechargedTransformation, sizeof(int)*MAX_SPECIAL_ABILITY);
		memcpy(swapRechargedTransformation, rechargedTransformation, sizeof(int)*MAX_SPECIAL_ABILITY);
		memcpy(rechargedTransformation, temp, sizeof(int)*MAX_SPECIAL_ABILITY);

		//clamp abilty recharged 0
		for (int s = 0; s < unitTypeInfo[type].specialPower.size(); s++) {
			rechargedSpecialAbility[s] = clampLow(rechargedSpecialAbility[s], 0);
		}
		for (int s = 0; s < unitTypeInfo[type].transformation.size(); s++) {
			rechargedTransformation[s] = clampLow(rechargedTransformation[s], 0);
		}

		//keep same special abilty recharge
		for (int s1 = 0; s1 < unitTypeInfo[type].specialPower.size(); s1++) {
			for (int s2 = 0; s2 < unitTypeInfo[newType].specialPower.size(); s2++) {
				if (unitTypeInfo[type].specialPower[s1].name == unitTypeInfo[newType].specialPower[s2].name) {
					float ratio = unitTypeInfo[newType].specialPower[s2].rechargeTime / unitTypeInfo[type].specialPower[s1].rechargeTime;
					rechargedSpecialAbility[s2] = swapRechargedSpecialAbility[s1] * ratio;
				}
			}
		}

		//particle Engine
		if (particleEngine != NULL) {
			delete particleEngine;
		}
		if (unitTypeInfo[newType].particleEngine == NULL) {
			particleEngine = NULL;
		} else {
			particleEngine = new ParticleEngine(*unitTypeInfo[newType].particleEngine);
			particleEngine->bind(&game->particleRenderer);
		}

		//change type
		type = newType;

		//change birth time
		birthTime = game->getGamePlayTime();
	}
	void Unit::_autoTransform() {
		if (getTypeInfo().autoTransformTime>0 && !getTypeInfo().transformation.empty() && unitTypeInfo[type].hasState(STATE_TRANSFORM_OUT)
			&& game->getGamePlayTime() - birthTime>getTypeInfo().autoTransformTime) {
			path.clear();
			_setState(STATE_TRANSFORM_OUT);
		}
	}
	void Unit::_autoHeal() {
		if (state != STATE_DEAD && state != STATE_BUILDING) {
			if (autoHealTimer == 0) {
				hitPoint = clamp(hitPoint + getHealAmount(), 0, getMaxHitPoint());
				autoHealTimer = getHealDelay();
			} else {
				autoHealTimer--;
			}
		}
	}
	void Unit::_calculateBlending() {
		if (team == 0)
			return;
		blended = 0;
		if (unitTypeInfo[type].category == UNIT_INFANTRY || unitTypeInfo[type].category == UNIT_BEAST || unitTypeInfo[type].category == UNIT_INSECT) {
			blended = game->getBlend(position());
		}
		if (state == STATE_MOVING || state == STATE_ATTACKING) {
			blended *= 0.66;
		}
		blended *= unitTypeInfo[type].camouflage;
		blended += unitTypeInfo[type].stealth;
		blended = clamp(blended, 0.0, 1.0);
	}
	void Unit::_paralyzeNearbyUnits() {
		for (UnitID u = 0; u < game->getNumberOfUnits(); u++) {
			if (game->unit[u].isAlive() && game->unit[u].team != team && manhattanDist(position2D(), game->unit[u].position2D()) <= getRange()) {
				game->unit[u].paralyze();
			}
		}
	}
	void Unit::_newCommand(CommandType commandType, Point2D dst, UnitID targetID, bool add, bool attackMove, bool aiCommand, bool atFront, int specialPowerIndex) {
		if (stance == STANCE_HOLDFIRE && commandType == COMMAND_ATTACK) {
			return;
		}
		if (commandType == COMMAND_GARRISON && getCategory() != UNIT_INFANTRY) {
			return;
		}
		if ((state == STATE_TRANSFORM_IN || state == STATE_TRANSFORM_OUT) && !add) {
			return;
		}

		if (add && aiCommand) {
			while (!command.empty() && command[0].aiCommand) {
				command.erase(command.begin());
			}
		}

		try {

			if (projected) {
				add = true;
			}

			if (add == true && command.size() > 10) {
				command.erase(command.end() - 1);
			}

			if (!dst.in(0, 0, game->getWidth() - 1, game->getHeight()) && targetID >= 0) {
				if (isConstruction(getCategory()) && commandType == COMMAND_BUILD) {
					game->unit[targetID].x = x;
					game->unit[targetID].y = y;
				}
				dst.x = game->unit[targetID].x;
				dst.y = game->unit[targetID].y;
			}

			//Add command
			Command c(dst, commandType, targetID, attackMove, aiCommand, specialPowerIndex);
			if (commandType == COMMAND_SPECIAL_ATTACK) {
				if (isValidSpecialAttack(c)) {
					path.clear();
					command.clear();
					command.insert(command.end(), c);
					frame_number = 0;
					rechargedSpecialAbility[specialPowerIndex] = 0;
				}
			} else {
				if (isConstruction(unitTypeInfo[type].category)) {
					if (commandType == COMMAND_BUILD) {
						command.insert(command.end(), c);
					} else {
						rallyPoint = dst;
					}
				}
				if (!isConstruction(unitTypeInfo[type].category) || unitTypeInfo[type].speed>0) {
					if (!add || atFront) {
						path.clear();
					}
					if (!add) {
						command.clear();
					}
					command.insert(atFront ? command.begin() : command.end(), c);
				}
			}
			maxWaitTimer = 10;

			onAttackMove = (!command.empty() && stance != STANCE_HOLDFIRE && command[0].attackMove);
		} catch (...) {}
	}
	void Unit::_playSound() const {
		float soundProbability = state==STATE_GENERAL ? 0.1
							   : state==STATE_MOVING ? 0.3
							   : state==STATE_ATTACKING ? 0.5
							   : state==STATE_DEAD ? 0.5
							   : state==STATE_DAMAGED ? 0.1
							   : 1.0;
		int nFrames = unitTypeInfo[type].getObject(state).length();
		if (!game->isPaused() && frame_number == nFrames / 2 && !unitTypeInfo[type].audio[state].empty() && satisfiesInProbability(soundProbability)) {
			int i = choice(0, unitTypeInfo[type].audio[state].size() - 1);
			playAudio(unitTypeInfo[type].audio[state][i], CHANNEL_SOUND, game->soundAngle(x, y), game->soundDistance(x, y));
		}
	}
	void Unit::_drown() {
		if (isNaturalUnit(getCategory())) {
			return;
		}
		float groundLevel = game->getGroundHeight(x, y);
		if (isGroundUnit(getCategory()) && getMaxZ() + z<game->getCurrentWaterLevel() - 0.5) {
			//Ground unit drowns under water
			_hit(1, 0);
		} else if (unitTypeInfo[type].drownInCurrent && groundLevel<game->getCurrentWaterLevel() && game->getWaterWaveAmplitude()>0 && game->getWaterWavePhase() == 0) {
			//fragile units drown in current
			_hit(100, 0);
		}
	}
	void Unit::_setRenderAlpha() {
		float targetRenderAlpha = 0.0;
		if (position().in(0, 0, game->getWidth() - 1, game->getHeight() - 1)) {
			if (exists()
				&& !(!isConstruction(unitTypeInfo[type].category) && builtPercentage < 100)
				&& (game->playerTeam == 0 || isVisibleToTeam(game->playerTeam))) {
				if (blended == 0.0) {
					targetRenderAlpha = 1.0;
				} else {
					targetRenderAlpha = 1.0 - blended * 0.9;
				}
			}
		}
		float d = 0.01;
		if (renderAlpha - d >= targetRenderAlpha) {
			renderAlpha -= d;
		} else if (renderAlpha + d <= targetRenderAlpha) {
			renderAlpha += d;
		} else {
			renderAlpha = targetRenderAlpha;
		}
	}

	//-----------------------------------------------------------Public Functions
	//most of the public functions contain thread locks
	Unit::Unit() :
		state(STATE_DEAD),
		has(Resource<>(0, 0, 0, 0)),
		garrisoned(false), capturedByUnit(INVALID_UNIT_INDEX), projected(false),
		foodGatherTimer(0), woodGatherTimer(0), stoneGatherTimer(0), metalGatherTimer(0),
		attackTimer(0), autoHealTimer(0), deathTimer(0),
		waitTimer(0), waiting(false), birthTime(0), fleeing(false),
		x(0), y(0), z(0), tilt(0), angle(0),
		particleEngine(NULL), renderAlpha(0.0),
		level(1) {}
	void Unit::unload() {
		Synchronizer sync(mutex);

		if (!exists()) {
			_kill();
		}

		tilt = 0;
		deathTimer = 0;
		has.food = has.wood = has.stone = has.metal = 0;
		garrisoned = false;
		waiting = false;
		projected = false;

	}
	void Unit::bind(UnitType type, int team, Point3D p, float angle, int level, UnitTypeInfo* unitTypeInfo, Game* game, int unitID, bool build) {
		if (isAlive()) {
			throw Exception("ramayana::Unit::init() called second time for same unit");
		}

		//Create Mutual Exclusion
		mutex.init();
		Synchronizer sync(mutex);

		try {
			//get parameters
			this->game = game;
			this->unitTypeInfo = unitTypeInfo;

			if (type<0 || type >= MAX_OBJECT_TYPE) throw Exception("Invalid type in ramayana::Unit::init()");
			this->type = type;
			if (team<0 || team >= Game::MAX_TEAM) throw Exception("Invalid team in ramayana::Unit::init()");
			this->team = team;
			this->x = p.x;
			this->y = p.y;
			this->z = p.z;
			this->angle = angle;
			this->level = level;
			if (unitTypeInfo[type].isHeroic) {
				scaling = 1.0;
			} else {
				scaling = choice(0.95, 1.05);
			}
			birthTime = game->getGamePlayTime();

			//Single Instance for Heroes
			if (this->unitTypeInfo[this->type].isHeroic) {
				for (int i = 0; i<game->getNumberOfUnits(); i++) {
					if (i != unitID && game->unit[i].type == this->type) {
						if (game->unit[i].isAlive()) {
							return;
						} else {
							this->level = clamp(game->unit[i].level, 1, 10);
						}
					}
				}
			}

			//Initialization
			_setState(STATE_GENERAL);
			hitPoint = unitTypeInfo[type].getMaxHitPoint(level);
			has = unitTypeInfo[type].has;
			deathTimer = DEAD_UNIT_VANISH_TIMER;
			builtPercentage = 100;
			stance = STANCE_GENERAL;
			rallyPoint = rotatePoint(angle, Point3D(x + currentFrame().getXMax()*OBJECT_SCALING + 1, y + currentFrame().getYMid()*OBJECT_SCALING, z + currentFrame().getZMid()*OBJECT_SCALING), centre());
			blended = 0;
			fleeing = false;
			_updatePosition();
			this->unitID = unitID;
			onAttackMove = false;
			command.clear();
			path.clear();
			standByTimer = 0;

			//Particle Engine
			if (unitTypeInfo[type].particleEngine == NULL) {
				particleEngine = NULL;
			} else {
				particleEngine = new ParticleEngine(*unitTypeInfo[type].particleEngine);
				particleEngine->bind(&game->particleRenderer);
			}

			//Update minimap
			_insertIntoMinimap();

			//find if tree and captured by a treehouse
			if (unitTypeInfo[type].category == UNIT_TREE) {
				for (int i = 0; i<game->getNumberOfUnits(); i++) {
					if (unitTypeInfo[game->unit[i].type].category == UNIT_TREE_HOUSE && fabs(game->unit[i].x - x) <= 1 && fabs(game->unit[i].y - y) <= 1) {
						capturedByUnit = i;
					}
				}
			} else if (unitTypeInfo[type].category == UNIT_TREE_HOUSE) {
				for (int i = 0; i<game->getNumberOfUnits(); i++) {
					if (unitTypeInfo[game->unit[i].type].category == UNIT_TREE && fabs(game->unit[i].x - x) <= 1 && fabs(game->unit[i].y - y) <= 1) {
						game->unit[i].capturedByUnit = unitID;
					}
				}
			}

			//Special ability
			for (int i = 0; i < MAX_SPECIAL_ABILITY; i++) {
				rechargedSpecialAbility[i] = rechargedTransformation[i] = swapRechargedSpecialAbility[i] = swapRechargedTransformation[i] = 0;
			}

			//Build
			if (build) {
				Resource<> cost = game->teams[team].getUnitCost(type);
				if (cost.food <= game->teams[team].resource.food
					&& cost.wood <= game->teams[team].resource.wood
					&& cost.stone <= game->teams[team].resource.stone
					&& cost.metal <= game->teams[team].resource.metal) {
					game->teams[team].resource.food -= cost.food;
					game->teams[team].resource.wood -= cost.wood;
					game->teams[team].resource.stone -= cost.stone;
					game->teams[team].resource.metal -= cost.metal;
				} else {
					return;
				}
			} else {
				game->teams[team].maxPopulation += unitTypeInfo[type].maxPopulationIncrease;
			}
			if (unitTypeInfo[type].population == 0 || unitTypeInfo[type].population + game->teams[team].population <= game->teams[team].maxPopulation) {
				game->teams[team].population += unitTypeInfo[type].population;
			} else {
				return;
			}

			if (build) {
				state = STATE_BUILDING;
				if (isConstruction(unitTypeInfo[type].category)) {
					hitPoint = 0;
				}
				builtPercentage = 0;
			}

			//set birth time
			birthTime = game->getGamePlayTime();
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::init()", true);
		} catch (...) {}
	}
	void Unit::update() {
		if (game->isEditable()) {
			_updatePosition();
			return;
		}

		Synchronizer sync(mutex);

		_setRenderAlpha();

		try {
			onAttackMove = (!command.empty() && stance != STANCE_HOLDFIRE && command[0].attackMove);

			if (command.empty()) {
				targetPosition = targetUnitPosition = Point2Di(-9999, -9999);
				targetID = -1;
			} else {
				if (command[0].targetID >= 0) {
					targetUnitPosition = targetPosition = game->unit[command[0].targetID].position();
					targetID = command[0].targetID;
				} else {
					targetPosition = command[0].dst;
					targetUnitPosition = Point2Di(-9999, -9999);
					targetID = -1;
				}
			}

			if (state == STATE_BUILDING || state == STATE_TRANSFORM_IN || state == STATE_TRANSFORM_OUT) {
				_updatePosition();
			} else if (state == STATE_DEAD) {
				_updatePosition();
				deathTimer--;
				if (unitTypeInfo[type].category == UNIT_TREE) {
					if (tilt < 100) {
						tilt += 5;
					}
				}
				if (deathTimer<DEAD_UNIT_VANISH_TIMER - unitTypeInfo[type].getObject(state).length() && getTypeInfo().transformOnDeath) {
					_revive();
					_transform(0);
					_setState(STATE_TRANSFORM_IN);
				}
			} else if (standByTimer>0) {
				standByTimer--;
			} else {
				_updatePosition();
				_generateRandomBirdFly();
				_calculateBlending();
				if (getTypeInfo().canParalyze) {
					_paralyzeNearbyUnits();
				}
				if (getTypeInfo().canSetFire && !isOnWater()) {
					game->addFire(position(), 3, 1000);
				}
				if (getTypeInfo().transformOnWater && state != STATE_TRANSFORM_OUT && isOnWater()) {
					_transform(0);
					_setState(STATE_TRANSFORM_IN);
				}

				if (!command.empty()) {
					if (command[0].completed) {
						path.clear();
						command.erase(command.begin());
						_setState(STATE_GENERAL);
					} else {
						switch (command[0].commandType) {
						case COMMAND_MOVE:
							_commandMove(command[0].dst.x, command[0].dst.y);
							break;
						case COMMAND_ATTACK:
							_commandAttack(command[0].dst.x, command[0].dst.y);
							break;
						case COMMAND_CUT_TREE:
							_commandCutTree(command[0].dst.x, command[0].dst.y);
							break;
						case COMMAND_GATHER_STONE:
							_commandGathertStone(command[0].dst.x, command[0].dst.y);
							break;
						case COMMAND_BUILD:
							_commandBuild(command[0].dst.x, command[0].dst.y);
							break;
						case COMMAND_GARRISON:
							_commandGarrison(command[0].dst.x, command[0].dst.y);
							break;
						case COMMAND_SPECIAL_ATTACK:
							_commandSpecialAttack(command[0].dst.x, command[0].dst.y);
							break;
						}
					}
				}
				_whenGarrisoned();

				_autoHeal();
				_autoTransform();
			}
			if (exists()) {
				_nextFrame();
			}
			if (state != STATE_DEAD) {
				for (int i = 0; i<unitTypeInfo[type].specialPower.size(); i++) {
					if (rechargedSpecialAbility[i] < unitTypeInfo[type].specialPower[i].rechargeTime) {
						rechargedSpecialAbility[i]++;
					}
				}
				for (int i = 0; i<unitTypeInfo[type].transformation.size(); i++) {
					if (rechargedTransformation[i] < unitTypeInfo[type].transformation[i].rechargeTime) {
						rechargedTransformation[i]++;
					}
				}
				if (particleEngine != NULL) {
					particleEngine->emit();
				}
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::update()", true);
		} catch (...) {}

	}
	void Unit::newUserCommand(CommandType commandType, Point2D dst, UnitID targetID, bool add, bool attackMove, bool atFront, int specialPowerIndex) {
		Synchronizer sync(mutex);
		_newCommand(commandType, dst, targetID, add, attackMove, false, atFront, specialPowerIndex);
	}
	void Unit::newUserCommand(CommandType commandType, Point2D dst, bool add, bool attackMove, bool atFront, int specialPowerIndex) {
		Synchronizer sync(mutex);
		_newCommand(commandType, dst, -1, add, attackMove, false, atFront, specialPowerIndex);
	}
	void Unit::newUserCommand(CommandType commandType, UnitID targetID, bool add, bool attackMove, bool atFront, int specialPowerIndex) {
		Synchronizer sync(mutex);
		_newCommand(commandType, Point2D(-1, -1), targetID, add, attackMove, false, atFront, specialPowerIndex);
	}
	void Unit::newAICommand(CommandType commandType, Point2D dst, UnitID targetID, bool add, bool attackMove, bool atFront, int specialPowerIndex) {
		Synchronizer sync(mutex);
		_newCommand(commandType, dst, targetID, add, attackMove, true, atFront, specialPowerIndex);
	}
	void Unit::newAICommand(CommandType commandType, Point2D dst, bool add, bool attackMove, bool atFront, int specialPowerIndex) {
		Synchronizer sync(mutex);
		_newCommand(commandType, dst, -1, add, attackMove, true, atFront, specialPowerIndex);
	}
	void Unit::newAICommand(CommandType commandType, UnitID targetID, bool add, bool attackMove, bool atFront, int specialPowerIndex) {
		Synchronizer sync(mutex);
		_newCommand(commandType, Point2D(-1, -1), targetID, add, attackMove, true, atFront, specialPowerIndex);
	}
	void Unit::deploy(int index) {
		Synchronizer sync(mutex);

		try {
			if (index<garrisonedUnits.size()) {
				game->unit[garrisonedUnits[index]].garrisoned = false;
				game->unit[garrisonedUnits[index]]._newCommand(COMMAND_MOVE, rallyPoint, (UnitID)-1, true, true, true, false, -1);
				vector<int>::iterator u = garrisonedUnits.begin();
				for (int i = 0; i<index; i++) {
					u++;
				}
				garrisonedUnits.erase(u);
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::deploy()", true);
		} catch (...) {}
	}
	void Unit::deployAll() {
		Synchronizer sync(mutex);

		try {
			while (!garrisonedUnits.empty()) {
				deploy(0);
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::deployAll()", true);
		} catch (...) {}
	}
	void Unit::render() {
		Synchronizer sync(mutex);
		try {
			WaveFrontObjSequence &aniobj = currentAnimation();
			if (game->isEditable()) {
				renderAlpha = 1.0;
			}
			//Object
			aniobj.render(frame_number, availableTeamColors[game->teams[team].color], Color(1.0, 1.0, 1.0, renderAlpha));

			//Particle engine
			if (!game->isPaused() && particleEngine != NULL) {
				particleEngine->setPosition(position());
			}

			_playSound();
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::render()", true);
		} catch (...) {}
	}
	void Unit::hit(ProjectileWeapon &p) {
		if (state == STATE_DEAD) {
			return;
		}

		Synchronizer sync(mutex);

		if (game->diplomacy(p.team, team) != DIPLOMACY_ENEMY) {
			return;
		}

		try {
			int theta = p.hAngle;
			float d = dist(Line3D(p.pos, p.lastpos), Point3D(x, y, z));
			if (d>1) {
				theta = Line2D(p.pos, Point2D(x, y)).tangent();
			}
			float ratio = 1;
			if (p.areaDamageRadius>0) {
				ratio = 1.0 - d / p.areaDamageRadius;
				ratio = (ratio<0) ? 0 : ratio;
				if (ratio >= 0.25) ratio = 1.0;
				else if (ratio >= 0.10) ratio = 0.5;
				else if ((ratio >= 0.0)) ratio = 0.25;
				else ratio = 0;
			}
			if (isConstruction(unitTypeInfo[type].category)) {
				_hit(p.siegeAttack*ratio, theta);
			} else {
				_hit(p.attack*ratio, theta);
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::hit()", true);
		} catch (...) {}
	}
	void Unit::hitTornedo(Point2D p) {
		if (state == STATE_DEAD || unitTypeInfo[type].category == UNIT_STONE || unitTypeInfo[type].tornedoResistant) {
			return;
		}

		Synchronizer sync(mutex);

		try {
			int damage = (game->getGroundHeight(p.x, p.y)>game->getCurrentWaterLevel()) ? 50 : 20;
			_hit(damage, 0);
			if (unitTypeInfo[type].category == UNIT_INFANTRY || unitTypeInfo[type].category == UNIT_BEAST || unitTypeInfo[type].category == UNIT_AIR || unitTypeInfo[type].category == UNIT_CHARIOT) {
				if (state != STATE_DEAD) {
					_setState(STATE_STANDBY);
				}
				Point2D pos = Point2D(x, y);
				pos = rotatePoint(30, pos, p);
				x = pos.x;
				y = pos.y;
				float theta = Line2D(p, pos).tangent();
				projection = Projectile(pos, theta, 90, 1, 1.0);
				projected = true;
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::hitTornedo()", true);
		} catch (...) {}
	}
	void Unit::burn(int fireIntensity) {
		if (state == STATE_DEAD || unitTypeInfo[type].category == UNIT_STONE) {
			return;
		}

		Synchronizer sync(mutex);

		try {
			if (getTypeInfo().transformOnBurn && !isOnWater()) {
				_transform(0);
				_setState(STATE_TRANSFORM_IN);
			}
			if (unitTypeInfo[type].category == UNIT_TREE || unitTypeInfo[type].category == UNIT_DECORATION) {
				game->addFire(Point3D(x, y, game->getGroundHeight(x, y)), 3, fireIntensity + 500);
			} else {
				game->addFire(Point3D(x, y, game->getGroundHeight(x, y)), 1, fireIntensity);
			}
			int damage = clamp(fireIntensity, 0, MAX_FIRE_DAMAGE);
			damage *= (1.0 - getTypeInfo().fireResistance);
			if (isConstruction(unitTypeInfo[type].category)) {
				damage *= 10;
			}
			hitPoint = clamp(hitPoint - damage, 0, getMaxHitPoint());
			if (hitPoint <= 0) {
				_kill();
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::burn()", true);
		} catch (...) {}
	}
	void Unit::extinguish() {
		Synchronizer sync(mutex);

		if (getTypeInfo().transformOnWater && state != STATE_TRANSFORM_OUT) {
			_transform(0);
			_setState(STATE_TRANSFORM_IN);
		}
	}
	void Unit::paralyze() {
		if (!isAlive() || getCategory() == UNIT_STONE || !isGroundUnit(getCategory())) {
			return;
		}

		Synchronizer sync(mutex);

		try {
			path.clear();
			command.clear();
			_setState(STATE_STANDBY);
			game->addPoison(position(), 1, 1);
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::paralyze()", true);
		} catch (...) {}
	}
	bool Unit::inRangeOf(ProjectileWeapon &p) {
		Line3D l(p.lastpos, p.pos);
		if (isConstruction(unitTypeInfo[type].category)) {
			Box box(x + getMinX(), y + getMinY(), z + getMinZ(), x + getMaxX(), y + getMaxY(), z + getMaxZ());
			Point3D pos(x, y, z);
			rotatePointAlongZ(360 - angle, l.A, pos);
			rotatePointAlongZ(360 - angle, l.B, pos);
			return box.intersect(l);
		} else {
			return dist(l, centre()) <= 2;
		}
	}
	void Unit::commandTransform(int index) {
		Synchronizer sync(mutex);

		try {
			if (index<0 || index >= unitTypeInfo[type].transformation.size() || level<unitTypeInfo[type].transformation[index].neededLevel) {
				return;
			}
			path.clear();
			if (unitTypeInfo[type].hasState(STATE_TRANSFORM_OUT)) {
				_setState(STATE_TRANSFORM_OUT);
			} else {
				_transform(index);
				_setState(STATE_TRANSFORM_IN);
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::commandTransform()", true);
		} catch (...) {}
	}
	void Unit::stop() {
		Synchronizer sync(mutex);

		try {
			//Deploy
			if (garrisoned) {
				_selfDeploy();
			}

			//Clear build queue
			if (isConstruction(unitTypeInfo[type].category)) {
				for (int i = 0; i<command.size(); i++) {
					if (command[i].commandType == COMMAND_BUILD) {
						game->teams[team].resource.food += unitTypeInfo[game->unit[command[i].targetID].type].cost.food;
						game->teams[team].resource.wood += unitTypeInfo[game->unit[command[i].targetID].type].cost.wood;
						game->teams[team].resource.stone += unitTypeInfo[game->unit[command[i].targetID].type].cost.stone;
						game->teams[team].resource.metal += unitTypeInfo[game->unit[command[i].targetID].type].cost.metal;
						game->unit[command[i].targetID]._kill();
					}
				}
			}
			//clear command
			path.clear();
			command.clear();
			_setState(STATE_GENERAL);
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::stop()", true);
		} catch (...) {}
	}
	void Unit::suicide() {
		Synchronizer sync(mutex);

		try {
			if (isConstruction(unitTypeInfo[type].category)) {
				game->addDustBlow(centre(), unitTypeInfo[type].getObject(state).length(), currentFrame().getRadius() / 2);
			}

			if (state == STATE_BUILDING) {
				UnitTypeInfo &o = unitTypeInfo[type];
				game->teams[team].resource.food += o.cost.food;
				game->teams[team].resource.wood += o.cost.wood;
				game->teams[team].resource.stone += o.cost.stone;
				game->teams[team].resource.metal += o.cost.metal;
			}
			_kill();
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::suicide()", true);
		} catch (...) {}
	}
	void Unit::remove() {
		suicide();
		deathTimer = 0;
	}
	void Unit::cancelCommand(int index) {
		Synchronizer sync(mutex);

		try {
			int i = 0;
			for (vector<Command>::iterator c = command.begin(); c != command.end(); c++, i++) {
				if (i == index) {
					command.erase(c);
					break;
				}
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::cancelCommand()", true);
		} catch (...) {}
	}
	void Unit::cancelBuild(int unitType) {
		Synchronizer sync(mutex);

		try {
			for (int i = command.size() - 1; i >= 0; i--) {
				if (command[i].commandType == COMMAND_BUILD && game->unit[command[i].targetID].type == unitType) {
					game->teams[team].resource.food += unitTypeInfo[game->unit[command[i].targetID].type].cost.food;
					game->teams[team].resource.wood += unitTypeInfo[game->unit[command[i].targetID].type].cost.wood;
					game->teams[team].resource.stone += unitTypeInfo[game->unit[command[i].targetID].type].cost.stone;
					game->teams[team].resource.metal += unitTypeInfo[game->unit[command[i].targetID].type].cost.metal;
					game->unit[command[i].targetID]._kill();
					cancelCommand(i);
					break;
				}
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Unit::cancelBuild()", true);
		} catch (...) {}
	}
	void Unit::flee(Vector2D dir, int distance) {
		Point2D pos = dir * distance;
		pos = rotatePoint(choice(-90, 90), pos);
		pos = position2D() - pos;
		_newCommand(COMMAND_MOVE, pos, UnitID(-1), false, false, false, false, -1);
		fleeing = true;
	}
	bool Unit::isValidSpecialAttack(Command c) {
		if (level<unitTypeInfo[type].specialPower[c.specialPowerIndex].neededLevel) {
			return false;
		}

		switch (unitTypeInfo[type].specialPower[c.specialPowerIndex].targetType) {
		case TARGET_UNIT:
			if (c.targetID<0) {
				return false;
			}
			break;
		case TARGET_GROUND_UNIT:
			if (c.targetID<0 || !isGroundUnit(unitTypeInfo[game->unit[c.targetID].type].category)) {
				return false;
			}
			break;
		case TARGET_AIR_UNIT:
			if (c.targetID<0 || !isAirUnit(unitTypeInfo[game->unit[c.targetID].type].category)) {
				return false;
			}
			break;
		case TARGET_WATER_UNIT:
			if (c.targetID<0 || !isWaterUnit(unitTypeInfo[game->unit[c.targetID].type].category)) {
				return false;
			}
			break;
		case TARGET_GIANT_UNIT:
			if (c.targetID<0 || unitTypeInfo[game->unit[c.targetID].type].category != UNIT_GIANT) {
				return false;
			}
			break;
		}
		if (c.targetID >= 0) {
			Unit &target = game->unit[c.targetID];
			if (unitTypeInfo[type].specialPower[c.specialPowerIndex].friendly) {
				if (game->diplomacy(team, target.team) == DIPLOMACY_ENEMY) {
					return false;
				}
			} else {
				if (target.team == 0 && unitTypeInfo[type].specialPower[c.specialPowerIndex].targetType != TARGET_ANY) {
					return false;
				}
				if (game->diplomacy(team, target.team) == DIPLOMACY_ALLY) {
					return false;
				}
			}
		}
		return true;
	}
	void Unit::rotateTowards(Unit& target) {
		angle = Line2D(position2D(), target.position2D()).tangent();
	}

	xml_node<char>* Unit::toXMLNode(xml_document<char> &doc) {
		xml_node<char> *nodeUnit = doc.allocate_node(node_element, "unit");
		nodeUnit->append_attribute(doc.allocate_attribute("unitID", doc.allocate_string(toString(unitID).data())));
		nodeUnit->append_attribute(doc.allocate_attribute("idForCampaign", idForCampaign.data()));
		if (exists()) {
			nodeUnit->append_node(doc.allocate_node(node_element, "type", doc.allocate_string(toString(type).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "team", doc.allocate_string(toString(team).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "level", doc.allocate_string(toString(level).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(x).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(y).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "z", doc.allocate_string(toString(z).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "angle", doc.allocate_string(toString(angle).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "tilt", doc.allocate_string(toString(tilt).data())));
			for (int i = 0; i < command.size() && i<1; i++) {
				xml_node<char> *nodeCommand = doc.allocate_node(node_element, "command");
				nodeUnit->append_node(nodeCommand);
				nodeCommand->append_node(doc.allocate_node(node_element, "commandType", doc.allocate_string(toString(command[i].commandType).data())));
				nodeCommand->append_node(doc.allocate_node(node_element, "targetID", doc.allocate_string(toString(command[i].targetID).data())));
				nodeCommand->append_node(doc.allocate_node(node_element, "dst"));
				nodeCommand->first_node("dst")->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(command[i].dst.x).data())));
				nodeCommand->first_node("dst")->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(command[i].dst.y).data())));
				nodeCommand->append_node(doc.allocate_node(node_element, "attackMove", doc.allocate_string(toString(command[i].attackMove).data())));
				nodeCommand->append_node(doc.allocate_node(node_element, "aiCommand", doc.allocate_string(toString(command[i].aiCommand).data())));
				nodeCommand->append_node(doc.allocate_node(node_element, "specialPowerIndex", doc.allocate_string(toString(command[i].specialPowerIndex).data())));
				nodeCommand->append_node(doc.allocate_node(node_element, "completed", doc.allocate_string(toString(command[i].completed).data())));
			}
			/*for(int i = 0; i < path.size(); i++) {
				xml_node<char> *nodePath=doc.allocate_node(node_element, "path");
				nodeUnit->append_node(nodePath);
				nodePath->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(path[i].x).data())));
				nodePath->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(path[i].y).data())));
			}*/
			nodeUnit->append_node(doc.allocate_node(node_element, "state", doc.allocate_string(toString(state).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "frame_number", doc.allocate_string(toString(frame_number).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "hitPoint", doc.allocate_string(toString(hitPoint).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "builtPercentage", doc.allocate_string(toString(builtPercentage).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "stance", doc.allocate_string(toString(stance).data())));

			nodeUnit->append_node(doc.allocate_node(node_element, "garrisoned", doc.allocate_string(toString(garrisoned).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "garrisonedBuildingIndex", doc.allocate_string(toString(garrisonedBuildingIndex).data())));
			for (int i = 0; i < garrisonedUnits.size(); i++) {
				nodeUnit->append_node(doc.allocate_node(node_element, "garrisonedUnits", doc.allocate_string(toString(garrisonedUnits[i]).data())));
			}

			nodeUnit->append_node(doc.allocate_node(node_element, "attackTimer", doc.allocate_string(toString(attackTimer).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "foodGatherTimer", doc.allocate_string(toString(foodGatherTimer).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "woodGatherTimer", doc.allocate_string(toString(woodGatherTimer).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "stoneGatherTimer", doc.allocate_string(toString(stoneGatherTimer).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "metalGatherTimer", doc.allocate_string(toString(metalGatherTimer).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "deathTimer", doc.allocate_string(toString(deathTimer).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "standByTimer", doc.allocate_string(toString(standByTimer).data())));
			nodeUnit->append_node(doc.allocate_node(node_element, "autoHealTimer", doc.allocate_string(toString(autoHealTimer).data())));

			nodeUnit->append_node(doc.allocate_node(node_element, "has"));
			nodeUnit->first_node("has")->append_node(doc.allocate_node(node_element, "food", doc.allocate_string(toString(has.food).data())));
			nodeUnit->first_node("has")->append_node(doc.allocate_node(node_element, "wood", doc.allocate_string(toString(has.wood).data())));
			nodeUnit->first_node("has")->append_node(doc.allocate_node(node_element, "stone", doc.allocate_string(toString(has.stone).data())));
			nodeUnit->first_node("has")->append_node(doc.allocate_node(node_element, "metal", doc.allocate_string(toString(has.metal).data())));

			for (int i = 0; i < getTypeInfo().specialPower.size(); i++) {
				nodeUnit->append_node(doc.allocate_node(node_element, "rechargedSpecialAbility", doc.allocate_string(toString(rechargedSpecialAbility[i]).data())));
			}
			for (int i = 0; i < getTypeInfo().transformation.size(); i++) {
				nodeUnit->append_node(doc.allocate_node(node_element, "rechargedTransformation", doc.allocate_string(toString(rechargedTransformation[i]).data())));
			}
			if (getTypeInfo().transformation.size() && getTypeInfo().hasState(STATE_TRANSFORM_OUT)) {
				for (int i = 0; i < unitTypeInfo[getTypeInfo().transformation[0].unitType].specialPower.size(); i++) {
					nodeUnit->append_node(doc.allocate_node(node_element, "swapRechargedSpecialAbility", doc.allocate_string(toString(swapRechargedSpecialAbility[i]).data())));
				}
				for (int i = 0; i < unitTypeInfo[getTypeInfo().transformation[0].unitType].transformation.size(); i++) {
					nodeUnit->append_node(doc.allocate_node(node_element, "swapRechargedTransformation", doc.allocate_string(toString(swapRechargedTransformation[i]).data())));
				}
			}
		}
		return nodeUnit;
	}
	void Unit::loadFromXMLNode(xml_node<char> *node, UnitTypeInfo* unitTypeInfo, Game* game) {
		if (node->first_attribute("unitID")) {
			unitID = toInt(node->first_attribute("unitID")->value());
		}
		if (node->first_attribute("idForCampaign")) {
			idForCampaign = node->first_attribute("idForCampaign")->value();
		}
		if (node->first_node("type")) {
			type = toInt(node->first_node("type")->value());
		}
		if (node->first_node("team")) {
			team = toInt(node->first_node("team")->value());
		}
		if (node->first_node("x")) {
			x = toFloat(node->first_node("x")->value());
		}
		if (node->first_node("y")) {
			y = toFloat(node->first_node("y")->value());
		}
		if (node->first_node("angle")) {
			angle = toFloat(node->first_node("angle")->value());
		}
		if (node->first_node("level")) {
			level = toInt(node->first_node("level")->value());
		}

		bind(type, team, Point3D(x, y, z), angle, level, unitTypeInfo, game, unitID, false);

		if (node->first_node("z")) {
			z = toFloat(node->first_node("z")->value());
		}
		if (node->first_node("tilt")) {
			tilt = toInt(node->first_node("tilt")->value());
		}
		if (node->first_node("state")) {
			state = (UnitState)toInt(node->first_node("state")->value());
		}
		if (node->first_node("frame_number")) {
			frame_number = toInt(node->first_node("frame_number")->value());
		}
		if (node->first_node("hitPoint")) {
			hitPoint = toInt(node->first_node("hitPoint")->value());
		}
		if (node->first_node("builtPercentage")) {
			builtPercentage = toFloat(node->first_node("builtPercentage")->value());
		}
		if (node->first_node("stance")) {
			stance = (UnitStance)toInt(node->first_node("stance")->value());
		}

		for (xml_node<> *internalNode = node->first_node("path"); internalNode; internalNode = internalNode->next_sibling("path")) {
			int x = toInt(internalNode->first_node("x")->value());
			int y = toInt(internalNode->first_node("y")->value());
			path.push_back(Point2Di(x, y));
		}
		for (xml_node<> *internalNode = node->first_node("command"); internalNode; internalNode = internalNode->next_sibling("command")) {
			float dstx = toFloat(internalNode->first_node("dst")->first_node("x")->value());
			float dsty = toFloat(internalNode->first_node("dst")->first_node("y")->value());
			CommandType commandType = (CommandType)toInt(internalNode->first_node("commandType")->value());
			int targetID = toInt(internalNode->first_node("targetID")->value());
			bool aiCommand = toBool(internalNode->first_node("aiCommand")->value());
			bool attackMove = toBool(internalNode->first_node("attackMove")->value());
			int specialPowerIndex = toInt(internalNode->first_node("specialPowerIndex")->value());
			command.push_back(Command(Point2D(dstx, dsty), commandType, targetID, aiCommand, attackMove, specialPowerIndex));
			command.back().completed = toBool(internalNode->first_node("completed")->value());
		}

		if (node->first_node("garrisoned")) {
			garrisoned = toBool(node->first_node("garrisoned")->value());
		}
		if (node->first_node("garrisonedBuildingIndex")) {
			garrisonedBuildingIndex = toInt(node->first_node("garrisonedBuildingIndex")->value());
		}
		for (xml_node<> *internalNode = node->first_node("garrisonedUnits"); internalNode; internalNode = internalNode->next_sibling("garrisonedUnits")) {
			garrisonedUnits.push_back(toInt(internalNode->value()));
		}

		if (node->first_node("attackTimer")) {
			attackTimer = toInt(node->first_node("attackTimer")->value());
		}
		if (node->first_node("foodGatherTimer")) {
			foodGatherTimer = toInt(node->first_node("foodGatherTimer")->value());
		}
		if (node->first_node("woodGatherTimer")) {
			woodGatherTimer = toInt(node->first_node("woodGatherTimer")->value());
		}
		if (node->first_node("stoneGatherTimer")) {
			stoneGatherTimer = toInt(node->first_node("stoneGatherTimer")->value());
		}
		if (node->first_node("metalGatherTimer")) {
			metalGatherTimer = toInt(node->first_node("metalGatherTimer")->value());
		}
		if (node->first_node("deathTimer")) {
			deathTimer = toInt(node->first_node("deathTimer")->value());
		}
		if (node->first_node("standByTimer")) {
			standByTimer = toInt(node->first_node("standByTimer")->value());
		}
		if (node->first_node("autoHealTimer")) {
			autoHealTimer = toInt(node->first_node("autoHealTimer")->value());
		}

		if (node->first_node("has")) {
			if (node->first_node("has")->first_node("food")) {
				has.food = toInt(node->first_node("has")->first_node("food")->value());
			}
			if (node->first_node("has")->first_node("wood")) {
				has.wood = toInt(node->first_node("has")->first_node("wood")->value());
			}
			if (node->first_node("has")->first_node("stone")) {
				has.stone = toInt(node->first_node("has")->first_node("stone")->value());
			}
			if (node->first_node("has")->first_node("metal")) {
				has.metal = toInt(node->first_node("has")->first_node("metal")->value());
			}
		}

		int i = 0;
		for (xml_node<> *internalNode = node->first_node("rechargedSpecialAbility"); internalNode; internalNode = internalNode->next_sibling("rechargedSpecialAbility")) {
			rechargedSpecialAbility[i++] = toInt(internalNode->value());
		}
		i = 0;
		for (xml_node<> *internalNode = node->first_node("rechargedTransformation"); internalNode; internalNode = internalNode->next_sibling("rechargedTransformation")) {
			rechargedTransformation[i++] = toInt(internalNode->value());
		}
		i = 0;
		for (xml_node<> *internalNode = node->first_node("swapRechargedSpecialAbility"); internalNode; internalNode = internalNode->next_sibling("swapRechargedSpecialAbility")) {
			swapRechargedSpecialAbility[i++] = toInt(internalNode->value());
		}
		i = 0;
		for (xml_node<> *internalNode = node->first_node("swapRechargedTransformation"); internalNode; internalNode = internalNode->next_sibling("swapRechargedTransformation")) {
			rechargedSpecialAbility[i++] = toInt(internalNode->value());
		}
	}
};
