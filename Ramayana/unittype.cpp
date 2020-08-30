#include "stdafx.h"

#include "common.h"
#include "unit.h"
#include "game.h"
#include "interface.h"

namespace ramayana {

	void UnitTypeInfo::saveUnitDump() {
		ofstream out(unitPath.data(), ios::binary);
		if (out.fail()) {
			throw FileWriteException(unitPath.data());
		}

		//number fo states
		int nStates = 0;
		for (int state = 0; state < MAX_UNIT_STATE; state++) {
			if (obj[state] != NULL && sameas[state].unit == NULL) {
				nStates++;
			}
		}
		out.write((const char*)&nStates, sizeof(nStates));
		//models
		for (int state = 0; state < MAX_UNIT_STATE; state++) {
			if (obj[state] != NULL && sameas[state].unit == NULL) {
				//state
				out.write((const char*)&state, sizeof(state));
				//wavefront sequence
				obj[state]->write(out);
			}
		}

		out.close();
	}
	void UnitTypeInfo::loadUnitDump(bool loadAnimation) {
		ifstream in(unitPath.data(), ios::binary);
		if (in.fail()) {
			throw FileReadException(unitPath.data());
		}

		//number fo states
		int nStates = 0;
		in.read((char*)&nStates, sizeof(nStates));
		//models
		for (int i = 0; i < (loadAnimation ? nStates : 1); i++) {
			//state
			UnitState state;
			in.read((char*)&state, sizeof(state));
			//wavefront sequence
			obj[state] = new WaveFrontObjSequence();
			obj[state]->read(in, getFilePath(unitPath), loadAnimation);
		}
		
		in.close();
	}

