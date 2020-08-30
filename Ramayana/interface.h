#ifndef __RAMAYANA_INTERFACE_H
#define __RAMAYANA_INTERFACE_H

#define MENU_MOTION_BLUR_AMOUNT			0.5

#define MIN_MAP_SIZE					50
#define MAX_MAP_SIZE					500
#define MIN_MAP_GROUND_HEIGHT			-10
#define MAX_MAP_GROUND_HEIGHT			10

#define TEXT_VGAP						6

#define CURSOR_SIZE						100

#define TOOL_TIP_HEIGHT					12
#define TOOL_TIP_BOTTOM					40
#define TOOL_TIP_LEFT					10
#define TOOL_TIP_SIDE_BORDER			1
#define TOOL_TIP_BOTTOM_BORDER			3

#define MAIN_MENU_BUTTON_TOP			200
#define MAIN_MENU_BUTTON_LEFT			100
#define MAIN_MENU_BUTTON_WIDTH			(Game::Settings::screenWidth/10)
#define MAIN_MENU_BUTTON_HEIGHT			MAIN_MENU_BUTTON_WIDTH
#define MAIN_MENU_BUTTON_GAP			5
#define MENU_BACK_BUTTON_LEFT			Game::Settings::screenWidth-200
#define MENU_BACK_BUTTON_TOP			20
#define MENU_BACK_BUTTON_WIDTH			75
#define MENU_BACK_BUTTON_HEIGHT			MENU_BACK_BUTTON_WIDTH

#define MENU_SLIDEBAR_WIDTH				(Game::Settings::screenWidth*0.5)
#define MENU_SLIDEBAR_HEIGHT			50
#define MENU_SLIDEBAR_VGAP				5

#define MINIMAP_SIZE					200
#define MINIMAP_BORDER					10
#define MINIMAP_LEFT					0
#define MINIMAP_TOP						(Game::Settings::screenHeight-MINIMAP_SIZE-2*MINIMAP_BORDER)
#define MINIMAP_RIGHT					(MINIMAP_SIZE+2*MINIMAP_BORDER)
#define MINIMAP_BOTTOM					Game::Settings::screenHeight

#define RESOURCE_TOP					10
#define RESOURCE_ICON_LEFT				10
#define RESOURCE_ICON_SIZE				20
#define RESOURCE_TEXT_LEFT				40
#define	RESOURCE_VGAP					2
#define RESOURCE_PANE_BOTTOM			(RESOURCE_TOP+(RESOURCE_ICON_SIZE+RESOURCE_VGAP)*5+10)
#define RESOURCE_PANE_RIGHT				100
#define IDLE_WORKER_BUTTON_TOP			150
#define IDLE_WORKER_BUTTON_SIZE			30
#define RESOURCE_ERROR_ICON_SIZE		(2*RESOURCE_ICON_SIZE/3)

#define INFO_PANE_LEFT					MINIMAP_RIGHT
#define INFO_PANE_RIGHT					(INFO_PANE_LEFT+400)
#define INFO_PANE_TOP					(MINIMAP_TOP+30)
#define INFO_PANE_BOTTOM				Game::Settings::screenHeight
#define INFO_LEFT						INFO_PANE_LEFT
#define INFO_RIGHT						INFO_PANE_RIGHT
#define INFO_TOP						(INFO_PANE_TOP+10)
#define INFO_BOTTOM						(Game::Settings::screenHeight-10)
#define INFO_BIG_ICON_SIZE				100
#define INFO_SMALL_ICON_SIZE			50
#define INFO_HEALTH_BAR_HEIGHT			5
#define INFO_SMALL_ICON_HGAP			30
#define INFO_SMALL_ICON_VGAP			5
#define INFO_PROPERTY_LEFT				(INFO_LEFT+INFO_BIG_ICON_SIZE+10)
#define INFO_PROPERTY_BOTTOM			(Game::Settings::screenHeight-50)
#define INFO_PROPERTY_ICON_SIZE			20
#define INFO_PROPERTY_ICON_HGAP			100
#define INFO_PROPERTY_ICON_VGAP			5

