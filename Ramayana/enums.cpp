#include "stdafx.h"

#include "enums.h"

namespace ramayana {
	WeaponType toWeaponType(string weaponType) {
		weaponType = trim(weaponType);
		if (weaponType == "none")					return WEAPON_NONE;
		else if (weaponType == "sword")			return WEAPON_SWORD;
		else if (weaponType == "mace")				return WEAPON_MACE;
		else if (weaponType == "stone")			return WEAPON_STONE;
		else if (weaponType == "spike")			return WEAPON_SPIKE;
		else if (weaponType == "arrow")			return WEAPON_ARROW;
		else if (weaponType == "fire_arrow")		return WEAPON_FIRE_ARROW;
		else if (weaponType == "wind_arrow")		return WEAPON_WIND_ARROW;
		else if (weaponType == "water_arrow")		return WEAPON_WATER_ARROW;
		else if (weaponType == "poison_arrow")		return WEAPON_POISON_ARROW;
		else if (weaponType == "dark_arrow")		return WEAPON_DARK_ARROW;
		else if (weaponType == "replicate_arrow")	return WEAPON_REPLICATE_ARROW;
		else if (weaponType == "special_arrow_1")	return WEAPON_SPECIAL_ARROW_1;
		else if (weaponType == "special_arrow_2")	return WEAPON_SPECIAL_ARROW_2;
		else return WEAPON_NONE;
	}

	UnitCategory toUnitCategory(string unitType) {
		unitType = trim(unitType);
		if (unitType == "decoration")				return UNIT_DECORATION;
		else if (unitType == "water_decor")			return UNIT_WATER_DECORATION;
		else if (unitType == "tree")				return UNIT_TREE;
		else if (unitType == "stone")				return UNIT_STONE;
		else if (unitType == "infantry")			return UNIT_INFANTRY;
		else if (unitType == "giant")				return UNIT_GIANT;
		else if (unitType == "beast")				return UNIT_BEAST;
		else if (unitType == "amphibian")			return UNIT_AMPHIBIAN;
		else if (unitType == "chariot")				return UNIT_CHARIOT;
		else if (unitType == "building")			return UNIT_BUILDING;
		else if (unitType == "tree_house")			return UNIT_TREE_HOUSE;
		else if (unitType == "mine")				return UNIT_MINE;
		else if (unitType == "bird")				return UNIT_BIRD;
		else if (unitType == "air")					return UNIT_AIR;
		else if (unitType == "insect")				return UNIT_INSECT;
		else if (unitType == "water")				return UNIT_WATER;
		else if (unitType == "water_construction")	return UNIT_WATER_CONSTRUCTION;
		else return UNIT_DECORATION;
	}

	UnitState toUnitState(string unitState) {
		unitState = trim(unitState);
		if (unitState == "general")			return STATE_GENERAL;
		else if (unitState == "standby")		return STATE_STANDBY;
		else if (unitState == "moving")		return STATE_MOVING;
		else if (unitState == "jump")			return STATE_JUMP;
		else if (unitState == "attack")		return STATE_ATTACKING;
		else if (unitState == "cut_tree")		return STATE_CUTTING_TREE;
		else if (unitState == "gather_food")	return STATE_GATHER_FOOD;
		else if (unitState == "gather_wood")	return STATE_GATHER_WOOD;
		else if (unitState == "gather_stone")	return STATE_GATHER_STONE;
		else if (unitState == "gather_metal")	return STATE_GATHER_METAL;
		else if (unitState == "dead")			return STATE_DEAD;
		else if (unitState == "damaged")		return STATE_DAMAGED;
		else if (unitState == "build")			return STATE_BUILD;
		else if (unitState == "building")		return STATE_BUILDING;
		else if (unitState == "transform_in")	return STATE_TRANSFORM_IN;
		else if (unitState == "transform_out")	return STATE_TRANSFORM_OUT;
		else if (unitState == "special1")		return STATE_SPECIAL_1;
		else if (unitState == "special2")		return STATE_SPECIAL_2;
		else if (unitState == "special3")		return STATE_SPECIAL_3;
		else return STATE_GENERAL;
	}

