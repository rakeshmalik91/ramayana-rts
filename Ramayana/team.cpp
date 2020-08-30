#include "stdafx.h"

#include "team.h"
#include "game.h"

namespace ramayana {
	void Team::init(Game* game, UnitTypeInfo *unitTypeInfo, int index) {
		this->game=game;
		this->unitTypeInfo=unitTypeInfo;
		teamIndex=index;
		
		for(int u=0; u<game->getNumberOfUnits(); u++) 
			if(game->unit[u].isAlive() && game->unit[u].team==teamIndex) {
				population+=game->unit[u].getTypeInfo().population;
				maxPopulation+=game->unit[u].getTypeInfo().maxPopulationIncrease;
			}

		visible=allocate<bool>(game->getWidth(), game->getHeight());
		explored=allocate<bool>(game->getWidth(), game->getHeight());
		if(index==0) {
			for(int r=0; r<game->getHeight(); r++)
				for(int c=0; c<game->getWidth(); c++)
					visible[r][c]=explored[r][c]=true;
		} else {
			for(int r=0; r<game->getHeight(); r++)
				for(int c=0; c<game->getWidth(); c++)
					explored[r][c]=false;
			update();
		}
	}
	void Team::calculateStatistics() {
		nWorker=nGatherer.food=nGatherer.wood=nGatherer.stone=nGatherer.metal=nBuilder=nIdleWorker=0;
		nHero=nMilitary=0;
		nBuilding=0;
		for(int i=0; i<MAX_OBJECT_TYPE; i++)
			nUnits[i]=0;

		for(int u=0; u<game->getNumberOfUnits(); u++) {
			if(game->unit[u].isAlive() && game->unit[u].team==teamIndex) {
				if(game->unit[u].getTypeInfo().isWorker) {
					nWorker++;
					if(game->unit[u].isIdle())							nIdleWorker++;	
					else if(game->unit[u].isFoodGatherer())				nGatherer.food++;	
					else if(game->unit[u].isWoodGatherer())				nGatherer.wood++;	
					else if(game->unit[u].isStoneGatherer())			nGatherer.stone++;
					else if(game->unit[u].isMetalGatherer())			nGatherer.metal++;
					else if(game->unit[u].isBuilder())					nBuilder++;		
				}
				if(game->unit[u].getTypeInfo().isHeroic)				nHero++;
				if(game->unit[u].getTypeInfo().isMilitaryUnit())		nMilitary++;
				if(game->unit[u].getTypeInfo().isConstruction())	nBuilding++;
				nUnits[game->unit[u].type]++;
			}
		}
	}
	void Team::checkStatus() {
		if(nBuilding<=0 && population<=0)
			defeated=true;
	}
	void Team::calculategetLOS() {
		if(revealAll) {
			exploreAll=true;
			setAll(visible, game->getWidth(), game->getHeight(), true);
		} else {
			setAll(visible, game->getWidth(), game->getHeight(), false);
			for(int u=0; u<game->getNumberOfUnits(); u++)
				if(game->unit[u].isAlive() && game->diplomacy(game->unit[u].team, teamIndex)==DIPLOMACY_ALLY)
					fillCircle(visible, game->getWidth(), game->getHeight(), true, game->unit[u].position(), game->unit[u].getLOS());
		}
		if(exploreAll) {
			setAll(explored, game->getWidth(), game->getHeight(), true);
		} else {
			for(int r=0; r<game->getHeight(); r++)
				for(int c=0; c<game->getWidth(); c++)
					explored[r][c]|=visible[r][c];
		}
	}
	void Team::update() {
		calculategetLOS();
		calculateStatistics();
		checkStatus();
	}
	Team::~Team() {
		if(visible)		deallocate(visible, game->getWidth(), game->getHeight());
		if(explored)	deallocate(explored, game->getWidth(), game->getHeight());
	}
	
	Resource<> Team::getUnitCost(UnitType unitType) {
		Resource<> cost=unitTypeInfo[unitType].cost;
		float inc=0.1;
		cost.food+=inc*cost.food*nUnits[unitType];
		cost.wood+=inc*cost.wood*nUnits[unitType];
		cost.stone+=inc*cost.stone*nUnits[unitType];
		cost.metal+=inc*cost.metal*nUnits[unitType];
		return cost;
	}

	void Team::setRevealAll(bool value) {
		revealAll = value;
	}
	void Team::setExploreAll(bool value) {
		exploreAll=value;
	}
	
