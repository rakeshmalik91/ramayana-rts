#include "stdafx.h"

#include "common.h"
#include "unit.h"
#include "campaign.h"
#include "audio.h"
	
namespace ramayana {
	int getFaction(string s) {
		s=toLower(s);
		for(int i=1; i<arrayLength(factionName); i++)
			if(toLower(factionName[i])==s)
				return i;
		return 0;
	}
	Unit& Campaign::getUnit(string id) {
		if (id.empty()) {
			showMessage("Empty campaign unit ID", "Error", false);
		}
		for (UnitID u = 0; u < nUnit; u++) {
			if (unit[u].idForCampaign == id) {
				return unit[u];
			}
		}
		return unit[0];
	}
	void Campaign::initTeams(Team teams[], int nTeams, int playerTeam, int food, int wood, int stone, int metal) {
		for(int t=0; t<=this->nTeams; t++)
			this->teams[t].init(this, unitTypeInfo, t);
	}
	void Campaign::readCampaignXML(xml_document<char> &doc) {
		nTeams=0;
		for(xml_node<char> *node=doc.first_node("campaign")->first_node("team"); node; node=node->next_sibling("team")) {
			nTeams++;
			teams[nTeams].name=trim(node->first_node("name")->value());
			teams[nTeams].faction=getFaction(node->first_node("faction")->value());
			teams[nTeams].teamGroup=toInt(node->first_node("teamGroup")->value());
			teams[nTeams].color=toInt(node->first_node("color")->value());
			if(node->first_node("resource")) {
				teams[nTeams].resource.food=toInt(node->first_node("resource")->first_node("food")->value());
				teams[nTeams].resource.wood=toInt(node->first_node("resource")->first_node("wood")->value());
				teams[nTeams].resource.stone=toInt(node->first_node("resource")->first_node("stone")->value());
				teams[nTeams].resource.metal=toInt(node->first_node("resource")->first_node("metal")->value());
			}
			if(node->first_node("hasAI")) teams[nTeams].hasAI=true;
		}
		diplomacyChart=allocate<Diplomacy>(nTeams, nTeams);
		for(int t1=0; t1<nTeams; t1++)
			for(int t2=0; t2<nTeams; t2++)
				if(t1==t2)
					diplomacyChart[t1][t2]=DIPLOMACY_ALLY;
				else
					diplomacyChart[t1][t2]=DIPLOMACY_NEUTRAL;
		for(xml_node<char> *node=doc.first_node("campaign")->first_node("diplomacy"); node; node=node->next_sibling("diplomacy")) {
			TeamID t1=toInt(node->first_attribute("team1")->value())-1;
			TeamID t2=toInt(node->first_attribute("team2")->value())-1;
			diplomacyChart[t1][t2]=diplomacyChart[t2][t1]=(trim(node->value())=="enemy"?DIPLOMACY_ENEMY:DIPLOMACY_ALLY);
		}
		playerTeam=toInt(doc.first_node("campaign")->first_node("playerTeam")->value());
		for(xml_node<char> *node=doc.first_node("campaign")->first_node("story")->first_node("line"); node; node=node->next_sibling("line")) {
			StoryLine line;
			line.work=trim(node->first_attribute("work")->value());
			if(node->first_attribute("unit"))		line.unit=trim(node->first_attribute("unit")->value());
			if(node->first_attribute("target"))		line.target=trim(node->first_attribute("target")->value());
			if(node->first_attribute("time"))		line.time=toInt(node->first_attribute("time")->value());
			if(node->first_attribute("team"))		line.team=toInt(node->first_attribute("team")->value());
			if(node->first_attribute("team2"))		line.team2=toInt(node->first_attribute("team2")->value());
			if(node->first_attribute("val"))		line.val=toInt(node->first_attribute("val")->value());
			if(node->first_attribute("speech"))	line.speech=trim(node->first_attribute("speech")->value());
			line.desc=trim(node->value());
			story.push_back(line);
		}
	}
	void Campaign::load(string fname, UnitTypeInfo* unitTypeInfo) {
		campaignFileName=fname;
		string text=readTextFile(fname.data());
		xml_document<char> doc;
		doc.parse<0>((char*)text.data());
		try {
			readCampaignXML(doc);
		} catch(parse_error &e) {
			throw Exception("XML Parse Error in file "+fname);
		}
			
		string mapFileName=removeExtension(campaignFileName)+".map";
		Game::load(mapFileName, unitTypeInfo, teams, 0, 1, 0, 0, 0, 0, false);
	}

