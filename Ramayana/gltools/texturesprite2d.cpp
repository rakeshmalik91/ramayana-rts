/*****************************************************************************************************
* Subject                   : 2D/3D Texture/Sprite Loader                                           *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include "graphics.h"

namespace graphics {

	Texture2DAnimated::Texture2DAnimated() : loaded(false), generated(false), nFrame(0) {}
	Texture2DAnimated::Texture2DAnimated(const char* dirname, bool gen, GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT) {
		loaded = generated = false;
		nFrame = 0;
		load(dirname, gen, magFilter, minFilter, wrapS, wrapT);
	}
	void Texture2DAnimated::load(const char* dirname, bool gen, GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT) {
		if (loaded)
			return;
		DIR* dir = opendir(dirname);
		if (dir == NULL)
			throw DirectoryNotFoundException(dirname);
		vector<string> filenames;
		for (dirent *d; (d = readdir(dir)) != NULL;)
		if (getExtension(d->d_name) == "jpg" || getExtension(d->d_name) == "png" || getExtension(d->d_name) == "bmp")
			filenames.push_back((string)dirname + "/" + (string)d->d_name);
		closedir(dir);
		nFrame = filenames.size();
		quicksort(filenames.data(), nFrame);
		frame = new Texture2D[nFrame];
		for (int f = 0; f<nFrame; f++)
			frame[f].load(filenames[f].data(), gen, magFilter, minFilter, wrapS, wrapT);
		loaded = true;
	}
	void Texture2DAnimated::generate(GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT) {
		if (loaded && !generated) {
			for (int f = 0; f<nFrame; f++)
				frame[f].generate(magFilter, minFilter, wrapS, wrapT);
			generated = true;
		}
	}
	void Texture2DAnimated::bind(int frameNumber) {
		frameNumber = frameNumber%nFrame;
		frame[frameNumber].bind();
	}
	void Texture2DAnimated::bind(int frameNumber, GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT) {
		frameNumber = frameNumber%nFrame;
		frame[frameNumber].bind(magFilter, minFilter, wrapS, wrapT);
	}
	int Texture2DAnimated::length() {
		return nFrame;
	}
	void Texture2DAnimated::unload() {
		delete[] frame;
	}
	Texture2DAnimated::~Texture2DAnimated() {
		unload();
	}
}