	UnitTypeInfo::UnitTypeInfo() :
		loaded(false), nStates(0), particleEngine(NULL),
		isHeroic(false), hideInEditor(false), isBridge(false),
		canRepair(false), isWorker(false), garrisonable(false), canJump(false), jumpSpeed(0), flyLevel(0),
		canGather(false, false, false, false), has(0, 0, 0, 0), cost(0, 0, 0, 0), population(0), maxPopulationIncrease(0),
		hitPoint(0), attack(0), siegeAttack(0), attackDelay(0), accuracy(1.0), armour(1.0), trampleDamage(0), areaDamageRadius(0), range(0), weaponType(WEAPON_NONE),
		speed(0), rotAngle(0), los(0), fireResistance(0.0), tornedoResistant(false),
		healAmount(0), healDelay(0), camouflage(1.0), stealth(0.0), standByTime(0), canParalyze(false),
		autoTransformTime(0), transformOnDeath(false), canSetFire(false), transformOnBurn(false), transformOnWater(false) {
		for (int state = 0; state<MAX_UNIT_STATE; state++) {
			obj[state] = NULL;
			path[state] = "";
			sameas[state].unit = NULL;
		}
		for (int i = 0; i < MAX_OBJECT_TYPE; i++) {
			canBuild[i] = false;
		}
	}
	void UnitTypeInfo::setPath(UnitState state, string path) {
		this->path[state] = path;
	}
	bool UnitTypeInfo::load(bool loadAnimation) {
		if (loaded) {
			return false;
		}
		bool hasInfo = false;
		if (fileExists(this->unitPath)) {
			if (!unitPath.empty()) {
				loadUnitDump(loadAnimation);
				hasInfo = true;
			}
		} else {
			for (int state = 0; state<MAX_UNIT_STATE; state++) {
				if (this->path[state] != "") {
					obj[state] = new WaveFrontObjSequence();
					obj[state]->load(this->path[state], false, loadAnimation);
					hasInfo = true;
				}
			}
			if (!unitPath.empty()) {
				saveUnitDump();
			}
		}

		for (int state = 0; state<(loadAnimation ? MAX_UNIT_STATE : 1); state++) {
			if (sameas[state].unit != NULL) {
				if (sameas[state].unit != this) {
					sameas[state].unit->load(loadAnimation);
				}
				obj[state] = sameas[state].unit->obj[sameas[state].state];
				hasInfo = true;
			}
		}

		if (hasInfo) {
			loaded = true;
		} else {
			//throw NoUnitDataException("Missing Unit Data");
		}
		return true;
	}
	void UnitTypeInfo::sameAs(UnitState state, UnitTypeInfo *targetUnit, UnitState targetState) {
		sameas[state].unit = targetUnit;
		sameas[state].state = targetState;
	}
	void UnitTypeInfo::unload() {
		if (!loaded) {
			return;
		}
		getLogger().print("unit " + name + " unloading...");
		for (int state = 0; state<MAX_UNIT_STATE; state++) {
			if (!this->path[state].empty() && obj[state] != NULL) {
				getLogger().print("Deleting state " + toString(state));
				delete obj[state];
			}
			obj[state] = NULL;
		}
		getLogger().print("unit " + name + " unloaded.");
		loaded = false;
	}
	WaveFrontObjSequence& UnitTypeInfo::getObject(UnitState state, int hp) {
		if (obj[state] == NULL) {
			if (hp<hitPoint / 2 && obj[STATE_DAMAGED] != NULL) {
				return *obj[STATE_DAMAGED];
			} else {
				return *obj[STATE_GENERAL];
			}
		} else {
			if (state == STATE_GENERAL && hp<hitPoint / 2 && obj[STATE_DAMAGED] != NULL) {
				return *obj[STATE_DAMAGED];
			} else {
				return *obj[state];
			}
		}
	}
	WaveFrontObjSequence& UnitTypeInfo::getObject(UnitState state) {
		if (obj[state] == NULL) {
			return *obj[STATE_GENERAL];
		} else {
			return *obj[state];
		}
	}
	UnitTypeInfo::~UnitTypeInfo() {
		if (!loaded) {
			return;
		}
		for (int state = 0; state<MAX_UNIT_STATE; state++) {
			if (this->path[state] != "" && obj[state] != NULL) {
				delete obj[state];
			}
			obj[state] = NULL;
		}
		for (int i = 0; i<transformation.size(); i++) {
			if(transformation[i].icon != NULL) {
				delete transformation[i].icon;
			}
		}
	}
	void UnitTypeInfo::compile() {
		if (!loaded) {
			return;
		}
		for (int state = 0; state<MAX_UNIT_STATE; state++) {
			if (obj[state] != NULL) {
				obj[state]->compile();
			}
		}
		for (int i = 0; i < transformation.size(); i++) {
			if (transformation[i].icon != NULL) {
				transformation[i].icon->generate();
			}
		}
		for (int i = 0; i < specialPower.size(); i++) {
			if (specialPower[i].icon != NULL) {
				specialPower[i].icon->generate();
			}
		}
		if (particleEngine != NULL) {
			particleTexture->generate();
		}
		icon.generate();
		iconSmall.generate();
	}
	bool UnitTypeInfo::hasState(UnitState state) const {
		return obj[state] != NULL;
	}
	vector<Point2Di> UnitTypeInfo::getOccupiedPoints(Point2Di pos, int angle) const {
		vector<Point2Di> points;
		for (int p = 0; p<occupiedPoint.size(); p++) {
			Point2Di pt = rotatePoint(angle, pos + occupiedPoint[p], pos);
			points.push_back(pt);
		}
		return points;
	}
	int UnitTypeInfo::getMaxHitPoint(int level) const {
		return hitPoint + (level - 1)*hitPoint / 10;
	}
	int UnitTypeInfo::getSiegeAttack(int level) const {
		return siegeAttack + (level - 1)*siegeAttack / 10;
	}
	int UnitTypeInfo::getAttack(int level) const {
		return attack + (level - 1)*attack / 10;
	}
	float UnitTypeInfo::getArmour(int level) const {
		return armour + (level - 1)*armour / 10;
	}
	int UnitTypeInfo::getLOS(int level) const {
		return los + (level - 1) / 2;
	}
	bool UnitTypeInfo::isMilitaryUnit() const {
		return !(isWorker || isConstruction());
	}
	bool UnitTypeInfo::isConstruction() const {
		return ramayana::isConstruction(category);
	}
}
