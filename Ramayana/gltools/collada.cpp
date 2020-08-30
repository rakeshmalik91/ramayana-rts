/*****************************************************************************************************
 * Subject                   : COLLADA(*.DAE) Loader                                                 *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/


// INCOMPLETE


#include "stdafx.h"

#include "graphics.h"

using namespace rapidxml;

namespace graphics {

	ColladaDAE::ColladaDAE() {

	}
	void ColladaDAE::load(string path,bool toBeCompiled,bool loadAnimation) {
		this->path=path;

		string text = readTextFile(path.data());
		xml_document<char> doc;
		doc.parse<0>((char*)text.data());
		try {
			for(xml_node<> *nodeImage = doc.first_node("COLLADA")->first_node("library_images")->first_node("image"); nodeImage; nodeImage = nodeImage->next_sibling("image")) {
				string img_path=getFilePath(path)+"/"+nodeImage->first_node("init_from")->value();
				images.push_back(new Texture2D());
				images.front()->load(img_path.data(), false);
			}
			
		} catch ( parse_error ) {
			throw Exception("XML Parse Error in file " + path);
		}
		loaded=true;

		if(toBeCompiled)
			compile();
	}
	void ColladaDAE::buildVBO() {

	}
	void ColladaDAE::compile() {
		if(loaded && !compiled) {
			//generate textures
			for(int i = 0; i<images.size(); i++)
				images[i]->generate(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT);

			//build vertex buffer
			buildVBO();

			compiled=true;
		}
	}
	ColladaDAE::~ColladaDAE() {

	}
}