#define ABILITY_PANE_LEFT				(Game::Settings::screenWidth-350)
#define ABILITY_PANE_RIGHT				Game::Settings::screenWidth
#define ABILITY_PANE_TOP				INFO_PANE_TOP
#define ABILITY_PANE_BOTTOM				Game::Settings::screenHeight
#define ABILITY_ICON_SIZE				50
#define ABILITY_ICON_LEFT				INFO_RIGHT
#define ABILITY_ICON_RIGHT				ABILITY_PANE_RIGHT
#define ABILITY_ICON_TOP				INFO_PANE_TOP
#define ABILITY_ICON_TOPGAP				10
#define ABILITY_ICON_VGAP				5
#define ABILITY_ICON_HGAP				10
#define ABILITY_ICON_BOTTOM				ABILITY_PANE_BOTTOM

#define PAUSE_BUTTON_SIZE				50
#define PAUSE_BUTTON_LEFT				(Game::Settings::screenWidth-50)
#define PAUSE_BUTTON_TOP				0

#define MENU_LOGO_LEFT					100
#define MENU_LOGO_TOP					10
#define MENU_LOGO_WIDTH					(Game::Settings::screenWidth*0.45)
#define MENU_LOGO_HEIGHT				(Game::Settings::screenWidth*0.16)

#define MENU_CREDITS_BUTTON_LEFT		(Game::Settings::screenWidth-60)
#define MENU_CREDITS_BUTTON_TOP			(Game::Settings::screenHeight-60)
#define MENU_CREDITS_BUTTON_SIZE		40
#define CREDITS_IMAGE_SIZE_RATIO		3				// credits_image_height/screen_size ratio

#define MENU_CONTROLS_BUTTON_LEFT		(Game::Settings::screenWidth-125)
#define CONTROLS_IMAGE_SIZE_RATIO		2

#define SAVEGAME_MENU_LIST_TOP			200
#define SAVEGAME_MENU_LIST_BOTTOM		(Game::Settings::screenHeight-200)
#define SAVEGAME_MENU_LIST_LEFT			(Game::Settings::screenWidth/2)
#define SAVEGAME_MENU_LIST_WIDTH		(Game::Settings::screenWidth/2-100)
#define SAVEGAME_MENU_LIST_BTN_HEIGHT	15

#define CAMPAIGNMENU_BUTTON_WIDTH		100
#define CAMPAIGNMENU_BUTTON_HEIGHT		200
#define CAMPAIGNMENU_BUTTON_GAP			10
#define CAMPAIGNMENU_MAP_SIZE			(Game::Settings::screenHeight-MAIN_MENU_BUTTON_TOP-100)

#define SETTINGS_MENU_SLIDEBAR_VGAP2	20

#define SKIRMISH_TEAM_LEFT				0
#define SKIRMISH_TEAM_TOP				200
#define SKIRMISH_TEAM_RIGHT				(Game::Settings::screenWidth-550)
#define SKIRMISH_TEAM_PANEL_WIDTH		(SKIRMISH_TEAM_RIGHT-SKIRMISH_TEAM_LEFT)
#define SKIRMISH_TEAM_BOTTOM			(Game::Settings::screenHeight-100)
#define SKIRMISH_TEAM_BUTTON_VGAP		40
#define SKIRMISH_TEAM_BUTTON_HGAP		5
#define SKIRMISH_TEAM_SMALL_BUTTON_SIZE	30
#define SKIRMISH_TEAM_BUTTON_WIDTH1		( 7*(SKIRMISH_TEAM_RIGHT-SKIRMISH_TEAM_LEFT-5*SKIRMISH_TEAM_BUTTON_HGAP-SKIRMISH_TEAM_SMALL_BUTTON_SIZE)/100)
#define SKIRMISH_TEAM_BUTTON_WIDTH2		(40*(SKIRMISH_TEAM_RIGHT-SKIRMISH_TEAM_LEFT-5*SKIRMISH_TEAM_BUTTON_HGAP-SKIRMISH_TEAM_SMALL_BUTTON_SIZE)/100)
#define SKIRMISH_TEAM_BUTTON_WIDTH3		(40*(SKIRMISH_TEAM_RIGHT-SKIRMISH_TEAM_LEFT-5*SKIRMISH_TEAM_BUTTON_HGAP-SKIRMISH_TEAM_SMALL_BUTTON_SIZE)/100)
#define SKIRMISH_TEAM_BUTTON_WIDTH4		( 6*(SKIRMISH_TEAM_RIGHT-SKIRMISH_TEAM_LEFT-5*SKIRMISH_TEAM_BUTTON_HGAP-SKIRMISH_TEAM_SMALL_BUTTON_SIZE)/100)
#define SKIRMISH_TEAM_BUTTON_WIDTH5		( 7*(SKIRMISH_TEAM_RIGHT-SKIRMISH_TEAM_LEFT-5*SKIRMISH_TEAM_BUTTON_HGAP-SKIRMISH_TEAM_SMALL_BUTTON_SIZE)/100)
#define SKIRMISH_TEAM_BUTTON_HEIGHT		30