	bool Campaign::StoryLine::isEndConstraintOf(const StoryLine& l) const {
		if (startsWith(work, l.work) 
			&& time == l.time && val == l.val && team == l.team && team2 == l.team2 
			&& unit == l.unit && target == l.target)
			return true;
		return false;
	}

	void Campaign::manageStory() {
		minimapMarker.clear();

		if(story.empty()) {
			pause();
			return;
		}
		string oldObjective=objective;
		objective="";

		if (!story.front().speech.empty() && !isPlaying(CHANNEL_SPEECH)) {
			playAudio("audio/"+story.front().speech, CHANNEL_SPEECH);
			story.front().speech = "";
		}
	
		//Story
		if (startsWith(story.front().work, "OBJ")) {
			if(story.front().work=="OBJ defeat") {																			//OBJ defeat			team
				if(teams[story.front().team].defeated) {
					story.erase(story.begin());
					setMessage("Objective complete : " + story.front().desc);
				} else {
					objective=story.front().desc+"\n";
				}
			} else if(story.front().work=="OBJ defeat all") {																//OBJ defeat
				bool allEnemyDefeated=true;
				for(int t=1; t<=nTeams; t++)
					if(diplomacy(playerTeam, t)==DIPLOMACY_ENEMY && !teams[t].defeated)
						allEnemyDefeated=false;
				if(allEnemyDefeated) {
					story.erase(story.begin());
					setMessage("Objective complete : " + story.front().desc);
				} else {
					objective=story.front().desc+"\n";
				}
			} else if(story.front().work=="OBJ move to") {																	//OBJ move to			unit	target
				Unit &playerUnit = getUnit(story.front().unit), &targetUnit = getUnit(story.front().target);
				if (dist((Point2D)playerUnit.position(), (Point2D)targetUnit.position()) < 5) {
					//erase storyline
					story.erase(story.begin());
					setMessage("Objective complete : " + story.front().desc);
				} else {
					//set objective
					objective=story.front().desc+"\n";
					minimapMarker.push_back(targetUnit.position());
				}
			} else if(story.front().work=="OBJ kill") {																		//OBJ kill				target
				Unit &targetUnit = getUnit(story.front().target);
				if (!targetUnit.isAlive()) {
					//erase storyline
					story.erase(story.begin());
					setMessage("Objective complete : " + story.front().desc);
				} else {
					//set objective
					objective=story.front().desc+"\n";
					minimapMarker.push_back(targetUnit.position());
				}
			} else if(story.front().work=="OBJ damage") {																	//OBJ damage			target	val
				if (getUnit(story.front().target).hitPoint < getUnit(story.front().target).getMaxHitPoint() * (100 - story.front().val) / 100) {
					//erase storyline
					story.erase(story.begin());
					setMessage("Objective complete : " + story.front().desc);
				} else {
					//set objective
					objective=story.front().desc+"\n";
					minimapMarker.push_back(getUnit(story.front().target).position());
				}
			} else if(story.front().work=="OBJ gather stone") {																//OBJ gather stone		val
				if (teams[playerTeam].resource.stone >= story.front().val) {
					//erase storyline
					story.erase(story.begin());
					setMessage("Objective complete : " + story.front().desc);
				} else {
					//set objective
					objective=story.front().desc+"("+(story.front().val-teams[playerTeam].resource.stone)+" more needed)\n";
				}
			} else {
				showMessage("Invalid objective " + story.front().work + ".", "Campaign XML Error");
			}
		} else if (startsWith(story.front().work, "CONST")) {
			if (endsWith(story.front().work, "END")) {
				for (vector<StoryLine>::iterator i = objectiveConstraint.begin(); i != objectiveConstraint.end(); i++) {
					if (story.front().isEndConstraintOf(*i)) {
						objectiveConstraint.erase(i);
						break;
					}
				}
			} else {
				objectiveConstraint.push_back(story.front());
			}
			story.erase(story.begin());
		} else {
			if(story.front().work=="unlock") {																				//unlock				val
				(*unlockCampaign)(story.front().val);
				story.erase(story.begin());
			} else if(story.front().work=="subtitle") {																		//subtitle				unit	time
				if(dist((Point2D)getUnit(story.front().unit).position(), Point2D(camX, camY))>0.1) {
					// move camera
					lockCommand();
					Point2D pos=(Point2D(camX, camY)+getUnit(story.front().unit).position())/2;
					goTo(pos.x, pos.y);
					subtitle = story.front().desc;
					story.front().time-=1000.0/UPDATE_THREAD_FRAME_RATE;
				} else if(isPlaying(CHANNEL_SPEECH)) {
					//wait for given time
					lockCommand();
					Point2D pos=(Point2D(camX, camY)+getUnit(story.front().unit).position())/2;
					goTo(pos.x, pos.y);
					story.front().time-=1000.0/UPDATE_THREAD_FRAME_RATE;
					subtitle = story.front().desc;
				} else {
					//finish
					unlockCommand();
					story.erase(story.begin());
					subtitle = "";
				}
			} else if(story.front().work=="show") {																			//show					unit	time
				if(dist((Point2D)getUnit(story.front().unit).position(), Point2D(camX, camY))>0.1) {
					Point2D pos=(Point2D(camX, camY)+getUnit(story.front().unit).position())/2;
					goTo(pos.x, pos.y);
				} else if(story.front().time>0) {
					Point2D pos=(Point2D(camX, camY)+getUnit(story.front().unit).position())/2;
					goTo(pos.x, pos.y);
					story.front().time-=1000.0/UPDATE_THREAD_FRAME_RATE;
				} else {
					story.erase(story.begin());
				}
			} else if(story.front().work=="zoom") {																			//zoom					val
				if (abs(camZ - (-story.front().val)) > 0.1) {
					camZ = (camZ + (-story.front().val)) / 2;
				} else {
					story.erase(story.begin());
				}
			} else if(story.front().work=="tilt") {																			//tilt					val
				if (abs(tilt - story.front().val) > 0.1) {
					tilt = (tilt + story.front().val) / 2;
				} else {
					story.erase(story.begin());
				}
			} else if(story.front().work=="rotate") {																		//rotate				val
				if (abs(rotation - story.front().val) > 0.1) {
					rotation = (rotation + story.front().val) / 2;
				} else {
					story.erase(story.begin());
				}
			} else if(story.front().work=="change team") {																	//change team			unit	team
				getUnit(story.front().unit).team=story.front().team;
				story.erase(story.begin());
			} else if(story.front().work=="set diplomacy") {																//set diplomacy			team	team2
				setDiplomacy(story.front().team, story.front().team2, toDiplomacyType(story.front().desc));
				story.erase(story.begin());
			} else if(story.front().work=="remove") {																		//remove				unit
				getUnit(story.front().unit).remove();
				story.erase(story.begin());	
			} else if(story.front().work=="kill") {																			//kill					unit
				getUnit(story.front().unit).suicide();
				story.erase(story.begin());
			} else if (story.front().work == "stop") {																		//stop					unit
				getUnit(story.front().unit).stop();
				story.erase(story.begin());
			} else if (story.front().work == "attack") {																	//attack				unit	target
				getUnit(story.front().unit).newUserCommand(COMMAND_MOVE, getUnit(story.front().target).position(), false, true);
				getUnit(story.front().unit).newUserCommand(COMMAND_ATTACK, getUnit(story.front().target).getID(), true, true);
				story.erase(story.begin());
			} else if(story.front().work=="special attack") {																//special attack		unit	target
				getUnit(story.front().unit).newUserCommand(COMMAND_MOVE, getUnit(story.front().target).position(), false, true);
				getUnit(story.front().unit).newUserCommand(COMMAND_SPECIAL_ATTACK, getUnit(story.front().target).getID(), false, true, false, story.front().val);
				story.erase(story.begin());
			} else if(story.front().work == "move") {																		//move					unit	target
				Unit &playerUnit = getUnit(story.front().unit), &targetUnit = getUnit(story.front().target);
				if (dist(playerUnit.position2D(), targetUnit.position2D()) < 5) {
					playerUnit.rotateTowards(targetUnit);
				} else {
					playerUnit.newUserCommand(COMMAND_MOVE, targetUnit.position());
				}
				story.erase(story.begin());
			} else if(story.front().work == "reveal all") {																	//reveal all
				teams[playerTeam].setRevealAll(true);
				story.erase(story.begin());
			} else if(story.front().work == "explore all") {																//explore all
				teams[playerTeam].setExploreAll(true);
				story.erase(story.begin());
			} else if(story.front().work == "transform") {																	//transform				unit	val
				getUnit(story.front().unit).commandTransform(story.front().val);
				story.erase(story.begin());
			} else if (story.front().work == "transform_tobase") {															//transform_tobase		unit
				if (!getUnit(story.front().unit).isBaseTransformation()) {
					getUnit(story.front().unit).commandTransform(0);
				}
				story.erase(story.begin());
			} else if (story.front().work == "wait") {																		//wait					time
				if(story.front().time>0) {
					if (story.front().desc != "") {
						objective = story.front().desc + "(" + timeAsString(story.front().time) + " remaining)\n";
					}
					story.front().time-=1000.0/UPDATE_THREAD_FRAME_RATE;
				} else {
					story.erase(story.begin());
				}
			} else {
				showMessage("Invalid storyline " + story.front().work + ".", "Campaign XML Error");
			}
		}


	
		//Constraint
		static int counter=100;
		if(counter>0) {
			counter--;
		} else {
			counter=100;
			for(int i=0; i<objectiveConstraint.size(); i++) {
				if(objectiveConstraint[i].work=="CONST upspotted") {
					UnitID t=getUnit(objectiveConstraint[i].unit).getID();
					for(UnitID u=0; u<nUnit; u++)
						if(unit[u].isAlive() && unit[u].team==objectiveConstraint[i].team && unit[u].getTargetID()==t) {
							defeat();
							break;
						}
				} else if(objectiveConstraint[i].work=="CONST survive") {
					if(!getUnit(objectiveConstraint[i].unit).isAlive()) {
						defeat();
						break;
					}
				} else if(objectiveConstraint[i].work=="CONST dont attack") {
					for(UnitID u=0; u<nUnit; u++)
						if(unit[u].isAlive() && unit[u].team==playerTeam && unit[u].isAttacking() && unit[u].getTarget().team==objectiveConstraint[i].team) {
							defeat();
							break;
						}
				} else {
					showMessage("Invalid constraint " + objectiveConstraint[i].work + ".", "Campaign XML Error");
				}
			}
		}
		for (int i = 0; i < objectiveConstraint.size(); i++) {
			if (objectiveConstraint[i].desc != "") {
				objective += "(" + objectiveConstraint[i].desc + ")\n";
			}
		}

		//objective Updated
		if(objective!=oldObjective) {
			setMessage("Objective Updated...");
		}
	}
	void Campaign::update() {
		if(isPaused())
			return;

		Game::update();
		manageStory();
	}

