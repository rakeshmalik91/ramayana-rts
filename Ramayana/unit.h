#ifndef __RAMAYANA_UNIT_H
#define __RAMAYANA_UNIT_H

#include "common.h"

#define INVALID_UNIT_INDEX		-1

namespace ramayana {
	
	class Game;

	class NoUnitDataException : public Exception {
	public:
		NoUnitDataException() {}
		NoUnitDataException(string msg) : Exception(msg) {}
	};

	class UnitTypeInfo {
		static const int MAX_UNIT_STATE							=20;
		string path[MAX_UNIT_STATE];
		WaveFrontObjSequence *obj[MAX_UNIT_STATE];
		struct SameAs {
			UnitTypeInfo *unit;
			UnitState state;
		} sameas[MAX_UNIT_STATE];
	public:
		vector<string> audio[MAX_UNIT_STATE];
		vector<string> selectSound, commandSound;
		string unitPath;
		string name;
		bool loaded;
		int nStates;
		UnitCategory category;
		bool isHeroic;
		float los;
		float speed, rotAngle, jumpSpeed, flyLevel;
		int hitPoint, attack, siegeAttack, attackDelay, buildTime, trampleDamage;
		float accuracy, armour, fireResistance, range;
		bool tornedoResistant, canParalyze, drownInCurrent;
		int healDelay, healAmount;
		int standByTime, standByDamage;
		float camouflage, stealth;																						//Multiplied with blending
		WeaponType weaponType;
		int areaDamageRadius;
		vector<Point2Di> occupiedPoint;
		bool canBuild[MAX_OBJECT_TYPE];
		bool canRepair, isWorker, garrisonable;
		Resource<bool> canGather;
		Resource<int> cost, has;
		int population, maxPopulationIncrease;
		bool canJump;
		bool canSetFire;
		vector<Point4D> garrisonedUnitPosition;
		struct SpecialPower {
			WeaponType weaponType;
			string name;
			Texture2D *icon;
			int rechargeTime;
			UnitState state;
			int attack, siegeAttack, areaDamageRadius, heal;
			float range;
			int hitFrameNumber;
			TargetType targetType;
			bool friendly;
			int neededLevel;
			int nTornedo;
			int addUnit_number;
			UnitID addUnit_id;
			SpecialPower() : weaponType(WEAPON_NONE), icon(NULL), rechargeTime(0), state(STATE_ATTACKING), 
				attack(0), siegeAttack(0), areaDamageRadius(0), range(0.0), heal(0), hitFrameNumber(0), targetType(TARGET_ANY),
				nTornedo(0), addUnit_id(0), addUnit_number(0),
				friendly(false), neededLevel(1) {}
		};
		vector<SpecialPower> specialPower;
		struct TransformationAbility {
			int unitType;
			string name;
			Texture2D *icon;
			int rechargeTime;
			int neededLevel;
			bool hide;
			TransformationAbility() : unitType(-1), icon(NULL), rechargeTime(0), neededLevel(1), hide(false) {}
		};
		vector<TransformationAbility> transformation;
		unsigned long autoTransformTime;
		bool transformOnDeath, transformOnBurn, transformOnWater;
		Texture2D *particleTexture;
		ParticleEngine *particleEngine;
		Texture2D icon, iconSmall;
		vector<UnitType> siblings;
		bool hideInEditor;
		bool isBridge;
	private:
		void saveUnitDump();
		void loadUnitDump(bool loadAnimation);
	public:
		UnitTypeInfo();
		void setPath(UnitState, string);
		void sameAs(UnitState, UnitTypeInfo*, UnitState);
		bool load(bool);
		void unload();
		void compile();
		WaveFrontObjSequence& getObject(UnitState);
		WaveFrontObjSequence& getObject(UnitState, int);
		bool hasState(UnitState) const;
		~UnitTypeInfo();
		vector<Point2Di> getOccupiedPoints(Point2Di, int) const;
		int getMaxHitPoint(int) const;
		int getAttack(int) const;
		int getSiegeAttack(int) const;
		float getArmour(int) const;
		int getLOS(int) const;
		bool isMilitaryUnit() const;
		bool isConstruction() const;
	};
	
	struct ProjectileWeapon : public Projectile {
		int team;
		int attack, siegeAttack, areaDamageRadius;
		bool hit;
		int time;
		int addUnit_number;
		UnitID addUnit_id;
		WeaponType weaponType;
		ParticleEngine *particleEngine;
		ProjectileWeapon();
		ProjectileWeapon(int team, Point3D ipos, Point3D dst, float hAngle, float v, float weight, bool overHorizon, int attack, int siegeAttack, int areaDamageRadius, WeaponType weaponType, UnitID addUnit_number=0, int addUnit_id=0);
		ProjectileWeapon(int team, Point3D pos, float vAngle, float hAngle, float v, float weight, int attack, int siegeAttack, int areaDamageRadius, WeaponType weaponType, UnitID addUnit_number = 0, int addUnit_id = 0);
		~ProjectileWeapon();
	};

