#include "stdafx.h"

#include "interface.h"
#include "common.h"
#include "team.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"
#include "steamStatsAndAchievements.h"

namespace ramayana {

	const float Game::MOVABLE_MAX_HEIGHT = 0.5;
	const float Game::MAX_SUITABLE_UNEVEN_BULDING_LAND = 0.5;

	void Game::getMapHeader(string fname, vector<Point2D> &startPos, Texture2D &texture, int &width, int &height) {
		string tex_filename = removeExtension(fname) + "_texture.bmp";
		string ter_filename = removeExtension(fname) + "_terrain.bmp";
		IplImage *tex = cvLoadImage(tex_filename.data(), CV_LOAD_IMAGE_COLOR);
		if (tex == NULL) throw FileNotFoundException(tex_filename);
		IplImage *ter = cvLoadImage(ter_filename.data(), CV_LOAD_IMAGE_GRAYSCALE);
		if (ter == NULL) throw FileNotFoundException(ter_filename);
		IplImage *image = cvCreateImage(cvGetSize(tex), IPL_DEPTH_8U, 3);
		width = image->width;
		height = image->height;
		cvCopyImage(tex, image);
		for (int r = 0; r<image->height; r++) {
			for (int c = 0; c<image->width; c++) {
				if (((uchar*)ter->imageData)[r*ter->widthStep + c*ter->nChannels]<127) {
					float weight = 1.0 - (float)(((uchar*)ter->imageData)[r*ter->widthStep + c*ter->nChannels]) / 255;
					((uchar*)image->imageData)[r*image->widthStep + c*image->nChannels + 2] = 0;
					((uchar*)image->imageData)[r*image->widthStep + c*image->nChannels + 1] = 127;
					((uchar*)image->imageData)[r*image->widthStep + c*image->nChannels + 0] = weight * 255;
				} else {
					float weight = 0.5 + (float)(((uchar*)ter->imageData)[r*ter->widthStep + c*ter->nChannels] - 127) / 255;
					((uchar*)image->imageData)[r*image->widthStep + c*image->nChannels + 2] *= weight;
					((uchar*)image->imageData)[r*image->widthStep + c*image->nChannels + 1] *= weight;
					((uchar*)image->imageData)[r*image->widthStep + c*image->nChannels + 0] *= weight;
				}
			}
		}
		texture.make(image, "Map icon", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		cvReleaseImage(&tex);
		cvReleaseImage(&ter);
		cvReleaseImage(&image);

		int nStartPos = 0;
		startPos.clear();
		xml_document<char> doc;
		string text = readTextFile(fname.data());
		doc.parse<0>((char*)text.data());
		try {
			for (xml_node<> *startPosNode = doc.first_node("map")->first_node("startPos"); startPosNode; startPosNode = startPosNode->next_sibling("startPos"))
				startPos.push_back(Point2D(
				toFloat(startPosNode->first_node("x")->value()),
				toFloat(startPosNode->first_node("y")->value())
				));
		} catch (parse_error &e) {
			getLogger().print("Game::getMapHeader() : XML Parse Error in file " + fname + " : " + e.what(), LOG_SEVERE);
			throw Exception("XML Parse Error in file " + fname);
		}
	}

	Game::Game() {
		paused = false;
		loaded = false;
		wind_flow = wind_flow_phase = 0.0;
		messageTimer = 0;
		message = "";
		objective = "";
		nSelected = 0;
		nUnit = 0;
		nProjectile = 0;
		minimapImageTex = NULL;
		gamePlayTime = gameStartTime = 0;
		frameCounter = 0;
		commandLocked = false;
		editState = MAPEDITSTATE_ADD_UNIT;
	}

	void Game::initTeams(Team teams[], int nTeams, int playerTeam, int food, int wood, int stone, int metal) {
		if (isEditable()) {																					//Map editor
			this->nTeams = MAX_TEAM - 1;
			for (int t = 1; t <= this->nTeams; t++)
				this->teams[t] = Team(t, false, 0, 0);
		} else if (nTeams <= 0 || teams == NULL) {																//Find teams if not given
			this->nTeams = 0;
			for (int u = 0; u<nUnit; u++)
			if (unit[u].team >= this->nTeams)
				this->nTeams = unit[u].team;
			this->nTeams = clamp(this->nTeams, 1, MAX_TEAM - 1);
			for (int t = 1; t <= this->nTeams; t++)
				this->teams[t] = Team(t, false, choice(1, 2), t);
		} else {																							//Copy given teams
			this->nTeams = clamp(this->nTeams, 1, MAX_TEAM - 1);
			for (int t = 1; t <= this->nTeams; t++)
				this->teams[t] = teams[t];
		}
		this->playerTeam = clamp(playerTeam, 0, this->nTeams);

		this->teams[0] = Team(NATURE_COLOR, false, 0, 0);														//Init nature
		this->teams[0].name = "Nature";
		this->teams[0].init(this, unitTypeInfo, 0);
		for (int t = 1; t <= this->nTeams; t++) {																//initialize teams
			this->teams[t].name = "Team " + toString(t);
			this->teams[t].resource = Resource<>(food, wood, stone, metal);
			this->teams[t].init(this, unitTypeInfo, t);
			this->teams[t].hasAI = (this->teams[t].faction == 1 || this->teams[t].faction == 2);
		}
	}
	void Game::loadUnit(int u, int level) {
		setMessage("Loading Unit (" + unitTypeInfo[u].name + ")...");
		bool loadAnimation = !isEditable();
		bool success = false;
		try {
			getLogger().print("Loading Unit " + unitTypeInfo[u].name + "(" + toString(u) + ")...");
			success = unitTypeInfo[u].load(loadAnimation);
		} catch (NoUnitDataException &e) {
			getLogger().print("Game::loadUnit() : No unit data in unitTypeInfo[" + toString(u) + "]", LOG_SEVERE);
			throw NoUnitDataException("No unit data in unitTypeInfo[" + toString(u) + "]");
		}
		if (!success) {
			return;
		}
		for (int i = 0; i < MAX_OBJECT_TYPE; i++) {
			if (unitTypeInfo[u].canBuild[i]) {
				loadUnit(i, level);
			}
		}
		for (int i = 0; i < unitTypeInfo[u].transformation.size(); i++) {
			if (unitTypeInfo[u].transformation[i].neededLevel <= level) {
				loadUnit(unitTypeInfo[u].transformation[i].unitType, level);
			}
		}
		for (int i = 0; i < unitTypeInfo[u].siblings.size(); i++) {
			loadUnit(unitTypeInfo[u].siblings[i], level);
		}
		for (int i = 0; i < unitTypeInfo[u].specialPower.size(); i++) {
			if (unitTypeInfo[u].specialPower[i].addUnit_number>0 && unitTypeInfo[u].specialPower[i].neededLevel <= level) {
				loadUnit(unitTypeInfo[u].specialPower[i].addUnit_id, level);
			}
		}
	}
	void Game::load(string filepath, UnitTypeInfo* unitTypeInfo, Team teams[], int nTeams, int playerTeam, int food, int wood, int stone, int metal, bool editable) {
		this->editable = editable;
		getLogger().print("Loading unit information...");

		if (editable) {
			for (int i = 0; i < MAX_OBJECT_TYPE; i++) {
				try {
					unitTypeInfo[i].load(false);
				} catch(Exception &e) {
					//do nothing
				}
			}
		}

		this->filename = filepath;
		this->unitTypeInfo = unitTypeInfo;

		getLogger().print("Loading Terrain...");
		setMessage("Loading Terrain...");
		loadTerrain(removeExtension(filename) + "_terrain.bmp", removeExtension(filename) + "_texture.bmp");
		getLogger().print("Terrain Loaded Successfully.");

		getLogger().print("Loading Map...");
		setMessage("Loading Map...");
		xml_document<char> doc;
		string text = readTextFile(filepath.data());
		doc.parse<0>((char*)text.data());
		try {
			for (xml_node<> *startPosNode = doc.first_node("map")->first_node("startPos"); startPosNode; startPosNode = startPosNode->next_sibling("startPos")) {
				startPos.push_back(Point2D(
					toFloat(startPosNode->first_node("x")->value()),
					toFloat(startPosNode->first_node("y")->value())
					));
				if (isEditable()) {
					loadUnit(BOARDPIN_UNIT_TYPE, 1);
					addUnit(startPos.back().x, height - startPos.back().y, BOARDPIN_UNIT_TYPE, 0, 0, 1);
				}
			}
			if (doc.first_node("map")->first_node("terrain")->first_node("waterWaveAmplitude")) {
				waterWaveAmplitude = toFloat(doc.first_node("map")->first_node("terrain")->first_node("waterWaveAmplitude")->value());
			}
			if (doc.first_node("map")->first_node("terrain")->first_node("waterColor")) {
				waterColor.set(
					toFloat(doc.first_node("map")->first_node("terrain")->first_node("waterColor")->first_node("r")->value()),
					toFloat(doc.first_node("map")->first_node("terrain")->first_node("waterColor")->first_node("g")->value()),
					toFloat(doc.first_node("map")->first_node("terrain")->first_node("waterColor")->first_node("b")->value())
					);
			}
			initMiniMap();

			getLogger().print("Loading Units...");
			setMessage("Loading Units...");
			for (xml_node<> *unitNode = doc.first_node("map")->first_node("unit"); unitNode; unitNode = unitNode->next_sibling("unit")) {
				if (!isSkirmish() || toInt(unitNode->first_node("team")->value()) == 0) {
					loadUnit(
						toInt(unitNode->first_node("type")->value()),
						unitNode->first_node("level") ? toInt(unitNode->first_node("level")->value()) : 1
						);
					targetUnit = addUnit(
						toFloat(unitNode->first_node("x")->value()),
						toFloat(unitNode->first_node("y")->value()),
						toInt(unitNode->first_node("type")->value()),
						toInt(unitNode->first_node("team")->value()),
						toFloat(unitNode->first_node("angle")->value()),
						unitNode->first_node("level") ? toInt(unitNode->first_node("level")->value()) : 1,
						false
						);
					if (unitNode->first_attribute("id")) {
						unit[targetUnit].idForCampaign = trim(unitNode->first_attribute("id")->value());
					}
				}
			}
		} catch (parse_error &e) {
			getLogger().print("Game::load() : XML Parse Error in file " + filepath, LOG_SEVERE);
			throw Exception("XML Parse Error in file " + filepath);
		}

		//init teams
		getLogger().print("Creating Teams...");
		setMessage("Creating Teams...");
		initTeams(teams, nTeams, playerTeam, food, wood, stone, metal);

		//Init sounds
		getLogger().print("Initializing sound...");
		setMessage("Initializing Sound...");
		initSound();
		getLogger().print("Setting ambience sound...");
		setAmbienceSound();

		loaded = true;
		setMessage("Please Wait (Generating Vertex Buffers)...");
	}
	void Game::create(int width, int height, float groundHeight, string fname, UnitTypeInfo* unitTypeInfo, Team teams[], int nTeams, int playerTeam, int food, int wood, int stone, int metal) {
		this->editable = true;
		this->filename = fname;
		this->width = width;
		this->height = height;
		this->unitTypeInfo = unitTypeInfo;
		this->nTeams = nTeams;
		this->playerTeam = playerTeam;
		this->teams[0] = Team(NATURE_COLOR, false, 0);

		for (int i = 0; i<nTeams && i<MAX_TEAM; i++) {
			this->teams[i + 1] = teams[i];
			this->teams[i + 1].resource.food = food;
			this->teams[i + 1].resource.wood = wood;
			this->teams[i + 1].resource.stone = stone;
			this->teams[i + 1].resource.metal = metal;
		}

		z = allocate<float>(width, height, groundHeight);
		textureIndex = allocate<int>(width, height, 2);
		initMiniMap();

		for (int i = 0; i<MAX_OBJECT_TYPE; i++) {
			unitTypeInfo[i].load(false);
		}

		resetCamera();
		goTo(0, 0);

		initTeams(teams, nTeams, playerTeam, 0, 0, 0, 0);

		loaded = true;
	}

	void Game::save() {
		saveTerrain(removeExtension(filename) + "_terrain.bmp", removeExtension(filename) + "_texture.bmp");

		string buffer[20000];
		int bCnt = 0;
		xml_document<char> doc;

		doc.append_node(doc.allocate_node(node_element, "map"));

		doc.first_node()->append_node(doc.allocate_node(node_element, "terrain"));
		doc.first_node()->first_node("terrain")->append_node(doc.allocate_node(node_element, "waterWaveAmplitude", doc.allocate_string(toString(waterWaveAmplitude).data())));
		doc.first_node()->first_node("terrain")->append_node(doc.allocate_node(node_element, "waterColor"));
		doc.first_node()->first_node("terrain")->first_node("waterColor")->append_node(doc.allocate_node(node_element, "r", doc.allocate_string(toString(waterColor.r()).data())));
		doc.first_node()->first_node("terrain")->first_node("waterColor")->append_node(doc.allocate_node(node_element, "g", doc.allocate_string(toString(waterColor.g()).data())));
		doc.first_node()->first_node("terrain")->first_node("waterColor")->append_node(doc.allocate_node(node_element, "b", doc.allocate_string(toString(waterColor.b()).data())));
		startPos.clear();
		for (int i = 0; i < nUnit; i++) {
			if (unit[i].getType() == BOARDPIN_UNIT_TYPE) {
				startPos.push_back(Point2D(unit[i].x, height - unit[i].y));
			}
		}
		for (int i = 0; i<startPos.size(); i++) {
			xml_node<char> *nodeStartPos = doc.allocate_node(node_element, "startPos");
			doc.first_node()->append_node(nodeStartPos);
			nodeStartPos->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(startPos[i].x).data())));
			nodeStartPos->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(startPos[i].y).data())));
		}
		for (int i = 0; i < nUnit; i++) {
			if (unit[i].getType() == BOARDPIN_UNIT_TYPE) {
				continue;
			}
			if (unit[i].isAlive()) {
				xml_node<char> *nodeUnit = doc.allocate_node(node_element, "unit");
				doc.first_node()->append_node(nodeUnit);
				nodeUnit->append_node(doc.allocate_node(node_element, "type", doc.allocate_string(toString(unit[i].type).data())));
				nodeUnit->append_node(doc.allocate_node(node_element, "team", doc.allocate_string(toString(unit[i].team).data())));
				nodeUnit->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(unit[i].x).data())));
				nodeUnit->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(unit[i].y).data())));
				nodeUnit->append_node(doc.allocate_node(node_element, "angle", doc.allocate_string(toString(unit[i].angle).data())));
				if (unitTypeInfo[unit[i].type].isHeroic) {
					nodeUnit->append_node(doc.allocate_node(node_element, "level", doc.allocate_string(toString(unit[i].level).data())));
				}
				if (unit[i].idForCampaign != "") {
					nodeUnit->append_attribute(doc.allocate_attribute("id", unit[i].idForCampaign.data()));
				} else if (unitTypeInfo[unit[i].type].isHeroic) {
					nodeUnit->append_attribute(doc.allocate_attribute("id", doc.allocate_string(toLower(unitTypeInfo[unit[i].type].name).data())));
				}
			}
		}

		string text;
		print(back_inserter(text), doc, 0);
		ofstream file(filename.data());
		file << text;
		file.close();

		if (getSteamAchievements())
			getSteamAchievements()->SetAchievement("ACH_CREATED_A_MAP");
	}
	void Game::renameMap(string newname) {
		save();
		newname += " (" + toString((int)startPos.size()) + ") [" + toString(width) + "x" + toString(height) + "]";
		rename(filename.data(), ("map/" + newname + ".map").data());
		rename((removeExtension(filename) + "_terrain.bmp").data(), ("map/" + newname + "_terrain.bmp").data());
		rename((removeExtension(filename) + "_texture.bmp").data(), ("map/" + newname + "_texture.bmp").data());
		filename = "map/" + newname + ".map";
	}

	void Game::compileUnits() {
		for (int i = 0; i<MAX_OBJECT_TYPE; i++)
			unitTypeInfo[i].compile();
	}
	void Game::loadObjects() {
		model.arrow.load("special/arrow/arrow.obj");
		model.spike.load("special/spike/spike.obj");
		model.flag.load("special/flag/flag.obj");
		model.treelog.load("special/treelog/treelog.obj");
	}
	void Game::loadUITextures() {
		uiTextureSet.image_blank_button.load("ui/blank.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		if (isEditable()) {
			uiTextureSet.icon_increase_level.load("ui/increase_level.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			uiTextureSet.icon_decrease_level.load("ui/decrease_level.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			uiTextureSet.icon_remove_unit.load("ui/remove_unit.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			for (int t = 0; t<N_TERRAIN_TEXTURE; t++) {
				uiTextureSet.icon_terrain_texture[t].load(("special/terrain/" + toString(t) + ".jpg").data(), true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP, 64, 64);
			}
		} else {
			uiTextureSet.icon_stop.load("ui/stop.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			uiTextureSet.icon_kill.load("ui/kill.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			uiTextureSet.icon_holdfire_stance.load("ui/hold_fire.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			uiTextureSet.icon_general_stance.load("ui/general_stance.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			uiTextureSet.icon_standground_stance.load("ui/standground.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
			uiTextureSet.icon_deploy.load("ui/deployall.png", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		}
	}
	void Game::initCamera() {
		resetCamera();

		goTo(0, 0);
		if (!editable) {
			for (UnitID u = 0; u<nUnit; u++) {
				if (unit[u].team == playerTeam) {
					goTo(unit[u].x, unit[u].y);
					break;
				}
			}
		}
	}
	void Game::initAI() {
		teams[playerTeam].hasAI = false;
		for (int t = 1; t <= nTeams; t++) {
			teamAI[t] = TeamAI(t, unitTypeInfo, this);
			teamAI[t].init();
		}
	}
	void Game::loadShaders() {
		objectShader.load("shaders/object.glsl");

		alphaMaskShader.load("shaders/alphamask.glsl");

		if (Settings::depthOfFieldOn) {
			dofShader.load("shaders/dof.glsl");
		}
		if (Settings::bloomOn) {
			blurShader.load("shaders/blur.glsl");
			addShader.load("shaders/add.glsl");
			brightPassShader.load("shaders/brightpass.glsl");
		}
		if (Settings::antialiasingOn) {
			antialiasShader.load("shaders/fxaa.glsl");
		}
		colorShader.load("shaders/color.glsl");
	}
	void Game::initFBOs() {
		for (int i = 0; i<arrayLength(tempFBOSet0); i++) {
			tempFBOSet0[i].create(Settings::screenWidth, Settings::screenHeight);
			tempFBOSet0[i].attachDepthTexture();
			tempFBOSet0[i].attachColorTexture();
			tempFBOSet0[i].checkStatus();
		}
		tempFBOSet0Index = 0;
		for (int i = 0; i<arrayLength(tempFBOSet1); i++) {
			int size = 256 / pow(2, i);

			tempFBOSet1[i].create(size, size);
			tempFBOSet1[i].attachColorTexture();
			tempFBOSet1[i].checkStatus();

			tempFBOSet2[i].create(size, size);
			tempFBOSet2[i].attachColorTexture();
			tempFBOSet2[i].checkStatus();
		}
	}

	void Game::init() {
		pause();

		if (loaded) {
			try {
				World::init();
				Terrain::init(Settings::screenWidth, Settings::screenHeight);
				Sky::init();
				Water::init();

				initFBOs();
				compileUnits();
				loadParticles();
				loadUITextures();
				loadObjects();
				loadShaders();
				initFire();
				initWeather();
				initRain();

				//AI
				if (!isEditable()) {
					initAI();
				}

				//Create Mutex
				projectileMutex.init();

				//Init Update Thread
				updateThread = SDL_CreateThread(updateThreadFunc, this);
				for (int i = 0; i < N_UNIT_UPDATE_THRAD; i++) {
					updateUnitThreadResource[i].index = i;
					updateUnitThreadResource[i].game = this;
					updateUnitThreadResource[i].pathfinder = new PathfinderAStar(width, height);
					updateUnitThreadResource[i].thread = SDL_CreateThread(updateUnitThreadFunc, &updateUnitThreadResource[i]);
				}

				//Camera
				initCamera();
			} catch (Exception &e) {
				showMessage(e.getMessage(), "Runtime Exception : ramayana::Game::init()", true);
			}
		}

		gameStartTime = SDL_GetTicks();
	}

	void Game::initMiniMap() {
		mapMutex.init();

		Synchronizer sync(mapMutex);

		//Initialize terrain on minimap
		try {
			minimap = allocate<MiniMapPosition>(width, height);
			for (int r = 0; r<height; r++) {
				for (int c = 0; c<width; c++) {
					if (z[r][c]<BASE_WATER_LEVEL - 3.0) {
						minimap[r][c].type = TERRAIN_DEEP_WATER;
					} else if (z[r][c]<BASE_WATER_LEVEL - 0.5) {
						minimap[r][c].type = TERRAIN_SHALLOW_WATER;
					} else if ((r - 1<0 || fabs(z[r][c] - z[r - 1][c]) <= Game::MOVABLE_MAX_HEIGHT)
						&& (r + 1 >= height || fabs(z[r][c] - z[r + 1][c]) <= Game::MOVABLE_MAX_HEIGHT)
						&& (c + 1 >= width || fabs(z[r][c] - z[r][c + 1]) <= Game::MOVABLE_MAX_HEIGHT)
						&& (c - 1<0 || fabs(z[r][c] - z[r][c - 1]) <= Game::MOVABLE_MAX_HEIGHT)
						&& (r - 1<0 || c - 1<0 || fabs(z[r][c] - z[r - 1][c - 1]) <= Game::MOVABLE_MAX_HEIGHT)
						&& (c - 1<0 || r + 1 >= height || fabs(z[r][c] - z[r + 1][c - 1]) <= Game::MOVABLE_MAX_HEIGHT)
						&& (r - 1<0 || c + 1 >= width || fabs(z[r][c] - z[r - 1][c + 1]) <= Game::MOVABLE_MAX_HEIGHT)
						&& (r + 1 >= height || c + 1 >= width || fabs(z[r][c] - z[r + 1][c + 1]) <= Game::MOVABLE_MAX_HEIGHT)) {
						minimap[r][c].type = TERRAIN_MOVABLE_LAND;
					} else {
						minimap[r][c].type = TERRAIN_UNMOVABLE_LAND;
					}
				}
			}
		} catch (...) {}

		//init blend
		blend = allocate<float>(width, height);
	}
	void Game::bindMinimapImage() {
		Synchronizer sync(mapMutex);

		IplImage* minimapImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
		for (int r = 0; r<height; r++) {
			for (int c = 0; c<width; c++) {
				uchar& rc = ((uchar*)minimapImage->imageData)[r*minimapImage->widthStep + c*minimapImage->nChannels + 2];
				uchar& gc = ((uchar*)minimapImage->imageData)[r*minimapImage->widthStep + c*minimapImage->nChannels + 1];
				uchar& bc = ((uchar*)minimapImage->imageData)[r*minimapImage->widthStep + c*minimapImage->nChannels + 0];
				uchar& ac = ((uchar*)minimapImage->imageData)[r*minimapImage->widthStep + c*minimapImage->nChannels + 3];
				//Explored/Visible
				if (teams[playerTeam].visible[r][c]) {
					ac = 255;
				} else if (teams[playerTeam].explored[r][c]) {
					ac = 127;
				} else {
					ac = 61;
				}
				//Terrain Texture
				Color color;
				switch (textureIndex[r][c]) {
				case 0: color = COLOR_GRAY75;				break;
				case 1: color = COLOR_DARK_GREY_GREEN;	break;
				case 2: color = COLOR_SAP_GREEN;			break;
				case 3: color = COLOR_LIME;				break;
				case 4: color = COLOR_ROSE;				break;
				case 5: color = COLOR_LIGHT_YELLOW;		break;
				case 6: color = COLOR_BROWN;				break;
				case 7: color = COLOR_DARK_VIRIDIAN;		break;
				}
				rc = color.r() * 127;
				gc = color.g() * 127;
				bc = color.b() * 127;
				//Terrain Height
				if (z[r][c]<BASE_WATER_LEVEL) {
					rc = 0;
					gc = 64 - z[r][c] * 64 / MIN_TERRAIN_HEIGHT;
					bc = 64;
				}
				//Resource
				if (teams[playerTeam].explored[r][c] && minimap[r][c].landUnit != -1 && unit[minimap[r][c].landUnit].team == 0) {
					switch (unitTypeInfo[unit[minimap[r][c].landUnit].type].category) {
					case UNIT_STONE:		rc = 127, gc = 127, bc = 127;	break;
					case UNIT_TREE:			rc = 255, gc = 255, bc = 0;	break;
					}
				}
				//Unit
				if (teams[playerTeam].visible[r][c]) {
					if (minimap[r][c].landUnit != -1 && unit[minimap[r][c].landUnit].team != 0) {
						Color color = availableTeamColors[teams[unit[minimap[r][c].landUnit].team].color];
						if (isSelected(minimap[r][c].landUnit))	{
							rc = 255, gc = 255, bc = 255;
						} else {
							rc = color.r() * 255, gc = color.g() * 255, bc = color.b() * 255;
						}
					} else if (minimap[r][c].waterUnit != -1 && unit[minimap[r][c].waterUnit].team != 0) {
						Color color = availableTeamColors[teams[unit[minimap[r][c].waterUnit].team].color];
						if (isSelected(minimap[r][c].waterUnit)) {
							rc = 255, gc = 255, bc = 255;
						} else {
							rc = color.r() * 255, gc = color.g() * 255, bc = color.b() * 255;
						}
					} else if (minimap[r][c].airUnit != -1 && unit[minimap[r][c].airUnit].team != 0) {
						Color color = availableTeamColors[teams[unit[minimap[r][c].airUnit].team].color];
						if (isSelected(minimap[r][c].airUnit)) {
							rc = 255, gc = 255, bc = 255;
						} else {
							rc = color.r() * 255, gc = color.g() * 255, bc = color.b() * 255;
						}
					}
				}
			}
		}

		float scaling = max(width, height) / 100.0;

		//Camera Position
		cvRectangle(minimapImage,
			cvPoint(camX - 15 * scaling, camY - 10 * scaling),
			cvPoint(camX + 15 * scaling, camY + 10 * scaling),
			cvScalar(255, 255, 255, 255),
			scaling);

		//Heroes
		for (UnitID u = 0; u < nUnit; u++) {
			if (unit[u].isAlive() && unit[u].team == playerTeam && unit[u].getTypeInfo().isHeroic) {
				cvLine(minimapImage,
					cvPoint(unit[u].x + scaling, unit[u].y + scaling),
					cvPoint(unit[u].x - scaling, unit[u].y - scaling),
					cvScalar(255, 255, 255, 255),
					scaling);
				cvLine(minimapImage,
					cvPoint(unit[u].x + scaling, unit[u].y - scaling),
					cvPoint(unit[u].x - scaling, unit[u].y + scaling),
					cvScalar(255, 255, 255, 255),
					scaling);
			}
		}

		//Destination
		for (int i = 0; i < minimapMarker.size(); i++) {
			cvLine(minimapImage,
				cvPoint(minimapMarker[i].x + scaling, minimapMarker[i].y + scaling),
				cvPoint(minimapMarker[i].x - scaling, minimapMarker[i].y - scaling),
				cvScalar(0, 0, 255, 255),
				scaling);
			cvLine(minimapImage,
				cvPoint(minimapMarker[i].x + scaling, minimapMarker[i].y - scaling),
				cvPoint(minimapMarker[i].x - scaling, minimapMarker[i].y + scaling),
				cvScalar(0, 0, 255, 255),
				scaling);
		}

		cvFlip(minimapImage, minimapImage, 0);
		if (minimapImageTex != NULL) {
			delete minimapImageTex;
		}
		minimapImageTex = new Texture2D();
		minimapImageTex->make(minimapImage, "Minimap", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		cvReleaseImage(&minimapImage);
	}
	void Game::setInMinimap(const vector<Point2Di>& points, UnitID unitID, bool land, bool water, bool air) {
		Synchronizer sync(mapMutex);
		try {
			for (int p = 0; p<points.size(); p++) {
				if (points[p].in(0, 0, getWidth() - 1, getHeight() - 1)) {
					if (land) {
						minimap[points[p].y][points[p].x].landUnit = unitID;
					}
					if (water) {
						minimap[points[p].y][points[p].x].waterUnit = unitID;
					}
					if (air) {
						minimap[points[p].y][points[p].x].airUnit = unitID;
					}
				}
			}
		} catch (...) {}
	}

	void Game::calculateBlend() {
		//SDL_LockMutex(unitMutex);
		setAll(blend, width, height, 0.0f);
		for (UnitID u = 0; u<nUnit; u++) {
			if (unit[u].isAlive() && (unit[u].getCategory() == UNIT_TREE || unit[u].getCategory() == UNIT_DECORATION)) {
				fillCircle(blend, width, height, 0.1f, unit[u].position(), unit[u].getRadius());
			}
		}
		//SDL_UnlockMutex(unitMutex);
	}

	int _SDL_THREAD Game::updateThreadFunc(void *param) {
		Game &game = *(Game*)param;

		while (game.paused) {
			SDL_Delay(1000 / UPDATE_THREAD_FRAME_RATE);
		}

		while (game.loaded) {
			while (game.loaded && game.paused) {
				SDL_Delay(1000 / UPDATE_THREAD_FRAME_RATE);
			}

			clock_t startTime = SDL_GetTicks();

			game.calculateBlend();
			game.updateProjectile();
			game.updateFire();
			game.updateTornedo();
			game.updateRain();
			game.updateSpecialEffect();
			game.updateWeather();
			game.setMusic();
			game.particleRenderer.update();

			game.updateFrameDuration = SDL_GetTicks() - startTime;
			SDL_Delay(clamp(1000 / UPDATE_THREAD_FRAME_RATE - game.updateFrameDuration, 0, 1000 / UPDATE_THREAD_FRAME_RATE));

			game.gamePlayTime += 1000 / UPDATE_THREAD_FRAME_RATE;
		}
		return 0;
	}
	int _SDL_THREAD Game::updateUnitThreadFunc(void *param) {
		UpdateUnitThreadResource &res = *(UpdateUnitThreadResource*)param;

		while (res.game->paused) {
			SDL_Delay(1000 / UPDATE_THREAD_FRAME_RATE);
		}

		while (res.game->loaded) {
			while (res.game->loaded && res.game->paused) {
				SDL_Delay(1000 / UPDATE_THREAD_FRAME_RATE);
			}

			clock_t startTime = SDL_GetTicks();

			for (int u = res.index; u < res.game->nUnit; u += N_UNIT_UPDATE_THRAD) {
				if (res.game->unit[u].exists() || res.game->unit[u].isRenderable()) {
					res.game->unit[u].update();
					if (!res.game->loaded) {
						break;
					}
				}
			}

			clock_t duration = SDL_GetTicks() - startTime;
			SDL_Delay(clamp(1000 / UPDATE_THREAD_FRAME_RATE - duration, 0, 1000 / UPDATE_THREAD_FRAME_RATE));
		}
		return 0;
	}
	void Game::update() {
		updateSelection();

		if (messageTimer>0)	{
			messageTimer--;
		} else {
			message = "";
		}

		if (!paused) {
			try {
				updateTeams();

				updateTerrain(false);
				updateWater(false);
			} catch (Exception &e) {
				showMessage(e.getMessage(), "Runtime Exception : ramayana::Game::update()", false);
			}
		}

		if (hasCompleted()) {
			teams[playerTeam].setRevealAll(true);
		}
	}

	void Game::updateTeams() {
		Synchronizer sync(mapMutex);
		try {
			for (int t = 1; t <= this->nTeams; t++) {
				teams[t].update();
			}
		} catch (...) {}
	}

	int Game::addUnit(float x, float y, UnitType unitType, int team, int angle, int level, bool build) {
		getLogger().print("Adding Unit " + toString(unitType) + " (Team " + toString(team) + ", Level " + toString(level) + ") at (" + toString(x) + "," + toString(y) + ")...");
		try {
			int i = 0;
			for (; i<nUnit; i++) {
				if (!unit[i].exists()) {
					break;
				}
			}
			if (i >= MAX_UNIT) {
				throw HUDMessage("Reached unit limit, Can't build more units");
			}
			vector<UnitType> possibilities = unitTypeInfo[unitType].siblings + unitType;
			unitType = randomVectorElement(possibilities);
			if (build) {
				Resource<> cost = teams[team].getUnitCost(unitType);
				if (unitTypeInfo[unitType].population>0 && teams[team].population + unitTypeInfo[unitType].population>getMaxPopulation(team)) {
					throw HUDMessage("Not enough population (Build some buildings)");
				} else if (cost.food>teams[team].resource.food) {
					throw HUDMessage("Needs more " + toString(cost.food - teams[team].resource.food) + " food to build/train the unit");
				} else if (cost.wood>teams[team].resource.wood) {
					throw HUDMessage("Needs more " + toString(cost.wood - teams[team].resource.wood) + " wood to build/train the unit");
				} else if (cost.stone>teams[team].resource.stone) {
					throw HUDMessage("Needs more " + toString(cost.stone - teams[team].resource.stone) + " stone to build/train the unit");
				} else if (cost.metal>teams[team].resource.metal) {
					throw HUDMessage("Needs more " + toString(cost.metal - teams[team].resource.metal) + " metal to build/train the unit");
				}
			}
			unit[i] = Unit();
			getLogger().print("Binding added unit as " + toString(unitType) + "...");
			unit[i].bind(unitType, team, Point3D(x, y, getGroundHeight(x, y)), angle, level, unitTypeInfo, this, i, build);
			if (i == nUnit) {
				nUnit++;
			}
			return i;
		} catch (HUDMessage &m) {
			if (team == playerTeam) {
				setMessage(m.msg);
			}
		} catch (Exception &e) {
			getLogger().print("Game::addUnit() : " + e.getMessage(), LOG_SEVERE);
			showMessage(e.getMessage(), "Runtime Exception : ramayana::Game::addUnit()", false);
		}
		return INVALID_UNIT_INDEX;
	}

	int Game::nearestUnitIndex(float x, float y, UnitCategory ut) const {
		float mind = FLT_MAX;
		int mini = -1;
		for (int i = 0; i<nUnit; i++) {
			if (unitTypeInfo[unit[i].type].category == ut) {
				float d = dist(Point2D(x, y), Point2D(unit[i].x, unit[i].y));
				if (d<mind) {
					mind = d;
					mini = i;
				}
			}
		}
		return mini;
	}

	void Game::pause() {
		pauseGameAudio();
		paused = true;
	}
	void Game::resume() {
		paused = false;
		resumeGameAudio();
	}

	bool Game::hasCompleted() {
		if (isEditable()) {
			return false;
		} else {
			if (nTeams <= 1) {
				return false;
			}
			TeamGroupID winningTeamGroupID = -1;
			for (TeamID t = 1; t <= nTeams; t++) {
				if (!teams[t].defeated) {
					winningTeamGroupID = teams[t].teamGroup;
					break;
				}
			}
			if (winningTeamGroupID != -1) {
				for (TeamID t = 1; t <= nTeams; t++) {
					if (!teams[t].defeated && teams[t].teamGroup != winningTeamGroupID) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
	}
	void Game::victory() {
		for (TeamID t = 1; t <= nTeams; t++) {
			teams[t].defeated = (t != playerTeam);
		}
	}
	void Game::defeat() {
		for (TeamID t = 1; t <= nTeams; t++) {
			teams[t].defeated = (t == playerTeam);
		}
	}

	Diplomacy Game::diplomacy(int t1, int t2) const {
		if (t1 <= 0 || t2 <= 0 || t1>nTeams || t2>nTeams) {
			return DIPLOMACY_NEUTRAL;
		} else if (teams[t1].teamGroup == teams[t2].teamGroup) {
			return DIPLOMACY_ALLY;
		} else {
			return DIPLOMACY_ENEMY;
		}
	}

	void Game::setMessage(string m) {
		message = m;
		messageTimer = MAX_MESSAGE_TIME;
	}

	Game::~Game() {
		if (loaded) {
			resume();
			loaded = false;

			getLogger().print("Deallocating game...");
			for (int t = 1; t <= nTeams; t++) {
				teamAI[t].close();
			}
			SDL_WaitThread(updateThread, NULL);
			getLogger().print("Game AI threads killed.");
			for (int i = 0; i < N_UNIT_UPDATE_THRAD; i++) {
				SDL_WaitThread(updateUnitThreadResource[i].thread, NULL);
				delete updateUnitThreadResource[i].pathfinder;
			}
			getLogger().print("Game update threads killed.");

			for (UnitID id = 0; id < nUnit; id++) {
				unit[id].unload();
			}
			getLogger().print("Units unloaded.");

			for (int i = 0; i < MAX_OBJECT_TYPE; i++) {
				unitTypeInfo[i].unload();
			}
			getLogger().print("Unit type informations unloaded.");

			deallocate(minimap, width, height);
			deallocate(fireIntensity, width, height);
			deallocate(z, width, height);
			deallocate(textureIndex, width, height);
			deallocate(blend, width, height);
			getLogger().print("Internal data structures unloaded.");

			if (minimapImageTex != NULL) {
				delete minimapImageTex;
			}
			getLogger().print("Minimap unloaded.");

			pause();
			getLogger().print("Game unloaded successfully.");
		}
	}

	AbilityCategory toWeatherType(AbilityType a) {
		switch (a) {
		case ABILITY_BUILD:						return ABILITY_CATEGORY_BUILD;
		case ABILTY_STANCE_GENERAL:				return ABILITY_CATEGORY_STANCE;
		case ABILITY_STANCE_STANDGROUND:		return ABILITY_CATEGORY_STANCE;
		case ABILITY_STANCE_HOLD_FIRE:			return ABILITY_CATEGORY_STANCE;
		case ABILITY_DEPLOY:					return ABILITY_CATEGORY_DEPLOY;
		case ABILITY_DEPLOY_ALL:				return ABILITY_CATEGORY_DEPLOY;
		case ABILITY_SPECIAL:					return ABILITY_CATEGORY_SPECIAL;
		case ABILITY_PASSIVE:					return ABILITY_CATEGORY_PASSIVE;
		case ABILITY_TRANSFORM:					return ABILITY_CATEGORY_TRANSFORM;
		case ABILITY_SET_TEAM:					return ABILITY_CATEGORY_EDIT;
		case ABILITY_SET_TERRAIN_TEXTURE:		return ABILITY_CATEGORY_EDIT;
		default:								return ABILITY_CATEGORY_GENERAL;
		}
	}

	// writes current snapshot passed xml document
	void Game::writeSnapshotXML(xml_document<char> &doc) {
		doc.append_node(doc.allocate_node(node_element, "save"));
		doc.first_node("save")->append_node(doc.allocate_node(node_element, "map", filename.data()));

		doc.first_node("save")->append_node(doc.allocate_node(node_element, "gamePlayTime", doc.allocate_string(toString(gamePlayTime).data())));

		doc.first_node("save")->append_node(doc.allocate_node(node_element, "camX", doc.allocate_string(toString(camX).data())));
		doc.first_node("save")->append_node(doc.allocate_node(node_element, "camY", doc.allocate_string(toString(camY).data())));
		doc.first_node("save")->append_node(doc.allocate_node(node_element, "camZ", doc.allocate_string(toString(camZ).data())));
		doc.first_node("save")->append_node(doc.allocate_node(node_element, "tilt", doc.allocate_string(toString(tilt).data())));
		doc.first_node("save")->append_node(doc.allocate_node(node_element, "rotation", doc.allocate_string(toString(rotation).data())));

		doc.first_node("save")->append_node(doc.allocate_node(node_element, "playerTeam", doc.allocate_string(toString(playerTeam).data())));

		for (TeamID t = 0; t <= nTeams; t++) {
			doc.first_node("save")->append_node(teams[t].toXMLNode(doc));
		}
		for (UnitID u = 0; u < nUnit; u++) {
			doc.first_node("save")->append_node(unit[u].toXMLNode(doc));
		}

		doc.first_node("save")->append_node(doc.allocate_node(node_element, "weatherType", doc.allocate_string(toString(weather.weatherType).data())));

		for (int i = 0; i <= MAX_TORNEDO; i++) {
			if (tornedo[i].time>0) {
				xml_node<char> *nodeTornedo = doc.allocate_node(node_element, "tornedo");
				doc.first_node("save")->append_node(nodeTornedo);
				nodeTornedo->append_node(doc.allocate_node(node_element, "pos"));
				nodeTornedo->first_node("pos")->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(tornedo[i].pos.x).data())));
				nodeTornedo->first_node("pos")->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(tornedo[i].pos.y).data())));
				nodeTornedo->append_node(doc.allocate_node(node_element, "time", doc.allocate_string(toString(tornedo[i].time).data())));
				nodeTornedo->append_node(doc.allocate_node(node_element, "dir"));
				nodeTornedo->first_node("dir")->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(tornedo[i].dir.x).data())));
				nodeTornedo->first_node("dir")->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(tornedo[i].dir.y).data())));
			}
		}

		for (int i = 0; i <= MAX_COLLISION; i++) {
			if (waterSplash[i].time>0) {
				xml_node<char> *nodeSplash = doc.allocate_node(node_element, "splash");
				doc.first_node("save")->append_node(nodeSplash);
				nodeSplash->append_node(doc.allocate_node(node_element, "pos"));
				nodeSplash->first_node("pos")->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(waterSplash[i].pos.x).data())));
				nodeSplash->first_node("pos")->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(waterSplash[i].pos.y).data())));
				nodeSplash->append_node(doc.allocate_node(node_element, "time", doc.allocate_string(toString(waterSplash[i].time).data())));
				nodeSplash->append_node(doc.allocate_node(node_element, "size", doc.allocate_string(toString(waterSplash[i].size).data())));
			}
		}

		for (int r = 0; r < height; r++) {
			for (int c = 0; c < width; c++) {
				if (fireIntensity[r][c]>0) {
					xml_node<char> *nodeFire = doc.allocate_node(node_element, "fire");
					doc.first_node("save")->append_node(nodeFire);
					nodeFire->append_node(doc.allocate_node(node_element, "pos"));
					nodeFire->first_node("pos")->append_node(doc.allocate_node(node_element, "x", doc.allocate_string(toString(c).data())));
					nodeFire->first_node("pos")->append_node(doc.allocate_node(node_element, "y", doc.allocate_string(toString(r).data())));
					nodeFire->append_node(doc.allocate_node(node_element, "intensity", doc.allocate_string(toString(fireIntensity[r][c]).data())));

				}
			}
		}
	}
	
	// save current snapshot of game to an xml file
	void Game::saveSnapshot(string saveFilename) {
		getLogger().print("Loading Game Snapshot from " + saveFilename + " ...");
		xml_document<char> doc;
		writeSnapshotXML(doc);
		string text;
		print(back_inserter(text), doc, 0);
		ofstream file(saveFilename.data());
		file << text;
		file.close();
		getLogger().print("Game Snapshot loaded successfully");
	}
	
	// reads passed xml document to load a game snapshot
	void Game::readSnapshotXML(xml_document<char> &doc, UnitTypeInfo* unitTypeInfo) {
		filename = doc.first_node("save")->first_node("map")->value();
		loadTerrain(removeExtension(filename) + "_terrain.bmp", removeExtension(filename) + "_texture.bmp");
		initMiniMap();

		if (doc.first_node("save")->first_node("gamePlayTime"))		gamePlayTime = toInt(doc.first_node("save")->first_node("gamePlayTime")->value());

		if (doc.first_node("save")->first_node("camX"))				camX = toInt(doc.first_node("save")->first_node("camX")->value());
		if (doc.first_node("save")->first_node("camY"))				camY = toInt(doc.first_node("save")->first_node("camY")->value());
		if (doc.first_node("save")->first_node("camZ"))				camZ = toInt(doc.first_node("save")->first_node("camZ")->value());
		if (doc.first_node("save")->first_node("tilt"))				tilt = toInt(doc.first_node("save")->first_node("tilt")->value());
		if (doc.first_node("save")->first_node("rotation"))			rotation = toInt(doc.first_node("save")->first_node("rotation")->value());

		if (doc.first_node("save")->first_node("playerTeam"))		playerTeam = toInt(doc.first_node("save")->first_node("playerTeam")->value());

		if (doc.first_node("save")->first_node("weatherType"))		setWeather((WeatherType)toInt(doc.first_node("save")->first_node("weatherType")->value()));

		nTeams = -1;
		for (xml_node<> *node = doc.first_node("save")->first_node("team"); node; node = node->next_sibling("team")) {
			nTeams++;
			teams[nTeams].loadFromXMLNode(node, unitTypeInfo, this);
		}
		nUnit = 0;
		for (xml_node<> *node = doc.first_node("save")->first_node("unit"); node; node = node->next_sibling("unit")) {
			UnitType type;
			if (node->first_node("type"))							type = toInt(node->first_node("type")->value());
			else continue;
			UnitID unitID;
			if (node->first_attribute("unitID"))						unitID = toInt(node->first_attribute("unitID")->value());
			else continue;
			int level = 1;
			if (node->first_node("level"))							level = toInt(node->first_node("level")->value());
			loadUnit(type, level);
			unit[unitID].loadFromXMLNode(node, unitTypeInfo, this);
			nUnit++;
		}
		loaded = true;
	}
	
	// reads an xml file to load snapshot of a previously saved game
	void Game::loadSnapshot(string saveFilename, UnitTypeInfo* unitTypeInfo) {
		getLogger().print("Saving Game Snapshot to " + saveFilename + " ...");
		this->editable = false;
		this->unitTypeInfo = unitTypeInfo;
		string text = readTextFile(saveFilename.data());
		xml_document<char> doc;
		doc.parse<0>((char*)text.data());
		try {
			readSnapshotXML(doc, unitTypeInfo);
			for (int t = 0; t <= this->nTeams; t++)
				teams[t].init(this, unitTypeInfo, t);
		} catch (parse_error &e) {
			throw Exception("XML Parse Error in file " + saveFilename);
			getLogger().print("Game::loadSnapshot() : XML Parse Error in file " + saveFilename, LOG_SEVERE);
		}
		getLogger().print("Game Snapshot Saved successfully");
	}

	bool Game::isTerrainEditMode() const {
		return isEditable() && (editState == MAPEDITSTATE_DECREASE_HEIGHT || editState == MAPEDITSTATE_INCREASE_HEIGHT || editState == MAPEDITSTATE_CHANGE_TEXTURE);
	}

	bool Game::isEditable() const {
		return editable;
	}

};
