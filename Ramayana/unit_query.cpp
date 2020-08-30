#include "stdafx.h"

#include "common.h"
#include "unit.h"
#include "game.h"

namespace ramayana {

	UnitID Unit::getID() const {
		return unitID;
	}

	// percentage amount of building/training unit built
	float Unit::getBuildPercentage(int selectedObject) {
		Synchronizer sync(mutex);

		if (!command.empty() && command[0].commandType == COMMAND_BUILD && game->unit[command[0].targetID].type == selectedObject)
			return game->unit[command[0].targetID].builtPercentage;
		return 0;
	}

	// unit to be built/trained waiting in command queue
	int Unit::getBuildCount(int selectedObject) {
		Synchronizer sync(mutex);

		int c = 0;
		for (vector<Command>::const_iterator i = command.begin(); i != command.end(); i++)
		if (i->commandType == COMMAND_BUILD && game->unit[i->targetID].type == selectedObject)
			c++;
		return c;
	}

	// percentage amount of special ability recharged
	int Unit::specialAbilityRecharged(int index) {
		return 100.0*(float)rechargedSpecialAbility[index] / (float)unitTypeInfo[type].specialPower[index].rechargeTime;
	}

	// special ability recharged recharge time(ms) remaining
	float Unit::specialAbilityRechargeTimeRemaining(int index) {
		float timeUnit = 1000.0 / float(UPDATE_THREAD_FRAME_RATE);
		return timeUnit * float(unitTypeInfo[type].specialPower[index].rechargeTime - rechargedSpecialAbility[index]);
	}

	// percentage amount of transformation ability recharged
	int Unit::transformationRecharged(int index) {
		return 100.0*(float)rechargedTransformation[index] / (float)unitTypeInfo[type].transformation[index].rechargeTime;
	}

	// transformation ability recharged recharge time(ms) remaining
	float Unit::transformationRechargeTimeRemaining(int index) {
		float timeUnit = 1000.0 / float(UPDATE_THREAD_FRAME_RATE);
		return timeUnit * float(unitTypeInfo[type].transformation[index].rechargeTime - rechargedTransformation[index]);
	}

	// time(ms) remaining to auto-transform to base
	long Unit::getAutoTransformTimeRemaining() {
		if (getTypeInfo().autoTransformTime > 0) {
			return getTypeInfo().autoTransformTime - game->getGamePlayTime() + birthTime;
		}
		return 0;
	}

	// is current transformation is general/base transformation
	bool Unit::isBaseTransformation() const {
		return !getTypeInfo().hasState(STATE_TRANSFORM_IN) && !getTypeInfo().hasState(STATE_TRANSFORM_OUT);
	}

	bool Unit::isOnWater() {
		return game->getGroundHeight(x, y) <= game->getCurrentWaterLevel();
	}

	//position related query
	Point3D Unit::position() const {
		return Point3D(x, y, z);
	}
	Point2D Unit::position2D() const {
		return Point2D(x, y);
	}
	Point3D Unit::centre() const {
		return Point3D(x + currentFrame().getXMid()*OBJECT_SCALING, y + currentFrame().getYMid()*OBJECT_SCALING, z + currentFrame().getZMid()*OBJECT_SCALING);
	}
	Point3D Unit::top() const {
		return Point3D(x + currentFrame().getXMid()*OBJECT_SCALING, y + currentFrame().getYMid()*OBJECT_SCALING, z + currentFrame().getZMax()*OBJECT_SCALING);
	}
	Point3D Unit::bottom() const {
		return Point3D(x + currentFrame().getXMid()*OBJECT_SCALING, y + currentFrame().getYMid()*OBJECT_SCALING, z + currentFrame().getZMin()*OBJECT_SCALING);
	}
	Point3D Unit::front() const {
		return rotatePoint(angle, Point3D(x + currentFrame().getXMax()*OBJECT_SCALING, y + currentFrame().getYMid()*OBJECT_SCALING, z + currentFrame().getZMid()*OBJECT_SCALING), centre());
	}
	Point3D Unit::frontLeft() const {
		return rotatePoint(angle, Point3D(x + currentFrame().getXMax()*OBJECT_SCALING, y + currentFrame().getYMax()*OBJECT_SCALING, z + currentFrame().getZMid()*OBJECT_SCALING), centre());
	}
	Point3D Unit::frontRight() const {
		return rotatePoint(angle, Point3D(x + currentFrame().getXMax()*OBJECT_SCALING, y + currentFrame().getYMin()*OBJECT_SCALING, z + currentFrame().getZMid()*OBJECT_SCALING), centre());
	}

