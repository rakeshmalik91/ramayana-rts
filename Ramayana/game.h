#ifndef __RAMAYANA_GAME_H
#define __RAMAYANA_GAME_H

#include "world.h"
#include "terrain.h"
#include "sky.h"
#include "water.h"
#include "unit.h"
#include "ai.h"
#include "common.h"
#include "team.h"

namespace ramayana {

#define OBJECT_SCALING 0.25

#define BOARDPIN_UNIT_TYPE 799

	struct Ability {
		AbilityType type;
		int unit;
		string name;
		Texture2D *icon;
		Color buttonColor;
		bool disabled;
		float percentage;
		int cnt;
		int specialPowerIndex;
		Ability(AbilityType type = ABILITY_SPECIAL, int unit = -1, string name = "", Texture2D *icon = NULL, bool disabled = false, int specialPowerIndex = 0, float percentage = 0, int cnt = 0, Color buttonColor = Color(1, 1, 1, 1))
			: type(type), unit(unit), name(name), icon(icon), disabled(disabled), percentage(percentage), cnt(cnt), specialPowerIndex(specialPowerIndex), buttonColor(buttonColor) {}
	};

	class Game : public Terrain, public Sky, public Water {
	public:		//static data members
		static const float MOVABLE_MAX_HEIGHT;
		static const float MAX_SUITABLE_UNEVEN_BULDING_LAND;
		static const int MAX_ABILITY = 100;
		static const int MAX_PROJECTILE = 500;
		static const int MAX_TORNEDO = 10;
		static const int MAX_POISON = 10;
		static const int MAX_COLLISION = 100;
		static const int MAX_MOVE_QUEUE = 25;
		static const int MAX_MESSAGE_TIME = FRAME_RATE * 5;
		static const int MAX_PROJECTILE_TIMER = 50;
		static const int MAX_UNIT = 5000;
		static const int MAX_TEAM = 9;
		static const int N_UNIT_UPDATE_THRAD = 4;
		static const int MAX_TOTAL_POPULATION = 800;
		struct Settings;																						//
		struct Weather {
			float brightness, contrast;
			int thunderTimeRemaining;
			ParticleEngine rainParticleEngine;
			WeatherType weatherType;
			Weather() : brightness(0), contrast(0), thunderTimeRemaining(0), weatherType(WEATHER_CLEAR) {}
		};
	private:	//private data members
		int gameStartTime, gamePlayTime;
		string filename;																						//
		bool loaded;																							//true if game is loaded
		bool paused;																							//true if game is paused
		FrameBuffer tempFBOSet0[2], tempFBOSet1[4], tempFBOSet2[4];												//
		unsigned int tempFBOSet0Index;																			//
		SDL_Thread *updateThread;																				//
		Lock projectileMutex, mapMutex;																			//
		struct UpdateUnitThreadResource {
			SDL_Thread *thread;
			int index;
			PathfinderAStar *pathfinder;
			Game *game;
		} updateUnitThreadResource[N_UNIT_UPDATE_THRAD];
		ShaderProgram objectShader;																				//
		ShaderProgram alphaMaskShader;																			//
		ShaderProgram dofShader, blurShader, brightPassShader, addShader, antialiasShader, colorShader;			//
		struct ParticleTextureSet {
			vector<Texture2D> fire, smoke, dustHeavy, dustLight, waterHeavy, waterLight, rain;
			Texture2D blue, rainbow, yellow, white, poison;
		} particleTexture;
		struct UITextureSet {
			Texture2D image_blank_button;
			Texture2D icon_deploy, icon_stop, icon_kill;
			Texture2D icon_general_stance, icon_standground_stance, icon_holdfire_stance;
			Texture2D icon_increase_level, icon_decrease_level, icon_remove_unit;
			Texture2D icon_terrain_texture[N_TERRAIN_TEXTURE];
		} uiTextureSet;
		struct ModelSet {
			WaveFrontObj arrow, spike, treelog, flag;
		} model;
		Texture2D *minimapImageTex;																				//
		float wind_flow;																						//
		int wind_flow_phase;																					//
		ProjectileWeapon projectile[MAX_PROJECTILE];															//
		int nProjectile;																						//
		ParticleEngine fireParticleEngine, smokeParticleEngine;
		int **fireIntensity;
		struct Tornedo {
			Point2D pos, dir;
			int time;
			ParticleEngine particleEngine, particleEngineDust, particleEngineWater;
			Tornedo() : time(0) {}
			Tornedo(Point2D pos, Point2D dir, int time) : pos(pos), dir(dir), time(time) {}
		};
		Tornedo tornedo[MAX_TORNEDO];
		struct ParticleEffect {
			Point2D pos;
			float size;
			ParticleEngine particleEngine;
			int time;
			ParticleEffect() : time(0) {}
			ParticleEffect(Point2D pos, float size, int time) : pos(pos), size(size), time(time) {}
		};
		ParticleEffect waterSplash[MAX_COLLISION], dustBlow[MAX_COLLISION], bloodSplatter[MAX_COLLISION], poison[MAX_POISON];
		struct ShockWave {
			Point2D pos;
			float radius, maxRadius;
			ShockWave() : radius(0), maxRadius(0) {}
			ShockWave(Point2D pos, float radius) : pos(pos), radius(0), maxRadius(radius) {}
		};
		ShockWave shockwave[MAX_COLLISION];
		GLuint selectList[SELECT_BUF_SIZE], nSelected;															//List of isSelected unit index+1
		int moveTo;																								//Index of location of last command
		int targetUnit;																							//Index of target unit + 1 of last command
		int messageTimer;																						//
		string message;
		Weather weather;																						//
		float **blend;
		enum MusicType { MUSIC_COMBAT, MUSIC_GENERAL } musicType;
		bool commandLocked;
		Point3Di commandPosition;
		long lastMusicCahngeTime;
	protected:	//protected data members
		bool editable;
		UnitTypeInfo *unitTypeInfo;																				//Pointer to 3D objects list
		int nUnit;																								//Length of unit list
		int nTeams;																								//number fo teams
		string objective;																						//
		vector<Point2D> startPos;																				//
		TeamAI teamAI[MAX_TEAM];																				//
		vector<Point2Di> minimapMarker;
	public:		//public data members																			//
		MapEditState editState;																					//
		int frameCounter;
		ParticleRenderer particleRenderer;																		//
		Team teams[MAX_TEAM];																					//
		Unit unit[MAX_UNIT];																					//
		int playerTeam;																							//Player team index
		struct MiniMapPosition {
			TerrainType type;
			UnitID landUnit, waterUnit, airUnit;
			MiniMapPosition() : landUnit(-1), waterUnit(-1), airUnit(-1) {};
		};
		MiniMapPosition **minimap;																				//
		vector<Ability> selectionAbility;																		//Abilities of isSelected units
		struct Cursor {
			Point2D pos;
			enum Type { NONE, BUILD, TARGET } type;
			int buildUnitType;
			int targetID;
			int angle;
			int specialPowerIndex;
			Cursor() : pos(0, 0), type(NONE), buildUnitType(-1),
				targetID(-1), angle(0), specialPowerIndex(-1) {}
		} cursor;																								//Object to be rendered under cursor
		int updateFrameDuration;
		int nUnitsRendered;
	protected:
		void loadUnit(int u, int level);																		//
	private:	//private functions
		void compileUnits();																					//
		virtual void initTeams(Team[], int, int, int, int, int, int);											//
		void initAI();																							//
		void initCamera();																						//
		void loadObjects();																						//
		void loadUITextures();																					//
		void loadParticles();																					//
		void loadTextureList(vector<Texture2D>&, string, bool, GLint, GLint, GLint, GLint);						//
		void initFire();																						//
		void initRain();																						//
		void updateRain();																						//
		void initWeather();																						//
		void initSound();																						//
		void updateWeather();																					//
		void loadShaders();																						//
		void initFBOs();																						//
		float getFog(unsigned int x, unsigned int y);															//
		void drawBlendMarker(float);																			//
		void drawMarkers();																						//Renders all unit highlight, flag, path ...
		void makeShadow();																						//
		void drawObjects(bool noShader, bool objectBumpmapOn);													//
		void drawObjectsUnderwater(bool noShader, bool objectBumpmapOn);										//
		void drawProjectiles();																					//
		void renderObjectsForReflection();																		//
		void makeReflection();																					//
		void renderObjectsForShadow();																			//
		void drawCursor();																						//
		void drawParticles();																					//
		void drawMeshes();																						//
		void applyDepthOfFieldEffect();																			//
		void applyBloom();																						//
		void applyAntialias();																					//
		void initMatrix();																						//
		void closeMatrix();																						//
		void initMiniMap();																						//
		void calculateVisitablity();																			//Calculates which places can be visited on map
		void setAmbienceSound();																				//
		void setMusic();																						//
		void updateProjectile();																				//
		void _makeProjectileTrail();																			//
		void _hitProjectile(ProjectileWeapon&, int);															//
		GLuint getMouseHits(int, int, int, int, bool);															//
		void processMouseHits(int, int, int, int, bool);														//
		void updateFire();																						//
		void updateTornedo();																					//
		void updateSpecialEffect();																				//
		void drawSpecialEffects();
		void positionLight();																					//
		void updateTeams();																						//
		void updateSelection();																					//
		void calculateBlend();																					//
		void _addProjectile(ProjectileWeapon);																	//
	public:		//public functions
		Game();
		~Game();
		virtual void load(string, UnitTypeInfo*, Team[], int, int, int, int, int, int, bool = false);			//Loads from a file
		void create(int, int, float, string, UnitTypeInfo*, Team[], int, int, int, int, int, int);				//Create map
		int addUnit(float x, float y, UnitType unitType, int team, int angle = 0, int level = 1, bool build = false);	//Add unit adt given position
		int getNumberOfUnits();																					//
		int getNumberOfTeams();																					//
		int getMaxPopulation(TeamID);																			//
		void init();																							//
		void save();																							//Save to file
		void renameMap(string);																					//
		void render();																							//Renders whole map, units, ... onto screen
		virtual void update();																					//Updates whole map, units ...
		void updateSelectionAbility();																			//
		void command(int screen_width, int screen_height, int x, int y, bool add, bool attackMove,
			CommandType commandtype = COMMAND_MOVE, int targetBuilding = -1, int specialPowerIndex = 0);		//Gives command to isSelected units to a specific screen location
		void deploy(UnitID);																					//
		void deployAll();																						//
		void addProjectile(ProjectileWeapon);																	//
		void setCursor(int, int, int, int);																		//Set unitTypeInfo under curseor (if any)
		void move(float, float, bool, bool);																	//Give move command isSelected units to a specific position in map
		void attack(float, float, bool);																		//Give attack move command isSelected units to a specific position in map
		void cutTree(float, float, bool);																		//Give cut tree command isSelected units to a specific position in map
		void cutStone(float, float, bool);																		//Give gather stone command isSelected units to a specific position in map
		void build(UnitType, bool);																				//Give build a given building command isSelected units to a specific position in map
		void train(UnitType);																					//
		void cancelBuild(UnitType);																				//
		void repair(float, float, bool);																		//Give reapir command isSelected units to a specific position in map
		void garrison(float, float, bool);																		//Give garrison command isSelected units to a specific position in map
		void stop();																							//Clears command queue of alla isSelected units
		void _kill();																							//Kill all isSelected player units
		void transform(UnitID);																					//
		void addFire(Point3D, int, int);																		//
		void removeFire(Point3D, int);																			//
		void addTornedo(Point2D, Point2D, int);																	//
		void addWaterSplash(Point2D, int, float);																//
		void addDustBlow(Point3D, int, float);																	//
		void addBloodSplatter(Point3D, float);																	//
		void addPoison(Point3D, int, float);																	//
		void addShockWave(Point3D pos, float radius);															//
		void setStance(UnitStance);																				//
		int nearestUnitIndex(float, float, UnitCategory) const;													//returns index of nearest given unit
		void setMessage(string);																				//
		string getMessage();																					//
		string getObjectives();																					//
		float getMessageTransparency();																			//
		int soundAngle(float, float);																			//
		float soundDistance(float, float);																		//
		void bindMinimapImage();																				//
		void select(int, int, int, int, int, int, bool, bool, bool);											//Selects unit(s) on a specific screen location
		void goToSelected();																					//Moves camera to first isSelected unit on map
		void selectUnit(int, bool);																				//
		void selectGroup(int, bool);																			//
		void clearSelection();																					//
		Unit& selectedUnit(int);																				//
		int numberOfSelectedUnit();																				//
		UnitID selectedUnitIndex(int);																			//
		void filterSelectionRemoveAll(int);																		//
		void filterSelectionRemoveFirst(int);																	//
		void filterSelectionKeep(int);																			//
		void selectIdleWorker(bool);																			//
		void selectFoodGatherer(bool);																			//
		void selectWoodGatherer(bool);																			//
		void selectStoneGatherer(bool);																			//
		void selectMetalGatherer(bool);																			//
		void selectHero(bool);																					//
		void selectMilitary(bool);																				//
		bool isSelected(int);																					//Returns true if given unit[param-1] is isSelected
		void deselect(int);																						//Removes seldection of unit[param-1]
		void setSelectedUnitTeam(TeamID);																		//
		void pause();																							//
		void resume();																							//
		virtual bool hasCompleted();																			//
		virtual void victory();																					//
		virtual void defeat();																					//
		virtual bool isCampaign();																				//
		virtual bool isSkirmish();																				//
		virtual Diplomacy diplomacy(TeamID, TeamID) const;														//
		bool hasEnoughResourceToBuild(int, int) const;															//
		UnitID getNearestFreeTree(Point2Di);																	//
		bool isTerrainSuitableToBuild(Point2D position, float angle, int unitType, bool suppressMessage = true) const;											//
		bool isTerrainFreeToBuild(Point2D position, float angle, int unitType, int team, bool suppressMessage = true) const;										//
		void setInMinimap(const vector<Point2Di>&, UnitID, bool, bool, bool);									//
		bool isLoaded() const;																					//
		bool isPaused() const;																					//
		int getGamePlayTime() const;																			//
		Tuple<Point2D, int> getBuildablePlace(Point2Di, int, int, bool**);										//
		float getBlend(Point2Di) const;																			//
		void setBlend(Point2Di, float);																			//
		void setWeather(WeatherType);																			//
		void increaseSelectedUnitLevel(int val);																//
		void writeSnapshotXML(xml_document<char>&);																//
		virtual void saveSnapshot(string);																		//
		void readSnapshotXML(xml_document<char>&, UnitTypeInfo*);												//
		virtual void loadSnapshot(string, UnitTypeInfo*);														//
		string getFileName() const;																				//
		void lockCommand();
		void unlockCommand();
		PathfinderAStar* getPathFinder(UnitID) const;
		bool isTerrainEditMode() const;
		bool isEditable() const;
	private:	//private static functions
		static int _SDL_THREAD updateThreadFunc(void*);															//
		static int _SDL_THREAD updateUnitThreadFunc(void*);														//
		static int unitindexComparator(const int&, const int&, void*);											//
	public:		//static static functions
		static void saveSettings();																				//
		static void initGameModeStrings();																		//
		static void checkGameMode();																			//
		static void loadSettings();																				//
		static void setSoundSettings();																			//
		static void getMapHeader(string, vector<Point2D>&, Texture2D&, int&, int&);								//
	};

	struct Game::Settings {
		static const int DEFAULT_WINDOWED_WIDTH = 800;
		static const int DEFAULT_WINDOWED_HEIGHT = 600;
		static vector<string> availableGameModeStrings;
		static bool fullscreen, gameMode;
		static int screenWidth, screenHeight;
		static string gameModeString;
		static bool noShader;
		static bool reflectionOn, shadowOn, antialiasingOn, terrainBumpmapOn, objectBumpmapOn, motionBlurOn, depthOfFieldOn, bloomOn;
		static bool showLOS, showPath;
		static bool renderObjectsOn, renderTerrainOn, renderWaterOn, renderSkyOn;
		static float scrollSpeed;
		static bool stereoscopicOn, wireframeOn;
		static float stereoSeperation;
		static bool showRenderDetails;
		static float brightness, contrast;
		static vector<string> availableVideoHeight;
		static string videoHeight;

		static bool noSound;
		static float volume, ambientVolume, soundVolume, musicVolume;
	};
};

#endif
