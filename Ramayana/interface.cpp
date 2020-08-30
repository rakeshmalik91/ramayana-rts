#include "stdafx.h"

#include "common.h"
#include "game.h"
#include "campaign.h"
#include "skirmish.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "audio.h"
#include "enums.h"
#include "interface.h"
#include "steamStatsAndAchievements.h"

namespace ramayana {
	static int DOUBLE_CLICK_DELAY = 300;
	static int VIDEO_FRAMERATE = 24;

	static bool keyPressed[127], specialPressed[200], buttonPressed[5];
	static unsigned int buttonUpTime = 0, buttonDownTime = 0;
	static bool doubleClicked = false;
	static int glutModifier = 0;
	static int mouseX = 0, mouseY = 0, dragStartX = 0, dragStartY = 0;
	static Quad dragbox;
	static bool dragboxEnabled = false;
	static bool loading = false;
	static float loadingProgress;
	static char message[1000] = "\0";
	static string toolTipText = "";

	static SDL_Thread *loadingGameThread;

	static UnitTypeInfo *unitTypeInfo = NULL;
	static Game *game = NULL;

	static bool console = false;
	static string consoleBuffer = "";
	static vector<string> consoleQueue;

	static bool confirmRemove = false;

	static enum GameState currentGameState;
	static stack<GameState> gameState;
	void setGameState(GameState state);
	void backGameState();

	Lock gameMutex;

	string gameStateAsString(GameState state) {
		switch (state) {
		case GAMESTATE_LOADING_GAME_PHASE1:			return "GAMESTATE_LOADING_GAME_PHASE1";
		case GAMESTATE_LOADING_GAME_PHASE2:			return "GAMESTATE_LOADING_GAME_PHASE2";
		case GAMESTATE_LOADING_MAP_PHASE1:			return "GAMESTATE_LOADING_MAP_PHASE1";
		case GAMESTATE_LOADING_MAP_PHASE2:			return "GAMESTATE_LOADING_MAP_PHASE2";
		case GAMESTATE_MAINMENU:					return "GAMESTATE_MAINMENU";
		case GAMESTATE_SETTINGSMENU_DISPLAY:		return "GAMESTATE_SETTINGSMENU_DISPLAY";
		case GAMESTATE_SETTINGSMENU_AUDIO:			return "GAMESTATE_SETTINGSMENU_AUDIO";
		case GAMESTATE_SETTINGSMENU_GAME:			return "GAMESTATE_SETTINGSMENU_GAME";
		case GAMESTATE_CAMPAIGNMENU:				return "GAMESTATE_CAMPAIGNMENU";
		case GAMESTATE_CHAPTER_SCREEN:				return "GAMESTATE_CHAPTER_SCREEN";
		case GAMESTATE_INTRO:						return "GAMESTATE_INTRO";
		case GAMESTATE_INTRO_ONLY:					return "GAMESTATE_INTRO_ONLY";
		case GAMESTATE_OUTRO:						return "GAMESTATE_OUTRO";
		case GAMESTATE_NETWORKMENU:					return "GAMESTATE_NETWORKMENU";
		case GAMESTATE_HOSTGAMEMENU:				return "GAMESTATE_HOSTGAMEMENU";
		case GAMESTATE_JOINGAMEMENU:				return "GAMESTATE_JOINGAMEMENU";
		case GAMESTATE_SKIRMISHMENU:				return "GAMESTATE_SKIRMISHMENU";
		case GAMESTATE_PLAYING:						return "GAMESTATE_PLAYING";
		case GAMESTATE_EDITORMENU:					return "GAMESTATE_EDITORMENU";
		case GAMESTATE_CREDITS_SCREEN:				return "GAMESTATE_CREDITS_SCREEN";
		case GAMESTATE_SAVEGAMESNAPSHOT_MENU:		return "GAMESTATE_SAVEGAMESNAPSHOT_MENU";
		default:									return "!!! INVALID_GAME_STATE (" + toString((int)state) + ") !!!";
		}
	}

	void toggleFullScreen() {
		Game::Settings::fullscreen = !Game::Settings::fullscreen;
		if (Game::Settings::fullscreen) {
			if (Game::Settings::gameMode && !glutGameModeGet(GLUT_GAME_MODE_ACTIVE) && glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
				glutEnterGameMode();
			} else {
				glutFullScreen();
			}
		} else {
			if (Game::Settings::gameMode && glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
				glutLeaveGameMode();
			}
			glutReshapeWindow(Game::Settings::screenWidth, Game::Settings::screenHeight);
		}
	}

	//------------------------------------------------------------------------------------------------Frame rate sync function/data

	static int frameDuration = 0;
	static clock_t frameStartTime = 0;

	int syncFrameRate(float frameRate) {
		frameDuration = SDL_GetTicks() - frameStartTime;
		int delay = 1000 / frameRate - frameDuration;
		SDL_Delay(clamp(delay, 0, 1000 / frameRate));
		frameStartTime = SDL_GetTicks();
		return clampLow(-delay, 0);
	}

#define MAX_FRAME_SKIP 2
	int syncVideoToFrameRate(float frameRate, CvCapture* cap) {
		int nFamesSkipped = 0;
		int delay = syncFrameRate(frameRate);
		IplImage* frame = (IplImage*)1;
		for (; delay > 0 && frame != NULL; delay -= 1000 / frameRate) {
			frame = cvQueryFrame(cap);
			nFamesSkipped++;
			if (nFamesSkipped >= MAX_FRAME_SKIP) {
				break;
			}
		}
		return nFamesSkipped;
	}

	//------------------------------------------------------------------------------------------------Save/Load/Unload resource functions/data

	static Texture2D logo, image_blank_button, image_loading, image_loading_text, texture_select, icon_campaign,
		icon_skirmish, icon_display_settings, icon_audio_settings, icon_game_settings, icon_network, icon_editor, icon_credits,
		icon_quit, icon_back, icon_pause, icon_error, icon_create,
		icon_reflection_on, icon_reflection_off, icon_shadow_on, icon_shadow_off, icon_antialiasing_on, icon_antialiasing_off,
		icon_terrain_bumpmap_off, icon_terrain_bumpmap_on, icon_object_bumpmap_off, icon_object_bumpmap_on,
		icon_motion_blur_on, icon_motion_blur_off, icon_depthoffield_on, icon_depthoffield_off, icon_bloom_on, icon_bloom_off,
		icon_stereoscopic_on, icon_stereoscopic_off, icon_fullscreen_on, icon_fullscreen_off,
		icon_resume, icon_restart, icon_load, icon_height, icon_width, icon_ground_height,
		icon_editmap, icon_newmap, icon_save, icon_inr_height, icon_dcr_height, icon_terrain_texture,
		icon_hostgame, icon_joingame, icon_delete, icon_yes, icon_no, icon_video,
		icon_add_unit, icon_add_building, icon_add_others, icon_change_team,
		icon_add, icon_remove, icon_startposition, icon_startposition_empty,
		icon_wood, icon_food, icon_stone, icon_metal, icon_population,
		icon_hitpoint, icon_healrate, icon_attack, icon_siegeattack, icon_armor, icon_speed, icon_accuracy, icon_fire_resistance,
		icon_blend, icon_hero, icon_deploy, texture_game_frame, texture_video_frame,
		texture_button_color, texture_button_medium, texture_button_wide,
		texture_slidebar, texture_slider, texture_slidebar_gap,
		icon_resolution, icon_volume, icon_music_volume, icon_sound_volume, icon_ambient_volume, icon_scrollspeed, icon_controls,
		image_victory, image_defeat, image_credits, image_controls,
		cursorGeneral, cursorDrag, cursorAttack, cursorTarget, cursorUp, cursorDown;

	static struct CampaignMenuData {
		struct Data {
			string name, path;
			string intro, outro;
			Color buttonColor;
			Texture2D *image;
			bool unlocked;
			Data() : name(""), path(""), intro(""), buttonColor(0, 0, 0), image(NULL), unlocked(false) {}
		};
		vector<Data> data;
		struct VideoData {
			CvCapture* videoCapture;
			Texture2D* frameTexture;
			VideoData() : videoCapture(NULL), frameTexture(NULL) {}
		} video;
		int selectedCampaignIndex;
		Texture2D *frameTexture;
		Texture2D *storyImage, *nameImage;
		float storyImageScrollOffset;
		CampaignMenuData() : frameTexture(NULL), storyImage(NULL), nameImage(NULL), storyImageScrollOffset(0), selectedCampaignIndex(-1) {}
	} campaignMenuData;

	static struct SkirmishMenuData {
		int selectedMapIndex, mapScrollOffset, mouseOverMapIndex;
		int mapWidth, mapHeight;
		vector<string> selectedMapName;
		Texture2D *selectedMapImage;
		vector<Point2D> gameStartPosition;
		vector<int> startPositionTeam;
		int mapEditTerrainTextureIndex;
		Team teams[10];
		int nTeams, playerTeam;
		SkirmishMenuData()
			: selectedMapIndex(0), mapScrollOffset(0), mouseOverMapIndex(-1), mapEditTerrainTextureIndex(0), selectedMapImage(NULL)
			, nTeams(2), playerTeam(1), mapWidth(0), mapHeight(0) {
			teams[0] = Team(1, true, 1, 1);
			teams[1] = Team(2, false, 2, 2);
		}
	} skirmishMenuData;

	static struct SavedGameMenuData {
		vector<string> savedGameList;
		int selectedIndex, scrollOffset;
		SavedGameMenuData() : selectedIndex(0), scrollOffset(0) {}
	} savedGameMenuData;

	static struct NewMapData {
		string filename;
		int width, height, groundHeight;
		NewMapData() : filename(""), width(150), height(150), groundHeight(1) {}
	} newMapData;

	void loadUnitInfo(char *message, float& loadingProgress) {
		unitTypeInfo = new UnitTypeInfo[MAX_OBJECT_TYPE];

		string filename = "data/unit.xml";
		string text = readTextFile(filename.data());
		xml_document<char> doc;
		doc.parse<0>((char*)text.data());

		try {
			xml_node<> *root = doc.first_node("unitList");
			for (xml_node<> *node = root->first_node("unit"); node; node = node->next_sibling("unit")) {
				UnitType id = toInt(node->first_attribute("id")->value());
				unitTypeInfo[id].category = toUnitCategory(node->first_attribute("category")->value());
				unitTypeInfo[id].name = trim(node->first_attribute("name")->value());
				getLogger().print("Loading unit info " + unitTypeInfo[id].name + "(" + id + ")...");

				if (node->first_attribute("hideInEditor")) {
					unitTypeInfo[id].hideInEditor = toBool(node->first_attribute("hideInEditor")->value());
				}
				for (xml_node<> *nodeA = node->first_node("sibling"); nodeA; nodeA = nodeA->next_sibling("sibling")) {
					unitTypeInfo[id].siblings.push_back(toInt(nodeA->value()));
				}
				if (node->first_node("icon")) {
					unitTypeInfo[id].icon.load(trim(node->first_node("icon")->value()).data(), false);
				}
				if (node->first_node("iconSmall")) {
					unitTypeInfo[id].iconSmall.load(trim(node->first_node("iconSmall")->value()).data(), false);
				} else if (node->first_node("icon")) {
					unitTypeInfo[id].iconSmall.load(trim(node->first_node("icon")->value()).data(), false);
				}
				if (node->first_node("unitPath")) {
					unitTypeInfo[id].unitPath = trim(node->first_node("unitPath")->value());
				}
				if (node->first_node("hitPoint")) {
					unitTypeInfo[id].hitPoint = toInt(node->first_node("hitPoint")->value());
				}
				if (node->first_node("attack")) {
					unitTypeInfo[id].attack = toInt(node->first_node("attack")->value());
				}
				if (node->first_node("armour")) {
					unitTypeInfo[id].armour = toFloat(node->first_node("armour")->value());
				}
				if (node->first_node("fireResistance")) {
					unitTypeInfo[id].fireResistance = toFloat(node->first_node("fireResistance")->value());
				}
				if (node->first_node("tornedoResistant")) {
					unitTypeInfo[id].tornedoResistant = true;
				}
				if (node->first_node("siegeAttack")) {
					unitTypeInfo[id].siegeAttack = toInt(node->first_node("siegeAttack")->value());
				}
				if (node->first_node("areaDamageRadius")) {
					unitTypeInfo[id].areaDamageRadius = toInt(node->first_node("areaDamageRadius")->value());
				}
				if (node->first_node("attackDelay")) {
					unitTypeInfo[id].attackDelay = toInt(node->first_node("attackDelay")->value());
				}
				if (node->first_node("weaponType")) {
					unitTypeInfo[id].weaponType = toWeaponType(node->first_node("weaponType")->value());
				}
				if (node->first_node("range")) {
					unitTypeInfo[id].range = toFloat(node->first_node("range")->value());
				}
				if (node->first_node("accuracy")) {
					unitTypeInfo[id].accuracy = toFloat(node->first_node("accuracy")->value());
				}
				if (node->first_node("speed")) {
					unitTypeInfo[id].speed = toFloat(node->first_node("speed")->value());
				}
				if (node->first_node("rotAngle")) {
					unitTypeInfo[id].rotAngle = toInt(node->first_node("rotAngle")->value());
				}
				if (node->first_node("los")) {
					unitTypeInfo[id].los = toInt(node->first_node("los")->value());
				}
				if (node->first_node("isHeroic")) {
					unitTypeInfo[id].isHeroic = true;
				}
				if (node->first_node("healDelay")) {
					unitTypeInfo[id].healDelay = toInt(node->first_node("healDelay")->value());
				}
				if (node->first_node("healAmount")) {
					unitTypeInfo[id].healAmount = toInt(node->first_node("healAmount")->value());
				}
				if (node->first_node("canParalyze")) {
					unitTypeInfo[id].canParalyze = true;
				}
				if (node->first_node("drownInCurrent")) {
					unitTypeInfo[id].drownInCurrent = true;
				}
				if (node->first_node("camouflage")) {
					unitTypeInfo[id].camouflage = toFloat(node->first_node("camouflage")->value());
				}
				if (node->first_node("stealth")) {
					unitTypeInfo[id].stealth = toFloat(node->first_node("stealth")->value());
				}
				if (node->first_node("trampleDamage")) {
					unitTypeInfo[id].trampleDamage = toInt(node->first_node("trampleDamage")->value());
				}
				if (node->first_node("standByTime")) {
					unitTypeInfo[id].standByTime = toInt(node->first_node("standByTime")->value());
				}
				if (node->first_node("standByDamage")) {
					unitTypeInfo[id].standByDamage = toInt(node->first_node("standByDamage")->value());
				}
				if (node->first_node("population")) {
					unitTypeInfo[id].population = toInt(node->first_node("population")->value());
				}
				if (node->first_node("maxPopulationIncrease")) {
					unitTypeInfo[id].maxPopulationIncrease = toInt(node->first_node("maxPopulationIncrease")->value());
				}
				if (node->first_node("buildTime")) {
					unitTypeInfo[id].buildTime = toInt(node->first_node("buildTime")->value());
				}
				if (node->first_node("isWorker")) {
					unitTypeInfo[id].isWorker = true;
				}
				if (node->first_node("canRepair")) {
					unitTypeInfo[id].canRepair = true;
				}
				for (xml_node<> *nodeB = node->first_node("canBuild"); nodeB; nodeB = nodeB->next_sibling("canBuild")) {
					unitTypeInfo[id].canBuild[toInt(nodeB->value())] = true;
				}
				if (node->first_node("canGatherFood")) {
					unitTypeInfo[id].canGather.food = true;
				}
				if (node->first_node("canGatherWood")) {
					unitTypeInfo[id].canGather.wood = true;
				}
				if (node->first_node("canGatherStone")) {
					unitTypeInfo[id].canGather.stone = true;
				}
				if (node->first_node("canGatherMetal")) {
					unitTypeInfo[id].canGather.metal = true;
				}
				if (node->first_node("flyLevel")) {
					unitTypeInfo[id].flyLevel = toFloat(node->first_node("flyLevel")->value());
				}
				if (node->first_node("canSetFire")) {
					unitTypeInfo[id].canSetFire = true;
				}
				if (node->first_node("isBridge")) {
					unitTypeInfo[id].isBridge = true;
				}
				if (node->first_node("canJump")) {
					unitTypeInfo[id].canJump = true;
					unitTypeInfo[id].jumpSpeed = toFloat(node->first_node("canJump")->first_attribute("speed")->value());
				}

				if (node->first_node("cost")) {
					if (node->first_node("cost")->first_node("food"))			unitTypeInfo[id].cost.food = toInt(node->first_node("cost")->first_node("food")->value());
					if (node->first_node("cost")->first_node("wood"))			unitTypeInfo[id].cost.wood = toInt(node->first_node("cost")->first_node("wood")->value());
					if (node->first_node("cost")->first_node("stone"))			unitTypeInfo[id].cost.stone = toInt(node->first_node("cost")->first_node("stone")->value());
					if (node->first_node("cost")->first_node("metal"))			unitTypeInfo[id].cost.metal = toInt(node->first_node("cost")->first_node("metal")->value());
				}

				if (node->first_node("has")) {
					if (node->first_node("has")->first_node("food"))				unitTypeInfo[id].has.food = toInt(node->first_node("has")->first_node("food")->value());
					if (node->first_node("has")->first_node("wood"))				unitTypeInfo[id].has.wood = toInt(node->first_node("has")->first_node("wood")->value());
					if (node->first_node("has")->first_node("stone"))			unitTypeInfo[id].has.stone = toInt(node->first_node("has")->first_node("stone")->value());
					if (node->first_node("has")->first_node("metal"))			unitTypeInfo[id].has.metal = toInt(node->first_node("has")->first_node("metal")->value());
				}

				for (xml_node<> *nodeP = node->first_node("occupiedPoint"); nodeP; nodeP = nodeP->next_sibling("occupiedPoint")) {
					int x = toInt(nodeP->first_node("x")->value());
					int y = toInt(nodeP->first_node("y")->value());
					unitTypeInfo[id].occupiedPoint.push_back(Point2Di(x, y));
				}

				for (xml_node<> *nodeP = node->first_node("garrisonedUnitPosition"); nodeP; nodeP = nodeP->next_sibling("garrisonedUnitPosition")) {
					float x = toFloat(nodeP->first_node("x")->value());
					float y = toFloat(nodeP->first_node("y")->value());
					float z = toFloat(nodeP->first_node("z")->value());
					float angle = toFloat(nodeP->first_node("angle")->value());
					unitTypeInfo[id].garrisonedUnitPosition.push_back(Point4D(x, y, z, angle));
				}

				for (xml_node<> *nodeO = node->first_node("obj"); nodeO; nodeO = nodeO->next_sibling("obj")) {
					UnitState state = toUnitState(nodeO->first_attribute("state")->value());
					if (nodeO->first_node("sameas")) {
						UnitType sameType = id;
						if (nodeO->first_node("sameas")->first_attribute("unit")) {
							sameType = toInt(nodeO->first_node("sameas")->first_attribute("unit")->value());
						}
						UnitState sameState = state;
						if (nodeO->first_node("sameas")->first_attribute("state")) {
							sameState = toUnitState(nodeO->first_node("sameas")->first_attribute("state")->value());
						}
						unitTypeInfo[id].sameAs(state, &unitTypeInfo[sameType], sameState);
					} else {
						unitTypeInfo[id].setPath(state, trim(nodeO->value()));
					}
				}

				for (xml_node<> *nodeA = node->first_node("audio"); nodeA; nodeA = nodeA->next_sibling("audio")) {
					UnitState state = toUnitState(nodeA->first_attribute("state")->value());
					unitTypeInfo[id].audio[state].push_back(trim(nodeA->value()));
				}
				for (xml_node<> *nodeA = node->first_node("selectSound"); nodeA; nodeA = nodeA->next_sibling("selectSound")) {
					unitTypeInfo[id].selectSound.push_back(trim(nodeA->value()));
				}
				for (xml_node<> *nodeA = node->first_node("commandSound"); nodeA; nodeA = nodeA->next_sibling("commandSound")) {
					unitTypeInfo[id].commandSound.push_back(trim(nodeA->value()));
				}
				for (xml_node<> *nodeT = node->first_node("transformation"); nodeT; nodeT = nodeT->next_sibling("transformation")) {
					UnitTypeInfo::TransformationAbility t;
					if (nodeT->first_attribute("name")) {
						t.name = trim(nodeT->first_attribute("name")->value());
					}
					if (nodeT->first_node("unitType")) {
						t.unitType = toInt(nodeT->first_node("unitType")->value());
					}
					if (nodeT->first_node("icon")) {
						t.icon = new Texture2D(trim(nodeT->first_node("icon")->value()).data(), false);
					}
					if (nodeT->first_node("rechargeTime")) {
						t.rechargeTime = toInt(nodeT->first_node("rechargeTime")->value());
					}
					if (nodeT->first_node("neededLevel")) {
						t.neededLevel = toInt(nodeT->first_node("neededLevel")->value());
					}
					if (nodeT->first_node("hide")) {
						t.hide = true;
					}
					unitTypeInfo[id].transformation.push_back(t);
				}
				if (node->first_node("autoTransformTime")) {
					unitTypeInfo[id].autoTransformTime = toInt(node->first_node("autoTransformTime")->value());
				}
				if (node->first_node("transformOnDeath")) {
					unitTypeInfo[id].transformOnDeath = true;
				}
				if (node->first_node("transformOnBurn")) {
					unitTypeInfo[id].transformOnBurn = true;
				}
				if (node->first_node("transformOnWater")) {
					unitTypeInfo[id].transformOnWater = true;
				}

				for (xml_node<> *nodeS = node->first_node("specialPower"); nodeS; nodeS = nodeS->next_sibling("specialPower")) {
					UnitTypeInfo::SpecialPower s;
					if (nodeS->first_attribute("name")) {
						s.name = trim(nodeS->first_attribute("name")->value());
					}
					if (nodeS->first_node("weaponType")) {
						s.weaponType = toWeaponType(nodeS->first_node("weaponType")->value());
					}
					if (nodeS->first_node("state")) {
						s.state = toUnitState(nodeS->first_node("state")->value());
					}
					if (nodeS->first_node("targetType")) {
						s.targetType = toTargetType(nodeS->first_node("targetType")->value());
					}
					if (nodeS->first_node("attack")) {
						s.attack = toInt(nodeS->first_node("attack")->value());
					}
					if (nodeS->first_node("range")) {
						s.range = toFloat(nodeS->first_node("range")->value());
					} else {
						s.range = unitTypeInfo[id].range;
					}
					if (nodeS->first_node("siegeAttack")) {
						s.siegeAttack = toInt(nodeS->first_node("siegeAttack")->value());
					}
					if (nodeS->first_node("areaDamageRadius")) {
						s.areaDamageRadius = toInt(nodeS->first_node("areaDamageRadius")->value());
					}
					if (nodeS->first_node("nTornedo")) {
						s.nTornedo = toInt(nodeS->first_node("nTornedo")->value());
					}
					if (nodeS->first_node("heal")) {
						s.heal = toInt(nodeS->first_node("heal")->value());
					}
					if (nodeS->first_node("addUnit")) {
						if (nodeS->first_node("addUnit")->first_node("id")) {
							s.addUnit_id = toInt(nodeS->first_node("addUnit")->first_node("id")->value());
						}
						if (nodeS->first_node("addUnit")->first_node("number")) {
							s.addUnit_number = toInt(nodeS->first_node("addUnit")->first_node("number")->value());
						}
					}
					if (nodeS->first_node("hitFrameNumber")) {
						s.hitFrameNumber = toInt(nodeS->first_node("hitFrameNumber")->value());
					}
					if (nodeS->first_node("friendly")) {
						s.friendly = true;
					}
					if (nodeS->first_node("icon")) {
						s.icon = new Texture2D(trim(nodeS->first_node("icon")->value()).data(), false);
					}
					if (nodeS->first_node("rechargeTime")) {
						s.rechargeTime = toInt(nodeS->first_node("rechargeTime")->value());
					}
					if (nodeS->first_node("neededLevel")) {
						s.neededLevel = toInt(nodeS->first_node("neededLevel")->value());
					}
					unitTypeInfo[id].specialPower.push_back(s);
				}

				if (node->first_node("particleEngine")) {
					xml_node<> *nodeP = node->first_node("particleEngine");
					string texture = trim(nodeP->first_node("texture")->value());
					unitTypeInfo[id].particleTexture = new Texture2D(texture.data(), false);
					unitTypeInfo[id].particleEngine = new ParticleEngine();
					int lifespan = toInt(nodeP->first_node("lifespan")->value());
					float hAngleMin = toFloat(nodeP->first_node("hAngle")->first_node("min")->value());
					float hAngleMax = toFloat(nodeP->first_node("hAngle")->first_node("max")->value());
					float vAngleMin = toFloat(nodeP->first_node("vAngle")->first_node("min")->value());
					float vAngleMax = toFloat(nodeP->first_node("vAngle")->first_node("max")->value());
					float size = toFloat(nodeP->first_node("size")->value());
					float dSize = toFloat(nodeP->first_node("dSize")->value());
					float velocityMin = toFloat(nodeP->first_node("velocity")->first_node("min")->value());
					float velocityMax = toFloat(nodeP->first_node("velocity")->first_node("max")->value());
					float weight = toFloat(nodeP->first_node("weight")->value());
					int rotation = toInt(nodeP->first_node("rotation")->value());
					float r = toFloat(nodeP->first_node("color")->first_node("r")->value());
					float g = toFloat(nodeP->first_node("color")->first_node("g")->value());
					float b = toFloat(nodeP->first_node("color")->first_node("b")->value());
					float density = toFloat(nodeP->first_node("density")->value());
					unitTypeInfo[id].particleEngine->set(
						Point3D(),
						lifespan,
						unitTypeInfo[id].particleTexture, 1,
						Range<float>(hAngleMin, hAngleMax),
						Range<float>(vAngleMin, vAngleMax),
						size, dSize,
						Range<float>(velocityMin, velocityMax),
						weight,
						rotation,
						Color(r, g, b, 1),
						density
						);
				}
			}
		} catch (parse_error &e) {
			throw Exception("XML Parse Error in file " + filename);
		}

		//Load objects that are not saved as memory dump, save them & unload
		/*for (UnitType type = 0; type < MAX_OBJECT_TYPE; type++) {
			if (!fileExists(unitTypeInfo[type].unitPath)) {
				try {
					unitTypeInfo[type].load(true);
					unitTypeInfo[type].unload();
				} catch (NoUnitDataException &e) {
					// do nothing
				}
			}
		}*/
	}

