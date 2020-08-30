#include "stdafx.h"

#include "common.h"
#include "game.h"
#include "audio.h"

namespace ramayana {

	bool Game::Settings::fullscreen = false;
	bool Game::Settings::gameMode = true;
	int Game::Settings::screenWidth = Game::Settings::DEFAULT_WINDOWED_WIDTH;
	int Game::Settings::screenHeight = Game::Settings::DEFAULT_WINDOWED_HEIGHT;
	vector<string> Game::Settings::availableGameModeStrings;
	string Game::Settings::gameModeString = "";

	vector<string> Game::Settings::availableVideoHeight;
	string Game::Settings::videoHeight = "1080p";

	bool Game::Settings::noShader = false;
	bool Game::Settings::reflectionOn = false;
	bool Game::Settings::shadowOn = false;
	bool Game::Settings::antialiasingOn = true;
	bool Game::Settings::terrainBumpmapOn = false;
	bool Game::Settings::objectBumpmapOn = false;
	bool Game::Settings::motionBlurOn = false;
	bool Game::Settings::depthOfFieldOn = false;
	bool Game::Settings::bloomOn = false;

	bool Game::Settings::showLOS = false;
	bool Game::Settings::showPath = false;
	bool Game::Settings::showRenderDetails = false;

	bool Game::Settings::renderObjectsOn = true;
	bool Game::Settings::renderTerrainOn = true;
	bool Game::Settings::renderWaterOn = true;
	bool Game::Settings::renderSkyOn = true;

	float Game::Settings::scrollSpeed = 0.1;

	bool Game::Settings::stereoscopicOn = false;
	float Game::Settings::stereoSeperation = 1.0;

	bool Game::Settings::wireframeOn = false;

	float Game::Settings::brightness = 1.0;
	float Game::Settings::contrast = 1.0;

	bool Game::Settings::noSound = false;
	float Game::Settings::volume = 1.0;
	float Game::Settings::ambientVolume = 1.0;
	float Game::Settings::soundVolume = 0.8;
	float Game::Settings::musicVolume = 0.6;

	void Game::initGameModeStrings() {
		string resolution[] = { "800x600", "960x720", "1024x768", "1280x720", "1280x768", "1360x768", "1366x768", "1280x960", "1440x1080", "1600x1200", "1920x1080" };
		string color[] = { "24", "32" };
		string fps[] = { "40", "60" };
		for (int r = 0; r<arrayLength(resolution); r++)
		for (int c = 0; c<arrayLength(color); c++)
		for (int f = 0; f < arrayLength(fps); f++) {
			string s = resolution[r] + ":" + color[c] + "@" + fps[f];
			glutGameModeString(s.data());
			if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
				Game::Settings::availableGameModeStrings.push_back(s);
		}
		checkGameMode();

		string h[] = { "480p", "720p", "1080p" };
		for (int i = 0; i<arrayLength(h); i++)
			Game::Settings::availableVideoHeight.push_back(h[i]);
	}

	void Game::checkGameMode() {
		if (!contains(Game::Settings::availableGameModeStrings, Game::Settings::gameModeString) && Game::Settings::availableGameModeStrings.size()>0)
			Game::Settings::gameModeString = Game::Settings::availableGameModeStrings.back();
	}