	xml_node<char>* Team::toXMLNode(xml_document<char> &doc) const {
		xml_node<char> *nodeTeam=doc.allocate_node(node_element, "team");
		nodeTeam->append_attribute(doc.allocate_attribute("teamIndex", doc.allocate_string(toString(teamIndex).data())));
		nodeTeam->append_node(doc.allocate_node(node_element, "name", (name).data()));
		nodeTeam->append_node(doc.allocate_node(node_element, "color", doc.allocate_string(toString(color).data())));
		nodeTeam->append_node(doc.allocate_node(node_element, "faction", doc.allocate_string(toString(faction).data())));
		nodeTeam->append_node(doc.allocate_node(node_element, "teamGroup", doc.allocate_string(toString(teamGroup).data())));
		nodeTeam->append_node(doc.allocate_node(node_element, "population", doc.allocate_string(toString(population).data())));
		nodeTeam->append_node(doc.allocate_node(node_element, "maxPopulation", doc.allocate_string(toString(maxPopulation).data())));
		if(hasAI) nodeTeam->append_node(doc.allocate_node(node_element, "hasAI"));
		nodeTeam->append_node(doc.allocate_node(node_element, "resource"));
		nodeTeam->first_node("resource")->append_node(doc.allocate_node(node_element, "food", doc.allocate_string(toString(resource.food).data())));
		nodeTeam->first_node("resource")->append_node(doc.allocate_node(node_element, "wood", doc.allocate_string(toString(resource.wood).data())));
		nodeTeam->first_node("resource")->append_node(doc.allocate_node(node_element, "stone", doc.allocate_string(toString(resource.stone).data())));
		nodeTeam->first_node("resource")->append_node(doc.allocate_node(node_element, "metal", doc.allocate_string(toString(resource.metal).data())));
		nodeTeam->append_node(doc.allocate_node(node_element, "exploreAll", doc.allocate_string(toString(exploreAll).data())));
		nodeTeam->append_node(doc.allocate_node(node_element, "revealAll", doc.allocate_string(toString(revealAll).data())));
		char* str_explored = doc.allocate_string("", game->getHeight()*game->getWidth());
		char* str_visible = doc.allocate_string("", game->getHeight()*game->getWidth());
		for(int r = 0; r < game->getHeight(); r++) {
			for(int c = 0; c < game->getWidth(); c++) {
				str_explored[game->getWidth()*r+c] = explored[r][c]?'1':'0';
				str_visible[game->getWidth()*r+c] = visible[r][c]?'1':'0';
			}
		}
		if(!exploreAll)		nodeTeam->append_node(doc.allocate_node(node_element, "explored", str_explored));
		if(!revealAll)		nodeTeam->append_node(doc.allocate_node(node_element, "visible", str_visible));
		return nodeTeam;
	}
	void Team::loadFromXMLNode(xml_node<char> *node, UnitTypeInfo* unitTypeInfo, Game* game) {
		this->unitTypeInfo = unitTypeInfo;
		this->game = game;
		
		visible=allocate<bool>(game->getWidth(), game->getHeight());
		explored=allocate<bool>(game->getWidth(), game->getHeight());

		if(node->first_attribute("teamIndex"))		teamIndex = toInt(node->first_attribute("teamIndex")->value());
		if(node->first_node("name"))				name = node->first_node("name")->value();
		if(node->first_node("color"))				color = toInt(node->first_node("color")->value());
		if(node->first_node("faction"))				faction = toInt(node->first_node("faction")->value());
		if(node->first_node("teamGroup"))			teamGroup = toInt(node->first_node("teamGroup")->value());
		if(node->first_node("hasAI"))				hasAI = true;
		//if(node->first_node("population"))			population = toInt(node->first_node("population")->value());
		//if(node->first_node("maxPopulation"))		population = toInt(node->first_node("maxPopulation")->value());
		if(node->first_node("teamGroup"))			teamGroup = toInt(node->first_node("teamGroup")->value());
		if(node->first_node("resource")) {
			if(node->first_node("resource")->first_node("food"))	resource.food = toInt(node->first_node("resource")->first_node("food")->value());
			if(node->first_node("resource")->first_node("wood"))	resource.wood = toInt(node->first_node("resource")->first_node("wood")->value());
			if(node->first_node("resource")->first_node("stone"))	resource.stone = toInt(node->first_node("resource")->first_node("stone")->value());
			if(node->first_node("resource")->first_node("metal"))	resource.metal = toInt(node->first_node("resource")->first_node("metal")->value());
		}
		if(node->first_node("exploreAll"))			exploreAll = toBool(node->first_node("exploreAll")->value());
		if(node->first_node("revealAll"))			revealAll = toBool(node->first_node("revealAll")->value());
		if(node->first_node("explored")) {
			string str_explored = node->first_node("explored")->value();
			for(int r = 0; r < game->getHeight(); r++)
				for(int c = 0; c < game->getWidth(); c++)
					explored[r][c] = str_explored[r*game->getWidth()+c]=='0'?false:true;
		}
		if(node->first_node("visible")) {
			string str_visible = node->first_node("visible")->value();
			for(int r = 0; r < game->getHeight(); r++)
				for(int c = 0; c < game->getWidth(); c++)
					visible[r][c] = str_visible[r*game->getWidth()+c]=='0'?false:true;
		}
	}
}