#define SKIRMISH_MAP_BORDER				25
#define SKIRMISH_MAP_SIZE				250
#define SKIRMISH_MAP_TOP				SKIRMISH_TEAM_TOP
#define SKIRMISH_MAP_LEFT				(Game::Settings::screenWidth-300)
#define SKIRMISH_MAP_RIGHT				Game::Settings::screenWidth
#define SKIRMISH_MAP_BOTTOM				(Game::Settings::screenHeight-100)
#define SKIRMISH_MAP_STRTPOS_BTN_SIZE	15
#define SKIRMISH_MAPNAMES_TOP			SKIRMISH_MAP_TOP
#define SKIRMISH_MAPNAMES_LEFT			(SKIRMISH_MAP_LEFT-200)
#define SKIRMISH_MAPNAMES_RIGHT			SKIRMISH_MAP_LEFT
#define SKIRMISH_MAPNAMES_BOTTOM		SKIRMISH_MAP_BOTTOM
#define SKIRMISH_MAPNAMES_BTN_HEIGHT	15
#define SKIRMISH_MAPNAMES_BTN_VGAP		1
#define SKIRMISH_MAP_SCROLLBAR_LEFT		(SKIRMISH_MAPNAMES_LEFT-10)
#define SKIRMISH_MAP_SCROLLBAR_RIGHT	(SKIRMISH_MAPNAMES_LEFT-2)

#define EDITOR_MENU_SLIDEBAR_WIDTH		(SKIRMISH_MAPNAMES_LEFT-MAIN_MENU_BUTTON_LEFT-100)


namespace ramayana {
	enum LogType {
		LOG_INFO,
		LOG_SEVERE
	};
	class Logger {
		ofstream log;
		static Logger logger;
		Lock mutex;
	public:
		Logger();
		void print(string msg, LogType type = LOG_INFO);
		friend Logger& getLogger();
	};

	Logger& getLogger();

	void loadGame();

	void displayGamePlaying();
	void mouseGamePlaying();
	void passivemotionGamePlaying();
	void motionGamePlaying();
	void mouseentryGamePlaying();
	void keyboardGamePlaying();
	void specialGamePlaying();
	void keyboardupGamePlaying();
	void specialupGamePlaying();
	void idleGamePlaying();
	void visibilityGamePlaying();

	void _GLUT_CALLBACK display();
	void _GLUT_CALLBACK reshape();
	void _GLUT_CALLBACK passivemotion();
	void _GLUT_CALLBACK mouse();
	void _GLUT_CALLBACK motion();
	void _GLUT_CALLBACK keyboard();
	void _GLUT_CALLBACK keyboardup();
	void _GLUT_CALLBACK special();
	void _GLUT_CALLBACK specialup();
	void _GLUT_CALLBACK idle();

	void registerGLUTCallBacks();
	void initInterface();
}

#endif