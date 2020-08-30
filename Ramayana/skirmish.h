#ifndef __RAMAYANA_SKIRMISH_H
#define __RAMAYANA_SKIRMISH_H

#include "common.h"
#include "game.h"


namespace ramayana {

	class Skirmish : public Game {
	private:
		void initTeams(Team[], int, int, int, int, int, int);													//
	public:
		void load(string, UnitTypeInfo*, Team[], int, int, int, int, int, int);								//Loads from a file
		bool isSkirmish();
	};
}

#endif