	//objct attributes
	float Unit::getRadius() const {
		return currentFrame().getRadius()*OBJECT_SCALING;
	}
	float Unit::getRadiusAcrossXPlane() const {
		return currentFrame().getRadiusAcrossXPlane()*OBJECT_SCALING;
	}
	float Unit::getRadiusAcrossYPlane() const {
		return currentFrame().getRadiusAcrossYPlane()*OBJECT_SCALING;
	}
	float Unit::getRadiusAcrossZPlane() const {
		return currentFrame().getRadiusAcrossZPlane()*OBJECT_SCALING;
	}
	float Unit::getMinX() const {
		return currentFrame().getXMin()*OBJECT_SCALING;
	}
	float Unit::getMinY() const {
		return currentFrame().getYMin()*OBJECT_SCALING;
	}
	float Unit::getMinZ() const {
		return currentFrame().getZMin()*OBJECT_SCALING;
	}
	float Unit::getMaxX() const {
		return currentFrame().getXMax()*OBJECT_SCALING;
	}
	float Unit::getMaxY() const {
		return currentFrame().getYMax()*OBJECT_SCALING;
	}
	float Unit::getMaxZ() const {
		return currentFrame().getZMax()*OBJECT_SCALING;
	}
	float Unit::getMidX() const {
		return currentFrame().getXMid()*OBJECT_SCALING;
	}
	float Unit::getMidY() const {
		return currentFrame().getYMid()*OBJECT_SCALING;
	}
	float Unit::getMidZ() const {
		return currentFrame().getZMid()*OBJECT_SCALING;
	}

	//target attributes
	UnitID Unit::getTargetID() {
		return targetID;
	}
	Unit& Unit::getTarget() {
		UnitID t = 0;
		if (command.size()>0)
			t = command[0].targetID;
		return game->unit[0];
	}
	Point2D Unit::getTargetPosition() {
		return targetPosition;
	}

	bool Unit::isVisibleToTeam(TeamID t) {
		if (game->diplomacy(team, t) == DIPLOMACY_ALLY) {
			return true;
		} else {
			int r = roundInt(y), c = roundInt(x);
			if (Point2Di(c, r).in(0, 0, game->getWidth() - 1, game->getHeight() - 1) && game->teams[t].visible[r][c]) {
				if (blended <= 0) {
					return true;
				} else {
					for (UnitID firendUnit = 0; firendUnit<game->getNumberOfUnits(); firendUnit++)
					if (game->diplomacy(t, game->unit[firendUnit].team) == DIPLOMACY_ALLY && isVisibleToUnit(firendUnit))
						return true;
					return false;
				}
			} else {
				return false;
			}
		}
	}
	bool Unit::isExploredToTeam(TeamID t) {
		int r = roundInt(y), c = roundInt(x);
		if (Point2Di(c, r).in(0, 0, game->getWidth() - 1, game->getHeight() - 1))
			return game->teams[t].explored[r][c] && team == 0;
		else
			return false;
	}
	bool Unit::isVisibleToUnit(UnitID u) {
		float sqrLOS = game->unit[u].getLOS()*(1.0 - blended);
		sqrLOS *= sqrLOS;
		return squareDist(position(), game->unit[u].position()) <= sqrLOS;
	}

	int Unit::nearestUnit(UnitCategory t) {
		float dmin = FLT_MAX;
		int imin = INVALID_UNIT_INDEX;
		for (int i = 0; i<game->getNumberOfUnits(); i++) {
			if (game->unit[i].state != STATE_DEAD && unitTypeInfo[game->unit[i].type].category == t && game->unit[i].capturedByUnit == INVALID_UNIT_INDEX) {
				float d = manhattanDist(Point2Di(x, y), Point2Di(game->unit[i].x, game->unit[i].y));
				if (d<dmin) {
					dmin = d;
					imin = i;
				}
			}
		}
		return imin;
	}

	UnitCategory Unit::getCategory() const {
		return unitTypeInfo[type].category;
	}
	UnitType Unit::getType() const {
		return type;
	}
	const UnitTypeInfo& Unit::getTypeInfo() const {
		return unitTypeInfo[type];
	}

	bool Unit::hasVacancy() {
		Synchronizer sync(mutex);
		bool result = garrisonedUnits.size() < unitTypeInfo[type].garrisonedUnitPosition.size();
		return result;
	}
	bool Unit::isVacantDefensiveUnit() {
		Synchronizer sync(mutex);
		bool result = getTypeInfo().isConstruction();
		result &= hasVacancy();
		result &= !getTypeInfo().canGather.food && !getTypeInfo().canGather.wood;
		result &= !getTypeInfo().canGather.stone && !getTypeInfo().canGather.metal;
		return result;
	}