	bool Campaign::hasCompleted() {
		return story.empty();
	}
	void Campaign::victory() {
		Game::victory();
		story.clear();
		unlockCommand();
	}
	void Campaign::defeat() {
		Game::defeat();
		story.clear();
		unlockCommand();
	}
	
	bool Campaign::isCampaign() {
		return true;
	}
	Diplomacy Campaign::diplomacy(int t1, int t2) const {
		if(t1==0 || t2==0)
			return DIPLOMACY_NEUTRAL;
		else
			return diplomacyChart[t1-1][t2-1];
	}
	void Campaign::setDiplomacy(int t1, int t2, Diplomacy d) {
		if (t1 >= 1 && t2 >= 1 && t1 <= nTeams && t2 <= nTeams) {
			diplomacyChart[t1 - 1][t2 - 1] = diplomacyChart[t2 - 1][t1 - 1] = d;
		}
	}

	string Campaign::getSubtitle() {
		return subtitle;
	}
	
	void Campaign::saveSnapshot(string saveFilename) {
		xml_document<char> doc;

		Game::writeSnapshotXML(doc);

		for (TeamID t1 = 1; t1 <= nTeams; t1++) {
			for (TeamID t2 = 1; t2 <= nTeams; t2++) {
				xml_node<char> *diplomacyNode = doc.allocate_node(node_element, "diplomacy", doc.allocate_string(diplomacyTypeToString(diplomacy(t1, t2)).data()));
				doc.first_node("save")->append_node(diplomacyNode);
				diplomacyNode->append_attribute(doc.allocate_attribute("team1", doc.allocate_string(toString(t1).data())));
				diplomacyNode->append_attribute(doc.allocate_attribute("team2", doc.allocate_string(toString(t2).data())));
			}
		}

		doc.first_node("save")->append_node(doc.allocate_node(node_element, "nRemainingStoryline", doc.allocate_string(toString((int)story.size()).data())));
		if(!story.empty())
			doc.first_node("save")->append_node(doc.allocate_node(node_element, "lastStoryLineTime", doc.allocate_string(toString(story.front().time).data())));

		string text;
		print(back_inserter(text), doc, 0);
		ofstream file(saveFilename.data());
		file<<text;
		file.close();
	}
	void Campaign::loadSnapshot(string saveFilename, UnitTypeInfo* unitTypeInfo) {
		this->editable = false;
		this->unitTypeInfo=unitTypeInfo;

		string savecmpn_text=readTextFile(saveFilename.data());
		xml_document<char> savecmpn;
		savecmpn.parse<0>((char*)savecmpn_text.data());
		
		try {
			string mapFileName = savecmpn.first_node("save")->first_node("map")->value();

			campaignFileName=removeExtension(mapFileName)+".cmpn";
			string cmpn_text=readTextFile(campaignFileName.data());
			xml_document<char> cmpn;
			cmpn.parse<0>((char*)cmpn_text.data());
			try {
				readCampaignXML(cmpn);
			} catch(parse_error) {
				throw Exception("XML Parse Error in file "+campaignFileName);
			}

			Game::readSnapshotXML(savecmpn, unitTypeInfo);

			int nRemainingStoryline=story.size();
			if (savecmpn.first_node("save")->first_node("nRemainingStoryline"))	{
				nRemainingStoryline = toInt(savecmpn.first_node("save")->first_node("nRemainingStoryline")->value());
			}
			while (story.size() > nRemainingStoryline) {
				if (startsWith(story.front().work, "CONST")) {
					if (endsWith(story.front().work, "END")) {
						for (vector<StoryLine>::iterator i = objectiveConstraint.begin(); i != objectiveConstraint.end(); i++) {
							if (story.front().isEndConstraintOf(*i)) {
								objectiveConstraint.erase(i);
								break;
							}
						}
					} else {
						objectiveConstraint.push_back(story.front());
					}
				}
				story.erase(story.begin());
			}
			if (!story.empty() && savecmpn.first_node("save")->first_node("lastStoryLineTime")) {
				story.front().time = toInt(savecmpn.first_node("save")->first_node("lastStoryLineTime")->value());
			}
			
			for (int t = 0; t <= this->nTeams; t++) {
				teams[t].init(this, unitTypeInfo, t);
			}

			for (xml_node<> *node = savecmpn.first_node("save")->first_node("diplomacy"); node; node = node->next_sibling("diplomacy")) {
				TeamID t1 = toInt(node->first_attribute("team1")->value());
				TeamID t2 = toInt(node->first_attribute("team2")->value());
				setDiplomacy(t1, t2, toDiplomacyType(node->value()));
			}
		} catch(parse_error &e) {
			throw Exception("XML Parse Error in file "+saveFilename);
		}
	}
}
