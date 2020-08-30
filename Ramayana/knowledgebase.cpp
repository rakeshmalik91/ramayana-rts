#include "stdafx.h"

#include "common.h"
#include "unit.h"
#include "ai.h"
#include "game.h"
#include "team.h"

namespace ramayana {

	vector<TeamKnowledgeBase::BuildableUnit> TeamKnowledgeBase::getUnitsFromXML(xml_node<char> *node, const char* type) {
		vector<BuildableUnit> vec;
		for(xml_node<> *nodeU=node->first_node(type); nodeU; nodeU=nodeU->next_sibling(type)) {
			BuildableUnit unit;
			unit.type=toInt(nodeU->first_node("type")->value());
			for(xml_node<> *nodeB=nodeU->first_node("builderType"); nodeB; nodeB=nodeB->next_sibling("builderType"))
				unit.builderType.push_back(toInt(nodeB->value()));
			vec.push_back(unit);
		}
		return vec;
	}
	TeamKnowledgeBase::BuildableUnit TeamKnowledgeBase::getUnitFromXML(xml_node<char> *node, const char* type) {
		BuildableUnit unit;
		unit.type=toInt(node->first_node(type)->first_node("type")->value());
		for(xml_node<> *nodeB=node->first_node(type)->first_node("builderType"); nodeB; nodeB=nodeB->next_sibling("builderType"))
			unit.builderType.push_back(toInt(nodeB->value()));
		return unit;
	}
	void TeamKnowledgeBase::loadKnowledge(string filename) {
		xml_document<char> doc;
		string filepath="kb/"+filename;
		string text=readTextFile(filepath.data());
		doc.parse<0>((char*)text.data());
		try {
			xml_node<> *buildableUnitNode=doc.first_node("knowledge")->first_node("buildableUnit");

			buildableUnit.farm=getUnitFromXML(buildableUnitNode, "farm");
			buildableUnit.lumbercamp=getUnitFromXML(buildableUnitNode, "lumbercamp");
			buildableUnit.mine=getUnitFromXML(buildableUnitNode, "mine");
			buildableUnit.barracks=getUnitFromXML(buildableUnitNode, "barracks");
			buildableUnit.archery=getUnitFromXML(buildableUnitNode, "archery");
			buildableUnit.beastiary=getUnitFromXML(buildableUnitNode, "beastiary");
			buildableUnit.citadel=getUnitFromXML(buildableUnitNode, "citadel");
			buildableUnit.outpost=getUnitFromXML(buildableUnitNode, "outpost");
			buildableUnit.worker=getUnitFromXML(buildableUnitNode, "worker");
			buildableUnit.lightMelee=getUnitsFromXML(buildableUnitNode, "lightMelee");
			buildableUnit.heavyMelee=getUnitsFromXML(buildableUnitNode, "heavyMelee");
			buildableUnit.lightRanged=getUnitsFromXML(buildableUnitNode, "lightRanged");
			buildableUnit.heavyRanged=getUnitsFromXML(buildableUnitNode, "heavyRanged");
			buildableUnit.lightBeast=getUnitsFromXML(buildableUnitNode, "lightBeast");
			buildableUnit.heavyBeast=getUnitsFromXML(buildableUnitNode, "heavyBeast");

			for(xml_node<> *node=doc.first_node("knowledge")->first_node("targetStat"); node; node=node->next_sibling("targetStat")) {
				targetStat.push_back(TargetStat(toInt(node->first_attribute("timeStamp")->value())));

				targetStat.back().resource.food=toInt(node->first_node("resource")->first_node("food")->value());
				targetStat.back().resource.wood=toInt(node->first_node("resource")->first_node("wood")->value());
				targetStat.back().resource.stone=toInt(node->first_node("resource")->first_node("stone")->value());
				targetStat.back().resource.metal=toInt(node->first_node("resource")->first_node("metal")->value());

				targetStat.back().nWorker=toInt(node->first_node("nWorker")->value());
				targetStat.back().gatherer.food=toInt(node->first_node("gatherer")->first_node("food")->value());
				targetStat.back().gatherer.wood=toInt(node->first_node("gatherer")->first_node("wood")->value());
				targetStat.back().gatherer.stone=toInt(node->first_node("gatherer")->first_node("stone")->value());
				targetStat.back().gatherer.metal=toInt(node->first_node("gatherer")->first_node("metal")->value());

				if(node->first_node("nScout")) targetStat.back().nScout=toInt(node->first_node("nScout")->value());
				if(node->first_node("nBarracks")) targetStat.back().nBarracks=toInt(node->first_node("nBarracks")->value());
				if(node->first_node("nLightMelee")) targetStat.back().nLightMelee=toInt(node->first_node("nLightMelee")->value());
				if(node->first_node("nHeavyMelee")) targetStat.back().nHeavyMelee=toInt(node->first_node("nHeavyMelee")->value());
				if(node->first_node("nLightRanged")) targetStat.back().nLightRanged=toInt(node->first_node("nLightRanged")->value());
				if(node->first_node("nHeavyRanged")) targetStat.back().nHeavyRanged=toInt(node->first_node("nHeavyRanged")->value());
				if(node->first_node("nLightBeast")) targetStat.back().nLightBeast=toInt(node->first_node("nLightBeast")->value());
				if(node->first_node("nHeavyBeast")) targetStat.back().nHeavyBeast=toInt(node->first_node("nHeavyBeast")->value());
			}
		} catch(parse_error &e) {
			throw Exception("XML Parse Error in file "+filepath);
		}
	}
	