	void Game::loadSettings() {
		string filename = "data/settings.xml";
		string text = readTextFile(filename.data());
		xml_document<char> doc;
		doc.parse<0>((char*)text.data());
		try {
			if (doc.first_node("settings")->first_node("graphics")) {
				xml_node<char> *graphicsNode = doc.first_node("settings")->first_node("graphics");
				if (graphicsNode->first_node("fullscreen")) Settings::fullscreen = toBool(graphicsNode->first_node("fullscreen")->value());
				if (graphicsNode->first_node("screenWidth")) Settings::screenWidth = toInt(graphicsNode->first_node("screenWidth")->value());
				if (graphicsNode->first_node("screenHeight")) Settings::screenHeight = toInt(graphicsNode->first_node("screenHeight")->value());
				if (graphicsNode->first_node("gameMode")) Settings::gameMode = toBool(graphicsNode->first_node("gameMode")->value());
				if (graphicsNode->first_node("gameModeString")) Settings::gameModeString = trim(graphicsNode->first_node("gameModeString")->value());
				if (graphicsNode->first_node("reflectionOn")) Settings::reflectionOn = toBool(graphicsNode->first_node("reflectionOn")->value());
				if (graphicsNode->first_node("shadowOn")) Settings::shadowOn = toBool(graphicsNode->first_node("shadowOn")->value());
				if (graphicsNode->first_node("antialiasingOn")) Settings::antialiasingOn = toBool(graphicsNode->first_node("antialiasingOn")->value());
				if (graphicsNode->first_node("terrainBumpmapOn")) Settings::terrainBumpmapOn = toBool(graphicsNode->first_node("terrainBumpmapOn")->value());
				if (graphicsNode->first_node("objectBumpmapOn")) Settings::objectBumpmapOn = toBool(graphicsNode->first_node("objectBumpmapOn")->value());
				if (graphicsNode->first_node("motionBlurOn")) Settings::motionBlurOn = toBool(graphicsNode->first_node("motionBlurOn")->value());
				if (graphicsNode->first_node("depthOfFieldOn")) Settings::depthOfFieldOn = toBool(graphicsNode->first_node("depthOfFieldOn")->value());
				if (graphicsNode->first_node("bloomOn")) Settings::bloomOn = toBool(graphicsNode->first_node("bloomOn")->value());
				if (graphicsNode->first_node("renderObjectsOn")) Settings::renderObjectsOn = toBool(graphicsNode->first_node("renderObjectsOn")->value());
				if (graphicsNode->first_node("renderTerrainOn")) Settings::renderTerrainOn = toBool(graphicsNode->first_node("renderTerrainOn")->value());
				if (graphicsNode->first_node("renderWaterOn")) Settings::renderWaterOn = toBool(graphicsNode->first_node("renderWaterOn")->value());
				if (graphicsNode->first_node("renderSkyOn")) Settings::renderSkyOn = toBool(graphicsNode->first_node("renderSkyOn")->value());
				if (graphicsNode->first_node("stereoscopicOn")) Settings::stereoscopicOn = toBool(graphicsNode->first_node("stereoscopicOn")->value());
				if (graphicsNode->first_node("stereoSeperation")) Settings::stereoSeperation = toFloat(graphicsNode->first_node("stereoSeperation")->value());
				if (graphicsNode->first_node("wireframeOn")) Settings::wireframeOn = toBool(graphicsNode->first_node("wireframeOn")->value());
				if (graphicsNode->first_node("noShader")) Settings::noShader = toBool(graphicsNode->first_node("noShader")->value());
				if (graphicsNode->first_node("videoHeight")) Settings::videoHeight = graphicsNode->first_node("videoHeight")->value();
			}

			if (doc.first_node("settings")->first_node("audio")) {
				xml_node<char> *audioNode = doc.first_node("settings")->first_node("audio");
				if (audioNode->first_node("volume")) Settings::volume = toFloat(audioNode->first_node("volume")->value());
				if (audioNode->first_node("ambientVolume")) Settings::ambientVolume = toFloat(audioNode->first_node("ambientVolume")->value());
				if (audioNode->first_node("musicVolume")) Settings::musicVolume = toFloat(audioNode->first_node("musicVolume")->value());
				if (audioNode->first_node("soundVolume")) Settings::soundVolume = toFloat(audioNode->first_node("soundVolume")->value());
			}

			if (doc.first_node("settings")->first_node("game")) {
				xml_node<char> *gameNode = doc.first_node("settings")->first_node("game");
				if (gameNode->first_node("scrollSpeed")) Settings::scrollSpeed = toFloat(gameNode->first_node("scrollSpeed")->value());
				if (gameNode->first_node("showLOS")) Settings::showLOS = toBool(gameNode->first_node("showLOS")->value());
				if (gameNode->first_node("showPath")) Settings::showPath = toBool(gameNode->first_node("showPath")->value());
				if (gameNode->first_node("showRenderDetails")) Settings::showRenderDetails = toBool(gameNode->first_node("showRenderDetails")->value());
			}
		} catch (parse_error &e) {
			throw Exception("XML Parse Error in file " + filename);
		}

		checkGameMode();
		setSoundSettings();
	}
	void Game::saveSettings() {
		xml_document<char> doc;
		doc.append_node(doc.allocate_node(node_element, "settings"));

		xml_node<char> *graphicsNode = doc.allocate_node(node_element, "graphics");
		doc.first_node("settings")->append_node(graphicsNode);
		graphicsNode->append_node(doc.allocate_node(node_element, "fullscreen", doc.allocate_string(toString(Settings::fullscreen).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "screenWidth", doc.allocate_string(toString(Settings::screenWidth).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "screenHeight", doc.allocate_string(toString(Settings::screenHeight).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "gameMode", doc.allocate_string(toString(Settings::gameMode).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "gameModeString", Settings::gameModeString.data()));
		graphicsNode->append_node(doc.allocate_node(node_element, "reflectionOn", doc.allocate_string(toString(Settings::reflectionOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "shadowOn", doc.allocate_string(toString(Settings::shadowOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "antialiasingOn", doc.allocate_string(toString(Settings::antialiasingOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "terrainBumpmapOn", doc.allocate_string(toString(Settings::terrainBumpmapOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "objectBumpmapOn", doc.allocate_string(toString(Settings::objectBumpmapOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "motionBlurOn", doc.allocate_string(toString(Settings::motionBlurOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "depthOfFieldOn", doc.allocate_string(toString(Settings::depthOfFieldOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "bloomOn", doc.allocate_string(toString(Settings::bloomOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "renderObjectsOn", doc.allocate_string(toString(Settings::renderObjectsOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "renderTerrainOn", doc.allocate_string(toString(Settings::renderTerrainOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "renderWaterOn", doc.allocate_string(toString(Settings::renderWaterOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "renderSkyOn", doc.allocate_string(toString(Settings::renderSkyOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "stereoscopicOn", doc.allocate_string(toString(Settings::stereoscopicOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "stereoSeperation", doc.allocate_string(toString(Settings::stereoSeperation).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "wireframeOn", doc.allocate_string(toString(Settings::wireframeOn).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "noShader", doc.allocate_string(toString(Settings::noShader).data())));
		graphicsNode->append_node(doc.allocate_node(node_element, "videoHeight", Settings::videoHeight.data()));

		xml_node<char> *audioNode = doc.allocate_node(node_element, "audio");
		doc.first_node("settings")->append_node(audioNode);
		audioNode->append_node(doc.allocate_node(node_element, "volume", doc.allocate_string(toString(Settings::volume).data())));
		audioNode->append_node(doc.allocate_node(node_element, "ambientVolume", doc.allocate_string(toString(Settings::ambientVolume).data())));
		audioNode->append_node(doc.allocate_node(node_element, "soundVolume", doc.allocate_string(toString(Settings::soundVolume).data())));
		audioNode->append_node(doc.allocate_node(node_element, "musicVolume", doc.allocate_string(toString(Settings::musicVolume).data())));

		xml_node<char> *gameNode = doc.allocate_node(node_element, "game");
		doc.first_node("settings")->append_node(gameNode);
		gameNode->append_node(doc.allocate_node(node_element, "scrollSpeed", doc.allocate_string(toString(Settings::scrollSpeed).data())));
		gameNode->append_node(doc.allocate_node(node_element, "showLOS", doc.allocate_string(toString(Settings::showLOS).data())));
		gameNode->append_node(doc.allocate_node(node_element, "showPath", doc.allocate_string(toString(Settings::showPath).data())));
		gameNode->append_node(doc.allocate_node(node_element, "showRenderDetails", doc.allocate_string(toString(Settings::showRenderDetails).data())));

		string text;
		print(back_inserter(text), doc, 0);
		string filename = "data/settings.xml";
		ofstream file(filename.data());
		file << text;
		file.close();

		setSoundSettings();
	}

	void Game::setSoundSettings() {
		setMusicVolume(Game::Settings::volume*Game::Settings::musicVolume);
		setAmbientVolume(Game::Settings::volume*Game::Settings::ambientVolume);
		setSoundVolume(Game::Settings::volume*Game::Settings::soundVolume);
		setSpeechVolume(Game::Settings::soundVolume);
	}
}
