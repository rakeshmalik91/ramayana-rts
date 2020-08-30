#include "stdafx.h"

#include "common.h"
#include "interface.h"
#include "terrain.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "audio.h"
#include "steamStatsAndAchievements.h"

using namespace ramayana;

//------------------------------------------------------------------------------------------------Close

void close() {
	Game::saveSettings();

	closeSDLAudio();
}


//------------------------------------------------------------------------------------------------Initialization

void initGL() {
	try {
		// Get Pointers To The GL Functions
		setUpGLShader();
		if(!setUp_ARB_shader_objects())				showMessage("ARB_shader_objects unsupported", "OpenGL");
		if(!setUpVertexBuffer())					showMessage("Vertex buffer unsupported", "OpenGL");
		if(!setUp_ARB_vertex_buffer_object())		showMessage("ARB_vertex_buffer_object unsupported", "OpenGL");
		if(!setUp_ARB_vertex_program())				showMessage("ARB_vertex_program unsupported", "OpenGL");
		if(!setUp_EXT_fog_coord())					showMessage("EXT_fog_coord unsupported", "OpenGL");
		if(!setUp_EXT_texture3D())					showMessage("EXT_texture3D unsupported", "OpenGL");
		if(!setUp_ARB_multitexture())				showMessage("ARB_multitexture unsupported", "OpenGL");
		if(!setUp_EXT_framebuffer_object())			showMessage("EXT_framebuffer_object unsupported", "OpenGL");
		//unsupported
		//if(!setUp_EXT_texture_object())				showMessage("EXT_texture_object unsupported", "OpenGL");
		//if(!setUp_EXT_vertex_array())				showMessage("EXT_vertex_array unsupported", "OpenGL");
		//if(!setUp_EXT_vertex_attrib_64bit())		showMessage("EXT_vertex_attrib_64bit unsupported", "OpenGL");
		//if(!setUp_ARB_multisample())				showMessage("ARB_multisample unsupported", "OpenGL");
		//if(!setUp_EXT_geometry_shader4())			showMessage("EXT_geometry_shader4 unsupported", "OpenGL");
		/*if(isEXTSupported("GL_EXT_texture_filter_anisotropic")) {
			glEnable(GL_TEXTURE_MAX_ANISOTROPY_EXT);
			GLfloat largest_supported_anisotropy;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
		} else showMessage("EXT_texture_filter_anisotropic unsupported", "OpenGL");*/
	} catch(Exception &e) {
		showMessage(e.getMessage(), "Runtime Exception : initGL()", true);
	}

	glShadeModel(GL_SMOOTH);

	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}
void initGLUT(int argc, char* argv[]) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH | GLUT_ACCUM | GLUT_MULTISAMPLE);
	glutInitWindowSize(Game::Settings::screenWidth, Game::Settings::screenHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Ramayana");

	Game::initGameModeStrings();
	glutGameModeString(Game::Settings::gameModeString.data());
	if (Game::Settings::gameMode && glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
		glutEnterGameMode();
	}
	if (Game::Settings::fullscreen && !glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
		glutFullScreen();
	}

	Game::Settings::screenWidth = glutGet(GLUT_WINDOW_WIDTH);
	Game::Settings::screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

	glutSetCursor(GLUT_CURSOR_NONE);
}
void init(int argc, char* argv[]) {
	initSteam();

	try {
		Game::loadSettings();
		getLogger().print("Settings Loaded.");
		initSDLAudio();
		getLogger().print("SDL initialized.");
		initGLUT(argc, argv);
		getLogger().print("GLUT initialized.");
		initGL();
		getLogger().print("OpenGL initialized.");

		getLogger().print("Texture Library initialized.");

		initInterface();
		Game::setSoundSettings();
		getLogger().print("Interface initialized.");

		loadGame();
		getLogger().print("Game loaded.");
	} catch(Exception &e) {
		showMessage(e.getMessage(), "Runtime Exception : init()", true);
	}
}


//------------------------------------------------------------------------------------------------Main function


int main(int argc, char* argv[]) {
	getLogger().print("Game started.");
	init(argc, argv);

	registerGLUTCallBacks();
	getLogger().print("GLUT callbacks registered.");

	getLogger().print("Starting GLUT main loop...");
	glutMainLoop();
	getLogger().print("GLUT main loop broke.");

	close();
	return 0;
}

#ifdef _MSC_VER

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	char* argv[] = { "" };
	main(1, argv);
	return 0;
}

#endif