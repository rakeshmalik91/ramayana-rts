#ifndef __RAMAYANA_CAMPAIGN_H
#define __RAMAYANA_CAMPAIGN_H

#include "common.h"
#include "game.h"


namespace ramayana {

	class Campaign : public Game {
	private:
		string campaignFileName;
		struct StoryLine {
			string work;
			string unit, target, speech;
			int time, team, team2, val;
			string desc;
			StoryLine() : work(""), unit(""), target(""), speech(""), time(0), team(0), team2(0), val(0), desc("") {}
			bool isEndConstraintOf(const StoryLine& l) const;
		};
		vector<StoryLine> story;
		vector<StoryLine> objectiveConstraint;
		Diplomacy **diplomacyChart;
		string subtitle;
	private:
		void manageStory();
		void initTeams(Team[], int, int, int, int, int, int);													//
		void readCampaignXML(xml_document<char>&);
	public:
		void(*unlockCampaign)(int);
		Unit& getUnit(string);
		void load(string, UnitTypeInfo*);
		void update();
		bool hasCompleted();
		bool isCampaign();
		virtual void victory();
		virtual void defeat();
		Diplomacy diplomacy(int, int) const;
		void setDiplomacy(int, int, Diplomacy);
		string getSubtitle();
		virtual void saveSnapshot(string);																//
		virtual void loadSnapshot(string, UnitTypeInfo*);														//
	};
}

#endif