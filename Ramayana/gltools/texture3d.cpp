/*****************************************************************************************************
* Subject                   : 3D Texture                                                            *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include "graphics.h"

namespace graphics {

	int getMaxTexture3DSize() {
		int maxsize = 0;
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxsize);
		return maxsize;
	}

	//default constructor
	Texture3D::Texture3D()
	 :loaded(false), generated(false), data(NULL) {
	}

	//create 3d texture from all images in a directory
	void Texture3D::load(string dir, int width, int height, int depth, bool gen, GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT, GLint wrapR) {
		if (loaded) {
			return;
		}
		this->dir = dir;
		int maxTexture3DSize = getMaxTexture3DSize();
		this->width = width = clampHigh(width, maxTexture3DSize);
		this->height = height = clampHigh(height, maxTexture3DSize);
		this->depth = depth = clampHigh(depth, maxTexture3DSize);
		int nChannels = 3;
		this->data = new unsigned char[width * height * depth * nChannels];
		char path[1000];
		for (int d = 0; d<depth; d++) {
			sprintf(path, "%s/%d.jpg", dir.data(), d);
			IplImage* image_original = cvLoadImage(path, CV_LOAD_IMAGE_UNCHANGED);
			if (image_original != NULL) {
				IplImage* image_final = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, nChannels);
				cvResize(image_original, image_final);
				cvFlip(image_final, image_final, 0);
				for (int r = 0; r < image_final->height; r++) {
					for (int c = 0; c < image_final->width; c++) {
						for (int i = 0; i < image_final->nChannels; i++) {
							int dstIndex = ((d * height + r) * width + c) * nChannels + i;
							int srcIndex = r * image_final->widthStep + c * image_final->nChannels + i;
							this->data[dstIndex] = ((unsigned char*)image_final->imageData)[srcIndex];
						}
					}
				}
				cvReleaseImage(&image_final);
				cvReleaseImage(&image_original);
			} else {
				//memset(data + d * width * height * nChannels, 0x00u, width * height * nChannels);
				throw Exception("Texture3D::load() " + string(path) + " image load error.");
			}
		}
		loaded = true;
		if (gen) {
			generate(magFilter, minFilter, wrapS, wrapT, wrapR);
		}
	}

	//generate opengl texture from data
	void Texture3D::generate(GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT, GLint wrapR) {
		if (loaded && !generated) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_3D, texture);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapR);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, width, height, depth, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
			delete[] data;
			generated = true;
		}
	}

	//bind texture to gl context
	void Texture3D::bind() {
		if (generated) {
			glBindTexture(GL_TEXTURE_3D, texture);
		}
	}

	//bind texture to gl context
	void Texture3D::bind(GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT, GLint wrapR) {
		if (generated) {
			glBindTexture(GL_TEXTURE_3D, texture);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapR);
		}
	}

	//unbind texture from gl context
	void Texture3D::bindNone() {
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	//Unload
	void Texture3D::unload() {
		if (loaded) {
			if (generated) {
				glDeleteTextures(1, &texture);
			} else {
				delete[] data;
			}
			loaded = false;
		}
	}

	//destructor
	Texture3D::~Texture3D() {
		unload();
	}
}