	//Classs for each unit instance
	class Unit {
	public:
		static const int UNDERWATERUNIT_LEVEL = -3;
		static const int MAX_SPECIAL_ABILITY = 10;
		static const int  GATHER_RATE = 20;
		static const int  MAX_SHORT_RANGE = 3;
		static const int  MAX_STORED_WOOD = 5000;
		static const int  DEAD_UNIT_VANISH_TIMER = 800;
		static const int MAX_FIRE_DAMAGE = 10;
	public:
		struct Command {
			Point2D dst;
			CommandType commandType;
			UnitID targetID;
			bool aiCommand, completed, attackMove;
			int specialPowerIndex;
			Command(Point2D dst, CommandType commandType, int targetID, bool attackMove=false, bool aiCommand=false, int specialPowerIndex=false) : 
				dst(dst), commandType(commandType), targetID(targetID), 
				aiCommand(aiCommand), attackMove(attackMove), completed(false), 
				specialPowerIndex(specialPowerIndex) {}
		};
	private:
		Lock mutex;
		UnitTypeInfo *unitTypeInfo;																				//unitTypeInfo list
		UnitID unitID;																							//own index in map
		Game *game;																								//map
		Path path;																								//path
		int frame_number;																						//current state animated unitTypeInfo frame_number
		bool waiting;																							//true if waiting
		int maxWaitTimer, waitTimer;																			//
		bool projected;																							//true if unit body is projectedby hit
		Projectile projection;																					//projection calculation when unit body is projectedby hit
		vector<Command> command;																				//command queue
		ParticleEngine *particleEngine;																			//
		vector<Point2Di> currentlyOccupiedPoints;
		long birthTime;
		Point2Di targetPosition, targetUnitPosition;
		UnitID targetID;
		int attackTimer;																						//
		int foodGatherTimer, woodGatherTimer, stoneGatherTimer, metalGatherTimer;								//resource gather timers
		int deathTimer;																							//
		int standByTimer;																						//
		int autoHealTimer;																						//
		int rechargedSpecialAbility[MAX_SPECIAL_ABILITY], swapRechargedSpecialAbility[MAX_SPECIAL_ABILITY];		//
		int rechargedTransformation[MAX_SPECIAL_ABILITY], swapRechargedTransformation[MAX_SPECIAL_ABILITY];		//
		bool onAttackMove;
		float renderAlpha;
	public:
		string idForCampaign;
		int hitPoint, level;																					//
		float blended;																							//ammount of units blending to environment =[0.0, 1.0]
		Resource<int> has;																						//
		float builtPercentage;																					//
		Point2D rallyPoint;																						//
		bool garrisoned;																						//
		vector<int> garrisonedUnits;																			//
		int garrisonedBuildingIndex;																			//
		UnitState state;																						//
		UnitType type;																							//
		float x, y, z, angle, tilt;																				//
		int team;																								//
		bool fleeing;
		UnitStance stance;																						//
		int capturedByUnit;																						//Index of unit if captured (i.e. by tree house for tree) else INVALID_UNIT_INDEX
		float scaling;
	private:
		void _removeFromMinimap();																				//removes unit from minimap
		void _insertIntoMinimap();																				//inserts unit into minimap
		void _calculatePath();																					//calculate path
		bool _pathBlocked();																						//returns true if calculate dpath is blocked
		bool _targetOutOfgetLOS();																				//
		bool _pathClearForRangedAttack();																		//
		bool _targetInRange();																					//returns true if target of first command is in range
		bool _rotate(float, float);																				//Rotates units twards given position
		void _move(float, float);																				//Moves unit towards given position
		void _commandMove(float, float);																			//Performs move command
		void _attackRanged();																					//
		void _attackMelee();																						//
		void _commandAttack(float, float);																		//Performs attack command
		void _commandSpecialAttack(float, float);																//Performs attack command
		void _commandCutTree(float, float);																		//Performs cut tree command
		void _commandGathertStone(float, float);																	//Performs gather stone command
		void _commandBuild(float, float);																		//Performs build command
		void _startJump();																						//
		void _garrison();																						//
		void _moveTowardsTarget();																				//
		void _commandGarrison(float, float);																		//Performs garrison command
		void _whenGarrisoned();																					//
		void _nextFrame();																						//Goes to next frame of animation
		bool _commandCompleted();																				//Returns true if currently processing command is completed
		void _hit(int, int);																						//hits unit by given attack point
		void _setState(UnitState);																				//Sets unit state to given state
		void _kill();																							//Kill unit
		void _selfDeploy();																						//Deploys unit itself
		void _updatePosition();																					//Sets x, y, z according to position, unitType
		void _transform(int);																					//
		void _autoTransform();																					//
		void _generateRandomBirdFly();																		//
		void _drown();
		void _autoHeal();																						//
		void _calculateBlending();																				//
		void _paralyzeNearbyUnits();
		void _newCommand(CommandType, Point2D, UnitID, bool, bool, bool, bool, int);								//Pushes new command
		void _heal(int);																							//
		void _revive();
		void _playSound() const;
		bool _canMoveToIfNoUnit(int, int);
		void _setRenderAlpha();
	protected:
		WaveFrontObjSequence& currentAnimation() const;
		WaveFrontObj& currentFrame() const;																			//Returns current frame as 3D unitTypeInfo
	public:
		Unit();																									//Default constructor
		void unload();
		void bind(UnitType, int, Point3D, float, int, UnitTypeInfo*, Game*, int, bool);							//Initialize to given map
		void newUserCommand(CommandType, Point2D, UnitID, bool add=false, bool attackMove=false, bool atFront=false, int specialPowerIndex=0);	//Pushes new command
		void newUserCommand(CommandType, Point2D, bool add=false, bool attackMove=false, bool atFront=false, int specialPowerIndex=0);							//Pushes new command
		void newUserCommand(CommandType, UnitID, bool add=false, bool attackMove=false, bool atFront=false, int specialPowerIndex=0);							//Pushes new command
		void newAICommand(CommandType, Point2D, UnitID, bool add = false, bool attackMove = false, bool atFront = false, int specialPowerIndex = 0);	//Pushes new command
		void newAICommand(CommandType, Point2D, bool add = false, bool attackMove = false, bool atFront = false, int specialPowerIndex = 0);							//Pushes new command
		void newAICommand(CommandType, UnitID, bool add = false, bool attackMove = false, bool atFront = false, int specialPowerIndex = 0);							//Pushes new command
		void cancelCommand(int);																				//Cancels command at given index
		void commandTransform(int);																				//
		void stop();																							//Clear commands
		void update();																							//updates unit by one frame
		void suicide();																							//Kill unit
		void remove();																							//
		void deploy(int);																						//Deploys a garrsioned unit from building at given index
		void deployAll();																						//Deploy all garrisoned units
		void hit(ProjectileWeapon&);																			//If hit by given projectile reduce hitpoint and return true
		void hitTornedo(Point2D);																				//
		void burn(int);																							//
		void extinguish();
		void paralyze();																						//
		bool inRangeOf(ProjectileWeapon&);																		//
		void cancelBuild(int);																					//Cancel Building of given unit type from building
		void flee(Vector2D dir, int distance);
		void rotateTowards(Unit&);
		bool isValidSpecialAttack(Command);																	//
		int nearestUnit(UnitCategory);																			//find nearest unit of given type
		void render();																							//renders unit to screen
		bool exists();																					//returns if unit is dead
		bool isRenderable();
		bool isSelectable();
		bool isAlive();																							//returns if unit is dead
		float getBuildPercentage(int);																			//Returns %age of currently building given unit
		int getBuildCount(int);																					//Returns number of currently building given unit
		int specialAbilityRecharged(int);																		//
		float specialAbilityRechargeTimeRemaining(int);															//
		int transformationRecharged(int);																		//
		float transformationRechargeTimeRemaining(int);															//
		bool isVisibleToTeam(TeamID);																			//
		bool isExploredToTeam(TeamID);																			//
		bool isVisibleToUnit(UnitID);																			//
		bool updateActive();																					//
		bool isOnWater();																						//
		Point3D position() const;																				//
		Point2D position2D() const;																				//
		Point3D centre() const;																					//
		Point3D top() const;																					//
		Point3D bottom() const;																					//
		Point3D front() const;																					//
		Point3D frontLeft() const;																				//
		Point3D frontRight() const;																				//
		UnitCategory getCategory() const;																		//
		UnitType getType() const;																				//
		UnitID getID() const;
		const UnitTypeInfo& getTypeInfo() const;																//
		bool hasVacancy();																				//
		bool isVacantDefensiveUnit();
		bool isIdle();																					//
		bool isMoving();																					//
		bool isFleeing();
		bool isAttacking();																				//
		bool isDying();																					//
		bool isGarrisoned();																				//
		bool isOnAttackMove();																			//
		bool isFoodGatherer();																			//
		bool isWoodGatherer();																			//
		bool isStoneGatherer();																			//
		bool isMetalGatherer();																			//
		bool isBuilder();																					//
		bool isActive();																					//
		bool isIncomplete();																				//
		bool isDamaged();																					//
		int commandListLength();																			//
		long getAutoTransformTimeRemaining();																//
		Path getPath();
		vector<Command> getCommandList();
		UnitID getTargetID();
		Unit& getTarget();
		Point2D getTargetPosition();
		vector<Point2Di> getOccupiedPoints();																//
		float getRadius() const;
		float getMinX() const;
		float getMinY() const;
		float getMinZ() const;
		float getMaxX() const;
		float getMaxY() const;
		float getMaxZ() const;
		float getMidX() const;
		float getMidY() const;
		float getMidZ() const;
		float getRadiusAcrossXPlane() const;
		float getRadiusAcrossYPlane() const;
		float getRadiusAcrossZPlane() const;
		bool isBaseTransformation() const;
		float getRange() const;
		int getLOS() const;
		float getArmour() const;
		int getHealAmount() const;
		int getHealDelay() const;
		int getAttack() const;
		int getSiegeAttack() const;
		int getMaxHitPoint() const;
		xml_node<char>* toXMLNode(xml_document<char>&);
		void loadFromXMLNode(xml_node<char>*, UnitTypeInfo*, Game*);
	private:
		static bool _unitCanMoveTo(void*, int, int);
	};
}

#endif
