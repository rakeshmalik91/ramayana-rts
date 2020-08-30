#include "stdafx.h"

namespace ramayana {

	bool gameHosted = false;
	bool connected = false;

	string hostedGameName = "New Hosted Game";
	
	class Players {
		string Name;
		string ip;
		string index;
	};

	void findGames() {
		//...
		connected = true;
	}

	int _SDL_THREAD findPlayers(void *unused) {
		while (connected && !gameHosted) {
			//...
		}
		return 0;
	}

	void hostGame() {
		SDL_CreateThread(&findPlayers, NULL);
	}

	void disconnectGame() {
		//...
		connected = false;
	}
}