	void loadCursor() {
		cursorGeneral.load("cursor/cursor.png");
		cursorDrag.load("cursor/drag.png");
		cursorTarget.load("cursor/target.png");
		cursorUp.load("cursor/up.png");
		cursorDown.load("cursor/down.png");
	}

	void unloadCampaignVideo() {
		try {
			if (campaignMenuData.video.videoCapture != NULL) {
				cvReleaseCapture(&campaignMenuData.video.videoCapture);
				campaignMenuData.video.videoCapture = NULL;
			}
		} catch (cv::Exception &e) {
			getLogger().print("unloadCampaignVideo() : " + e.msg);
		}
	}
	void unloadCampaignMenuData() {
		try {
			for (int i = 0; i < campaignMenuData.data.size(); i++) {
				if (campaignMenuData.data[i].image != NULL) {
					delete campaignMenuData.data[i].image;
					campaignMenuData.data[i].image = NULL;
				}
			}
			campaignMenuData.data.clear();
		} catch (cv::Exception &e) {
			getLogger().print("unloadCampaignMenuData() : " + e.msg);
		}
	}
	void loadCampaignMenuData() {
		unloadCampaignMenuData();
		string filename = "data/campaign.xml";
		string text = readTextFile(filename.data());
		xml_document<char> doc;
		doc.parse<0>((char*)text.data());
		int chapterNo = 0;
		try {
			for (xml_node<char> *node = doc.first_node("campaign")->first_node("scenario"); node; node = node->next_sibling("scenario")) {
				CampaignMenuData::Data data;
				if (node->first_node("unlocked")) {
					data.unlocked = true;
				}
#ifdef _DEMO
				if (chapterNo > 1) {
					data.unlocked = false;
				}
#endif
				data.buttonColor.r(toFloat(node->first_node("buttonColor")->first_node("r")->value()));
				data.buttonColor.g(toFloat(node->first_node("buttonColor")->first_node("g")->value()));
				data.buttonColor.b(toFloat(node->first_node("buttonColor")->first_node("b")->value()));
				data.name = sqeeze(node->first_node("name")->value());
				data.path = trim(node->first_node("path")->value());
				if (node->first_node("intro")) {
					data.intro = trim(node->first_node("intro")->value());
				}
				if (node->first_node("outro")) {
					data.outro = trim(node->first_node("outro")->value());
				}
				campaignMenuData.data.push_back(data);
				chapterNo++;
			}
		} catch (parse_error &e) {
			throw Exception("XML Parse Error in file " + filename);
		}
		try {
			for (int chapterNo = 0; chapterNo < campaignMenuData.data.size(); chapterNo++) {
				if (campaignMenuData.data[chapterNo].unlocked) {
					string fname = "ui/campaign_icon/" + toString(chapterNo + 1) + ".png";
					campaignMenuData.data[chapterNo].image = new Texture2D(fname.data(), true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				}
			}
		} catch (FileNotFoundException &ex) {
			showMessage(ex.getMessage(), "Campaign icon missing...", true);
		}
	}
	void saveCampaignMenuData() {
		string filepath = "data/campaign.xml";
		xml_document<char> doc;

		doc.append_node(doc.allocate_node(node_element, "campaign"));

		for (int i = 0; i < campaignMenuData.data.size(); i++) {
			xml_node<char> *node = doc.allocate_node(node_element, "scenario");
			doc.first_node("campaign")->append_node(node);
			node->append_node(doc.allocate_node(node_element, "buttonColor"));
			node->first_node("buttonColor")->append_node(doc.allocate_node(node_element, "r", doc.allocate_string(toString(campaignMenuData.data[i].buttonColor.r()).data())));
			node->first_node("buttonColor")->append_node(doc.allocate_node(node_element, "g", doc.allocate_string(toString(campaignMenuData.data[i].buttonColor.g()).data())));
			node->first_node("buttonColor")->append_node(doc.allocate_node(node_element, "b", doc.allocate_string(toString(campaignMenuData.data[i].buttonColor.b()).data())));
			node->append_node(doc.allocate_node(node_element, "name", campaignMenuData.data[i].name.data()));
			node->append_node(doc.allocate_node(node_element, "path", campaignMenuData.data[i].path.data()));
			if (!campaignMenuData.data[i].intro.empty()) {
				node->append_node(doc.allocate_node(node_element, "intro", campaignMenuData.data[i].intro.data()));
			}
			if (!campaignMenuData.data[i].outro.empty()) {
				node->append_node(doc.allocate_node(node_element, "outro", campaignMenuData.data[i].outro.data()));
			}
			if (campaignMenuData.data[i].unlocked) {
				node->append_node(doc.allocate_node(node_element, "unlocked"));
			}
		}

		string text;
		print(back_inserter(text), doc, 0);
		ofstream file(filepath.data());
		file << text;
		file.close();
	}
	void unlockCampaign(int campaignIndex) {
		loadCampaignMenuData();
		if (campaignIndex - 1 < campaignMenuData.data.size()) {
			campaignMenuData.data[campaignIndex - 1].unlocked = true;
		}
		saveCampaignMenuData();

		//steam achievements
		if (getSteamAchievements()) {
			if (campaignIndex >= 2)
				getSteamAchievements()->SetAchievement("ACH_CMPN_2");
			if (campaignIndex >= 5)
				getSteamAchievements()->SetAchievement("ACH_CMPN_5");
			if (campaignIndex >= 10)
				getSteamAchievements()->SetAchievement("ACH_CMPN_10");
			if (campaignIndex >= 12)
				getSteamAchievements()->SetAchievement("ACH_CMPN_12");
			if (campaignIndex >= 17)
				getSteamAchievements()->SetAchievement("ACH_CMPN_17");
			if (campaignIndex >= 20)
				getSteamAchievements()->SetAchievement("ACH_CMPN_20");
		}
	}

	void loadSkirmishMapHeader() {
		static int lastIndex = -1;
		int index = skirmishMenuData.mouseOverMapIndex;
		if (index == -1) {
			index = skirmishMenuData.selectedMapIndex;
		}
		if (lastIndex == index) {
			return;
		}
		try {
			if (skirmishMenuData.selectedMapImage != NULL) {
				delete skirmishMenuData.selectedMapImage;
			}
			skirmishMenuData.selectedMapImage = new Texture2D();
			Game::getMapHeader("map/" + skirmishMenuData.selectedMapName[index], skirmishMenuData.gameStartPosition, *skirmishMenuData.selectedMapImage, skirmishMenuData.mapWidth, skirmishMenuData.mapHeight);
			skirmishMenuData.startPositionTeam.resize(skirmishMenuData.gameStartPosition.size(), -1);
			skirmishMenuData.nTeams = (skirmishMenuData.nTeams>skirmishMenuData.gameStartPosition.size()) ? skirmishMenuData.gameStartPosition.size() : skirmishMenuData.nTeams;
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Loading Error : loadSkirmishMapHeader()", true);
			getLogger().print("Loading Error : loadSkirmishMapHeader() : " + e.getMessage(), LOG_SEVERE);
			skirmishMenuData.selectedMapImage = NULL;
			skirmishMenuData.gameStartPosition.clear();
			skirmishMenuData.startPositionTeam.clear();
		}
		lastIndex = index;
	}
	void loadSkirmishMenuData() {
		skirmishMenuData.selectedMapName.clear();
		string mapDirName = "map";
		DIR *mapdir = opendir(mapDirName.data());
		if (mapdir != NULL) {
			for (dirent *mapdirent; (mapdirent = readdir(mapdir)) != NULL;)
			if (getExtension((string)mapdirent->d_name) == mapDirName)
				skirmishMenuData.selectedMapName.push_back((string)mapdirent->d_name);
			closedir(mapdir);
		}
		if (skirmishMenuData.selectedMapIndex<0 || skirmishMenuData.selectedMapIndex>skirmishMenuData.selectedMapName.size())
			skirmishMenuData.selectedMapIndex = skirmishMenuData.mapScrollOffset = 0;
	}

	void loadSavedGameList() {
		savedGameMenuData.savedGameList = getFiles("save");
		quicksort(savedGameMenuData.savedGameList.data(), savedGameMenuData.savedGameList.size(), SORT_DESCENDING(string));
		savedGameMenuData.selectedIndex = clamp(savedGameMenuData.selectedIndex, 0, savedGameMenuData.savedGameList.size());
		savedGameMenuData.scrollOffset = clamp(savedGameMenuData.scrollOffset, 0, savedGameMenuData.savedGameList.size());
	}

	void loadInitialResource() {
		texture_video_frame.load("ui/video_frame.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		texture_select.load("ui/select.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		logo.load("ui/logo.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		image_loading_text.load("ui/loading_text.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		image_loading.load("ui/loading.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
	}
	void loadCurrentTemporaryResource(GameState state) {
		try {
			switch (state) {
			case GAMESTATE_LOADING_GAME_PHASE1:
			case GAMESTATE_LOADING_GAME_PHASE2:
			case GAMESTATE_LOADING_MAP_PHASE1:
			case GAMESTATE_LOADING_MAP_PHASE2:
				image_loading_text.load("ui/loading_text.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				image_loading.load("ui/loading.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_MAINMENU:
				icon_quit.load("ui/quit.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_campaign.load("ui/campaign.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_skirmish.load("ui/skirmish.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_load.load("ui/load.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_display_settings.load("ui/display_settings.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_audio_settings.load("ui/audio_settings.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_game_settings.load("ui/game_settings.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_network.load("ui/network.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_editor.load("ui/editor.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_credits.load("ui/credits_icon.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_controls.load("ui/controls_icon.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_SETTINGSMENU_DISPLAY:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				texture_slidebar.load("ui/slidebar.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				texture_slider.load("ui/slider.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				texture_slidebar_gap.load("ui/slidebar_gap.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_resolution.load("ui/resolution.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_video.load("ui/video.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_reflection_on.load("ui/reflection_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_reflection_off.load("ui/reflection_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_shadow_on.load("ui/shadow_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_shadow_off.load("ui/shadow_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_antialiasing_on.load("ui/antialiasing_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_antialiasing_off.load("ui/antialiasing_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_stereoscopic_on.load("ui/stereoscopic_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_stereoscopic_off.load("ui/stereoscopic_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_fullscreen_on.load("ui/fullscreen_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_fullscreen_off.load("ui/fullscreen_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_terrain_bumpmap_off.load("ui/terrain_bumpmap_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_terrain_bumpmap_on.load("ui/terrain_bumpmap_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_object_bumpmap_off.load("ui/object_bumpmap_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_object_bumpmap_on.load("ui/object_bumpmap_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_depthoffield_off.load("ui/depthoffield_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_depthoffield_on.load("ui/depthoffield_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_motion_blur_on.load("ui/motion_blur_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_motion_blur_off.load("ui/motion_blur_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_bloom_on.load("ui/bloom_on.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_bloom_off.load("ui/bloom_off.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_SETTINGSMENU_AUDIO:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				texture_slidebar.load("ui/slidebar.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				texture_slider.load("ui/slider.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_volume.load("ui/volume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_music_volume.load("ui/music_volume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_sound_volume.load("ui/sound_volume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_ambient_volume.load("ui/ambient_volume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_SETTINGSMENU_GAME:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				texture_slidebar.load("ui/slidebar.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				texture_slider.load("ui/slider.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_scrollspeed.load("ui/scrollspeed.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_CAMPAIGNMENU:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_startposition.load("ui/start_position.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				loadCampaignMenuData();
				break;
			case GAMESTATE_CHAPTER_SCREEN:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_resume.load("ui/resume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_video.load("ui/video.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_SKIRMISHMENU:
			case GAMESTATE_HOSTGAMEMENU:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				texture_button_color.load("ui/button_color.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				texture_button_medium.load("ui/button_medium.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				texture_button_wide.load("ui/button_wide.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_add.load("ui/add.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_remove.load("ui/remove.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_startposition.load("ui/start_position.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_startposition_empty.load("ui/start_position_empty.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_resume.load("ui/resume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				loadSkirmishMenuData();
				break;
			case GAMESTATE_NETWORKMENU:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_hostgame.load("ui/hostgame.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_joingame.load("ui/joingame.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_JOINGAMEMENU:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			case GAMESTATE_EDITORMENU:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_startposition_empty.load("ui/start_position_empty.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_resume.load("ui/resume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_editmap.load("ui/editmap.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_newmap.load("ui/newmap.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				texture_slidebar.load("ui/slidebar.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				texture_slider.load("ui/slider.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_width.load("ui/width.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_height.load("ui/height.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_ground_height.load("ui/ground_height.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				loadSkirmishMenuData();
				break;
			case GAMESTATE_SAVEGAMESNAPSHOT_MENU:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_resume.load("ui/resume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_delete.load("ui/delete.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_yes.load("ui/yes.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_no.load("ui/no.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				loadSavedGameList();
				break;
			case GAMESTATE_CREDITS_SCREEN:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				image_credits.load("ui/credits.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP, 2048, 2048);
				break;
			case GAMESTATE_CONTROLS_SCREEN:
				icon_back.load("ui/back.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				image_controls.load("ui/controls.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP, 2048, 2048);
				break;
			case GAMESTATE_PLAYING:
				texture_game_frame.load("ui/frame.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_error.load("ui/error.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_pause.load("ui/pause.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_resume.load("ui/resume.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_quit.load("ui/quit.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_restart.load("ui/restart.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_display_settings.load("ui/display_settings.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_audio_settings.load("ui/audio_settings.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_game_settings.load("ui/game_settings.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_wood.load("ui/wood.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_food.load("ui/food.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_stone.load("ui/stone.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_metal.load("ui/metal.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_population.load("ui/population.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_hitpoint.load("ui/hitpoint.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_healrate.load("ui/healrate.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_attack.load("ui/attack.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_siegeattack.load("ui/siege_attack.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_armor.load("ui/armour.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_speed.load("ui/speed.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_accuracy.load("ui/accuracy.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_blend.load("ui/blend.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_hero.load("ui/hero.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_fire_resistance.load("ui/fire.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				image_blank_button.load("ui/blank.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_save.load("ui/save.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_inr_height.load("ui/inr_height.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_dcr_height.load("ui/dcr_height.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_terrain_texture.load("ui/terrain_texture.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_add_unit.load("ui/add_unit.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_add_building.load("ui/add_building.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_add_others.load("ui/add_others.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_change_team.load("ui/change_team.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				icon_create.load("ui/create.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				icon_deploy.load("ui/deploy.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);

				image_victory.load("ui/victory.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				image_defeat.load("ui/defeat.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				break;
			}
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingSavedGamePhase1()", true);
			getLogger().print("Loading Error : loadingSavedGamePhase1() : " + e.getMessage(), LOG_SEVERE);
		}
	}
	void unloadAllTemporaryResource() {
		unloadCampaignVideo();

		if (campaignMenuData.storyImage != NULL) {
			delete campaignMenuData.storyImage;
			campaignMenuData.storyImage = NULL;
		}
		if (campaignMenuData.nameImage != NULL) {
			delete campaignMenuData.nameImage;
			campaignMenuData.nameImage = NULL;
		}

		texture_game_frame.unload();
		image_blank_button.unload();
		icon_campaign.unload();
		icon_skirmish.unload();
		icon_display_settings.unload();
		icon_audio_settings.unload();
		icon_game_settings.unload();
		icon_network.unload();
		icon_editor.unload();
		icon_quit.unload();
		icon_back.unload();
		icon_pause.unload();
		icon_error.unload();
		icon_load.unload();
		icon_credits.unload();
		icon_controls.unload();
		icon_delete.unload();
		icon_hostgame.unload();
		icon_joingame.unload();
		icon_yes.unload();
		icon_no.unload();
		icon_video.unload();

		texture_button_color.unload();
		texture_button_medium.unload();
		texture_button_wide.unload();
		texture_slidebar.unload();
		texture_slider.unload();
		texture_slidebar_gap.unload();

		icon_resolution.unload();
		icon_volume.unload();
		icon_music_volume.unload();
		icon_sound_volume.unload();
		icon_ambient_volume.unload();
		icon_height.unload();
		icon_width.unload();
		icon_ground_height.unload();

		icon_scrollspeed.unload();

		icon_reflection_on.unload();
		icon_reflection_off.unload();
		icon_shadow_on.unload();
		icon_shadow_off.unload();
		icon_antialiasing_on.unload();
		icon_antialiasing_off.unload();
		icon_stereoscopic_on.unload();
		icon_stereoscopic_off.unload();
		icon_fullscreen_on.unload();
		icon_fullscreen_off.unload();
		icon_depthoffield_on.unload();
		icon_depthoffield_off.unload();
		icon_motion_blur_on.unload();
		icon_motion_blur_off.unload();
		icon_bloom_on.unload();
		icon_bloom_off.unload();

		icon_resume.unload();
		icon_restart.unload();

		icon_editmap.unload();
		icon_newmap.unload();
		icon_save.unload();
		icon_inr_height.unload();
		icon_dcr_height.unload();
		icon_terrain_texture.unload();
		icon_add_unit.unload();
		icon_add_building.unload();
		icon_add_others.unload();
		icon_change_team.unload();

		icon_add.unload();
		icon_remove.unload();
		icon_startposition.unload();
		icon_startposition_empty.unload();
		icon_deploy.unload();
		icon_create.unload();

		icon_wood.unload();
		icon_food.unload();
		icon_stone.unload();
		icon_metal.unload();
		icon_population.unload();

		icon_hitpoint.unload();
		icon_healrate.unload();
		icon_attack.unload();
		icon_siegeattack.unload();
		icon_armor.unload();
		icon_speed.unload();
		icon_accuracy.unload();
		icon_fire_resistance.unload();

		image_victory.unload();
		image_defeat.unload();

		image_credits.unload();
		image_controls.unload();

		image_loading.unload();
		image_loading_text.unload();
	}

	void setSound(GameState oldState, GameState newState) {
		vector<string> files;
		switch (newState) {
		case GAMESTATE_LOADING_GAME_PHASE1:
			break;
		case GAMESTATE_LOADING_MAP_PHASE1:
			stopAudio(CHANNEL_MUSIC);
			files = getFilePaths("audio/music/combat");
			playAudio(randomVectorElement(files), CHANNEL_MUSIC);
			break;
		case GAMESTATE_PLAYING:
			break;
		case GAMESTATE_MAINMENU:
			if (oldState == GAMESTATE_PLAYING || oldState == GAMESTATE_LOADING_GAME_PHASE2) {
				stopAudio(CHANNEL_MUSIC);
				files = getFilePaths("audio/music/menu");
				playAudio(randomVectorElement(files), CHANNEL_MUSIC);
			}
			break;
		}
		Game::setSoundSettings();
	}

	//------------------------------------------------------------------------------------------------Video Background function/data

	static struct VideoBackgroundData {
		CvCapture* videoCapture;
		Texture2D* frameTexture;
		string lastPlayedVideo;
		int nFrames, frameNumber;
		VideoBackgroundData() :
			videoCapture(NULL), frameTexture(NULL),
			lastPlayedVideo(""), nFrames(0), frameNumber(0){}
	} videoBackgroundData;

	void unloadVideo() {
		//release capture
		if (videoBackgroundData.videoCapture != NULL) {
			cvReleaseCapture(&videoBackgroundData.videoCapture);
			videoBackgroundData.videoCapture = NULL;
		}
		//unload textures
		if (videoBackgroundData.frameTexture != NULL) {
			delete videoBackgroundData.frameTexture;
			videoBackgroundData.frameTexture = NULL;
		}
	}
	void drawVideoBackground(string dir, bool loop) {
		bool newVideo = !startsWith(videoBackgroundData.lastPlayedVideo, dir);
		bool replay = ((loop && videoBackgroundData.frameNumber >= videoBackgroundData.nFrames) || videoBackgroundData.videoCapture == NULL);
		if (newVideo) {
			//unload video
			if (videoBackgroundData.videoCapture != NULL) {
				unloadVideo();
			}
			//find random video
			vector<string> files = getFilePaths(dir + "/" + Game::Settings::videoHeight);
			if (files.empty()) {
				throw Exception("directory " + dir + " is empty.");
			}
			videoBackgroundData.lastPlayedVideo = randomVectorElement(files);
		}
		if (newVideo || replay) {
			//release old capture
			if (videoBackgroundData.videoCapture != NULL) {
				cvReleaseCapture(&videoBackgroundData.videoCapture);
			}
			//load new capture
			videoBackgroundData.videoCapture = cvCreateFileCapture(videoBackgroundData.lastPlayedVideo.data());
			videoBackgroundData.nFrames = (int)cvGetCaptureProperty(videoBackgroundData.videoCapture, CV_CAP_PROP_FRAME_COUNT);
			videoBackgroundData.frameNumber = 0;
			//play audio
			if (videoBackgroundData.videoCapture != NULL) {
				string audio_track = dir + "/audio_track/" + removeFilePath(removeExtension(videoBackgroundData.lastPlayedVideo)) + ".ogg";
				if (fileExists(audio_track)) {
					playAudio(audio_track, CHANNEL_MUSIC);
					Game::setSoundSettings();
				}
			} else {
				getLogger().print("drawVideoBackground() : Video load error (may be opencv_ffmpeg runtime library missing)", LOG_SEVERE);
			}
		}

		//render
		if (videoBackgroundData.videoCapture != NULL) {
			//get frame
			IplImage* frame = NULL;
			if (videoBackgroundData.frameNumber < videoBackgroundData.nFrames) {
				frame = cvQueryFrame(videoBackgroundData.videoCapture);
			}
			if (frame != NULL) {
				//make texture
				if (videoBackgroundData.frameTexture != NULL) {
					delete videoBackgroundData.frameTexture;
				}
				videoBackgroundData.frameTexture = new Texture2D();
				videoBackgroundData.frameTexture->make(frame, "videoBackground", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				//render frame
				glColor4f(1, 1, 1, 1);
				videoBackgroundData.frameTexture->bind();
				glDrawFullScreenRectangle();
				//next frame
				videoBackgroundData.frameNumber++;
				//sync
				videoBackgroundData.frameNumber += syncVideoToFrameRate(VIDEO_FRAMERATE, videoBackgroundData.videoCapture);
			} else { //loop disabled
				//render frame
				if (videoBackgroundData.frameTexture != NULL) {
					glColor4f(1, 1, 1, 1);
					videoBackgroundData.frameTexture->bind();
					glDrawFullScreenRectangle();
					//sync
					syncFrameRate(VIDEO_FRAMERATE);
				}
			}
		}

		//foreground texture
		glColor3f(1, 1, 1);
		texture_video_frame.bind();
		glDrawFullScreenRectangle();
	}


	//------------------------------------------------------------------------------------------------Game state change functions

	void setGameState(GameState state) {
		setSound(currentGameState, state);
		unloadAllTemporaryResource();
		if (currentGameState != state) {
			gameState.push(currentGameState);
			currentGameState = state;
		}
		loadCurrentTemporaryResource(currentGameState);
		confirmRemove = false;
		getLogger().print("Game state pushed " + gameStateAsString(currentGameState) + ".");
	}
	void backGameState() {
		GameState oldState = currentGameState;
		if (isSettingsMenuGameState(currentGameState)) {
			Game::saveSettings();
		} 
		do {
			currentGameState = gameState.top();
			gameState.pop();
		} while (isLoadingGameState(currentGameState) || currentGameState == GAMESTATE_INTRO);
		setSound(oldState, currentGameState);
		loadCurrentTemporaryResource(currentGameState);
		confirmRemove = false;
		getLogger().print("Game state popped " + gameStateAsString(currentGameState) + ".");
	}


	//------------------------------------------------------------------------------------------------Loading Threads

	int _SDL_THREAD loadingGamePhase1(void* unused = NULL) {
		if (loading) return 0;
		loading = true;
		loadingProgress = 0;

		try {
			sprintf(message, "Loading Objects");
			loadUnitInfo(message, loadingProgress);

			sprintf(message, "Loading User Interface");
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingGamePhase1()", true);
			getLogger().print("Loading Error : loadingGamePhase1() : " + e.getMessage(), LOG_SEVERE);
		}

		sprintf(message, "Creating Display Lists");
		setGameState(GAMESTATE_LOADING_GAME_PHASE2);

		return 1;
	}
	void loadingGamePhase2() {
		try {
			sprintf(message, "Initializing");
			loadCursor();
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingGamePhase2()", true);
			getLogger().print("Loading Error : loadingGamePhase2() : " + e.getMessage(), LOG_SEVERE);
		}

		loading = false;
		loadingProgress = 0;
		setGameState(GAMESTATE_MAINMENU);
	}

	template<typename GameType> void newGame() {
		Synchronizer sync(gameMutex);
		if (game != NULL) {
			delete game;
		}
		game = new GameType;
	}
	int getFreeStartPosition() {
		for (int s = 0; s<skirmishMenuData.startPositionTeam.size(); s++)
		if (skirmishMenuData.startPositionTeam[s]<0)
			return s;
		return -1;
	}
	int _SDL_THREAD loadingCampaignMapPhase1(void* unused = NULL) {
		loadingProgress = 0;
		sprintf(message, "Loading Map");
		newGame<Campaign>();
		try {
			((Campaign*)game)->load(campaignMenuData.data[campaignMenuData.selectedCampaignIndex].path, unitTypeInfo);
			((Campaign*)game)->unlockCampaign = &unlockCampaign;
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingCampaignMapPhase1()", true);
			getLogger().print("Loading Error : loadingCampaignMapPhase1() : " + e.getMessage(), LOG_SEVERE);
		}
		sprintf(message, "Initializing");
		setGameState(GAMESTATE_LOADING_MAP_PHASE2);
		return 1;
	}
	int _SDL_THREAD loadingSavedGamePhase1(void* unused = NULL) {
		loadingProgress = 0;
		sprintf(message, "Loading Map");
		try {
			if (getExtension(savedGameMenuData.savedGameList[savedGameMenuData.selectedIndex]) == "save") {
				newGame<Game>();
				game->loadSnapshot("save/" + savedGameMenuData.savedGameList[savedGameMenuData.selectedIndex], unitTypeInfo);
			} else {
				newGame<Campaign>();
				game->loadSnapshot("save/" + savedGameMenuData.savedGameList[savedGameMenuData.selectedIndex], unitTypeInfo);
				((Campaign*)game)->unlockCampaign = &unlockCampaign;
			}
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingSavedGamePhase1()", true);
			getLogger().print("Loading Error : loadingSavedGamePhase1() : " + e.getMessage(), LOG_SEVERE);
		}
		sprintf(message, "Initializing");
		setGameState(GAMESTATE_LOADING_MAP_PHASE2);
		return 1;
	}
	int _SDL_THREAD loadingMapPhase1(void* unused = NULL) {
		loadingProgress = 0;
		sprintf(message, "Loading Map");
		{
			Synchronizer sync(gameMutex);
			if (game != NULL) {
				delete game;
			}
			newGame<Game>();
		}
		try {
			game->load("map/" + skirmishMenuData.selectedMapName[skirmishMenuData.selectedMapIndex], unitTypeInfo, NULL, 0, 1, 500, 500, 500, 500);
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingMapPhase1()", true);
			getLogger().print("Loading Error : loadingMapPhase1() : " + e.getMessage(), LOG_SEVERE);
		}
		sprintf(message, "Initializing");
		setGameState(GAMESTATE_LOADING_MAP_PHASE2);
		return 1;
	}
	int _SDL_THREAD loadingSkirmishMapPhase1(void* unused = NULL) {
		loadingProgress = 0;
		sprintf(message, "Loading Map");
		for (int i = 0; i<skirmishMenuData.nTeams; i++)
			skirmishMenuData.teams[i].startPositionIndex = -1;
		for (int s = 0; s<skirmishMenuData.startPositionTeam.size(); s++)
		if (skirmishMenuData.startPositionTeam[s] >= 0)
			skirmishMenuData.teams[skirmishMenuData.startPositionTeam[s]].startPositionIndex = s;
		for (int i = 0; i<skirmishMenuData.nTeams; i++)
		if (skirmishMenuData.teams[i].startPositionIndex == -1) {
			skirmishMenuData.teams[i].startPositionIndex = getFreeStartPosition();
			skirmishMenuData.startPositionTeam[skirmishMenuData.teams[i].startPositionIndex] = i;
		}
		skirmishMenuData.playerTeam = 0;
		for (TeamID t = 1; t<skirmishMenuData.nTeams; t++) {
			if (skirmishMenuData.teams[t].human) {
				skirmishMenuData.playerTeam = t;
				break;
			}
		}
		newGame<Skirmish>();
		try {
			((Skirmish*)game)->load("map/" + skirmishMenuData.selectedMapName[skirmishMenuData.selectedMapIndex], 
				unitTypeInfo, 
				skirmishMenuData.teams, skirmishMenuData.nTeams, 
				skirmishMenuData.playerTeam, 
				500, 500, 500, 500);
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingSkirmishMapPhase1()", true);
			getLogger().print("Loading Error : loadingSkirmishMapPhase1() : " + e.getMessage(), LOG_SEVERE);
		}
		sprintf(message, "Initializing");
		setGameState(GAMESTATE_LOADING_MAP_PHASE2);
		return 1;
	}
	int _SDL_THREAD loadingNewMapPhase1(void* unused = NULL) {
		loadingProgress = 0;
		sprintf(message, "Loading Map");
		time_t currentTime = time(NULL);
		newMapData.filename = "map/"
			+ toString(1900 + localtime(&currentTime)->tm_year, 4) + "-"
			+ toString(localtime(&currentTime)->tm_mon, 2) + "-"
			+ toString(localtime(&currentTime)->tm_mday, 2) + "."
			+ toString(localtime(&currentTime)->tm_hour, 2) + "-"
			+ toString(localtime(&currentTime)->tm_min, 2) + "-"
			+ toString(localtime(&currentTime)->tm_sec, 2) + ".map";
		newGame<Game>();
		Team default_teams[Game::MAX_TEAM - 1] = { Team(1, true, 1, 1), Team(2, false, 2, 2), Team(3, false, 3, 3), Team(4, false, 4, 4), Team(5, true, 5, 5), Team(6, false, 6, 6), Team(7, false, 7, 7), Team(8, false, 8, 8) };
		game->create(newMapData.width, newMapData.height, newMapData.groundHeight, newMapData.filename, unitTypeInfo, skirmishMenuData.teams, 8, 1, 0, 0, 0, 0);
		console = true;
		consoleBuffer = "rename " + newMapData.filename;
		sprintf(message, "Initializing");
		setGameState(GAMESTATE_LOADING_MAP_PHASE2);
		return 1;
	}
	int _SDL_THREAD loadingEditMapPhase1(void* unused = NULL) {
		loadingProgress = 0;
		sprintf(message, "Loading Map");
		newGame<Game>();
		try {
			string fname = "map/" + skirmishMenuData.selectedMapName[skirmishMenuData.selectedMapIndex];
			game->load(fname, unitTypeInfo, NULL, 8, 1, 500, 500, 500, 500, true);
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingEditMapPhase1()", true);
			getLogger().print("Loading Error : loadingEditMapPhase1() : " + e.getMessage(), LOG_SEVERE);
		}
		sprintf(message, "Initializing");
		setGameState(GAMESTATE_LOADING_MAP_PHASE2);
		return 1;
	}

	void loadingMapPhase2() {
		try {
			game->init();
			if (game->getGamePlayTime() == 0) {//not saved game
				game->resume();
			}
		} catch (Exception& e) {
			showMessage(e.getMessage(), "Loading Error : loadingMapPhase2()", true);
			getLogger().print("Loading Error : loadingMapPhase2() : " + e.getMessage(), LOG_SEVERE);
		}
		setGameState(GAMESTATE_PLAYING);
	}


	//------------------------------------------------------------------------------------------------Cursor functions

	void displayToolTipText() {
		if (toolTipText == "") {
			return;
		}
		int w = glPrintWidth(GLUT_BITMAP_HELVETICA_12, toolTipText.data());
		if (w>Game::Settings::screenWidth - mouseX) {
			glColor4f(0, 0.1, 0, 0.6);
			glDrawRectangle(mouseX - w - TOOL_TIP_LEFT - TOOL_TIP_SIDE_BORDER, mouseY + TOOL_TIP_BOTTOM + TOOL_TIP_BOTTOM_BORDER, mouseX - TOOL_TIP_LEFT + TOOL_TIP_SIDE_BORDER, mouseY + TOOL_TIP_BOTTOM - TOOL_TIP_HEIGHT);
			glColor4f(1, 1, 1, 0.8);
			glPrint(mouseX - w - TOOL_TIP_LEFT, mouseY + TOOL_TIP_BOTTOM, GLUT_BITMAP_HELVETICA_12) << toolTipText;
		} else {
			glColor4f(0, 0.1, 0, 0.6);
			glDrawRectangle(mouseX + TOOL_TIP_LEFT - TOOL_TIP_SIDE_BORDER, mouseY + TOOL_TIP_BOTTOM + TOOL_TIP_BOTTOM_BORDER, mouseX + w + TOOL_TIP_LEFT + TOOL_TIP_SIDE_BORDER, mouseY + TOOL_TIP_BOTTOM - TOOL_TIP_HEIGHT);
			glColor4f(1, 1, 1, 0.8);
			glPrint(mouseX + TOOL_TIP_LEFT, mouseY + TOOL_TIP_BOTTOM, GLUT_BITMAP_HELVETICA_12) << toolTipText;
		}
		toolTipText = "";
	}
	void displayCursor() {
		glEnable(GL_BLEND);
		displayToolTipText();
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1);
		if (dragbox.area()<25) {
			if (currentGameState == GAMESTATE_PLAYING) {
				switch (game->cursor.type) {
				case Game::Cursor::NONE:
					switch (game->editState) {
					case MAPEDITSTATE_DECREASE_HEIGHT:
						cursorDown.bind();
						break;
					case MAPEDITSTATE_INCREASE_HEIGHT:
						cursorUp.bind();
						break;
					default:
						cursorGeneral.bind();
						break;
					}
					break;
				case Game::Cursor::BUILD:
					cursorDrag.bind();
					break;
				case Game::Cursor::TARGET:
					cursorTarget.bind();
					break;
				}
			} else {
				cursorGeneral.bind();
			}
		} else {
			cursorDrag.bind();
		}
		glDrawRectangle(mouseX - CURSOR_SIZE / 2, mouseY - CURSOR_SIZE / 2, mouseX + CURSOR_SIZE / 2, mouseY + CURSOR_SIZE / 2, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
	}
	void drawDragbox() {
		if (dragboxEnabled) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (dragbox.area()>0) {
				glColor4f(0.8, 0.9, 1.0, 0.1);
				glDrawRectangle(dragbox.x1, dragbox.y1, dragbox.x2, dragbox.y2);
				glColor4f(0.8, 0.9, 1.0, 1);
				glDrawWireRectangle(dragbox.x1, dragbox.y1, dragbox.x2, dragbox.y2);
			}
		}
	}


	//------------------------------------------------------------------------------------------------Button, Slibebar


	struct Button {
		static void draw(Texture2D &icon, string text, int left, int top, int w, int h, Color color = Color(1, 1, 1, 1)) {
			float colorBuffer[4];
			glColor4fv(color.rgba().toArray(colorBuffer));
			icon.bind();
			glDrawRectangle(left, top, left + w, top + h);

			if (Point2Di(mouseX, mouseY).in(left, top, left + w, top + h)) {
				if (text != "") toolTipText = text;
				float glow = 1.0 - clamp((Point2D(mouseX, mouseY) - Point2D(left + w / 2, top + h / 2)).hypot() / min(w / 2, h / 2), 0.0, 1.0);
				glColor4f(1, 1, 1, glow);
				texture_select.bind();
				glDrawRectangle(mouseX - w, mouseY - w, mouseX + w, mouseY + w);
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		static bool clicked(int button, int state, int x, int y, int left, int top, int width, int height) {
			bool reallyClicked = state == GLUT_UP
				&& button == GLUT_LEFT_BUTTON
				&& Point2Di(x, y).in(left, top, left + width, top + height);
			if (reallyClicked) {
				playAudio("audio/click.ogg", CHANNEL_SOUND);
			}
			return reallyClicked;
		}
	};
	struct MenuButton : Button {
		static void draw(Texture2D &tex, string text, int &left, int &top, bool side) {
			Button::draw(tex, text, left, top, MAIN_MENU_BUTTON_WIDTH, MAIN_MENU_BUTTON_HEIGHT);
			if (side) {
				left += MAIN_MENU_BUTTON_WIDTH + MAIN_MENU_BUTTON_GAP;
			} else {
				left = MAIN_MENU_BUTTON_LEFT, top += MAIN_MENU_BUTTON_HEIGHT + MAIN_MENU_BUTTON_GAP;
			}
		}
		static bool clicked(int button, int state, int x, int y, int &left, int &top, bool side) {
			bool reallyClicked = state == GLUT_UP
				&& button == GLUT_LEFT_BUTTON
				&& Point2Di(x, y).in(left, top, left + MAIN_MENU_BUTTON_WIDTH, top + MAIN_MENU_BUTTON_HEIGHT);
			if (side) {
				left += MAIN_MENU_BUTTON_WIDTH + MAIN_MENU_BUTTON_GAP;
			} else {
				left = MAIN_MENU_BUTTON_LEFT, top += MAIN_MENU_BUTTON_HEIGHT + MAIN_MENU_BUTTON_GAP;
			}
			if (reallyClicked) {
				playAudio("audio/click.ogg", CHANNEL_SOUND);
			}
			return reallyClicked;
		}
	};
	struct BackButton : Button {
		static void draw(Texture2D &icon = icon_back, string text = "Back") {
			Button::draw(icon, text, MENU_BACK_BUTTON_LEFT, MENU_BACK_BUTTON_TOP, MENU_BACK_BUTTON_WIDTH, MENU_BACK_BUTTON_HEIGHT);
		}
		static bool clicked(int button, int state, int x, int y) {
			bool reallyClicked =
				button == GLUT_LEFT_BUTTON
				&& state == GLUT_UP
				&& Point2Di(x, y).in(MENU_BACK_BUTTON_LEFT, MENU_BACK_BUTTON_TOP, MENU_BACK_BUTTON_LEFT + MENU_BACK_BUTTON_WIDTH, MENU_BACK_BUTTON_TOP + MENU_BACK_BUTTON_HEIGHT);
			if (reallyClicked) {
				playAudio("audio/click.ogg", CHANNEL_SOUND);
			}
			return reallyClicked;
		}
	};

	struct ToggleButton : Button {
		static void draw(Texture2D &tex_enabled, Texture2D &tex_disabled, string text, int left, int top, int w, int h, bool enabled = true, bool vertical = false, Color color = Color(1, 1, 1, 1)) {
			if (enabled) {
				tex_enabled.bind();
			} else {
				tex_disabled.bind();
				color = Color(color.r() / 2, color.g() / 2, color.b() / 2, 1.0);
			}
			float colorBuffer[4];
			glColor4fv(color.rgba().toArray(colorBuffer));
			glDrawRectangle(left, top, left + w, top + h);
			glBindTexture(GL_TEXTURE_2D, 0);
			if (Point2Di(mouseX, mouseY).in(left, top, left + w, top + h)) {
				if (text != "") toolTipText = text;
				float glow = 1.0 - clamp((Point2D(mouseX, mouseY) - Point2D(left + w / 2, top + h / 2)).hypot() / min(w / 2, h / 2), 0.0, 1.0);
				glColor4f(1, 1, 1, glow);
				texture_select.bind();
				glDrawRectangle(mouseX - w, mouseY - w, mouseX + w, mouseY + w);
			}
			Texture2D::bindNone();
		}
	};
	struct MenuToggleButton : ToggleButton {
		static void draw(Texture2D &tex_on, Texture2D &tex_off, string text, int &left, int &top, bool enabled, bool side) {
			ToggleButton::draw(tex_on, tex_off, text + " : " + (enabled ? "on" : "off"), left, top, MAIN_MENU_BUTTON_WIDTH, MAIN_MENU_BUTTON_HEIGHT, enabled);
			if (side) {
				left += MAIN_MENU_BUTTON_WIDTH + MAIN_MENU_BUTTON_GAP;
			} else {
				left = MAIN_MENU_BUTTON_LEFT, top += MAIN_MENU_BUTTON_HEIGHT + MAIN_MENU_BUTTON_GAP;
			}
		}
	};


	/*					Slidebar														---
	*	||====||				  |		 |												 |
	*	||icon||||||||============|Slider|=============slidebar===================|||||| H
	*	||====||				  |		 |												 |
	*	|---h--|-h/2-|----------------------------------(w-h)---------------------|-h/2|---
	*		   |---------------------------------w-------------------------------------|
	*							  |--h/2-|
	*/
	struct SlideBar {
		static void draw(Texture2D &icon, float value, int left, int top, int w, int h, string text = "") {
			value = clamp(value, 0.0, 1.0);

			glColor3f(1, 1, 1);
			icon.bind();
			glDrawRectangle(left, top, left + h, top + h);

			left += h;

			texture_slidebar.bind();
			glDrawRectangle(left, top, left + w, top + h);

			int length = w - h, pos = h / 2 + value*length;
			texture_slider.bind();
			glDrawRectangle(left + pos - h / 4, top, left + pos + h / 4, top + h);

			if (Point2Di(mouseX, mouseY).in(left + pos - h / 4, top, left + pos + h / 4, top + h)) {
				glColor4f(1, 1, 1, 0.5);
				texture_select.bind();
				glDrawRectangle(left + pos - h, top - h / 2, left + pos + h, top + 3 * h / 2);
			}

			Texture2D::bindNone();
		}
	};
	struct MenuSlideBar : SlideBar {
		static void draw(Texture2D &icon, string text, string value, const string list[], int nItems, int &left, int &top, int w = MENU_SLIDEBAR_WIDTH) {
			int index = find(list, nItems, value);
			float fValue = float(index) / float(nItems - 1);
			glColor3f(1, 1, 1);
			for (int i = 0; i < nItems; i++) {
				texture_slidebar_gap.bind();
				glDrawRectangle(left + MENU_SLIDEBAR_HEIGHT + i*(w - MENU_SLIDEBAR_HEIGHT) / (nItems - 1), top,
					left + 2 * MENU_SLIDEBAR_HEIGHT + i*(w - MENU_SLIDEBAR_HEIGHT) / (nItems - 1), top + MENU_SLIDEBAR_HEIGHT);
			}
			SlideBar::draw(icon, fValue, left, top, w, MENU_SLIDEBAR_HEIGHT, text);
			if (Point2Di(mouseX, mouseY).in(left + MENU_SLIDEBAR_HEIGHT, top, left + w, top + MENU_SLIDEBAR_HEIGHT))
				toolTipText = text + " : " + value;
			glColor3f(1, 1, 1);
			glPrint(left + 3 * MENU_SLIDEBAR_HEIGHT / 2, top, GLUT_BITMAP_HELVETICA_12) << list[0];
			glPrint(left + 3 * MENU_SLIDEBAR_HEIGHT / 2 + (MENU_SLIDEBAR_WIDTH + MENU_SLIDEBAR_HEIGHT) / 2, top, GLUT_BITMAP_HELVETICA_12) << list[nItems / 2];
			glPrint(left + 3 * MENU_SLIDEBAR_HEIGHT / 2 + (MENU_SLIDEBAR_WIDTH + MENU_SLIDEBAR_HEIGHT), top, GLUT_BITMAP_HELVETICA_12) << list[nItems - 1];
			glPrint(left + w + MENU_SLIDEBAR_HEIGHT, top + MENU_SLIDEBAR_HEIGHT / 2, GLUT_BITMAP_HELVETICA_12) << value;
			top += MENU_SLIDEBAR_HEIGHT + MENU_SLIDEBAR_VGAP;
		}
		static void action(int x, int y, string &value, const string list[], int nItems, int &left, int &top, void(*callback)() = NULL, string sound = "audio/click.ogg", ChannelTag channelGrp = CHANNEL_SOUND, int w = MENU_SLIDEBAR_WIDTH) {
			int index = find(list, nItems, value);
			float fValue = float(index) / float(nItems - 1);
			if (Point2Di(x, y).in(left + 3 * MENU_SLIDEBAR_HEIGHT / 2, top, left + w + 2 * MENU_SLIDEBAR_HEIGHT, top + MENU_SLIDEBAR_HEIGHT)) {
				fValue = clamp(float(x - left - 3 * MENU_SLIDEBAR_HEIGHT / 2) / float(w - 3 * MENU_SLIDEBAR_HEIGHT / 2), 0.0, 1.0);
				if (callback) {
					(*callback)();
				}
				playAudio(sound, channelGrp);
			}
			value = list[roundInt(fValue*(nItems - 1))];
			top += MENU_SLIDEBAR_HEIGHT + MENU_SLIDEBAR_VGAP;
		}
		static void draw(Texture2D &icon, string text, float value, int &left, int &top, int w = MENU_SLIDEBAR_WIDTH) {
			SlideBar::draw(icon, value, left, top, w, MENU_SLIDEBAR_HEIGHT, text);
			if (Point2Di(mouseX, mouseY).in(left + MENU_SLIDEBAR_HEIGHT, top, left + w, top + MENU_SLIDEBAR_HEIGHT)) {
				toolTipText = text + " : " + toString(value, 2);
			}
			top += MENU_SLIDEBAR_HEIGHT + MENU_SLIDEBAR_VGAP;
		}
		static void action(int x, int y, float &value, int &left, int &top, void(*callback)() = NULL, string sound = "audio/click.ogg", ChannelTag channelGrp = CHANNEL_SOUND, int w = MENU_SLIDEBAR_WIDTH) {
			if (Point2Di(x, y).in(left + 3 * MENU_SLIDEBAR_HEIGHT / 2, top, left + w + 2 * MENU_SLIDEBAR_HEIGHT, top + MENU_SLIDEBAR_HEIGHT)) {
				value = clamp(float(x - left - 3 * MENU_SLIDEBAR_HEIGHT / 2) / float(w - 3 * MENU_SLIDEBAR_HEIGHT / 2), 0.0, 1.0);
				if (callback) {
					(*callback)();
				}
				playAudio(sound, channelGrp);
			}
			top += MENU_SLIDEBAR_HEIGHT + MENU_SLIDEBAR_VGAP;
		}
		static void draw(Texture2D &icon, string text, int value, int min, int max, int &left, int &top, int w = MENU_SLIDEBAR_WIDTH) {
			SlideBar::draw(icon, float(value - min) / float(max - min), left, top, w, MENU_SLIDEBAR_HEIGHT, text);
			if (Point2Di(mouseX, mouseY).in(left + MENU_SLIDEBAR_HEIGHT, top, left + w, top + MENU_SLIDEBAR_HEIGHT)) {
				toolTipText = text + " : " + toString(value, 2);
			}
			top += MENU_SLIDEBAR_HEIGHT + MENU_SLIDEBAR_VGAP;
		}
		static void action(int x, int y, int &value, int min, int max, int &left, int &top, void(*callback)() = NULL, string sound = "audio/click.ogg", ChannelTag channelGrp = CHANNEL_SOUND, int w = MENU_SLIDEBAR_WIDTH) {
			if (Point2Di(x, y).in(left + 3 * MENU_SLIDEBAR_HEIGHT / 2, top, left + w + 2 * MENU_SLIDEBAR_HEIGHT, top + MENU_SLIDEBAR_HEIGHT)) {
				value = clamp(float(x - left - 3 * MENU_SLIDEBAR_HEIGHT / 2) / float(w - 3 * MENU_SLIDEBAR_HEIGHT / 2), 0.0, 1.0)*(max - min) + min;
				if (callback) {
					(*callback)();
				}
				playAudio(sound, channelGrp);
			}
			top += MENU_SLIDEBAR_HEIGHT + MENU_SLIDEBAR_VGAP;
		}
	};


	//------------------------------------------------------------------------------------------------Common functions

	void quit() {
		glutLeaveMainLoop();
		exit(EXIT_SUCCESS);
	}

	void loadGame() {
		loadingGameThread = SDL_CreateThread(loadingGamePhase1, NULL);
	}

	void saveGameSnapshot(string postfix = "") {
		game->setMessage("Saving Game...");
		time_t currentTime = time(NULL);
		string ext = ".save";
		if (game->isCampaign()) {
			ext = ".savecmpn";
		}
		string dir = "save";
		string prefix = "test game";
		if (game->isCampaign()) {
			prefix = "campaign";
		} else if (game->isSkirmish()) {
			prefix = "skirmish";
		}
		string saveFilename = dir + "/"
			+ prefix + " - "
			+ toString(1900 + localtime(&currentTime)->tm_year, 4) + "-"
			+ toString(1 + localtime(&currentTime)->tm_mon, 2) + "-"
			+ toString(localtime(&currentTime)->tm_mday, 2) + "."
			+ toString(localtime(&currentTime)->tm_hour, 2) + "-"
			+ toString(localtime(&currentTime)->tm_min, 2) + "-"
			+ toString(localtime(&currentTime)->tm_sec, 2) + "."
			+ "[" + removeFilePath(game->getFileName()) + "]."
			+ toString((int)choice(0, 99999), 5) + postfix + ext;
		game->saveSnapshot(saveFilename);
		game->setMessage("Game saved to \"" + saveFilename + "\"");
	}
	void loadGameSnapshot() {
		setGameState(GAMESTATE_LOADING_MAP_PHASE1);
		loadingGameThread = SDL_CreateThread(loadingSavedGamePhase1, NULL);
		confirmRemove = false;
	}
	void removeSavedGame() {
		remove(("save/" + savedGameMenuData.savedGameList[savedGameMenuData.selectedIndex]).data());
		loadSavedGameList();
		confirmRemove = false;
	}

	void restartGame() {
		Synchronizer sync(gameMutex);
		game->resume();
		bool skirmish = game->isSkirmish(), campaign = game->isCampaign();
		delete game;
		game = NULL;
		backGameState();
		setGameState(GAMESTATE_LOADING_MAP_PHASE1);
		if (skirmish) {
			loadingGameThread = SDL_CreateThread(loadingSkirmishMapPhase1, NULL);
		} else if (campaign) {
			loadingGameThread = SDL_CreateThread(loadingCampaignMapPhase1, NULL);
		} else {
			loadingGameThread = SDL_CreateThread(loadingMapPhase1, NULL);
		}
	}
	void resignGame() {
		Synchronizer sync(gameMutex);
		if (!game->isEditable() && !game->hasCompleted()) {
			saveGameSnapshot(" - autosave");
		}
		backGameState();
		//nb: game needs to be de deleted before outro starts or it would make outro hanged
		bool hasOutro = game->isCampaign() && game->hasCompleted() && !campaignMenuData.data[campaignMenuData.selectedCampaignIndex].outro.empty();
		delete game;
		if (hasOutro) {
			setGameState(GAMESTATE_OUTRO);
		}
		game = NULL;
	}
	void resumeGame() {
		game->resume();
		game->setMessage("Game Resumed");
	}
	void pauseGame() {
		game->pause();
		game->setMessage("Game Paused");
	}

	void loadCampaign(int index) {
		if (!campaignMenuData.data[index].intro.empty()) {
			setGameState(GAMESTATE_INTRO);
		} else {
			setGameState(GAMESTATE_LOADING_MAP_PHASE1);
			loadingGameThread = SDL_CreateThread(loadingCampaignMapPhase1, NULL);
		}
	}


	//------------------------------------------------------------------------------------------------Gameplay screen functions


	void displayMiniMap() {
		game->bindMinimapImage();
		glColor4f(1.0, 1.0, 1.0, 1.0);
		float size = clampHigh(min((float)MINIMAP_SIZE / game->getWidth(), (float)MINIMAP_SIZE / game->getHeight()), 1.0);
		int w = MINIMAP_SIZE*((float)game->getWidth() / MINIMAP_SIZE)*size;
		int h = MINIMAP_SIZE*((float)game->getHeight() / MINIMAP_SIZE)*size;
		int left = MINIMAP_BORDER, top = Game::Settings::screenHeight - MINIMAP_SIZE - MINIMAP_BORDER;
		glDrawRectangle(left + (MINIMAP_SIZE - w) / 2, top + (MINIMAP_SIZE - h) / 2, left + (MINIMAP_SIZE + w) / 2, top + (MINIMAP_SIZE + h) / 2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	bool handleMouseMinimap(int button, int state, int x, int y) {
		if (dragbox.area()>0) {
			return false;
		}

		float size = clampHigh(min((float)MINIMAP_SIZE / game->getWidth(), (float)MINIMAP_SIZE / game->getHeight()), 1.0);
		int w = MINIMAP_SIZE*((float)game->getWidth() / MINIMAP_SIZE)*size;
		int h = MINIMAP_SIZE*((float)game->getHeight() / MINIMAP_SIZE)*size;
		int left = MINIMAP_BORDER, top = Game::Settings::screenHeight - MINIMAP_SIZE - MINIMAP_BORDER;
		Point2D hit((float)(x - left - (MINIMAP_SIZE - w) / 2) / w, (float)(y - top - (MINIMAP_SIZE - h) / 2) / h);

		if (hit.in(0, 0, 1, 1)) {
			if (button == GLUT_LEFT_BUTTON) {
				float camX = (float)game->getWidth()*hit.x;
				float camY = (float)game->getHeight()*hit.y;
				game->goTo(camX, game->getHeight() - camY);
			} else if (button == GLUT_RIGHT_BUTTON) {
				float camX = (float)game->getWidth()*hit.x;
				float camY = (float)game->getHeight()*hit.y;
				game->move(camX, game->getHeight() - camY, (glutGetModifiers() == GLUT_ACTIVE_SHIFT), (glutGetModifiers() == GLUT_ACTIVE_CTRL));
			}
			return true;
		}
		return false;
	}


	void displayResources() {
		glColor4f(1, 1, 1, 1);
		int top = RESOURCE_TOP, dtop = RESOURCE_ICON_SIZE + RESOURCE_VGAP;
		Button::draw(icon_population,
			"Population / Maximum population",
			RESOURCE_ICON_LEFT, top,
			RESOURCE_ICON_SIZE, RESOURCE_ICON_SIZE);
		Button::draw(icon_food,
			"Food (Number of food gatherer)",
			RESOURCE_ICON_LEFT, top += dtop,
			RESOURCE_ICON_SIZE, RESOURCE_ICON_SIZE);
		Button::draw(icon_wood,
			"Wood (Number of wood gatherer)",
			RESOURCE_ICON_LEFT, top += dtop,
			RESOURCE_ICON_SIZE, RESOURCE_ICON_SIZE);
		Button::draw(icon_stone,
			"Stone (Number of stone gatherer)",
			RESOURCE_ICON_LEFT, top += dtop,
			RESOURCE_ICON_SIZE, RESOURCE_ICON_SIZE);
		Button::draw(icon_metal,
			"Metal (Number of metal gatherer)",
			RESOURCE_ICON_LEFT, top += dtop,
			RESOURCE_ICON_SIZE, RESOURCE_ICON_SIZE);
		Button::draw(icon_siegeattack,
			"Number of workers",
			RESOURCE_ICON_LEFT, top += dtop,
			RESOURCE_ICON_SIZE, RESOURCE_ICON_SIZE);
		if (game->teams[game->playerTeam].nIdleWorker > 0) {
			Button::draw(icon_error,
				toString(game->teams[game->playerTeam].nIdleWorker) + " idle workers",
				RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE / 2, top + RESOURCE_ICON_SIZE / 2,
				RESOURCE_ERROR_ICON_SIZE, RESOURCE_ERROR_ICON_SIZE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		top = RESOURCE_TOP + RESOURCE_ICON_SIZE - TEXT_VGAP;
		glPrint(RESOURCE_TEXT_LEFT, top, GLUT_BITMAP_HELVETICA_12)
			<< game->teams[game->playerTeam].population << "/" << game->getMaxPopulation(game->playerTeam) << glPrint::down(dtop)
			<< game->teams[game->playerTeam].resource.food << "(" << game->teams[game->playerTeam].nGatherer.food << ")" << glPrint::down(dtop)
			<< game->teams[game->playerTeam].resource.wood << "(" << game->teams[game->playerTeam].nGatherer.wood << ")" << glPrint::down(dtop)
			<< game->teams[game->playerTeam].resource.stone << "(" << game->teams[game->playerTeam].nGatherer.stone << ")" << glPrint::down(dtop)
			<< game->teams[game->playerTeam].resource.metal << "(" << game->teams[game->playerTeam].nGatherer.metal << ")" << glPrint::down(dtop)
			<< game->teams[game->playerTeam].nWorker;
	}
	bool handleMouseResources(int button, int state, int x, int y) {
		if (state == GLUT_DOWN)
			return false;

		int top = RESOURCE_TOP;
		top += RESOURCE_ICON_SIZE + RESOURCE_VGAP;
		if (Point2Di(x, y).in(RESOURCE_ICON_LEFT, top, RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE, top + RESOURCE_ICON_SIZE)) {
			game->selectFoodGatherer(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
			return true;
		}
		top += RESOURCE_ICON_SIZE + RESOURCE_VGAP;
		if (Point2Di(x, y).in(RESOURCE_ICON_LEFT, top, RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE, top + RESOURCE_ICON_SIZE)) {
			game->selectWoodGatherer(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
			return true;
		}
		top += RESOURCE_ICON_SIZE + RESOURCE_VGAP;
		if (Point2Di(x, y).in(RESOURCE_ICON_LEFT, top, RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE, top + RESOURCE_ICON_SIZE)) {
			game->selectStoneGatherer(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
			return true;
		}
		top += RESOURCE_ICON_SIZE + RESOURCE_VGAP;
		if (Point2Di(x, y).in(RESOURCE_ICON_LEFT, top, RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE, top + RESOURCE_ICON_SIZE)) {
			game->selectMetalGatherer(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
			return true;
		}
		top += RESOURCE_ICON_SIZE + RESOURCE_VGAP;
		if (Point2Di(x, y).in(RESOURCE_ICON_LEFT, top, RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE, top + RESOURCE_ICON_SIZE) || Point2Di(x, y).in(RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE / 2, top + RESOURCE_ICON_SIZE / 2, RESOURCE_ICON_LEFT + RESOURCE_ICON_SIZE / 2 + RESOURCE_ERROR_ICON_SIZE, top + RESOURCE_ICON_SIZE / 2 + RESOURCE_ERROR_ICON_SIZE)) {
			game->selectIdleWorker(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
			return true;
		}
		return false;
	}

	void drawPropertyIcon(Texture2D& icon, string title, int &top, int &left, string value) {
		if (Point2Di(mouseX, mouseY).in(left, top, left + INFO_PROPERTY_ICON_SIZE, top + INFO_PROPERTY_ICON_SIZE)) {
			toolTipText = title;
		}
		icon.bind();
		glDrawRectangle(left, top, left + INFO_PROPERTY_ICON_SIZE, top + INFO_PROPERTY_ICON_SIZE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glPrint(left + INFO_PROPERTY_ICON_SIZE + 5, top + INFO_PROPERTY_ICON_SIZE - TEXT_VGAP, GLUT_BITMAP_HELVETICA_12) << value;
		top += INFO_PROPERTY_ICON_SIZE + INFO_PROPERTY_ICON_VGAP;
		if (top >= INFO_PROPERTY_BOTTOM) {
			left = left + INFO_PROPERTY_ICON_SIZE + INFO_PROPERTY_ICON_HGAP;
			top = INFO_TOP;
		}
	}
	void displayInfo() {
		if (game->numberOfSelectedUnit() == 1) {
			Unit &unit = game->selectedUnit(0);
			UnitTypeInfo &obj = unitTypeInfo[unit.type];
			int left = INFO_LEFT, top = INFO_TOP;

			//Icon
			obj.icon.bind();
			glColor4f(1, 1, 1, 0.8);
			glDrawRectangle(left, top, left + INFO_BIG_ICON_SIZE, top + INFO_BIG_ICON_SIZE, 0.1);
			glBindTexture(GL_TEXTURE_2D, 0);

			//Health Bar
			int healthPercentage = 0;
			if (obj.hitPoint>0) healthPercentage = INFO_BIG_ICON_SIZE*unit.hitPoint / unit.getMaxHitPoint();
			glColor4f(0, 0.5, 0, 0.8);
			glDrawRectangle(left, top + INFO_BIG_ICON_SIZE, left + healthPercentage, top + INFO_BIG_ICON_SIZE + INFO_HEALTH_BAR_HEIGHT);
			glColor4f(0.5, 0, 0, 0.8);
			glDrawRectangle(left + healthPercentage, top + INFO_BIG_ICON_SIZE, left + INFO_BIG_ICON_SIZE, top + INFO_BIG_ICON_SIZE + INFO_HEALTH_BAR_HEIGHT);

			//Name
			glColor4f(1, 1, 1, 0.8);
			int nameWidth = glPrintWidth(GLUT_BITMAP_HELVETICA_18, obj.name);
			glPrint(left, INFO_BOTTOM, GLUT_BITMAP_HELVETICA_18) << obj.name;
			float colorBufer[4];
			glColor4fv(availableTeamColors[game->teams[unit.team].color].rgba().toArray(colorBufer));
			glPrint(left + nameWidth + 10, INFO_BOTTOM, GLUT_BITMAP_HELVETICA_10)
				<< "(" << factionName[game->teams[unit.team].faction] << " : "
				<< game->teams[unit.team].name << " : "
				<< (game->diplomacy(game->playerTeam, unit.team) == DIPLOMACY_ALLY ? "Ally" : game->diplomacy(game->playerTeam, unit.team) == DIPLOMACY_ENEMY ? "Enemy" : "Neutral")
				<< ")";

			//Properties
			glColor4f(1, 1, 1, 0.8);
			top = INFO_TOP, left = INFO_PROPERTY_LEFT;
			if (game->isEditable())							drawPropertyIcon(icon_hero, "unitID", top, left, toString(unit.getID()));
			if (game->isEditable())							drawPropertyIcon(icon_hero, "unitID (campaign)", top, left, unit.idForCampaign);
			if (obj.isHeroic)								drawPropertyIcon(icon_hero, "Level", top, left, toString(unit.level));
			drawPropertyIcon(icon_hitpoint, "Hit Point", top, left, toString(unit.hitPoint) + "/" + toString(unit.getMaxHitPoint()));
			if (unit.getHealAmount()>0)						drawPropertyIcon(icon_healrate, "Auto Heal Rate", top, left, toString(unit.getHealAmount()) + "/" + toString(float(unit.getHealDelay()) / UPDATE_THREAD_FRAME_RATE) + "sec");
			drawPropertyIcon(icon_armor, "Armour", top, left, toString(unit.getArmour()));
			if (obj.fireResistance>0)						drawPropertyIcon(icon_fire_resistance, "Fire Resistance", top, left, toString(obj.fireResistance * 100) + "%");
			if (unit.getAttack()>0)							drawPropertyIcon(icon_attack, "Normal Attack", top, left, toString(unit.getAttack()) + "/" + toString(float(obj.attackDelay) / UPDATE_THREAD_FRAME_RATE) + "sec");
			if (unit.getSiegeAttack()>0)						drawPropertyIcon(icon_siegeattack, "Siege Attack", top, left, toString(unit.getSiegeAttack()));
			if (unit.getAttack()>0)							drawPropertyIcon(icon_accuracy, "Accuracy", top, left, toString(float(obj.accuracy)*100.0, 2) + "%");
			if (obj.speed>0)									drawPropertyIcon(icon_speed, "Speed", top, left, toString(obj.speed*FRAME_RATE) + "/sec");
			if (unit.has.food>0)								drawPropertyIcon(icon_food, "Food", top, left, toString(unit.has.food));
			if (unit.has.wood>0)								drawPropertyIcon(icon_wood, "Wood", top, left, toString(unit.has.wood));
			if (unit.has.stone>0)							drawPropertyIcon(icon_stone, "Stone", top, left, toString(unit.has.stone));
			if (unit.has.metal>0)							drawPropertyIcon(icon_metal, "Metal", top, left, toString(unit.has.metal));
			if (unit.blended>0)								drawPropertyIcon(icon_blend, "Blended", top, left, toString(unit.blended * 100) + "%" + ((obj.stealth>0.0) ? (" (+" + toString(100 * obj.stealth) + "%)") : "") + ((obj.camouflage != 1.0) ? (" (x" + toString(obj.camouflage) + ")") : ""));
			if (unit.getAutoTransformTimeRemaining()>0)		drawPropertyIcon(icon_speed, "Remaining Time", top, left, timeAsString(unit.getAutoTransformTimeRemaining()));
		} else if (game->numberOfSelectedUnit()>1) {
			int left = INFO_LEFT, moveRight = INFO_RIGHT, iconSize = INFO_SMALL_ICON_SIZE, hgap = INFO_SMALL_ICON_HGAP, vgap = INFO_SMALL_ICON_VGAP, top = INFO_BOTTOM - iconSize;

			//Calculation
			int unitCount[MAX_OBJECT_TYPE], nUnitCategorysSelected = 0;
			float totalHitPoint[MAX_OBJECT_TYPE];
			for (int i = 0; i<MAX_OBJECT_TYPE; i++) {
				unitCount[i] = totalHitPoint[i] = 0;
			}
			for (int i = 0; i<game->numberOfSelectedUnit(); i++) {
				unitCount[game->selectedUnit(i).type]++;
				totalHitPoint[game->selectedUnit(i).type] += (float)game->selectedUnit(i).hitPoint / game->selectedUnit(i).getMaxHitPoint();
			}
			for (int i = 0; i<MAX_OBJECT_TYPE; i++) {
				if (unitCount[i]>0) {
					nUnitCategorysSelected++;
				}
			}

			//Icons
			for (int i = 0; i<MAX_OBJECT_TYPE; i++) {
				if (unitCount[i]>0) {
					ToggleButton::draw(unitTypeInfo[i].iconSmall, unitTypeInfo[i].iconSmall, unitTypeInfo[i].name, left, top, iconSize, iconSize);
					float hpRatio = totalHitPoint[i] / unitCount[i];
					glColor4f(0, 1, 0, 1);
					glDrawRectangle(left, top + iconSize, left + iconSize*hpRatio, top + iconSize + 2);
					glColor4f(1, 0, 0, 1);
					glDrawRectangle(left + iconSize*hpRatio, top + iconSize, left + iconSize, top + iconSize + 2);
					glColor4f(1, 1, 1, 1);
					glPrint(left + iconSize, top + iconSize, GLUT_BITMAP_TIMES_ROMAN_10) << "x" << unitCount[i];
					top -= iconSize + vgap;
					if (top<INFO_TOP) {
						left += iconSize + hgap;
						top = INFO_BOTTOM - iconSize;
					}
				}
			}
		}
	}
	bool handleMouseInfoButton(int button, int state, int x, int y) {
		if (game->numberOfSelectedUnit()>1) {
			int left = INFO_LEFT, moveRight = INFO_RIGHT, iconSize = INFO_SMALL_ICON_SIZE, hgap = INFO_SMALL_ICON_HGAP, vgap = INFO_SMALL_ICON_VGAP, top = INFO_BOTTOM - iconSize;
			int unitCount[MAX_OBJECT_TYPE], nUnitCategorysSelected = 0;
			for (int i = 0; i<MAX_OBJECT_TYPE; i++) {
				unitCount[i] = 0;
			}
			for (int i = 0; i<game->numberOfSelectedUnit(); i++) {
				unitCount[game->selectedUnit(i).type]++;
			}
			for (int i = 0; i<MAX_OBJECT_TYPE; i++) {
				if (unitCount[i]>0) {
					nUnitCategorysSelected++;
				}
			}
			//Icons
			for (int i = 0; i<MAX_OBJECT_TYPE; i++) {
				if (unitCount[i]>0) {
					if (Point2Di(x, y).in(left, top, left + iconSize, top + iconSize)) {
						if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
							if (glutGetModifiers() == 0) {
								game->filterSelectionKeep(i);
							} else if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
								game->filterSelectionRemoveAll(i);
							} else if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
								game->filterSelectionRemoveFirst(i);
							}
						}
						return true;
					}
					top -= iconSize + vgap;
					if (top<INFO_TOP) {
						left += iconSize + hgap;
						top = INFO_BOTTOM - iconSize;
					}
				}
			}
		}
		return false;
	}

	void displayAbility() {
		if (game->cursor.type != Game::Cursor::NONE) {
			return;
		}
		int top = ABILITY_ICON_TOP + ABILITY_ICON_TOPGAP, left = ABILITY_ICON_RIGHT - ABILITY_ICON_SIZE - ABILITY_ICON_HGAP;
		for (int i = 0; i<game->selectionAbility.size(); i++) {
			if (game->selectionAbility[i].type == ABILITY_DESTROY && confirmRemove) {
				ToggleButton::draw(*game->selectionAbility[i].icon, *game->selectionAbility[i].icon, "Click to Confirm Kill/Destroy", left, top, ABILITY_ICON_SIZE, ABILITY_ICON_SIZE, false, true, game->selectionAbility[i].buttonColor);
				glColor3f(1, 0, 0);
				texture_select.bind();
				glDrawRectangle(left - ABILITY_ICON_SIZE / 2, top - ABILITY_ICON_SIZE / 2, left + 3 * ABILITY_ICON_SIZE / 2, top + 3 * ABILITY_ICON_SIZE / 2);
			} else {
				ToggleButton::draw(*game->selectionAbility[i].icon, *game->selectionAbility[i].icon, game->selectionAbility[i].name, left, top, ABILITY_ICON_SIZE, ABILITY_ICON_SIZE, !game->selectionAbility[i].disabled, true, game->selectionAbility[i].buttonColor);
			}
			glColor4f(0, 1, 0, 0.5);
			glDrawRectangle(left, top, left + ABILITY_ICON_SIZE*game->selectionAbility[i].percentage / 100, top + ABILITY_ICON_SIZE);
			glColor4f(1, 1, 1, 1);
			if (game->selectionAbility[i].cnt>0) {
				int width = glPrintWidth(GLUT_BITMAP_HELVETICA_12, toString(game->selectionAbility[i].cnt));
				glPrint(left + ABILITY_ICON_SIZE - width, top + 12, GLUT_BITMAP_HELVETICA_12) << game->selectionAbility[i].cnt;
			}
			if (game->selectionAbility[i].type == ABILITY_BUILD) {
				icon_create.bind();
				glDrawRectangle(left - 5, top + ABILITY_ICON_SIZE - 15, left + 15, top + ABILITY_ICON_SIZE + 5);
			} else if (game->selectionAbility[i].type == ABILITY_DEPLOY) {
				icon_deploy.bind();
				glDrawRectangle(left - 5, top + ABILITY_ICON_SIZE - 15, left + 15, top + ABILITY_ICON_SIZE + 5);
			}
			top += ABILITY_ICON_SIZE + ABILITY_ICON_VGAP;
			if (top + ABILITY_ICON_SIZE >= ABILITY_ICON_BOTTOM || i + 1<game->selectionAbility.size() && toAbilityCategory(game->selectionAbility[i].type) != toAbilityCategory(game->selectionAbility[i + 1].type)) {
				top = ABILITY_ICON_TOP + ABILITY_ICON_TOPGAP;
				left -= ABILITY_ICON_SIZE + ABILITY_ICON_HGAP;
			}
		}
	}
	bool handleMouseAbilityButton(int button, int state, int x, int y) {
		if (game->cursor.type != Game::Cursor::NONE) {
			return false;
		}
		int index = -1;
		int top = ABILITY_ICON_TOP + ABILITY_ICON_TOPGAP, left = ABILITY_ICON_RIGHT - ABILITY_ICON_SIZE - ABILITY_ICON_HGAP;
		for (int i = 0; i<game->selectionAbility.size(); i++) {
			if (x >= left && y >= top && x <= left + ABILITY_ICON_SIZE && y <= top + ABILITY_ICON_SIZE) {
				index = i;
				break;
			}
			top += ABILITY_ICON_SIZE + ABILITY_ICON_VGAP;
			if (top + ABILITY_ICON_SIZE >= ABILITY_ICON_BOTTOM || i + 1<game->selectionAbility.size() && toAbilityCategory(game->selectionAbility[i].type) != toAbilityCategory(game->selectionAbility[i + 1].type)) {
				top = ABILITY_ICON_TOP + ABILITY_ICON_TOPGAP;
				left -= ABILITY_ICON_SIZE + ABILITY_ICON_HGAP;
			}
		}
		if (index == -1) {
			return false;
		}
		if (game->selectionAbility[index].disabled) {
			return true;
		}
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			switch (game->selectionAbility[index].type) {
			case ABILITY_BUILD:
				if (game->isEditable() || !isConstruction(unitTypeInfo[game->selectedUnit(0).type].category)) {
					game->cursor.type = Game::Cursor::BUILD;
					game->cursor.angle = 270;
					game->cursor.buildUnitType = game->selectionAbility[index].unit;
				} else {
					game->train(game->selectionAbility[index].unit);
					if (glutModifier == GLUT_ACTIVE_SHIFT) {
						for (int i = 0; i<4; i++) {
							game->train(game->selectionAbility[index].unit);
						}
					}
				}
				break;
			case ABILTY_STANCE_GENERAL:
				game->setStance(STANCE_GENERAL);
				break;
			case ABILITY_STANCE_STANDGROUND:
				game->setStance(STANCE_STANDGROUND);
				break;
			case ABILITY_STANCE_HOLD_FIRE:
				game->setStance(STANCE_HOLDFIRE);
				break;
			case ABILITY_STOP:
				game->stop();
				break;
			case ABILITY_DEPLOY:
				game->deploy(game->selectionAbility[index].unit);
				break;
			case ABILITY_DEPLOY_ALL:
				game->deployAll();
				break;
			case ABILITY_DESTROY:
				if (confirmRemove || game->isEditable()) {
					game->_kill();
				} else {
					confirmRemove = true;
				}
				break;
			case ABILITY_TRANSFORM:
				game->transform(game->selectionAbility[index].specialPowerIndex);
				break;
			case ABILITY_SPECIAL:
				game->cursor.type = Game::Cursor::TARGET;
				game->cursor.specialPowerIndex = game->selectionAbility[index].specialPowerIndex;
				break;
			case ABILITY_SET_TEAM:
				game->playerTeam = game->selectionAbility[index].specialPowerIndex;
				game->setSelectedUnitTeam(game->playerTeam);
				game->editState = MAPEDITSTATE_ADD_UNIT;
				break;
			case ABILITY_SET_TERRAIN_TEXTURE:
				skirmishMenuData.mapEditTerrainTextureIndex = game->selectionAbility[index].specialPowerIndex;
				break;
			case ABILITY_INCREASE_LEVEL:
				game->increaseSelectedUnitLevel(1);
				break;
			case ABILITY_DECREASE_LEVEL:
				game->increaseSelectedUnitLevel(-1);
				break;
			}
		} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
			if (!game->isEditable() && game->selectionAbility[index].type == ABILITY_BUILD && isConstruction(unitTypeInfo[game->selectedUnit(0).type].category)) {
				game->cancelBuild(game->selectionAbility[index].unit);
			}
		}
		return true;
	}

	void displayMessage() {
		glEnable(GL_TEXTURE_2D);
		//game end
		if (game->hasCompleted()) {
			glColor3f(1, 1, 1);
			if (game->teams[game->playerTeam].defeated) {
				image_defeat.bind();
			} else {
				image_victory.bind();
			}
			int w = Game::Settings::screenWidth / 2, h = w / 4;
			glDrawRectangle(Game::Settings::screenWidth / 2 - w / 2, Game::Settings::screenHeight / 2 - h / 2, Game::Settings::screenWidth / 2 + w / 2, Game::Settings::screenHeight / 2 + h / 2);
		}
		Texture2D::bindNone();

		int top = Game::Settings::screenHeight - (MINIMAP_SIZE + MINIMAP_BORDER * 2) - 10;
		//Message
		glColor4f(0.9, 1.0, 0.2, game->getMessageTransparency());
		glPrint(MINIMAP_BORDER, top, GLUT_BITMAP_HELVETICA_18) << game->getMessage();
		top -= glutBitmapHeight(GLUT_BITMAP_HELVETICA_18) + glPrintHeight(GLUT_BITMAP_HELVETICA_12, game->getObjectives());
		glColor4f(1, 1, 1, 0.8);
		glPrint(MINIMAP_BORDER, top, GLUT_BITMAP_HELVETICA_12) << game->getObjectives();

		//Subtitle
		if (game->isCampaign()) {
			glColor4f(1, 1, 1, 1);
			glPrint(300, Game::Settings::screenHeight*0.75, GLUT_BITMAP_HELVETICA_18) << ((Campaign*)game)->getSubtitle();
		}

		//info
		if (Game::Settings::showRenderDetails) {
			glColor4f(1, 1, 1, 0.8);
			top = 200;
			float frameRate = clamp(1000.0 / frameDuration, 0, FRAME_RATE);
			float updateFrameRate = clamp(1000.0 / game->updateFrameDuration, 0, UPDATE_THREAD_FRAME_RATE);
			GLint avail_mem = 0, total_mem = 0;
			if (isEXTSupported("GL_NVX_gpu_memory_info")) {
				glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &total_mem);
				glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &avail_mem);
			}
			glPrint(10, top += 10, GLUT_BITMAP_HELVETICA_10)
				<< toString(frameRate, 2) << " (" << toString(updateFrameRate, 2) << ")" << glPrint::down(10)
				<< "Game time : " << timeAsString(game->getGamePlayTime()) << glPrint::down(10)
				<< "Camera Position : " << game->getCameraPosition() << "/" << game->rotation << "," << game->tilt << glPrint::down(10)
				<< "Units Rendered : " << game->getNumberOfUnits() << "/" << game->nUnitsRendered << glPrint::down(10)
				<< "Particles : " << game->particleRenderer.getNumberOfParticles() << glPrint::down(10)
				<< "Memory usage (nVidia) : " << avail_mem - total_mem << "/" << avail_mem;
		}

		//Console
		if (console) {
			static bool blink = true;
			blink = !blink;
			glPrint(30, Game::Settings::screenHeight - 30, GLUT_BITMAP_HELVETICA_18) << ">" << consoleBuffer << (blink ? "_" : "");
		}
	}

	void displayGameToolbar() {
		int top = PAUSE_BUTTON_TOP, left = PAUSE_BUTTON_LEFT;
		if (!game->isEditable()) {
			if (game->isPaused()) {
				if (!game->hasCompleted()) {
					Button::draw(icon_resume, "Resume (P)", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
					left -= PAUSE_BUTTON_SIZE;
				}
				Button::draw(icon_quit, "Resign (Q)", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
				left -= PAUSE_BUTTON_SIZE;
				Button::draw(icon_restart, "Restart (R)", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
				left -= PAUSE_BUTTON_SIZE;
				Button::draw(icon_save, "Save (F6)", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
				left -= PAUSE_BUTTON_SIZE;
				Button::draw(icon_display_settings, "Graphics/Display Settings", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
				left -= PAUSE_BUTTON_SIZE;
				Button::draw(icon_audio_settings, "Audio Settings", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
				left -= PAUSE_BUTTON_SIZE;
				Button::draw(icon_game_settings, "Game Settings", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			} else {
				Button::draw(icon_pause, "Pause (P)", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			}
		} else {
			Button::draw(icon_quit, "Leave Editor (F10)", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left = 0;
			Button::draw(icon_save, "Save (F2)", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left += PAUSE_BUTTON_SIZE;
			Button::draw(icon_dcr_height, "Decrease Ground Height", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left += PAUSE_BUTTON_SIZE;
			Button::draw(icon_inr_height, "Increase Ground Height", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left += PAUSE_BUTTON_SIZE;
			Button::draw(icon_terrain_texture, "Terrain", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left += PAUSE_BUTTON_SIZE;
			Button::draw(icon_add_unit, "Add Unit", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left += PAUSE_BUTTON_SIZE;
			Button::draw(icon_add_building, "Add Building", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left += PAUSE_BUTTON_SIZE;
			Button::draw(icon_add_others, "Add Others", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
			left += PAUSE_BUTTON_SIZE;
			Button::draw(image_blank_button, "Team", left, top, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE, availableTeamColors[game->playerTeam]);
		}
	}
	bool handleMouseGameToolbar(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			int top = PAUSE_BUTTON_TOP, left = PAUSE_BUTTON_LEFT;
			if (!game->isEditable()) {
				if (game->isPaused()) {
					if (!game->hasCompleted()) {
						if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {			//Resume
							game->resume();
							return true;
						}
						left -= PAUSE_BUTTON_SIZE;
					}
					if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Resign
						resignGame();
						return true;
					}
					left -= PAUSE_BUTTON_SIZE;
					if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Restart
						restartGame();
						return true;
					}
					left -= PAUSE_BUTTON_SIZE;
					if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Save
						saveGameSnapshot();
						return true;
					}
					left -= PAUSE_BUTTON_SIZE;
					if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Display Settings
						setGameState(GAMESTATE_SETTINGSMENU_DISPLAY);
						return true;
					}
					left -= PAUSE_BUTTON_SIZE;
					if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Audio Settings
						setGameState(GAMESTATE_SETTINGSMENU_AUDIO);
						return true;
					}
					left -= PAUSE_BUTTON_SIZE;
					if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Game Settings
						setGameState(GAMESTATE_SETTINGSMENU_GAME);
						return true;
					}
				} else {
					if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Pause
						pauseGame();
						return true;
					}
				}
			} else {
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Back
					resignGame();
					return true;
				}
				left = 0;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Save
					game->save();
					return true;
				}
				left += PAUSE_BUTTON_SIZE;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE))	{				//Decrease Ground Height
					game->editState = MAPEDITSTATE_DECREASE_HEIGHT;
					return true;
				}
				left += PAUSE_BUTTON_SIZE;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Increase Ground Height
					game->editState = MAPEDITSTATE_INCREASE_HEIGHT;
					return true;
				}
				left += PAUSE_BUTTON_SIZE;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Terrain Texture
					game->editState = MAPEDITSTATE_CHANGE_TEXTURE;
					return true;
				}
				left += PAUSE_BUTTON_SIZE;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Add Units
					game->editState = MAPEDITSTATE_ADD_UNIT;
					if (game->playerTeam == 0)
						game->playerTeam = 1;
					return true;
				}
				left += PAUSE_BUTTON_SIZE;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Add Building
					game->editState = MAPEDITSTATE_ADD_BUILDING;
					if (game->playerTeam == 0)
						game->playerTeam = 1;
					return true;
				}
				left += PAUSE_BUTTON_SIZE;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Add Others
					game->editState = MAPEDITSTATE_ADD_OTHERS;
					game->playerTeam = 0;
					return true;
				}
				left += PAUSE_BUTTON_SIZE;
				if (Point2Di(x, y).in(left, top, left + PAUSE_BUTTON_SIZE, top + PAUSE_BUTTON_SIZE)) {				//Team
					game->editState = MAPEDITSTATE_CHANGE_TEAM;
					return true;
				}
			}
		}
		return false;
	}

	void drawPanel() {
		glColor4f(1, 1, 1, 1);
		texture_game_frame.bind();
		glDrawRectangle(0, 0, Game::Settings::screenWidth, Game::Settings::screenHeight);
	}

	void consoleSpecialUp(unsigned char key) {
		if (console) {
			specialPressed[key] = false;
			switch (key) {
			case GLUT_KEY_UP:
				consoleBuffer = trim(consoleBuffer);
				if (!consoleQueue.empty()) {
					if (!consoleBuffer.empty() && consoleBuffer != consoleQueue.front()) {
						consoleQueue.insert(consoleQueue.begin(), consoleBuffer);
					}
					consoleBuffer = consoleQueue.back();
					consoleQueue.erase(consoleQueue.end() - 1);
				}
				break;
			case GLUT_KEY_DOWN:
				consoleBuffer = trim(consoleBuffer);
				if (!consoleQueue.empty()) {
					if (!consoleBuffer.empty() && consoleBuffer != consoleQueue.back()) {
						consoleQueue.insert(consoleQueue.end(), consoleBuffer);
					}
					consoleBuffer = consoleQueue.front();
					consoleQueue.erase(consoleQueue.begin());
				}
				break;
			}
		}
	}
	void consoleKeyboardUp(unsigned char key) {
		if (!console) {
			if (key == '`') {
				console = true;
				game->pause();
			}
		} else {
			keyPressed[key] = false;
			switch (key) {
			case '\b':
				consoleBuffer = consoleBuffer.substr(0, consoleBuffer.length() - 1);
				break;
			case GLUT_KEY_RETURN:
				//game->resume();
				consoleBuffer = trim(consoleBuffer);
				if (consoleQueue.empty() || !consoleBuffer.empty() && consoleBuffer != consoleQueue.back())
					consoleQueue.insert(consoleQueue.end(), consoleBuffer);
				if (consoleBuffer == "victory") {
					game->victory();
				} else if (consoleBuffer == "defeat") {
					game->defeat();
				} else if (startsWith(consoleBuffer, "shader")) {
					Game::Settings::noShader = !Game::Settings::noShader;
					game->setMessage("Game::Settings::noShader = " + toString(Game::Settings::noShader));
				} else if (startsWith(consoleBuffer, "render object")) {
					Game::Settings::renderObjectsOn = !Game::Settings::renderObjectsOn;
					game->setMessage("Game::Settings::renderObjectsOn = " + toString(Game::Settings::renderObjectsOn));
				} else if (startsWith(consoleBuffer, "render terrain")) {
					Game::Settings::renderTerrainOn = !Game::Settings::renderTerrainOn;
					game->setMessage("Game::Settings::renderTerrainOn = " + toString(Game::Settings::renderTerrainOn));
				} else if (startsWith(consoleBuffer, "render water")) {
					Game::Settings::renderWaterOn = !Game::Settings::renderWaterOn;
					game->setMessage("Game::Settings::renderWaterOn = " + toString(Game::Settings::renderWaterOn));
				} else if (startsWith(consoleBuffer, "render sky")) {
					Game::Settings::renderSkyOn = !Game::Settings::renderSkyOn;
					game->setMessage("Game::Settings::renderSkyOn = " + toString(Game::Settings::renderSkyOn));
				} else if (startsWith(consoleBuffer, "object bumpmap") || startsWith(consoleBuffer, "object normalmap")) {
					Game::Settings::objectBumpmapOn = !Game::Settings::objectBumpmapOn;
					game->setMessage("Game::Settings::objectBumpmapOn = " + toString(Game::Settings::objectBumpmapOn));
				} else if (startsWith(consoleBuffer, "terrain bumpmap") || startsWith(consoleBuffer, "terrain normalmap")) {
					Game::Settings::terrainBumpmapOn = !Game::Settings::terrainBumpmapOn;
					game->setMessage("Game::Settings::terrainBumpmapOn = " + toString(Game::Settings::terrainBumpmapOn));
				} else if (startsWith(consoleBuffer, "bumpmap") || startsWith(consoleBuffer, "normalmap")) {
					Game::Settings::objectBumpmapOn = !Game::Settings::objectBumpmapOn;
					Game::Settings::terrainBumpmapOn = !Game::Settings::terrainBumpmapOn;
					game->setMessage("Game::Settings::terrainBumpmapOn = " + toString(Game::Settings::terrainBumpmapOn) + ", Game::Settings::objectBumpmapOn = " + toString(Game::Settings::objectBumpmapOn));
				} else if (startsWith(consoleBuffer, "antialiasing")) {
					Game::Settings::antialiasingOn = !Game::Settings::antialiasingOn;
					game->setMessage("Game::Settings::antialiasingOn = " + toString(Game::Settings::antialiasingOn));
				} else if (startsWith(consoleBuffer, "shadow")) {
					Game::Settings::shadowOn = !Game::Settings::shadowOn;
					game->setMessage("Game::Settings::shadowOn = " + toString(Game::Settings::shadowOn));
				} else if (startsWith(consoleBuffer, "reflection")) {
					Game::Settings::reflectionOn = !Game::Settings::reflectionOn;
					game->setMessage("Game::Settings::reflectionOn = " + toString(Game::Settings::reflectionOn));
				} else if (startsWith(consoleBuffer, "bloom")) {
					Game::Settings::bloomOn = !Game::Settings::bloomOn;
					game->setMessage("Game::Settings::bloomOn = " + toString(Game::Settings::bloomOn));
				} else if (startsWith(consoleBuffer, "dof")) {
					Game::Settings::depthOfFieldOn = !Game::Settings::depthOfFieldOn;
					game->setMessage("Game::Settings::depthOfFieldOn = " + toString(Game::Settings::depthOfFieldOn));
				} else if (startsWith(consoleBuffer, "motion blur")) {
					Game::Settings::motionBlurOn = !Game::Settings::motionBlurOn;
					game->setMessage("Game::Settings::motionBlurOn = " + toString(Game::Settings::motionBlurOn));
				} else if (startsWith(consoleBuffer, "stereo sep ")) {
					Game::Settings::stereoSeperation = toFloat(split(consoleBuffer)[2]);
				} else if (startsWith(consoleBuffer, "stereo")) {
					Game::Settings::stereoscopicOn = !Game::Settings::stereoscopicOn;
					game->setMessage("Game::Settings::stereoscopicOn = " + toString(Game::Settings::stereoscopicOn));
				} else if (startsWith(consoleBuffer, "wireframe")) {
					Game::Settings::wireframeOn = !Game::Settings::wireframeOn;
					game->setMessage("Game::Settings::wireframeOn = " + toString(Game::Settings::wireframeOn));
				} else if (startsWith(consoleBuffer, "show los")) {
					Game::Settings::showLOS = !Game::Settings::showLOS;
					game->setMessage("Game::Settings::showLOS = " + toString(Game::Settings::showLOS));
				} else if (startsWith(consoleBuffer, "show path")) {
					Game::Settings::showPath = !Game::Settings::showPath;
					game->setMessage("Game::Settings::showPath = " + toString(Game::Settings::showPath));
				} else if (startsWith(consoleBuffer, "team ") || startsWith(consoleBuffer, "t ")) {
					int newteam = toInt(split(consoleBuffer)[1]);
					newteam = clamp(newteam, 0, game->getNumberOfTeams());
					game->playerTeam = newteam;
					game->setMessage("Team set to " + toString(game->playerTeam));
				} else if (startsWith(consoleBuffer, "double click delay ")) {
					DOUBLE_CLICK_DELAY = toInt(split(consoleBuffer)[3]);
					game->setMessage("DOUBLE_CLICK_DELAY = " + toString(DOUBLE_CLICK_DELAY));
				} else if (startsWith(consoleBuffer, "scroll speed ")) {
					Game::Settings::scrollSpeed = toInt(split(consoleBuffer)[2]);
					game->setMessage("Game::Settings::scrollSpeed = " + toString(Game::Settings::scrollSpeed));
				} else if (consoleBuffer == "reveal all") {
					game->teams[game->playerTeam].revealAll = true;
					game->setMessage("game->teams[game->playerTeam].revealAll = " + toString(game->teams[game->playerTeam].revealAll));
				} else if (consoleBuffer == "explore all") {
					game->teams[game->playerTeam].exploreAll = true;
					game->setMessage("game->teams[game->playerTeam].exploreAll = " + toString(game->teams[game->playerTeam].exploreAll));
				} else if (startsWith(consoleBuffer, "weather ")) {
					game->setWeather(toWeatherType(split(consoleBuffer)[1]));
					game->setMessage("Game::Weather::weatherType = " + toUpper(split(consoleBuffer)[1]));
				} else if (startsWith(consoleBuffer, "max particle ") || startsWith(consoleBuffer, "max particles ")) {
					ParticleRenderer::MAX_PARTICLES = toInt(split(consoleBuffer)[2]);
					game->setMessage("ParticleRenderer::MAX_PARTICLES = " + toString(ParticleRenderer::MAX_PARTICLES));
				} else if (startsWith(consoleBuffer, "rename ")) {
					game->renameMap(split(consoleBuffer, ' ', 2)[1]);
					game->setMessage("Map renamed to \"" + game->getFileName() + "\"");
				}
				console = false;
				consoleBuffer = "";
				break;
			case GLUT_KEY_ESCAPE:
			case '`':
				console = false;
				consoleBuffer = "";
				//game->resume();
				break;
			default:
				consoleBuffer += tolower(key);
				break;
			}
		}
	}

	void displayGamePlaying() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glClearColor(1, 1, 1, 1);
		game->update();

		// Terrain & objects
		if (Game::Settings::wireframeOn) {
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glEnable(GL_DEPTH_TEST);
		game->render();
		glDisable(GL_DEPTH_TEST);
		if (Game::Settings::wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_POLYGON_OFFSET_FILL);
		}

		//UI
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, Game::Settings::screenWidth, Game::Settings::screenHeight, 0, -1, 1);
		glEnable(GL_BLEND);
		if (game->cursor.type == Game::Cursor::NONE)
			drawDragbox();
		glEnable(GL_TEXTURE_2D);
		drawPanel();
		displayMiniMap();
		if (!game->isEditable())
			displayResources();
		displayInfo();
		displayAbility();
		displayGameToolbar();
		displayCursor();
		displayMessage();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		syncFrameRate(FRAME_RATE);
	}

	//mouse callbacks
	void mouseGamePlaying(int button, int state, int x, int y) {
		bool confirmRemoveBackUp = confirmRemove;
		glutModifier = glutGetModifiers();
		if (handleMouseGameToolbar(button, state, x, y)
			|| handleMouseInfoButton(button, state, x, y)
			|| handleMouseResources(button, state, x, y)
			|| handleMouseMinimap(button, state, x, y)
			|| handleMouseAbilityButton(button, state, x, y)) {																						//click on ability
			//Do nothing... (Already Done)
		} else {
			switch (game->cursor.type) {
			case Game::Cursor::NONE:
				if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {																						//Drag start on Screen
					dragbox.x1 = dragbox.x2 = x;
					dragbox.y1 = dragbox.y2 = y;
					if (game->editState == MAPEDITSTATE_ADD_UNIT || game->editState == MAPEDITSTATE_ADD_BUILDING || game->editState == MAPEDITSTATE_ADD_OTHERS)
						dragboxEnabled = true;
				} else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {																					//Drag end or Left click on Screen
					if (glutGetModifiers() == GLUT_ACTIVE_SHIFT || glutGetModifiers() == GLUT_ACTIVE_CTRL)
						game->select(Game::Settings::screenWidth, Game::Settings::screenHeight, dragbox.x1, dragbox.y1, dragbox.x2, dragbox.y2, true, dragbox.area()>10, doubleClicked);
					else if (glutGetModifiers() == 0)
						game->select(Game::Settings::screenWidth, Game::Settings::screenHeight, dragbox.x1, dragbox.y1, dragbox.x2, dragbox.y2, false, dragbox.area()>10, doubleClicked);
					dragbox.clear();
				} else if (state == GLUT_UP && button == GLUT_RIGHT_BUTTON && dist(Point2Di(x, y), Point2Di(dragStartX, dragStartY))<5) {																				//Right click on screen
					game->command(Game::Settings::screenWidth, Game::Settings::screenHeight, x, y, (glutGetModifiers() == GLUT_ACTIVE_SHIFT), (glutGetModifiers() == GLUT_ACTIVE_CTRL));
				} else if (state == GLUT_UP && button == GLUT_MIDDLE_BUTTON && dragStartX == x && dragStartY == y) {												//Middle click on screen
					game->resetCamera();
				} else if (state == GLUT_UP && button == 4) {																								//Scroll moveUp
					game->zoomOut(Game::Settings::scrollSpeed * 10);
				} else if (state == GLUT_UP && button == 3) {																								//Scroll down
					game->zoomOut(-Game::Settings::scrollSpeed * 10);
				}
				break;
			case Game::Cursor::BUILD:
				if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
					int angle = 270;
					if (x != dragStartX)
						angle = Line2D(Point2D(dragStartX, dragStartY), Point2D(x, y)).tangent();
					if (game->isEditable()) {
						game->addUnit(game->cursor.pos.x, game->cursor.pos.y, game->cursor.buildUnitType, game->playerTeam, game->cursor.angle);
						if (glutModifier == GLUT_ACTIVE_CTRL)
						for (int i = 0; i<9; i++)
							game->addUnit(game->cursor.pos.x + choice(-5, 5), game->cursor.pos.y + choice(-5, 5), game->cursor.buildUnitType, game->playerTeam, choice(0, 360));
						else if (glutModifier == GLUT_ACTIVE_ALT)
						for (int i = 0; i<49; i++)
							game->addUnit(game->cursor.pos.x + choice(-20, 20), game->cursor.pos.y + choice(-20, 20), game->cursor.buildUnitType, game->playerTeam, choice(0, 360));
					} else
						game->command(Game::Settings::screenWidth, Game::Settings::screenHeight, dragStartX, dragStartY, (glutGetModifiers() == GLUT_ACTIVE_SHIFT), false, COMMAND_BUILD, game->cursor.buildUnitType);
					if (glutGetModifiers() == 0)
						game->cursor.type = Game::Cursor::NONE;
				} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
					game->cursor.type = Game::Cursor::NONE;
				}
				break;
			case Game::Cursor::TARGET:
				if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
					game->command(Game::Settings::screenWidth, Game::Settings::screenHeight, dragStartX, dragStartY, false, false, COMMAND_SPECIAL_ATTACK, game->cursor.buildUnitType, game->cursor.specialPowerIndex);
					game->cursor.type = Game::Cursor::NONE;
				} else if (button == GLUT_RIGHT_BUTTON) {
					game->cursor.type = Game::Cursor::NONE;
				}
				break;
			}
		}

		if (state == GLUT_UP && confirmRemoveBackUp == confirmRemove)
			confirmRemove = false;
	}
	void passivemotionGamePlaying(int x, int y) {
		glutModifier = glutGetModifiers();
		if (game->cursor.type != Game::Cursor::NONE) {
			game->setCursor(Game::Settings::screenWidth, Game::Settings::screenHeight, x, y);
			if (buttonPressed[GLUT_LEFT_BUTTON]) {
				game->cursor.angle = 270;
				if (x != dragStartX) {
					game->cursor.angle = atan((y - dragStartX) / (x - dragStartX)) * 180 / PI;
				}
			}
		}
	}
	void motionGamePlaying(int x, int y) {
		glutModifier = glutGetModifiers();

		if (buttonPressed[GLUT_LEFT_BUTTON])
			handleMouseMinimap(GLUT_LEFT_BUTTON, GLUT_DOWN, x, y);

		if (buttonPressed[GLUT_RIGHT_BUTTON]) {
			game->moveRight(-(mouseX - x)*Game::Settings::scrollSpeed);
			game->moveUp((mouseY - y)*Game::Settings::scrollSpeed);
		} else if (buttonPressed[GLUT_MIDDLE_BUTTON]) {
			game->tiltBack(-(mouseY - y)*Game::Settings::scrollSpeed);
			game->rotateLeft((mouseX - x)*Game::Settings::scrollSpeed);
		} else switch (game->cursor.type) {
		case Game::Cursor::NONE:
			if (dragboxEnabled && buttonPressed[GLUT_LEFT_BUTTON]) {
				dragbox.x2 = x;
				dragbox.y2 = y;
			}
			break;
		case Game::Cursor::BUILD:
			if (buttonPressed[GLUT_LEFT_BUTTON]) {
				game->cursor.angle = 270;
				if (x != dragStartX) {
					float dx = dragStartX - x, dy = y - dragStartY;
					game->cursor.angle = atan(dy / dx) * 180 / PI;
					game->cursor.angle = (dx<0) ? game->cursor.angle : (game->cursor.angle + 180);
				}
			}
			break;
		}
	}
	void mouseentryGamePlaying(int state) {
		if (!game->isEditable()) {
			if (state == GLUT_LEFT)
				game->pause();
		}
	}
	//keyboard callbacks
	void keyboardGamePlaying(unsigned char key, int x, int y) {
		glutModifier = glutGetModifiers();
		if (!console) {
			if (glutGetModifiers() == 0 || glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
				switch (key) {
				case GLUT_KEY_ESCAPE:
					game->clearSelection();
					break;
				case 's': case 'S':
					game->stop();
					break;
				case 'i': case 'I':
					game->selectIdleWorker(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
					break;
				case 'h': case 'H':
					game->selectHero(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
					break;
				case 'm': case 'M':
					game->selectMilitary(glutGetModifiers() == GLUT_ACTIVE_SHIFT);
					break;
				}
				if (game->isPaused()) {
					switch (key) {
					case 'r': case 'R':
						restartGame();
						break;
					case 'q': case 'Q':
						resignGame();
						break;
					}
				}
			}
		}
		if (key != GLUT_KEY_DEL)
			confirmRemove = false;
	}
	void specialGamePlaying(unsigned char key, int x, int y) {
		glutModifier = glutGetModifiers();
		confirmRemove = false;
	}
	void keyboardupGamePlaying(unsigned char key, int x, int y) {
		glutModifier = glutGetModifiers();
		if (!console) {
			if (glutGetModifiers() == 0 || glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
				switch (key) {
				case 'p': case 'P':
					if (!game->isEditable()) {
						if (game->isPaused()) {
							if (!game->hasCompleted())
								resumeGame();
						} else
							pauseGame();
					}
					break;
				case GLUT_KEY_DEL:
					if (confirmRemove || game->isEditable())
						game->_kill();
					else
						confirmRemove = true;
					break;
				case '=':
					Game::Settings::stereoSeperation += 0.05;
					break;
				case '-':
					Game::Settings::stereoSeperation -= 0.05;
					break;
				}
			}
		}

		switch (game->cursor.type) {
		case Game::Cursor::BUILD:
			game->setCursor(Game::Settings::screenWidth, Game::Settings::screenHeight, x, y);
			break;
		}

		consoleKeyboardUp(key);
	}
	void specialupGamePlaying(unsigned char key, int x, int y) {
		glutModifier = glutGetModifiers();
		if (!console) {
			if (game->cursor.type == Game::Cursor::BUILD)
				game->setCursor(Game::Settings::screenWidth, Game::Settings::screenHeight, x, y);
			if (game->isEditable()) {
				switch (key) {
				case GLUT_KEY_F2:
					game->save();
					game->setMessage("Map saved");
					break;
				case GLUT_KEY_F10:
					resignGame();
					break;
				}
			} else {
				switch (key) {
				case GLUT_KEY_F10:
					if (game->isPaused()) {
						if (!game->hasCompleted())
							resumeGame();
					} else
						pauseGame();
					break;
				case GLUT_KEY_F3:
					Game::Settings::noShader = !Game::Settings::noShader;
					game->setMessage(Game::Settings::noShader ? "Shaders off" : "Shaders on");
					break;
				case GLUT_KEY_F4:
					Game::Settings::showRenderDetails = !Game::Settings::showRenderDetails;
					break;
				case GLUT_KEY_F6:
					saveGameSnapshot();
					break;
				case GLUT_KEY_F12:
					Game::Settings::stereoscopicOn = !Game::Settings::stereoscopicOn;
					game->setMessage(Game::Settings::stereoscopicOn ? "Stereoscopic on" : "Stereoscopic off");
					break;
				case GLUT_KEY_HOME:
					game->resetCamera();
					break;
				}
			}
		}
		consoleSpecialUp(key);
	}
	//Idle callbacks
	void idleGamePlaying() {
		// Key/Button hold
		switch (glutModifier) {
		case 0:
			if (specialPressed[GLUT_KEY_LEFT])		game->moveRight(-Game::Settings::scrollSpeed * 20);
			if (specialPressed[GLUT_KEY_RIGHT])		game->moveRight(Game::Settings::scrollSpeed * 20);
			if (specialPressed[GLUT_KEY_UP])		game->moveUp(Game::Settings::scrollSpeed * 20);
			if (specialPressed[GLUT_KEY_DOWN])		game->moveUp(-Game::Settings::scrollSpeed * 20);
			if (specialPressed[GLUT_KEY_PAGE_UP])	game->zoomOut(Game::Settings::scrollSpeed * 10);
			if (specialPressed[GLUT_KEY_PAGE_DOWN])	game->zoomOut(-Game::Settings::scrollSpeed * 10);
			if (keyPressed[' '])					game->goToSelected();
			if (dragStartX == mouseX && dragStartY == mouseY) {
				if (mouseX<10)									game->moveRight(-Game::Settings::scrollSpeed * 10);
				if (mouseX>Game::Settings::screenWidth - 10)	game->moveRight(Game::Settings::scrollSpeed * 10);
				if (mouseY<10)									game->moveUp(Game::Settings::scrollSpeed * 10);
				if (mouseY>Game::Settings::screenHeight - 10)	game->moveUp(-Game::Settings::scrollSpeed * 10);
			}
			break;
		case GLUT_ACTIVE_CTRL:
			if (specialPressed[GLUT_KEY_LEFT])		game->rotateLeft(5);
			if (specialPressed[GLUT_KEY_RIGHT])		game->rotateLeft(-5);
			if (specialPressed[GLUT_KEY_UP])		game->tiltBack(-2);
			if (specialPressed[GLUT_KEY_DOWN])		game->tiltBack(2);
			break;
		}

		{
			Synchronizer sync(gameMutex);
			if (game->isEditable()) {
				switch (game->editState) {
				case MAPEDITSTATE_DECREASE_HEIGHT:
					if (buttonPressed[GLUT_LEFT_BUTTON])
						game->increaseHeight(Game::Settings::screenWidth, Game::Settings::screenHeight, mouseX, mouseY, (glutGetModifiers() == GLUT_ACTIVE_SHIFT) ? -0.3 : -0.1);
					break;
				case MAPEDITSTATE_INCREASE_HEIGHT:
					if (buttonPressed[GLUT_LEFT_BUTTON])
						game->increaseHeight(Game::Settings::screenWidth, Game::Settings::screenHeight, mouseX, mouseY, (glutGetModifiers() == GLUT_ACTIVE_SHIFT) ? 0.3 : 0.1);
					break;
				case MAPEDITSTATE_CHANGE_TEXTURE:
					if (buttonPressed[GLUT_LEFT_BUTTON])
						game->setTexture(Game::Settings::screenWidth, Game::Settings::screenHeight, mouseX, mouseY, skirmishMenuData.mapEditTerrainTextureIndex);
					break;
				default:
					break;
				}
			}
		}

		// Check mission complete
		if (game->hasCompleted())
			game->pause();

	}
	//window callbacks
	void visibilityGamePlaying(int state) {
		if (!game->isEditable()) {
			if (state == GLUT_NOT_VISIBLE)
				game->pause();
		}
	}


	//------------------------------------------------------------------------------------------------Loading screen function

	void displayGameLoading() {
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glClearColor(0, 0, 0, 1);
		glOrtho(0, Game::Settings::screenWidth, Game::Settings::screenHeight, 0, 1, -1);

		if (currentGameState == GAMESTATE_LOADING_GAME_PHASE1)		drawVideoBackground("video/loading_game", false);
		else if (currentGameState == GAMESTATE_LOADING_MAP_PHASE1)	drawVideoBackground("video/loading_map", true);

		glColor3f(1, 1, 1);
		image_loading_text.bind();
		glDrawRectangle(100, 50, 500, 150);

		glColor3f(1, 1, 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		image_loading.bind();
		int iconSize = 50;
		glPushMatrix();
		static int angle = 0;
		const int iconX = 200, iconY = Game::Settings::screenHeight - 100;
		glTranslatef(iconX, iconY, 0);
		glRotatef(angle, 0, 0, 1);
		angle = (angle + 30) % 360;
		glDrawRectangle(-iconSize, -iconSize, iconSize, iconSize);
		glPopMatrix();

		glColor4f(1, 1, 1, 0.5);
		Texture2D::bindNone();
		{
			Synchronizer sync(gameMutex);
			if (game != NULL && !game->getMessage().empty()) {
				glPrint(iconX - iconSize, Game::Settings::screenHeight - 30, GLUT_BITMAP_HELVETICA_12) << game->getMessage();
			} else {
				glPrint(iconX - iconSize, Game::Settings::screenHeight - 30, GLUT_BITMAP_HELVETICA_12) << "Please Wait...";
			} 
		}

		if (Game::Settings::motionBlurOn) {
			motionBlur(MENU_MOTION_BLUR_AMOUNT);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	}


	//------------------------------------------------------------------------------------------------Cutscene screen functions

	void exitCutscene(bool loadGameAtEnd) {
		stopAudio(CHANNEL_SOUND);
		if (loadGameAtEnd) {
			setGameState(GAMESTATE_LOADING_MAP_PHASE1);
			loadingGameThread = SDL_CreateThread(loadingCampaignMapPhase1, NULL);
		} else {
			backGameState();
		}
	}

	void displayCutscene(bool intro, bool loadGameAtEnd) {
		glEnable(GL_TEXTURE_2D);

		if (campaignMenuData.video.videoCapture == NULL) {
			string filename = "video/cutscene/" + Game::Settings::videoHeight + "/";
			if (intro) {
				filename += campaignMenuData.data[campaignMenuData.selectedCampaignIndex].intro;
			} else {
				filename += campaignMenuData.data[campaignMenuData.selectedCampaignIndex].outro;
			}
			campaignMenuData.video.videoCapture = cvCreateFileCapture(filename.data());
			stopAudio(CHANNEL_MUSIC);
			string audio_track = "video/cutscene/audio_track/";
			if (intro) {
				audio_track += removeExtension(campaignMenuData.data[campaignMenuData.selectedCampaignIndex].intro) + ".ogg";
			} else {
				audio_track += removeExtension(campaignMenuData.data[campaignMenuData.selectedCampaignIndex].outro) + ".ogg";
			}
			if(campaignMenuData.video.videoCapture != NULL) {
				if (fileExists(audio_track)) {
					playAudio(audio_track, CHANNEL_SOUND);
					Game::setSoundSettings();
				}
			} else {
				getLogger().print("displayCutscene() : Video load error (may be opencv_ffmpeg runlime library missing)", LOG_SEVERE);
			}
		}
		if (campaignMenuData.video.videoCapture != NULL) {
			IplImage* frame = cvQueryFrame(campaignMenuData.video.videoCapture);
			if (frame != NULL) {
				if (campaignMenuData.video.frameTexture != NULL) {
					delete campaignMenuData.video.frameTexture;
					campaignMenuData.video.frameTexture = NULL;
				}
				campaignMenuData.video.frameTexture = new Texture2D();
				campaignMenuData.video.frameTexture->make(frame, "cutscene", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			} else {
				exitCutscene(loadGameAtEnd);
			}
			if (campaignMenuData.video.frameTexture != NULL) {
				glColor4f(1, 1, 1, 1);
				campaignMenuData.video.frameTexture->bind();
				glDrawFullScreenRectangle();
			}
			syncVideoToFrameRate(VIDEO_FRAMERATE, campaignMenuData.video.videoCapture);
		}

		glDisable(GL_TEXTURE_2D);

	}
	void keyboardupCutscene(unsigned char key, int x, int y, bool loadGameAtEnd) {
		if (key == GLUT_KEY_ESCAPE)
			exitCutscene(loadGameAtEnd);
	}


	//------------------------------------------------------------------------------------------------Main menu screen functions


	void initMenu() {
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glClearColor(0.2, 0.3, 0.1, 1);
		glOrtho(0, Game::Settings::screenWidth, Game::Settings::screenHeight, 0, -1, 1);
	}
	void closeMenu() {
		if (Game::Settings::motionBlurOn)
			motionBlur(MENU_MOTION_BLUR_AMOUNT);
		displayCursor();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	void displayMenubackground() {
		drawVideoBackground("video/menu", true);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(0, 0, 0, 0.5);
		glDrawRectangle(0, 0, Game::Settings::screenWidth, 100);

		glColor4f(1, 1, 1, 1);
		logo.bind();
		glDrawRectangle(MENU_LOGO_LEFT, MENU_LOGO_TOP, MENU_LOGO_LEFT + MENU_LOGO_WIDTH, MENU_LOGO_TOP + MENU_LOGO_HEIGHT);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void displayMainMenu() {
		initMenu();
		displayMenubackground();
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuButton::draw(icon_campaign, "Campaign", left, top, true);
#ifndef _DEMO
		MenuButton::draw(icon_skirmish, "Skirmish", left, top, true);
#else
		MenuButton::draw(icon_skirmish, "Skirmish (Unavailable in Current Version)", left, top, true);
#endif
		MenuButton::draw(icon_load, "Load Saved Game", left, top, false);
		//MenuButton::draw(icon_network, "Network", left, top, false);
		MenuButton::draw(icon_display_settings, "Graphics/Display Settings", left, top, true);
		MenuButton::draw(icon_audio_settings, "Audio Settings", left, top, true);
		MenuButton::draw(icon_game_settings, "Game Settings", left, top, true);
#ifndef _DEMO
		MenuButton::draw(icon_editor, "Map Editor", left, top, false);
#else
		MenuButton::draw(icon_editor, "Map Editor (Unavailable in Current Version)", left, top, false);
#endif
		BackButton::draw(icon_quit, "Quit");
		Button::draw(icon_credits, "Credits", MENU_CREDITS_BUTTON_LEFT, MENU_CREDITS_BUTTON_TOP, MENU_CREDITS_BUTTON_SIZE, MENU_CREDITS_BUTTON_SIZE);
		Button::draw(icon_controls, "Controls", MENU_CONTROLS_BUTTON_LEFT, MENU_CREDITS_BUTTON_TOP, MENU_CREDITS_BUTTON_SIZE, MENU_CREDITS_BUTTON_SIZE);
		closeMenu();
	}
	void mouseMainMenu(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
			if (MenuButton::clicked(button, state, x, y, left, top, true)) {
				setGameState(GAMESTATE_CAMPAIGNMENU);
			}
			if (MenuButton::clicked(button, state, x, y, left, top, true)) {
#ifndef _DEMO
				setGameState(GAMESTATE_SKIRMISHMENU);
#endif
			}
			if (MenuButton::clicked(button, state, x, y, left, top, false)) {
				setGameState(GAMESTATE_SAVEGAMESNAPSHOT_MENU);
			}
			//if (MenuButton::clicked(button, state, x, y, left, top, false)) {
			//	setGameState(GAMESTATE_NETWORKMENU);
			//}
			if (MenuButton::clicked(button, state, x, y, left, top, true)) {
				setGameState(GAMESTATE_SETTINGSMENU_DISPLAY);
			}
			if (MenuButton::clicked(button, state, x, y, left, top, true)) {
				setGameState(GAMESTATE_SETTINGSMENU_AUDIO);
			}
			if (MenuButton::clicked(button, state, x, y, left, top, true)) {
				setGameState(GAMESTATE_SETTINGSMENU_GAME);
			}
			if (MenuButton::clicked(button, state, x, y, left, top, false))	{
#ifndef _DEMO
				setGameState(GAMESTATE_EDITORMENU);
#endif
			}

			if (BackButton::clicked(button, state, x, y)) {
				quit();
			}

			if (Button::clicked(button, state, x, y, MENU_CREDITS_BUTTON_LEFT, MENU_CREDITS_BUTTON_TOP, MENU_CREDITS_BUTTON_SIZE, MENU_CREDITS_BUTTON_SIZE)) {
				setGameState(GAMESTATE_CREDITS_SCREEN);
			}
			if (Button::clicked(button, state, x, y, MENU_CONTROLS_BUTTON_LEFT, MENU_CREDITS_BUTTON_TOP, MENU_CREDITS_BUTTON_SIZE, MENU_CREDITS_BUTTON_SIZE)) {
				setGameState(GAMESTATE_CONTROLS_SCREEN);
			}
		}
	}
	void keyboardupMainMenu(unsigned char key, int x, int y) {
		if (key == GLUT_KEY_ESCAPE)
			quit();
	}


	//------------------------------------------------------------------------------------------------Save Game screen functions

	void displaySavedGameList() {
		if (savedGameMenuData.savedGameList.size() <= 0)
			return;
		int top = SAVEGAME_MENU_LIST_TOP, left = SAVEGAME_MENU_LIST_LEFT, w = SAVEGAME_MENU_LIST_WIDTH, h = SAVEGAME_MENU_LIST_BTN_HEIGHT;
		glColor4f(0, 0, 0, 0.5);
		int scrollBarPosition = (SAVEGAME_MENU_LIST_BOTTOM - SAVEGAME_MENU_LIST_TOP) * float(savedGameMenuData.scrollOffset) / float(savedGameMenuData.savedGameList.size());
		glDrawRectangle(left + w + 5, (SAVEGAME_MENU_LIST_BOTTOM + SAVEGAME_MENU_LIST_TOP) / 2, left + w + 15, top + scrollBarPosition + h);
		for (int i = savedGameMenuData.scrollOffset; i<savedGameMenuData.savedGameList.size(); i++) {
			glColor4f(0, 0, 0, 0.5);
			glDrawRectangle(left, top, left + w, top + h);
			if (i == savedGameMenuData.selectedIndex)
				glColor4f(1, 1, 1, 1);
			else if (Point2Di(mouseX, mouseY).in(left, top, left + w, top + h))
				glColor4f(1, 1, 1, 0.9);
			else
				glColor4f(1, 1, 1, 0.7);
			glPrint(left + 2, top + 12, GLUT_BITMAP_HELVETICA_12) << removeExtension(savedGameMenuData.savedGameList[i]);
			top += h + 1;
			if (top + h>SAVEGAME_MENU_LIST_BOTTOM)
				break;
		}
	}
	void mouseSavedGameList(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON) {
			int top = SAVEGAME_MENU_LIST_TOP, left = SAVEGAME_MENU_LIST_LEFT, w = SAVEGAME_MENU_LIST_WIDTH, h = SAVEGAME_MENU_LIST_BTN_HEIGHT;
			for (int i = savedGameMenuData.scrollOffset; i<savedGameMenuData.savedGameList.size(); i++) {
				if (Point2Di(x, y).in(left, top, left + w, top + h)) {
					savedGameMenuData.selectedIndex = i;
					confirmRemove = false;
					break;
				}
				top += h + 1;
				if (top + h>SAVEGAME_MENU_LIST_BOTTOM)
					break;
			}
		} else if (button == GLUT_SCROLL_DOWN) {
			savedGameMenuData.scrollOffset = clamp(savedGameMenuData.scrollOffset + 1, 0, savedGameMenuData.savedGameList.size() - 1);
		} else if (button == GLUT_SCROLL_UP) {
			savedGameMenuData.scrollOffset = clamp(savedGameMenuData.scrollOffset - 1, 0, savedGameMenuData.savedGameList.size() - 1);
		}
	}

	void displaysaveGameSnapshotMenu() {
		initMenu();
		displayMenubackground();
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuButton::draw(icon_resume, "Load Game", left, top, false);
		if (confirmRemove) {
			MenuButton::draw(icon_no, "Don't Delete", left, top, true);
			MenuButton::draw(icon_yes, "Confime Delete", left, top, true);
		} else {
			MenuButton::draw(icon_delete, "Delete Saved Game", left, top, true);
		}
		BackButton::draw();
		displaySavedGameList();
		closeMenu();
	}
	void mouseSaveGameSnapshotMenu(int button, int state, int x, int y) {
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		mouseSavedGameList(button, state, x, y);
		if (MenuButton::clicked(button, state, x, y, left, top, false))					loadGameSnapshot();
		if (MenuButton::clicked(button, state, x, y, left, top, true))					confirmRemove = !confirmRemove;
		if (confirmRemove && MenuButton::clicked(button, state, x, y, left, top, true))	removeSavedGame();
		if (BackButton::clicked(button, state, x, y))									backGameState();
	}
	void keyboardupSaveGameSnapshotMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}


	//------------------------------------------------------------------------------------------------Campaign menu screen functions

	void displayCampaignMenu() {
		initMenu();
		displayMenubackground();

		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;

		for (int i = 0; i<campaignMenuData.data.size(); i++) {
			if (campaignMenuData.data[i].unlocked) {
				Color color = campaignMenuData.data[i].buttonColor;
				if (Point2Di(mouseX, mouseY).in(left, top, left + CAMPAIGNMENU_BUTTON_WIDTH, top + CAMPAIGNMENU_BUTTON_HEIGHT)) {
#ifndef _DEMO
					glColor4f(color.r(), color.g(), color.b(), 1.0);
#else
					if (i > 0) {
						glColor4f(color.r()*0.66, color.g()*0.66, color.b()*0.66, 1.0);
						toolTipText = "(Unavailable in Current Version)";
					} else {
						glColor4f(color.r(), color.g(), color.b(), 1.0);
					}
#endif
				} else {
					glColor4f(color.r()*0.66, color.g()*0.66, color.b()*0.66, 1.0);
				}
			} else {
				glColor4f(0.1, 0.1, 0.1, 0.5);
			}
			glDrawRectangle(left, top, left + CAMPAIGNMENU_BUTTON_WIDTH, top + CAMPAIGNMENU_BUTTON_HEIGHT);

			if (campaignMenuData.data[i].unlocked) {
				glColor4f(1.0, 1.0, 1.0, 1.0);
				glPrint(left + 5, top + CAMPAIGNMENU_BUTTON_WIDTH + CAMPAIGNMENU_BUTTON_GAP + 12, GLUT_BITMAP_HELVETICA_12)
					<< "Chapter " << toString(i + 1) << ":\n" << campaignMenuData.data[i].name;
				glColor4f(1, 1, 1, 1);
				campaignMenuData.data[i].image->bind();
				glDrawRectangle(left, top, left + CAMPAIGNMENU_BUTTON_WIDTH, top + CAMPAIGNMENU_BUTTON_WIDTH);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			left += CAMPAIGNMENU_BUTTON_GAP + CAMPAIGNMENU_BUTTON_WIDTH;
			if (left + CAMPAIGNMENU_BUTTON_GAP + CAMPAIGNMENU_BUTTON_WIDTH>Game::Settings::screenWidth) {
				top += CAMPAIGNMENU_BUTTON_GAP + CAMPAIGNMENU_BUTTON_HEIGHT;
				left = MAIN_MENU_BUTTON_LEFT;
			}
		}

		BackButton::draw();

		closeMenu();
	}
	void mouseCampaignMenu(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;

			for (int i = 0; i<campaignMenuData.data.size(); i++) {
				if (campaignMenuData.data[i].unlocked) {
					if (Point2Di(x, y).in(left, top, left + CAMPAIGNMENU_BUTTON_WIDTH, top + CAMPAIGNMENU_BUTTON_HEIGHT)) {
#ifndef _DEMO
						campaignMenuData.selectedCampaignIndex = i;
						setGameState(GAMESTATE_CHAPTER_SCREEN);
#else
						if (i <= 0) {
							campaignMenuData.selectedCampaignIndex = i;
							setGameState(GAMESTATE_CHAPTER_SCREEN);
						}
#endif
					}
				}
				left += CAMPAIGNMENU_BUTTON_GAP + CAMPAIGNMENU_BUTTON_WIDTH;
				if (left + CAMPAIGNMENU_BUTTON_GAP + CAMPAIGNMENU_BUTTON_WIDTH>Game::Settings::screenWidth) {
					top += CAMPAIGNMENU_BUTTON_GAP + CAMPAIGNMENU_BUTTON_HEIGHT;
					left = MAIN_MENU_BUTTON_LEFT;
				}
			}

			if (BackButton::clicked(button, state, x, y)) {
				backGameState();
			}
		}
	}
	void keyboardupCampaignMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	//------------------------------------------------------------------------------------------------Chapter screen functions

	void displayChapterScreen() {
		initMenu();
		displayMenubackground();

		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		if (!campaignMenuData.data[campaignMenuData.selectedCampaignIndex].intro.empty()) {
			MenuButton::draw(icon_video, "Play Intro", left, top, true);
		}
		MenuButton::draw(icon_resume, "Start Chapter", left, top, true);
		if (!campaignMenuData.data[campaignMenuData.selectedCampaignIndex].outro.empty()) {
			MenuButton::draw(icon_video, "Play Outro", left, top, true);
		}

		try {
			if (campaignMenuData.storyImage == NULL) {
				campaignMenuData.storyImage = new Texture2D();
				string name = "ui/campaign_image/" + toString(campaignMenuData.selectedCampaignIndex + 1) + ".png";
				campaignMenuData.storyImage->load(name.data(), true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			}
			left = Game::Settings::screenWidth / 2 - 100;
			top = MAIN_MENU_BUTTON_TOP;
			int width = Game::Settings::screenWidth / 2;
			int height = Game::Settings::screenHeight - top - 100;
			float scrollWindowHeight = (float(campaignMenuData.storyImage->getActualWidth()) / float(campaignMenuData.storyImage->getActualHeight())) / (float(width) / float(height));
			campaignMenuData.storyImageScrollOffset = clamp(campaignMenuData.storyImageScrollOffset + 0.00001 * (mouseY - Game::Settings::screenHeight / 2), 0, 1 - scrollWindowHeight);
			campaignMenuData.storyImage->bind();
			glColor4f(1, 1, 1, 1);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 1 - campaignMenuData.storyImageScrollOffset);
			glVertex3f(left, top, 0);
			glTexCoord2f(1, 1 - campaignMenuData.storyImageScrollOffset);
			glVertex3f(left + width, top, 0);
			glTexCoord2f(1, 1 - scrollWindowHeight - campaignMenuData.storyImageScrollOffset);
			glVertex3f(left + width, top + height, 0);
			glTexCoord2f(0, 1 - scrollWindowHeight - campaignMenuData.storyImageScrollOffset);
			glVertex3f(left, top + height, 0);
			glEnd();
			Texture2D::bindNone();
		} catch (FileNotFoundException &e) {
			//no chapter story image
		}
		try {
			if (campaignMenuData.nameImage == NULL) {
				campaignMenuData.nameImage = new Texture2D();
				string name = "ui/campaign_name/" + toString(campaignMenuData.selectedCampaignIndex + 1) + ".png";
				campaignMenuData.nameImage->load(name.data(), true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
				campaignMenuData.storyImageScrollOffset = 0;
			}
			left = MAIN_MENU_BUTTON_LEFT;
			top = Game::Settings::screenHeight - 150;
			int height = 130;
			int width = height * float(campaignMenuData.nameImage->getActualWidth()) / float(campaignMenuData.nameImage->getActualHeight());
			campaignMenuData.nameImage->bind();
			glColor4f(1, 1, 1, 1);
			glDrawRectangle(left, top, left + width, top + height);
			Texture2D::bindNone();
		} catch (FileNotFoundException &e) {
			//no chapter story image
		}
		

		BackButton::draw();

		closeMenu();
	}
	void mouseChapterScreen(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
			if (!campaignMenuData.data[campaignMenuData.selectedCampaignIndex].intro.empty()) {
				if (MenuButton::clicked(button, state, x, y, left, top, true)) {
					setGameState(GAMESTATE_INTRO_ONLY);
				}
			}
			if (MenuButton::clicked(button, state, x, y, left, top, true)) {
				loadCampaign(campaignMenuData.selectedCampaignIndex);
			}
			if (!campaignMenuData.data[campaignMenuData.selectedCampaignIndex].outro.empty()) {
				if (MenuButton::clicked(button, state, x, y, left, top, true)) {
					setGameState(GAMESTATE_OUTRO);
				}
			}

			if (BackButton::clicked(button, state, x, y))
				backGameState();
		}
	}
	void keyboardupChapterScreen(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	//------------------------------------------------------------------------------------------------Settings menu screen functions

	void displayDisplaySettingsMenu() {
		initMenu();
		displayMenubackground();

		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::draw(icon_resolution, "Screen Resolution", Game::Settings::gameModeString, Game::Settings::availableGameModeStrings.data(), Game::Settings::availableGameModeStrings.size(), left, top);
		MenuSlideBar::draw(icon_video, "Video Quality", Game::Settings::videoHeight, Game::Settings::availableVideoHeight.data(), Game::Settings::availableVideoHeight.size(), left, top);
		MenuToggleButton::draw(icon_fullscreen_on, icon_fullscreen_off, "Fullscreen", left, top, Game::Settings::fullscreen, true);
		MenuToggleButton::draw(icon_stereoscopic_on, icon_stereoscopic_off, "Stereoscopic", left, top, Game::Settings::stereoscopicOn, true);
		MenuToggleButton::draw(icon_antialiasing_on, icon_antialiasing_off, "Antialiasing", left, top, Game::Settings::antialiasingOn, true);
		MenuToggleButton::draw(icon_motion_blur_on, icon_motion_blur_off, "Motion Blur", left, top, Game::Settings::motionBlurOn, false);
		MenuToggleButton::draw(icon_terrain_bumpmap_on, icon_terrain_bumpmap_off, "Terrain Bump Mapping", left, top, Game::Settings::terrainBumpmapOn, true);
		MenuToggleButton::draw(icon_object_bumpmap_on, icon_object_bumpmap_off, "Object Bump Mapping", left, top, Game::Settings::objectBumpmapOn, true);
		MenuToggleButton::draw(icon_reflection_on, icon_reflection_off, "Reflection", left, top, Game::Settings::reflectionOn, true);
		MenuToggleButton::draw(icon_shadow_on, icon_shadow_off, "Shadow", left, top, Game::Settings::shadowOn, true);
		MenuToggleButton::draw(icon_bloom_on, icon_bloom_off, "HDR Bloom", left, top, Game::Settings::bloomOn, true);
		MenuToggleButton::draw(icon_depthoffield_on, icon_depthoffield_off, "Depth of Field", left, top, Game::Settings::depthOfFieldOn, false);
		BackButton::draw();

		closeMenu();
	}
	void mouseDisplaySettingsMenu(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
			MenuSlideBar::action(x, y, Game::Settings::gameModeString, Game::Settings::availableGameModeStrings.data(), Game::Settings::availableGameModeStrings.size(), left, top, NULL, "audio/click.ogg");
			MenuSlideBar::action(x, y, Game::Settings::videoHeight, Game::Settings::availableVideoHeight.data(), Game::Settings::availableVideoHeight.size(), left, top, NULL, "audio/click.ogg");
			if (MenuButton::clicked(button, state, x, y, left, top, true)) {
				toggleFullScreen();
			}
			if (MenuButton::clicked(button, state, x, y, left, top, true))			Game::Settings::stereoscopicOn = !Game::Settings::stereoscopicOn;
			if (MenuButton::clicked(button, state, x, y, left, top, true))			Game::Settings::antialiasingOn = !Game::Settings::antialiasingOn;
			if (MenuButton::clicked(button, state, x, y, left, top, false))			Game::Settings::motionBlurOn = !Game::Settings::motionBlurOn;
			if (MenuButton::clicked(button, state, x, y, left, top, true))			Game::Settings::terrainBumpmapOn = !Game::Settings::terrainBumpmapOn;
			if (MenuButton::clicked(button, state, x, y, left, top, true))			Game::Settings::objectBumpmapOn = !Game::Settings::objectBumpmapOn;
			if (MenuButton::clicked(button, state, x, y, left, top, true))			Game::Settings::reflectionOn = !Game::Settings::reflectionOn;
			if (MenuButton::clicked(button, state, x, y, left, top, true))			Game::Settings::shadowOn = !Game::Settings::shadowOn;
			if (MenuButton::clicked(button, state, x, y, left, top, true))			Game::Settings::bloomOn = !Game::Settings::bloomOn;
			if (MenuButton::clicked(button, state, x, y, left, top, false))			Game::Settings::depthOfFieldOn = !Game::Settings::depthOfFieldOn;

			if (BackButton::clicked(button, state, x, y))							backGameState();
		}
	}
	void motionDisplaySettingsMenu(int x, int y) {
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::action(x, y, Game::Settings::gameModeString, Game::Settings::availableGameModeStrings.data(), Game::Settings::availableGameModeStrings.size(), left, top, NULL, "audio/nosound.ogg");
		MenuSlideBar::action(x, y, Game::Settings::videoHeight, Game::Settings::availableVideoHeight.data(), Game::Settings::availableVideoHeight.size(), left, top, NULL, "audio/nosound.ogg");
	}
	void keyboardupDisplaySettingsMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	void displayAudioSettingsMenu() {
		initMenu();
		displayMenubackground();

		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::draw(icon_volume, "Volume", Game::Settings::volume, left, top);
		top += SETTINGS_MENU_SLIDEBAR_VGAP2;
		MenuSlideBar::draw(icon_music_volume, "Music Volume", Game::Settings::musicVolume, left, top);
		MenuSlideBar::draw(icon_ambient_volume, "Ambient Sound Volume", Game::Settings::ambientVolume, left, top);
		MenuSlideBar::draw(icon_sound_volume, "Other Sound Volume", Game::Settings::soundVolume, left, top);
		BackButton::draw();

		closeMenu();
	}
	void mouseAudioSettingsMenu(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
			MenuSlideBar::action(x, y, Game::Settings::volume, left, top, Game::setSoundSettings, "audio/ding.ogg", CHANNEL_SOUND);
			top += SETTINGS_MENU_SLIDEBAR_VGAP2;
			MenuSlideBar::action(x, y, Game::Settings::musicVolume, left, top, Game::setSoundSettings, "audio/ding.ogg", CHANNEL_MUSIC);
			MenuSlideBar::action(x, y, Game::Settings::ambientVolume, left, top, Game::setSoundSettings, "audio/water_splash_small1.ogg", CHANNEL_AMBIENCE_WATER);
			MenuSlideBar::action(x, y, Game::Settings::soundVolume, left, top, Game::setSoundSettings, "audio/swordattack0.ogg", CHANNEL_SOUND);

			if (BackButton::clicked(button, state, x, y))
				backGameState();
		}
	}
	void motionAudioSettingsMenu(int x, int y) {
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::action(x, y, Game::Settings::volume, left, top, Game::setSoundSettings, "audio/nosound.ogg");
		top += SETTINGS_MENU_SLIDEBAR_VGAP2;
		MenuSlideBar::action(x, y, Game::Settings::musicVolume, left, top, Game::setSoundSettings, "audio/nosound.ogg");
		MenuSlideBar::action(x, y, Game::Settings::ambientVolume, left, top, Game::setSoundSettings, "audio/nosound.ogg");
		MenuSlideBar::action(x, y, Game::Settings::soundVolume, left, top, Game::setSoundSettings, "audio/nosound.ogg");
	}
	void keyboardupAudioSettingsMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	void displayGameSettingsMenu() {
		initMenu();
		displayMenubackground();

		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::draw(icon_scrollspeed, "Scroll Speed", Game::Settings::scrollSpeed, left, top);
		BackButton::draw();

		closeMenu();
	}
	void mouseGameSettingsMenu(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
			MenuSlideBar::action(x, y, Game::Settings::scrollSpeed, left, top);

			if (BackButton::clicked(button, state, x, y))
				backGameState();
		}
	}
	void motionGameSettingsMenu(int x, int y) {
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::action(x, y, Game::Settings::scrollSpeed, left, top, NULL, "audio/nosound.ogg");
	}
	void keyboardupGameSettingsMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}


	//------------------------------------------------------------------------------------------------Skirmish menu screen functions

	int nextFreeTeam(int t) {
		if (t == -1)
			t = 0;
		for (; t<skirmishMenuData.nTeams; t++) {
			bool free = true;
			for (int s = 0; s<skirmishMenuData.startPositionTeam.size(); s++)
			if (skirmishMenuData.startPositionTeam[s] == t)
				free = false;
			if (free)
				break;
		}
		if (t == skirmishMenuData.nTeams)
			t = -1;
		return t;
	}
	int nextFreeColor(int c) {
		for (; c<arrayLength(availableTeamColors); c++) {
			bool free = true;
			for (int t = 0; t<skirmishMenuData.nTeams; t++)
			if (skirmishMenuData.teams[t].color == c)
				free = false;
			if (free)
				break;
		}
		if (c == arrayLength(availableTeamColors)) {
			for (c = 1; c<arrayLength(availableTeamColors); c++) {
				bool free = true;
				for (int t = 0; t<skirmishMenuData.nTeams; t++)
				if (skirmishMenuData.teams[t].color == c)
					free = false;
				if (free)
					break;
			}
		}
		return c;
	}
	int previousFreeColor(int c) {
		for (; c >= 0; c--) {
			bool free = true;
			for (int t = 0; t<skirmishMenuData.nTeams; t++)
			if (skirmishMenuData.teams[t].color == c)
				free = false;
			if (free)
				break;
		}
		if (c == -1) {
			for (c = arrayLength(availableTeamColors) - 1; c >= 0; c--) {
				bool free = true;
				for (int t = 0; t<skirmishMenuData.nTeams; t++)
				if (skirmishMenuData.teams[t].color == c)
					free = false;
				if (free)
					break;
			}
		}
		return c;
	}
	int firstFreeColor() {
		return nextFreeColor(1);
	}

	void displayTeams() {
		int top = SKIRMISH_TEAM_TOP, left = SKIRMISH_TEAM_LEFT;
		glColor4f(0, 0, 0, 0.5);
		glDrawRectangle(SKIRMISH_TEAM_LEFT - 10, SKIRMISH_TEAM_TOP - 10, SKIRMISH_TEAM_RIGHT + 10, SKIRMISH_TEAM_BOTTOM + 10);
		for (int i = 0; i<skirmishMenuData.nTeams; i++) {
			//Team number
			glColor4f(0.8, 0.8, 0.8, 1);
			texture_button_medium.bind();
			glDrawRectangle(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH1, top + SKIRMISH_TEAM_BUTTON_HEIGHT);
			glColor4f(0, 0, 0, 1);
			Texture2D::bindNone();
			glPrint(left, top, SKIRMISH_TEAM_BUTTON_WIDTH1, SKIRMISH_TEAM_BUTTON_HEIGHT, GLUT_BITMAP_HELVETICA_12) << i + 1;
			left += SKIRMISH_TEAM_BUTTON_WIDTH1 + SKIRMISH_TEAM_BUTTON_HGAP;
			//Controller
			if (Point2Di(mouseX, mouseY).in(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH2, top + SKIRMISH_TEAM_BUTTON_HEIGHT))
				glColor4f(1, 1, 1, 1);
			else
				glColor4f(0.8, 0.8, 0.8, 1);
			texture_button_wide.bind();
			glDrawRectangle(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH2, top + SKIRMISH_TEAM_BUTTON_HEIGHT);
			glColor4f(0, 0, 0, 1);
			Texture2D::bindNone();
			glPrint(left, top, SKIRMISH_TEAM_BUTTON_WIDTH2, SKIRMISH_TEAM_BUTTON_HEIGHT, GLUT_BITMAP_HELVETICA_12) << (skirmishMenuData.teams[i].human ? "Human" : "Computer");
			left += SKIRMISH_TEAM_BUTTON_WIDTH2 + SKIRMISH_TEAM_BUTTON_HGAP;
			//Faction
			if (Point2Di(mouseX, mouseY).in(left, top, left + 200, top + 30))
				glColor4f(1, 1, 1, 1);
			else
				glColor4f(0.8, 0.8, 0.8, 1);
			texture_button_wide.bind();
			glDrawRectangle(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH3, top + SKIRMISH_TEAM_BUTTON_HEIGHT);
			glColor4f(0, 0, 0, 1);
			Texture2D::bindNone();
			glPrint(left, top, SKIRMISH_TEAM_BUTTON_WIDTH3, SKIRMISH_TEAM_BUTTON_HEIGHT, GLUT_BITMAP_HELVETICA_12) << factionName[skirmishMenuData.teams[i].faction];
			left += SKIRMISH_TEAM_BUTTON_WIDTH3 + SKIRMISH_TEAM_BUTTON_HGAP;
			//Color
			if (Point2Di(mouseX, mouseY).in(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH4, top + SKIRMISH_TEAM_BUTTON_HEIGHT))
				glColor4f(1, 1, 1, 1);
			else
				glColor4f(0.8, 0.8, 0.8, 1);
			glColor4f(availableTeamColors[skirmishMenuData.teams[i].color].r()
				, availableTeamColors[skirmishMenuData.teams[i].color].g()
				, availableTeamColors[skirmishMenuData.teams[i].color].b(), 1);
			texture_button_color.bind();
			glDrawRectangle(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH4, top + SKIRMISH_TEAM_BUTTON_HEIGHT);
			left += SKIRMISH_TEAM_BUTTON_WIDTH4 + SKIRMISH_TEAM_BUTTON_HGAP;
			//Group
			if (Point2Di(mouseX, mouseY).in(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH5, top + SKIRMISH_TEAM_BUTTON_HEIGHT))
				glColor4f(1, 1, 1, 1);
			else
				glColor4f(0.8, 0.8, 0.8, 1);
			texture_button_medium.bind();
			glDrawRectangle(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH5, top + SKIRMISH_TEAM_BUTTON_HEIGHT);
			glColor4f(0, 0, 0, 1);
			Texture2D::bindNone();
			glPrint(left, top, SKIRMISH_TEAM_BUTTON_WIDTH5, SKIRMISH_TEAM_BUTTON_HEIGHT, GLUT_BITMAP_HELVETICA_12) << skirmishMenuData.teams[i].teamGroup;
			left += SKIRMISH_TEAM_BUTTON_WIDTH5 + SKIRMISH_TEAM_BUTTON_HGAP;
			//Remove button
			if (skirmishMenuData.nTeams>1)
				Button::draw(icon_remove, "", left, top, SKIRMISH_TEAM_SMALL_BUTTON_SIZE, SKIRMISH_TEAM_SMALL_BUTTON_SIZE);
			top += SKIRMISH_TEAM_BUTTON_VGAP;
			left = SKIRMISH_TEAM_LEFT;
		}
		//Add button
		if (skirmishMenuData.nTeams<Game::MAX_TEAM - 1 && skirmishMenuData.nTeams<skirmishMenuData.gameStartPosition.size())
			Button::draw(icon_add, "", left, top, SKIRMISH_TEAM_SMALL_BUTTON_SIZE, SKIRMISH_TEAM_SMALL_BUTTON_SIZE);
	}
	void displaySkirmishMap() {
		loadSkirmishMapHeader();
		//Panel
		glColor4f(0, 0, 0, 0.5);
		glDrawRectangle(SKIRMISH_MAP_LEFT, SKIRMISH_MAP_TOP, SKIRMISH_MAP_RIGHT, SKIRMISH_MAP_BOTTOM);

		if (!skirmishMenuData.selectedMapName.empty()) {
			int top = SKIRMISH_MAP_TOP + SKIRMISH_MAP_BORDER, left = SKIRMISH_MAP_LEFT + SKIRMISH_MAP_BORDER, w = SKIRMISH_MAP_SIZE;
			int minimapLeft, minimapTop, minimapWidth, minimapHeight;

			//Map
			if (skirmishMenuData.selectedMapImage) {
				skirmishMenuData.selectedMapImage->bind();
				glColor3f(1, 1, 1);
				float size = clampHigh(min((float)SKIRMISH_MAP_SIZE / skirmishMenuData.mapWidth, (float)SKIRMISH_MAP_SIZE / skirmishMenuData.mapHeight), 1.0);
				minimapWidth = SKIRMISH_MAP_SIZE*((float)skirmishMenuData.mapWidth / SKIRMISH_MAP_SIZE)*size;
				minimapHeight = SKIRMISH_MAP_SIZE*((float)skirmishMenuData.mapHeight / SKIRMISH_MAP_SIZE)*size;
				minimapLeft = left + (SKIRMISH_MAP_SIZE - minimapWidth) / 2;
				minimapTop = top + (SKIRMISH_MAP_SIZE - minimapHeight) / 2;
				glDrawRectangle(minimapLeft, minimapTop, left + (SKIRMISH_MAP_SIZE + minimapWidth) / 2, top + (SKIRMISH_MAP_SIZE + minimapHeight) / 2);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//Start Positions
			for (int s = 0; s<skirmishMenuData.gameStartPosition.size(); s++) {
				if (skirmishMenuData.startPositionTeam[s] >= 0) {
					glColor4f(availableTeamColors[skirmishMenuData.teams[skirmishMenuData.startPositionTeam[s]].color].r(), availableTeamColors[skirmishMenuData.teams[skirmishMenuData.startPositionTeam[s]].color].g(), availableTeamColors[skirmishMenuData.teams[skirmishMenuData.startPositionTeam[s]].color].b(), 1);
					icon_startposition.bind();
				} else {
					glColor4f(0, 0, 0, 1);
					icon_startposition_empty.bind();
				}
				glDrawRectangle(minimapLeft + skirmishMenuData.gameStartPosition[s].x*minimapWidth / skirmishMenuData.mapWidth - SKIRMISH_MAP_STRTPOS_BTN_SIZE,
					minimapTop + skirmishMenuData.gameStartPosition[s].y*minimapHeight / skirmishMenuData.mapHeight - SKIRMISH_MAP_STRTPOS_BTN_SIZE,
					minimapLeft + skirmishMenuData.gameStartPosition[s].x*minimapWidth / skirmishMenuData.mapWidth + SKIRMISH_MAP_STRTPOS_BTN_SIZE,
					minimapTop + skirmishMenuData.gameStartPosition[s].y*minimapHeight / skirmishMenuData.mapHeight + SKIRMISH_MAP_STRTPOS_BTN_SIZE);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//Map size
			top += SKIRMISH_MAP_SIZE + SKIRMISH_MAP_BORDER;
			glColor4f(1, 1, 1, 1);
			glPrint(left + 2, top, GLUT_BITMAP_HELVETICA_10)
				<< "Max " << (int)skirmishMenuData.gameStartPosition.size() << " Players : "
				<< skirmishMenuData.mapWidth << "x" << skirmishMenuData.mapHeight;

			//Map names
			top = SKIRMISH_MAPNAMES_TOP;
			skirmishMenuData.mouseOverMapIndex = -1;
			for (int m = skirmishMenuData.mapScrollOffset; m<skirmishMenuData.selectedMapName.size(); m++) {
				glColor4f(0, 0, 0, 0.5);
				glDrawRectangle(SKIRMISH_MAPNAMES_LEFT, top, SKIRMISH_MAPNAMES_RIGHT, top + SKIRMISH_MAPNAMES_BTN_HEIGHT);
				if (m == skirmishMenuData.selectedMapIndex)
					glColor4f(1, 1, 1, 1);
				else if (Point2Di(mouseX, mouseY).in(SKIRMISH_MAPNAMES_LEFT, top, SKIRMISH_MAPNAMES_RIGHT, top + SKIRMISH_MAPNAMES_BTN_HEIGHT)) {
					glColor4f(1, 1, 1, 0.9);
					if (currentGameState == GAMESTATE_EDITORMENU)
						skirmishMenuData.mouseOverMapIndex = m;
				} else
					glColor4f(1, 1, 1, 0.7);
				glPrint(SKIRMISH_MAPNAMES_LEFT + 2, top + 12, GLUT_BITMAP_HELVETICA_12) << removeExtension(skirmishMenuData.selectedMapName[m]).data();
				top += SKIRMISH_MAPNAMES_BTN_HEIGHT + SKIRMISH_MAPNAMES_BTN_VGAP;
				if (top >= SKIRMISH_MAPNAMES_BOTTOM)
					break;
			}

			//Scrollbar
			glColor4f(0, 0, 0, 0.5);
			int scroll_pos = (SKIRMISH_MAPNAMES_BOTTOM - SKIRMISH_MAPNAMES_TOP)*skirmishMenuData.mapScrollOffset / skirmishMenuData.selectedMapName.size();
			glDrawRectangle(SKIRMISH_MAP_SCROLLBAR_LEFT, (SKIRMISH_MAPNAMES_BOTTOM + SKIRMISH_MAPNAMES_TOP) / 2, SKIRMISH_MAP_SCROLLBAR_RIGHT, SKIRMISH_MAPNAMES_TOP + scroll_pos);
		}
	}
	void mouseSkirmishMapSelect(int button, int state, int x, int y, bool startPositionSelect = false) {
		//Map
		int top = SKIRMISH_MAP_TOP + SKIRMISH_MAP_BORDER;
		int left = SKIRMISH_MAP_LEFT + SKIRMISH_MAP_BORDER;
		//Start position Select
		if (startPositionSelect) {
			if (Point2Di(mouseX, mouseY).in(left, top, left + SKIRMISH_MAP_SIZE, top + SKIRMISH_MAP_SIZE)) {
				float size = clampHigh(min((float)SKIRMISH_MAP_SIZE / skirmishMenuData.mapWidth, (float)SKIRMISH_MAP_SIZE / skirmishMenuData.mapHeight), 1.0);
				float minimapWidth = SKIRMISH_MAP_SIZE*((float)skirmishMenuData.mapWidth / SKIRMISH_MAP_SIZE)*size;
				float minimapHeight = SKIRMISH_MAP_SIZE*((float)skirmishMenuData.mapHeight / SKIRMISH_MAP_SIZE)*size;
				float minimapLeft = left + (SKIRMISH_MAP_SIZE - minimapWidth) / 2;
				float minimapTop = top + (SKIRMISH_MAP_SIZE - minimapHeight) / 2;
				for (int s = 0; s<skirmishMenuData.gameStartPosition.size(); s++) {
					if (button == GLUT_LEFT_BUTTON) {
						if (Point2Di(mouseX, mouseY).in(
							minimapLeft + skirmishMenuData.gameStartPosition[s].x*minimapWidth / skirmishMenuData.mapWidth - SKIRMISH_MAP_STRTPOS_BTN_SIZE,
							minimapTop + skirmishMenuData.gameStartPosition[s].y*minimapHeight / skirmishMenuData.mapHeight - SKIRMISH_MAP_STRTPOS_BTN_SIZE,
							minimapLeft + skirmishMenuData.gameStartPosition[s].x*minimapWidth / skirmishMenuData.mapWidth + SKIRMISH_MAP_STRTPOS_BTN_SIZE,
							minimapTop + skirmishMenuData.gameStartPosition[s].y*minimapHeight / skirmishMenuData.mapHeight + SKIRMISH_MAP_STRTPOS_BTN_SIZE))
							skirmishMenuData.startPositionTeam[s] = nextFreeTeam(skirmishMenuData.startPositionTeam[s]);
					} else {
						if (Point2Di(mouseX, mouseY).in(
							minimapLeft + skirmishMenuData.gameStartPosition[s].x*minimapWidth / skirmishMenuData.mapWidth - SKIRMISH_MAP_STRTPOS_BTN_SIZE,
							minimapTop + skirmishMenuData.gameStartPosition[s].y*minimapHeight / skirmishMenuData.mapHeight - SKIRMISH_MAP_STRTPOS_BTN_SIZE,
							minimapLeft + skirmishMenuData.gameStartPosition[s].x*minimapWidth / skirmishMenuData.mapWidth + SKIRMISH_MAP_STRTPOS_BTN_SIZE,
							minimapTop + skirmishMenuData.gameStartPosition[s].y*minimapHeight / skirmishMenuData.mapHeight + SKIRMISH_MAP_STRTPOS_BTN_SIZE))
							skirmishMenuData.startPositionTeam[s] = -1;
					}
				}
			}
		}

		//Map select
		top = SKIRMISH_MAPNAMES_TOP;
		int w = SKIRMISH_MAP_SIZE;
		for (int m = skirmishMenuData.mapScrollOffset; m<skirmishMenuData.selectedMapName.size(); m++) {
			if (Point2Di(mouseX, mouseY).in(SKIRMISH_MAPNAMES_LEFT, top, SKIRMISH_MAPNAMES_RIGHT, top + SKIRMISH_MAPNAMES_BTN_HEIGHT)) {
				if (button == GLUT_LEFT_BUTTON) {
					skirmishMenuData.selectedMapIndex = m;
					break;
				}
			}
			top += SKIRMISH_MAPNAMES_BTN_HEIGHT + SKIRMISH_MAPNAMES_BTN_VGAP;
			if (top >= SKIRMISH_MAPNAMES_BOTTOM)
				break;
		}
		if (button == GLUT_SCROLL_DOWN)
			skirmishMenuData.mapScrollOffset = clamp(skirmishMenuData.mapScrollOffset + 1, 0, skirmishMenuData.selectedMapName.size() - 1);
		else if (button == GLUT_SCROLL_UP)
			skirmishMenuData.mapScrollOffset = clamp(skirmishMenuData.mapScrollOffset - 1, 0, skirmishMenuData.selectedMapName.size() - 1);
	}

	void displaySkirmishMenu() {
		initMenu();
		displayMenubackground();

		displayTeams();
		displaySkirmishMap();
		ToggleButton::draw(icon_resume, icon_resume, "Start Game", SKIRMISH_MAP_LEFT - 250, Game::Settings::screenHeight - 150, 75, 75);
		BackButton::draw();

		closeMenu();
	}
	void mouseSkirmishMenu(int button, int state, int x, int y) {
		if (state == GLUT_UP) {
			//Back Button
			if (BackButton::clicked(button, state, x, y)) {
				backGameState();
			}

			//Edit skirmish settings
			int top = SKIRMISH_TEAM_TOP, left = SKIRMISH_TEAM_LEFT;
			for (int i = 0; i<skirmishMenuData.nTeams; i++) {
				//Team number
				left += SKIRMISH_TEAM_BUTTON_WIDTH1 + SKIRMISH_TEAM_BUTTON_HGAP;
				//Controller
				if (Point2Di(x, y).in(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH2, top + SKIRMISH_TEAM_BUTTON_HEIGHT)) {
					skirmishMenuData.teams[i].human = !skirmishMenuData.teams[i].human;
				}
				left += SKIRMISH_TEAM_BUTTON_WIDTH2 + SKIRMISH_TEAM_BUTTON_HGAP;
				//Faction
				if (button == GLUT_LEFT_BUTTON) {
					if (Point2Di(x, y).in(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH3, top + SKIRMISH_TEAM_BUTTON_HEIGHT)) {
						skirmishMenuData.teams[i].faction = (skirmishMenuData.teams[i].faction + 1) % arrayLength(factionName);
						if (skirmishMenuData.teams[i].faction == 0)
							skirmishMenuData.teams[i].faction = 1;
					}
				} else {
					if (Point2Di(x, y).in(left, top, left + 200, top + 30)) {
						skirmishMenuData.teams[i].faction = (skirmishMenuData.teams[i].faction - 1 + arrayLength(factionName)) % arrayLength(factionName);
						if (skirmishMenuData.teams[i].faction == 0)
							skirmishMenuData.teams[i].faction = arrayLength(factionName) - 1;
					}
				}
				left += SKIRMISH_TEAM_BUTTON_WIDTH3 + SKIRMISH_TEAM_BUTTON_HGAP;
				//Color
				if (button == GLUT_LEFT_BUTTON) {
					if (Point2Di(x, y).in(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH4, top + SKIRMISH_TEAM_BUTTON_HEIGHT)) {
						skirmishMenuData.teams[i].color = nextFreeColor(skirmishMenuData.teams[i].color);
						/*teams[i].color=(teams[i].color+1)%arrayLength(availableTeamColors);
						if(teams[i].color==0)
						teams[i].color=1;*/
					}
				} else {
					if (Point2Di(x, y).in(left, top, left + 50, top + 30)) {
						skirmishMenuData.teams[i].color = (skirmishMenuData.teams[i].color - 1 + arrayLength(availableTeamColors)) % arrayLength(availableTeamColors);
						if (skirmishMenuData.teams[i].color == 0)
							skirmishMenuData.teams[i].color = arrayLength(availableTeamColors) - 1;
					}
				}
				left += SKIRMISH_TEAM_BUTTON_WIDTH4 + SKIRMISH_TEAM_BUTTON_HGAP;
				//Group
				if (button == GLUT_LEFT_BUTTON) {
					if (Point2Di(x, y).in(left, top, left + SKIRMISH_TEAM_BUTTON_WIDTH5, top + SKIRMISH_TEAM_BUTTON_HEIGHT)) {
						skirmishMenuData.teams[i].teamGroup = (skirmishMenuData.teams[i].teamGroup + 1) % Game::MAX_TEAM;
						if (skirmishMenuData.teams[i].teamGroup == 0)
							skirmishMenuData.teams[i].teamGroup = 1;
					}
				} else {
					if (Point2Di(x, y).in(left, top, left + 50, top + 30)) {
						skirmishMenuData.teams[i].teamGroup = (skirmishMenuData.teams[i].teamGroup - 1 + Game::MAX_TEAM) % Game::MAX_TEAM;
						if (skirmishMenuData.teams[i].teamGroup == 0)
							skirmishMenuData.teams[i].teamGroup = Game::MAX_TEAM - 1;
					}
				}
				left += SKIRMISH_TEAM_BUTTON_WIDTH5 + SKIRMISH_TEAM_BUTTON_HGAP;
				//Remove team button
				if (skirmishMenuData.nTeams>1) {
					if (Point2Di(x, y).in(left, top, left + SKIRMISH_TEAM_SMALL_BUTTON_SIZE, top + SKIRMISH_TEAM_SMALL_BUTTON_SIZE)) {
						for (int t = i; t<skirmishMenuData.nTeams - 1; t++)
							skirmishMenuData.teams[t] = skirmishMenuData.teams[t + 1];
						skirmishMenuData.nTeams--;
						return;
					}
				}
				top += SKIRMISH_TEAM_BUTTON_VGAP;
				left = SKIRMISH_TEAM_LEFT;
			}
			//Add team button
			if (skirmishMenuData.nTeams<Game::MAX_TEAM - 1 && skirmishMenuData.nTeams<skirmishMenuData.gameStartPosition.size()) {
				if (Point2Di(x, y).in(left, top, left + SKIRMISH_TEAM_SMALL_BUTTON_SIZE, top + SKIRMISH_TEAM_SMALL_BUTTON_SIZE))
					skirmishMenuData.teams[skirmishMenuData.nTeams++] = Team(firstFreeColor(), false, 1, 2);
			}

			//Map
			mouseSkirmishMapSelect(button, state, x, y, true);

			//Start Game button
			if (Point2Di(x, y).in(SKIRMISH_MAP_LEFT - 250, Game::Settings::screenHeight - 150, SKIRMISH_MAP_LEFT - 175, Game::Settings::screenHeight - 75)) {
				setGameState(GAMESTATE_LOADING_MAP_PHASE1);
				loadingGameThread = SDL_CreateThread(loadingSkirmishMapPhase1, NULL);
			}
		}
	}
	void keyboardupSkirmishMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	//------------------------------------------------------------------------------------------------Network menu screen functions

	void displayNetworkMenu() {
		initMenu();
		displayMenubackground();

		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuButton::draw(icon_hostgame, "Host Game", left, top, false);
		MenuButton::draw(icon_joingame, "Join Game", left, top, false);
		BackButton::draw();

		closeMenu();
	}
	void mouseNetworkMenu(int button, int state, int x, int y) {
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		if (MenuButton::clicked(button, state, x, y, left, top, false))		setGameState(GAMESTATE_HOSTGAMEMENU);
		if (MenuButton::clicked(button, state, x, y, left, top, false))		setGameState(GAMESTATE_JOINGAMEMENU);
		if (BackButton::clicked(button, state, x, y))						backGameState();
	}
	void keyboardupNetworkMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	//------------------------------------------------------------------------------------------------Host Game menu screen functions

	void displayHostGameMenu() {
		displaySkirmishMenu();
	}
	void mouseHostGameMenu(int button, int state, int x, int y) {
		mouseSkirmishMenu(button, state, x, y);
	}
	void keyboardupHostGameMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	//------------------------------------------------------------------------------------------------Join Game menu screen functions

	void displayJoinGameMenu() {
		initMenu();
		displayMenubackground();

		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		BackButton::draw();

		closeMenu();
	}
	void mouseJoinGameMenu(int button, int state, int x, int y) {
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		if (BackButton::clicked(button, state, x, y))				backGameState();
	}
	void keyboardupJoinGameMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}


	//------------------------------------------------------------------------------------------------Editor menu functions

	void displayEditorMenu() {
		initMenu();
		displayMenubackground();

		displaySkirmishMap();
		int top = MAIN_MENU_BUTTON_TOP, left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::draw(icon_width, "New Map Width", newMapData.width, MIN_MAP_SIZE, MAX_MAP_SIZE, left, top, EDITOR_MENU_SLIDEBAR_WIDTH);
		MenuSlideBar::draw(icon_height, "New Map Height", newMapData.height, MIN_MAP_SIZE, MAX_MAP_SIZE, left, top, EDITOR_MENU_SLIDEBAR_WIDTH);
		MenuSlideBar::draw(icon_ground_height, "New Map Ground Height", newMapData.groundHeight, MIN_MAP_GROUND_HEIGHT, MAX_MAP_GROUND_HEIGHT, left, top, EDITOR_MENU_SLIDEBAR_WIDTH);
		MenuButton::draw(icon_newmap, "Create New Map", left, top, true);
		MenuButton::draw(icon_editmap, "Edit Map", left, top, true);
		MenuButton::draw(icon_resume, "Test Map", left, top, true);
		BackButton::draw();

		closeMenu();
	}
	void mouseEditorMenu(int button, int state, int x, int y) {
		if (state == GLUT_UP) {
			//Map Select
			mouseSkirmishMapSelect(button, state, x, y);

			//Buttons
			int top = MAIN_MENU_BUTTON_TOP;
			int left = MAIN_MENU_BUTTON_LEFT;
			MenuSlideBar::action(x, y, newMapData.width, MIN_MAP_SIZE, MAX_MAP_SIZE, left, top, NULL, "audio/click.ogg", CHANNEL_SOUND, EDITOR_MENU_SLIDEBAR_WIDTH);
			MenuSlideBar::action(x, y, newMapData.height, MIN_MAP_SIZE, MAX_MAP_SIZE, left, top, NULL, "audio/click.ogg", CHANNEL_SOUND, EDITOR_MENU_SLIDEBAR_WIDTH);
			MenuSlideBar::action(x, y, newMapData.groundHeight, MIN_MAP_GROUND_HEIGHT, MAX_MAP_GROUND_HEIGHT, left, top, NULL, "audio/click.ogg", CHANNEL_SOUND, EDITOR_MENU_SLIDEBAR_WIDTH);
			if (MenuButton::clicked(button, state, x, y, left, top, true))	setGameState(GAMESTATE_LOADING_MAP_PHASE1), loadingGameThread = SDL_CreateThread(loadingNewMapPhase1, NULL);
			if (MenuButton::clicked(button, state, x, y, left, top, true))	setGameState(GAMESTATE_LOADING_MAP_PHASE1), loadingGameThread = SDL_CreateThread(loadingEditMapPhase1, NULL);
			if (MenuButton::clicked(button, state, x, y, left, top, true))	setGameState(GAMESTATE_LOADING_MAP_PHASE1), loadingGameThread = SDL_CreateThread(loadingMapPhase1, NULL);
			if (BackButton::clicked(button, state, x, y))					backGameState();
		}
	}
	void motionEditorMenu(int x, int y) {
		int top = MAIN_MENU_BUTTON_TOP;
		int left = MAIN_MENU_BUTTON_LEFT;
		MenuSlideBar::action(x, y, newMapData.width, MIN_MAP_SIZE, MAX_MAP_SIZE, left, top, NULL, "audio/click.ogg", CHANNEL_SOUND, EDITOR_MENU_SLIDEBAR_WIDTH);
		MenuSlideBar::action(x, y, newMapData.height, MIN_MAP_SIZE, MAX_MAP_SIZE, left, top, NULL, "audio/click.ogg", CHANNEL_SOUND, EDITOR_MENU_SLIDEBAR_WIDTH);
		MenuSlideBar::action(x, y, newMapData.groundHeight, MIN_MAP_GROUND_HEIGHT, MAX_MAP_GROUND_HEIGHT, left, top, NULL, "audio/click.ogg", CHANNEL_SOUND, EDITOR_MENU_SLIDEBAR_WIDTH);
	}
	void keyboardupEditorMenu(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}


	//------------------------------------------------------------------------------------------------Credits screen functions

	static float creditsBackgroundOffset = 0.0;

	void displayCredits() {
		initMenu();

		//creditsBackgroundOffset = clamp(creditsBackgroundOffset + 0.01*(mouseY - Game::Settings::screenHeight / 2) / Game::Settings::screenHeight, -0.25, ((CREDITS_IMAGE_SIZE_RATIO-1) / CREDITS_IMAGE_SIZE_RATIO) + 0.25);
		float scrollWindowHeight = (float(image_credits.getActualWidth()) / float(image_credits.getActualHeight())) / (float(Game::Settings::screenWidth) / float(Game::Settings::screenHeight));
		creditsBackgroundOffset = clamp(creditsBackgroundOffset + 0.00001 * (mouseY - Game::Settings::screenHeight / 2), 0, 1 - scrollWindowHeight);
		glColor3f(1, 1, 1);
		image_credits.bind();
		glBegin(GL_QUADS);
		float endOffset = 1 - 1.0 / CREDITS_IMAGE_SIZE_RATIO - creditsBackgroundOffset;
		glTexCoord2f(0, 1 - creditsBackgroundOffset); glVertex2f(0, 0);
		glTexCoord2f(0, endOffset); glVertex2f(0, Game::Settings::screenHeight);
		glTexCoord2f(1, endOffset); glVertex2f(Game::Settings::screenWidth, Game::Settings::screenHeight);
		glTexCoord2f(1, 1 - creditsBackgroundOffset); glVertex2f(Game::Settings::screenWidth, 0);
		glEnd();

		BackButton::draw();
		closeMenu();

		syncFrameRate(FRAME_RATE);
	}
	void mouseCredits(int button, int state, int x, int y) {
		if (BackButton::clicked(button, state, x, y))			backGameState();
	}
	void keyboardupCredits(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}

	//------------------------------------------------------------------------------------------------Controls screen functions

	static float controlsBackgroundOffset = 0.0;

	void displayControls() {
		initMenu();

		float scrollWindowHeight = (float(image_controls.getActualWidth()) / float(image_controls.getActualHeight())) / (float(Game::Settings::screenWidth) / float(Game::Settings::screenHeight));
		controlsBackgroundOffset = clamp(controlsBackgroundOffset + 0.00001 * (mouseY - Game::Settings::screenHeight / 2), 0, 1 - scrollWindowHeight);
		glColor3f(1, 1, 1);
		image_controls.bind();
		glBegin(GL_QUADS);
		float endOffset = 1 - 1.0 / CONTROLS_IMAGE_SIZE_RATIO - controlsBackgroundOffset;
		glTexCoord2f(0, 1 - controlsBackgroundOffset); glVertex2f(0, 0);
		glTexCoord2f(0, endOffset); glVertex2f(0, Game::Settings::screenHeight);
		glTexCoord2f(1, endOffset); glVertex2f(Game::Settings::screenWidth, Game::Settings::screenHeight);
		glTexCoord2f(1, 1 - controlsBackgroundOffset); glVertex2f(Game::Settings::screenWidth, 0);
		glEnd();

		BackButton::draw();
		closeMenu();

		syncFrameRate(FRAME_RATE);
	}
	void mouseControls(int button, int state, int x, int y) {
		if (BackButton::clicked(button, state, x, y))			backGameState();
	}
	void keyboardupControls(unsigned char key, int x, int y) {
		switch (key) {
		case GLUT_KEY_ESCAPE:
			backGameState();
			break;
		}
	}


	//------------------------------------------------------------------------------------------------Display callback function

	void _GLUT_CALLBACK display() {
		try {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			switch (currentGameState) {
			case GAMESTATE_LOADING_GAME_PHASE2:
				loadingGamePhase2();
				break;
			case GAMESTATE_LOADING_MAP_PHASE2:
				loadingMapPhase2();
				break;
			case GAMESTATE_LOADING_GAME_PHASE1:
			case GAMESTATE_LOADING_MAP_PHASE1:
				displayGameLoading();
				break;
			case GAMESTATE_MAINMENU:
				displayMainMenu();
				break;
			case GAMESTATE_SETTINGSMENU_DISPLAY:
				displayDisplaySettingsMenu();
				break;
			case GAMESTATE_SETTINGSMENU_AUDIO:
				displayAudioSettingsMenu();
				break;
			case GAMESTATE_SETTINGSMENU_GAME:
				displayGameSettingsMenu();
				break;
			case GAMESTATE_CAMPAIGNMENU:
				displayCampaignMenu();
				break;
			case GAMESTATE_CHAPTER_SCREEN:
				displayChapterScreen();
				break;
			case GAMESTATE_SKIRMISHMENU:
				displaySkirmishMenu();
				break;
			case GAMESTATE_NETWORKMENU:
				displayNetworkMenu();
				break;
			case GAMESTATE_HOSTGAMEMENU:
				displayHostGameMenu();
				break;
			case GAMESTATE_JOINGAMEMENU:
				displayJoinGameMenu();
				break;
			case GAMESTATE_PLAYING:
				displayGamePlaying();
				break;
			case GAMESTATE_EDITORMENU:
				displayEditorMenu();
				break;
			case GAMESTATE_SAVEGAMESNAPSHOT_MENU:
				displaysaveGameSnapshotMenu();
				break;
			case GAMESTATE_CREDITS_SCREEN:
				displayCredits();
				break;
			case GAMESTATE_CONTROLS_SCREEN:
				displayControls();
				break;
			case GAMESTATE_INTRO:
				displayCutscene(true, true);
				break;
			case GAMESTATE_INTRO_ONLY:
				displayCutscene(true, false);
				break;
			case GAMESTATE_OUTRO:
				displayCutscene(false, false);
				break;
			}

			glFlush();
			glutPostRedisplay();
			glutSwapBuffers();

			SteamAPI_RunCallbacks();
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception : display() CALLBACK", true);
		}
	}


	//------------------------------------------------------------------------------------------------Reshape callback function

	void _GLUT_CALLBACK reshape(int w, int h) {
		h = (h == 0) ? 1 : h;
		Game::Settings::screenWidth = w, Game::Settings::screenHeight = h;
		glViewport(0, 0, Game::Settings::screenWidth, Game::Settings::screenHeight);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, (float)Game::Settings::screenWidth / Game::Settings::screenHeight, 0.1, 1000.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}


	//------------------------------------------------------------------------------------------------Mouse callback functions

	void _GLUT_CALLBACK passivemotion(int x, int y) {
		switch (currentGameState) {
		case GAMESTATE_PLAYING:
			passivemotionGamePlaying(x, y);
			break;
		}
		mouseX = x, mouseY = y;
		dragStartX = x, dragStartY = y;
	}
	void _GLUT_CALLBACK mouse(int button, int state, int x, int y) {
		static clock_t lastClickTime = 0;
		if (button<5)
		if (state == GLUT_UP) {
			buttonUpTime = SDL_GetTicks();
			buttonPressed[button] = false;
		} else {
			buttonDownTime = SDL_GetTicks();
			buttonPressed[button] = true;
		}
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && x == dragStartX && y == dragStartY && SDL_GetTicks() - lastClickTime <= DOUBLE_CLICK_DELAY)
			doubleClicked = true;
		dragboxEnabled = false;

		switch (currentGameState) {
		case GAMESTATE_LOADING_GAME_PHASE1:
			break;
		case GAMESTATE_MAINMENU:
			mouseMainMenu(button, state, x, y);
			break;
		case GAMESTATE_SETTINGSMENU_DISPLAY:
			mouseDisplaySettingsMenu(button, state, x, y);
			break;
		case GAMESTATE_SETTINGSMENU_AUDIO:
			mouseAudioSettingsMenu(button, state, x, y);
			break;
		case GAMESTATE_SETTINGSMENU_GAME:
			mouseGameSettingsMenu(button, state, x, y);
			break;
		case GAMESTATE_CAMPAIGNMENU:
			mouseCampaignMenu(button, state, x, y);
			break;
		case GAMESTATE_CHAPTER_SCREEN:
			mouseChapterScreen(button, state, x, y);
			break;
		case GAMESTATE_SKIRMISHMENU:
			mouseSkirmishMenu(button, state, x, y);
			break;
		case GAMESTATE_NETWORKMENU:
			mouseNetworkMenu(button, state, x, y);
			break;
		case GAMESTATE_JOINGAMEMENU:
			mouseJoinGameMenu(button, state, x, y);
			break;
		case GAMESTATE_HOSTGAMEMENU:
			mouseHostGameMenu(button, state, x, y);
			break;
		case GAMESTATE_PLAYING:
			mouseGamePlaying(button, state, x, y);
			break;
		case GAMESTATE_EDITORMENU:
			mouseEditorMenu(button, state, x, y);
			break;
		case GAMESTATE_SAVEGAMESNAPSHOT_MENU:
			mouseSaveGameSnapshotMenu(button, state, x, y);
			break;
		case GAMESTATE_CREDITS_SCREEN:
			mouseCredits(button, state, x, y);
			break;
		case GAMESTATE_CONTROLS_SCREEN:
			mouseControls(button, state, x, y);
			break;
		}

		if (button == GLUT_LEFT_BUTTON || button == GLUT_MIDDLE_BUTTON || button == GLUT_RIGHT_BUTTON) dragStartX = x, dragStartY = y;
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) lastClickTime = SDL_GetTicks();
		doubleClicked = false;
	}
	void _GLUT_CALLBACK motion(int x, int y) {
		switch (currentGameState) {
		case GAMESTATE_SETTINGSMENU_DISPLAY:
			motionDisplaySettingsMenu(x, y);
			break;
		case GAMESTATE_SETTINGSMENU_AUDIO:
			motionAudioSettingsMenu(x, y);
			break;
		case GAMESTATE_SETTINGSMENU_GAME:
			motionGameSettingsMenu(x, y);
			break;
		case GAMESTATE_EDITORMENU:
			motionEditorMenu(x, y);
			break;
		case GAMESTATE_PLAYING:
			motionGamePlaying(x, y);
			break;
		}
		mouseX = x, mouseY = y;
	}

	//------------------------------------------------------------------------------------------------Keyboard callback functions

	void _GLUT_CALLBACK keyboard(unsigned char key, int x, int y) {
		if (key<127)
			keyPressed[key] = true;
		switch (currentGameState) {
		case GAMESTATE_PLAYING:
			keyboardGamePlaying(key, x, y);
			break;
		}
	}
	void _GLUT_CALLBACK keyboardup(unsigned char key, int x, int y) {
		if (key<127)
			keyPressed[key] = false;
		if (glutGetModifiers() == GLUT_ACTIVE_ALT && key == GLUT_KEY_RETURN) {
			toggleFullScreen();
		}
		switch (currentGameState) {
		case GAMESTATE_PLAYING:
			keyboardupGamePlaying(key, x, y);
			break;
		case GAMESTATE_MAINMENU:
			keyboardupMainMenu(key, x, y);
			break;
		case GAMESTATE_CAMPAIGNMENU:
			keyboardupCampaignMenu(key, x, y);
			break;
		case GAMESTATE_CHAPTER_SCREEN:
			keyboardupChapterScreen(key, x, y);
			break;
		case GAMESTATE_SETTINGSMENU_DISPLAY:
			keyboardupDisplaySettingsMenu(key, x, y);
			break;
		case GAMESTATE_SETTINGSMENU_AUDIO:
			keyboardupAudioSettingsMenu(key, x, y);
			break;
		case GAMESTATE_SETTINGSMENU_GAME:
			keyboardupGameSettingsMenu(key, x, y);
			break;
		case GAMESTATE_INTRO:
			keyboardupCutscene(key, x, y, true);
			break;
		case GAMESTATE_INTRO_ONLY:
		case GAMESTATE_OUTRO:
			keyboardupCutscene(key, x, y, false);
			break;
		case GAMESTATE_EDITORMENU:
			keyboardupEditorMenu(key, x, y);
			break;
		case GAMESTATE_NETWORKMENU:
			keyboardupNetworkMenu(key, x, y);
			break;
		case GAMESTATE_HOSTGAMEMENU:
			keyboardupHostGameMenu(key, x, y);
			break;
		case GAMESTATE_JOINGAMEMENU:
			keyboardupJoinGameMenu(key, x, y);
			break;
		case GAMESTATE_CREDITS_SCREEN:
			keyboardupCredits(key, x, y);
			break;
		case GAMESTATE_CONTROLS_SCREEN:
			keyboardupControls(key, x, y);
			break;
		}
	}
	void _GLUT_CALLBACK special(int key, int x, int y) {
		specialPressed[key] = true;
		switch (currentGameState) {
		case GAMESTATE_PLAYING:
			specialGamePlaying(key, x, y);
			break;
		}
	}
	void _GLUT_CALLBACK specialup(int key, int x, int y) {
		specialPressed[key] = false;
		if (key == GLUT_KEY_F4 && glutGetModifiers() == GLUT_ACTIVE_ALT)
			glutLeaveMainLoop();
		switch (currentGameState) {
		case GAMESTATE_PLAYING:
			specialupGamePlaying(key, x, y);
			break;
		}
	}

	//------------------------------------------------------------------------------------------------Idle callback function

	void _GLUT_CALLBACK idle() {
		try {
			switch (currentGameState) {
			case GAMESTATE_PLAYING:
				idleGamePlaying();
				break;
			}
		} catch (Exception &e) {
			showMessage(e.getMessage(), "Runtime Exception", true);
		} catch (...) {}
	}


	//------------------------------------------------------------------------------------------------Register callbacks
	void registerGLUTCallBacks() {
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutMouseFunc(mouse);
		glutKeyboardFunc(keyboard);
		glutKeyboardUpFunc(keyboardup);
		glutSpecialFunc(special);
		glutSpecialUpFunc(specialup);
		glutMotionFunc(motion);
		glutPassiveMotionFunc(passivemotion);
		glutIdleFunc(idle);
	}


	//------------------------------------------------------------------------------------------------Init
	void initInterface() {
		currentGameState = GAMESTATE_LOADING_GAME_PHASE1;
		gameMutex.init();
		loadInitialResource();
	}
};