	void TeamKnowledgeBase::saveKnowledge(string fname) {
		string filepath="kb/"+fname;
		xml_document<char> doc;

		doc.append_node(doc.allocate_node(node_element, "knowledge"));

		xml_node<char> *nodeBuildableUnit=doc.allocate_node(node_element, "buildableUnit");
		doc.first_node("knowledge")->append_node(nodeBuildableUnit);
		Tuple<string, BuildableUnit*> bu[]={
			Tuple<string, BuildableUnit*>("farm", &buildableUnit.farm), 
			Tuple<string, BuildableUnit*>("lumbercamp", &buildableUnit.lumbercamp), 
			Tuple<string, BuildableUnit*>("mine", &buildableUnit.mine), 
			Tuple<string, BuildableUnit*>("citadel", &buildableUnit.citadel), 
			Tuple<string, BuildableUnit*>("barracks", &buildableUnit.barracks), 
			Tuple<string, BuildableUnit*>("outpost", &buildableUnit.outpost), 
			Tuple<string, BuildableUnit*>("worker", &buildableUnit.worker)
		};
		for(int i=0; i<arrayLength(bu); i++) {
			xml_node<char> *nodeUnit=doc.allocate_node(node_element, bu[i].e1.data());
			nodeBuildableUnit->append_node(nodeUnit);
			nodeUnit->append_node(doc.allocate_node(node_element, "type", doc.allocate_string(toString(bu[i].e2->type).data())));
			for(int b=0; b<bu[i].e2->builderType.size(); b++) {
				nodeUnit->append_node(doc.allocate_node(node_element, "builderType", doc.allocate_string(toString(bu[i].e2->builderType[b]).data())));
			}
		}
		Tuple<string, vector<BuildableUnit>*> vbu[]={
			Tuple<string, vector<BuildableUnit>*>("lightMelee", &buildableUnit.lightMelee), 
			Tuple<string, vector<BuildableUnit>*>("heavyMelee", &buildableUnit.heavyMelee), 
			Tuple<string, vector<BuildableUnit>*>("lightRanged", &buildableUnit.lightRanged), 
			Tuple<string, vector<BuildableUnit>*>("heavyRanged", &buildableUnit.heavyRanged), 
			Tuple<string, vector<BuildableUnit>*>("lightBeast", &buildableUnit.lightBeast),  
			Tuple<string, vector<BuildableUnit>*>("heavyBeast", &buildableUnit.heavyBeast)
		};
		for(int i=0; i<arrayLength(vbu); i++) {
			for(int j=0; j<vbu[i].e2->size(); j++) {
				xml_node<char> *nodeUnit=doc.allocate_node(node_element, vbu[i].e1.data());
				nodeBuildableUnit->append_node(nodeUnit);
				nodeUnit->append_node(doc.allocate_node(node_element, "type", doc.allocate_string(toString(vbu[i].e2->at(j).type).data())));
				for(int b=0; b<vbu[i].e2->at(j).builderType.size(); b++) {
					nodeUnit->append_node(doc.allocate_node(node_element, "builderType", doc.allocate_string(toString(vbu[i].e2->at(j).builderType[b]).data())));
				}
			}
		}

		for(int t=0; t<targetStat.size(); t++) {
			xml_node<char> *nodeTargetStat=doc.allocate_node(node_element, "targetStat");
			doc.first_node("knowledge")->append_node(nodeTargetStat);

			nodeTargetStat->append_attribute(doc.allocate_attribute("timeStamp", doc.allocate_string(toString(targetStat[t].timeStamp).data())));
			
			xml_node<char> *nodeResource=doc.allocate_node(node_element, "resource");
			nodeTargetStat->append_node(nodeResource);
			nodeResource->append_node(doc.allocate_node(node_element, "food", doc.allocate_string(toString(targetStat[t].resource.food).data())));
			nodeResource->append_node(doc.allocate_node(node_element, "wood", doc.allocate_string(toString(targetStat[t].resource.wood).data())));
			nodeResource->append_node(doc.allocate_node(node_element, "stone", doc.allocate_string(toString(targetStat[t].resource.stone).data())));
			nodeResource->append_node(doc.allocate_node(node_element, "metal", doc.allocate_string(toString(targetStat[t].resource.metal).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nWorker", doc.allocate_string(toString(targetStat[t].nWorker).data())));
			xml_node<char> *nodeGatherer=doc.allocate_node(node_element, "resource");
			nodeTargetStat->append_node(nodeGatherer);
			nodeGatherer->append_node(doc.allocate_node(node_element, "food", doc.allocate_string(toString(targetStat[t].gatherer.food).data())));
			nodeGatherer->append_node(doc.allocate_node(node_element, "wood", doc.allocate_string(toString(targetStat[t].gatherer.wood).data())));
			nodeGatherer->append_node(doc.allocate_node(node_element, "stone", doc.allocate_string(toString(targetStat[t].gatherer.stone).data())));
			nodeGatherer->append_node(doc.allocate_node(node_element, "metal", doc.allocate_string(toString(targetStat[t].gatherer.metal).data())));

			nodeTargetStat->append_node(doc.allocate_node(node_element, "nScout", doc.allocate_string(toString(targetStat[t].nScout).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nBarracks", doc.allocate_string(toString(targetStat[t].nBarracks).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nLightMelee", doc.allocate_string(toString(targetStat[t].nLightMelee).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nHeavyMelee", doc.allocate_string(toString(targetStat[t].nHeavyMelee).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nLightRanged", doc.allocate_string(toString(targetStat[t].nLightRanged).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nHeavyRanged", doc.allocate_string(toString(targetStat[t].nHeavyRanged).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nLightBeast", doc.allocate_string(toString(targetStat[t].nLightBeast).data())));
			nodeTargetStat->append_node(doc.allocate_node(node_element, "nHeavyBeast", doc.allocate_string(toString(targetStat[t].nHeavyBeast).data())));
		}
		
		try {
			xml_node<> *buildableUnitNode=doc.first_node("knowledge")->first_node("buildableUnit");
		} catch(parse_error &e) {
			throw Exception("XML Parse Error in file "+filepath);
		}

		string text;
		print(back_inserter(text), doc, 0);
		ofstream file(filepath.data());
		file<<text;
		file.close();
	}

	TeamKnowledgeBase::TargetStat& TeamKnowledgeBase::getTargetStat(unsigned int time) {
		int index;
		for(index=0; index<targetStat.size() && targetStat[index].timeStamp<time; index++);
		return targetStat[index];
	}
	void TeamKnowledgeBase::addTargetStat(unsigned int time, Team &team) {
		TargetStat stat(time);
		stat.nWorker=team.nWorker;
		stat.gatherer=team.nGatherer;
		stat.resource=team.resource;

		targetStat.push_back(stat);
	}

	TeamKnowledgeBase::BuildableUnitType& TeamKnowledgeBase::getBuildableUnitType() {
		return buildableUnit;
	}
		
	void TeamKnowledgeBase::forget() {
		targetStat.clear();
	}
}