	bool isResourceUnit(UnitCategory ut) {
		return ut == UNIT_TREE
			|| ut == UNIT_STONE;
	}
	bool isConstruction(UnitCategory ut) {
		return ut == UNIT_BUILDING
			|| ut == UNIT_TREE_HOUSE
			|| ut == UNIT_MINE
			|| ut == UNIT_WATER_CONSTRUCTION;
	}
	bool isDecoration(UnitCategory ut) {
		return ut == UNIT_DECORATION
			|| ut == UNIT_WATER_DECORATION;
	}
	bool isNaturalUnit(UnitCategory ut) {
		return ut == UNIT_TREE
			|| ut == UNIT_STONE
			|| isDecoration(ut);
	}
	bool isGroundUnit(UnitCategory ut) {
		return ut == UNIT_INFANTRY
			|| ut == UNIT_BEAST
			|| ut == UNIT_GIANT
			|| ut == UNIT_CHARIOT
			|| ut == UNIT_AMPHIBIAN
			|| ut == UNIT_TREE
			|| ut == UNIT_STONE
			|| ut == UNIT_DECORATION
			|| isConstruction(ut);
	}
	bool isWaterUnit(UnitCategory ut) {
		return ut == UNIT_WATER
			|| ut == UNIT_AMPHIBIAN
			|| ut == UNIT_WATER_DECORATION
			|| ut == UNIT_WATER_CONSTRUCTION;
	}
	bool isAirUnit(UnitCategory ut) {
		return ut == UNIT_BIRD
			|| ut == UNIT_AIR
			|| ut == UNIT_INSECT;
	}
	bool isJumpableUnit(UnitCategory ut) {
		return ut == UNIT_TREE
			|| ut == UNIT_TREE_HOUSE;
	}

	bool isRangedWeapon(WeaponType w) {
		return (w == WEAPON_SPIKE
			|| w == WEAPON_ARROW
			|| w == WEAPON_STONE
			|| w == WEAPON_DARK_ARROW
			|| w == WEAPON_SPECIAL_ARROW_2
			|| w == WEAPON_FIRE_ARROW
			|| w == WEAPON_WATER_ARROW
			|| w == WEAPON_REPLICATE_ARROW
			|| w == WEAPON_WIND_ARROW
			|| w == WEAPON_POISON_ARROW
			|| w == WEAPON_SPECIAL_ARROW_1);
	}
	bool isMeleeWeapon(WeaponType w) {
		return !isRangedWeapon(w);
	}

	TargetType toTargetType(string targetType) {
		targetType = trim(targetType);
		if (targetType == "any")				return TARGET_ANY;
		else if (targetType == "unit")			return TARGET_UNIT;
		else if (targetType == "ground")		return TARGET_GROUND_UNIT;
		else if (targetType == "air")			return TARGET_AIR_UNIT;
		else if (targetType == "water")		return TARGET_WATER_UNIT;
		else if (targetType == "giant")		return TARGET_GIANT_UNIT;
		else								return TARGET_ANY;
	}

	Diplomacy toDiplomacyType(string diplomacyType) {
		diplomacyType = trim(diplomacyType);
		if (diplomacyType == "enemy")			return DIPLOMACY_ENEMY;
		else if (diplomacyType == "ally")		return DIPLOMACY_ALLY;
		else								return DIPLOMACY_NEUTRAL;
	}
	string diplomacyTypeToString(Diplomacy d) {
		string str = "neutral";
		switch (d) {
		case DIPLOMACY_ALLY: str = "ally"; break;
		case DIPLOMACY_ENEMY: str = "enemy"; break;
		}
		return str;
	}

	WeatherType toWeatherType(string s) {
		if (s == "rainy") return WEATHER_RAINY;
		else if (s == "windy") return WEATHER_WINDY;
		else if (s == "stormy") return WEATHER_STORMY;
		else return WEATHER_CLEAR;
	}

	bool isLoadingGameState(GameState state) {
		return state == GAMESTATE_LOADING_GAME_PHASE1
			|| state == GAMESTATE_LOADING_GAME_PHASE2
			|| state == GAMESTATE_LOADING_MAP_PHASE1
			|| state == GAMESTATE_LOADING_MAP_PHASE2;
	}
	bool isSettingsMenuGameState(GameState state) {
		return state == GAMESTATE_SETTINGSMENU_DISPLAY
			|| state == GAMESTATE_SETTINGSMENU_AUDIO
			|| state == GAMESTATE_SETTINGSMENU_GAME;
	}

}