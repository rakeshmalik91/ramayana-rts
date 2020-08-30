#include "stdafx.h"

#include "enums.h"
#include "unit.h"

namespace ramayana {
	
	ProjectileWeapon::ProjectileWeapon() : Projectile(), hit(false), particleEngine(NULL), addUnit_number(0){}
	ProjectileWeapon::ProjectileWeapon(int team, Point3D ipos, Point3D dst, float hAngle, float v, float weight, bool overHorizon, int attack, int siegeAttack, int areaDamageRadius, WeaponType weaponType, UnitID addUnit_number, int addUnit_id) :
		Projectile(ipos, dst, hAngle, v, weight, overHorizon), team(team), attack(attack), siegeAttack(siegeAttack), areaDamageRadius(areaDamageRadius), weaponType(weaponType), hit(false), time(0), particleEngine(NULL), addUnit_id(addUnit_id), addUnit_number(addUnit_number) {
	}
	ProjectileWeapon::ProjectileWeapon(int team, Point3D pos, float vAngle, float hAngle, float v, float weight, int attack, int siegeAttack, int areaDamageRadius, WeaponType weaponType, UnitID addUnit_number, int addUnit_id) :
		Projectile(pos, hAngle, vAngle, v, weight), team(team), attack(attack), siegeAttack(siegeAttack), areaDamageRadius(areaDamageRadius), weaponType(weaponType), hit(false), time(0), particleEngine(NULL), addUnit_id(addUnit_id), addUnit_number(addUnit_number) {
	}
	ProjectileWeapon::~ProjectileWeapon() {
		if(particleEngine!=NULL)
			delete particleEngine;
	}
};