	//state related query
	bool Unit::isIdle() {
		return command.empty() && !garrisoned && garrisonedUnits.empty();
	}
	bool Unit::isMoving() {
		return state == STATE_MOVING;
	}
	bool Unit::isFleeing(){
		return fleeing;
	}
	bool Unit::isAttacking() {
		return state == STATE_ATTACKING || state == STATE_SPECIAL_1 || state == STATE_SPECIAL_2 || state == STATE_SPECIAL_3;
	}
	bool Unit::isDying() {
		return state == STATE_DEAD && deathTimer>0;
	}
	bool Unit::isGarrisoned() {
		return garrisoned;
	}
	bool Unit::isOnAttackMove() {
		return onAttackMove;
	}
	bool Unit::isFoodGatherer() {
		return state == STATE_GATHER_FOOD
			|| (!command.empty() && (command[0].commandType == COMMAND_GARRISON && unitTypeInfo[game->unit[command[0].targetID].type].canGather.food));
	}
	bool Unit::isWoodGatherer() {
		return state == STATE_GATHER_WOOD || state == STATE_CUTTING_TREE
			|| (!command.empty()
			&& (command[0].commandType == COMMAND_CUT_TREE
			|| (command[0].commandType == COMMAND_GARRISON && unitTypeInfo[game->unit[command[0].targetID].type].canGather.wood)));
	}
	bool Unit::isStoneGatherer() {
		return state == STATE_GATHER_STONE
			|| (!command.empty() && command[0].commandType == COMMAND_GATHER_STONE);
	}
	bool Unit::isMetalGatherer() {
		return state == STATE_GATHER_METAL
			|| (!command.empty() && (command[0].commandType == COMMAND_GARRISON && unitTypeInfo[game->unit[command[0].targetID].type].canGather.metal));
	}
	bool Unit::isBuilder() {
		return state == STATE_BUILD || (!command.empty() && command[0].commandType == COMMAND_BUILD);
	}
	bool Unit::exists()  {
		return state != STATE_DEAD || deathTimer>0;
	}
	bool Unit::isRenderable() {
		if (game != NULL && game->isEditable()) {
			return exists();
		} else {
			return renderAlpha > 0.0;
		}
	}
	bool Unit::isSelectable() {
		return isRenderable() && isAlive();
	}
	bool Unit::isAlive()  {
		return state != STATE_DEAD;
	}
	bool Unit::isActive() {
		return state != STATE_DEAD && state != STATE_BUILDING && state != STATE_STANDBY;
	}
	bool Unit::isIncomplete() {
		return state == STATE_BUILDING;
	}
	bool Unit::isDamaged() {
		return hitPoint<getMaxHitPoint();
	}

	//unit-type attribute related query (dependant on level, garrisoned)
	float Unit::getRange() const {
		float bonus = 0;
		if (garrisoned) bonus += 2;
		return getTypeInfo().range + bonus;
	}
	int Unit::getLOS() const {
		float bonus = 0;
		if (garrisoned) bonus += 3;
		return getTypeInfo().getLOS(level) + bonus;
	}
	float Unit::getArmour() const {
		float bonus = 0;
		if (garrisoned) bonus += game->unit[garrisonedBuildingIndex].getTypeInfo().getArmour(game->unit[garrisonedBuildingIndex].level) / 5;
		return getTypeInfo().getArmour(level) + bonus;
	}
	int Unit::getHealAmount() const {
		float bonus = 0;
		if (garrisoned)
			bonus += getTypeInfo().healAmount * 2 + 5;
		return getTypeInfo().healAmount + bonus;
	}
	int Unit::getHealDelay() const {
		if (garrisoned && getTypeInfo().healAmount == 0 && getTypeInfo().healDelay == 0)
			return 10;
		else
			return getTypeInfo().healDelay;
	}
	int Unit::getAttack() const {
		float bonus = 0;
		return getTypeInfo().getAttack(level) + bonus;
	}
	int Unit::getSiegeAttack() const {
		float bonus = 0;
		return getTypeInfo().getSiegeAttack(level) + bonus;
	}
	int Unit::getMaxHitPoint() const {
		return getTypeInfo().getMaxHitPoint(level);
	}

	vector<Point2Di> Unit::getOccupiedPoints() {
		Synchronizer sync(mutex);
		vector<Point2Di> currentlyOccupiedPointsCopy = currentlyOccupiedPoints;
		return currentlyOccupiedPointsCopy;
	}

	int Unit::commandListLength() {
		Synchronizer sync(mutex);
		int length = command.size();
		return length;
	}
	Path Unit::getPath() {
		Synchronizer sync(mutex);
		Path pathCopy = path;
		return pathCopy;
	}
	vector<Unit::Command> Unit::getCommandList() {
		Synchronizer sync(mutex);
		vector<Unit::Command> commandCopy = command;
		return commandCopy;
	}
};
