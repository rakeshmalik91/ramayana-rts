/*****************************************************************************************************
 * Subject                   : 2D/3D Texture/Sprite Loader                                           *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"

#include "graphics.h"

namespace graphics {

	int getMaxTexture2DSize() {
		int maxsize = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);
		return maxsize;
	}


	//------------------------------------------------------------------------------------------------------------------Texture Library

	class TextureLibrary {
		struct TextureLibraryEntry {
			GLuint id;
			int nReferences;
			TextureLibraryEntry() : id(0), nReferences(0) {}
			TextureLibraryEntry(GLuint id) : id(id), nReferences(1) {}
		};
		map<string, TextureLibraryEntry> textures;
	public:
		void add(string path, GLuint id);
		void add(string path);
		void remove(string path);
		GLuint get(string path);
		bool has(string path);
		~TextureLibrary();
	} _textureLibrary;

	void TextureLibrary::add(string path, GLuint id) {
		if (textures.find(path) == textures.end()) {
			textures.insert(pair<string, TextureLibraryEntry>(path, TextureLibraryEntry(id)));
		} else {
			textures[path].nReferences++;
		}
	}
	void TextureLibrary::add(string path) {
		if (textures.find(path) != textures.end()) {
			textures[path].nReferences++;
		}
	}
	void TextureLibrary::remove(string path) {
		if (has(path)) {
			textures[path].nReferences--;
			if (textures[path].nReferences == 0) {
				glDeleteTextures(1, &(textures[path].id));
				textures.erase(path);
			}
		}
	}
	GLuint TextureLibrary::get(string path) {
		if (textures.find(path) == textures.end()) {
			return 0;
		} else {
			return textures[path].id;
		}
	}
	bool TextureLibrary::has(string path) {
		return !textures.empty() && textures.find(path) != textures.end();
	}
	TextureLibrary::~TextureLibrary() {
		for (map<string, TextureLibraryEntry>::iterator i = textures.begin(); i != textures.end(); i++) {
			glDeleteTextures(1, &(i->second.id));
		}
		textures.clear();
	}

	
	//------------------------------------------------------------------------------------------------------------------Texture2D

	//Default Constructor
	Texture2D::Texture2D() {
		data = NULL;
		loaded=generated=false;
	}
	//Copy Constructor
	Texture2D::Texture2D(const Texture2D &t) {
		data = NULL;
		loaded=generated=false;
		if(t.loaded)
			throw TextureCopiedException(path);
	}
	//Assignment operator
	Texture2D& Texture2D::operator=(const Texture2D &t) {
		throw TextureCopiedException(path);
	}
	//Makes texture from opencv iplImage
	void Texture2D::make(IplImage* image_original, string name, bool gen, GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT, int width, int height) {
		if (loaded) {
			return;
		}
		this->path = name;
		this->actualWidth = image_original->width;
		this->actualHeight = image_original->height;
		int maxTexture2DSize = getMaxTexture2DSize();
		width = clampHigh(width, maxTexture2DSize);
		height = clampHigh(height, maxTexture2DSize);
		this->width = pow(2, ceil(log2((width > 0 ? width : image_original->width))));
		this->height = pow(2, ceil(log2((height > 0 ? height : image_original->height))));
		IplImage *image_final;
		nChannels = image_original->nChannels;
		image_final = cvCreateImage(cvSize(this->width, this->height), IPL_DEPTH_8U, nChannels);
		cvResize(image_original, image_final);
		cvFlip(image_final, image_final, 0);
		this->data = new unsigned char[this->height*this->width*nChannels];
		for (int r = 0; r < this->height; r++) {
			for (int c = 0; c < this->width; c++) {
				for (int i = 0; i < nChannels; i++) {
					int dstIndex = (r * this->width + c) * nChannels + i;
					int srcIndex = r * image_final->widthStep + c * image_final->nChannels + i;
					this->data[dstIndex] = ((unsigned char*)image_final->imageData)[srcIndex];
				}
			}
		}
		cvReleaseImage(&image_final);
		loaded=true;
		if (gen) {
			generate(magFilter, minFilter, wrapS, wrapT);
		}
	}
	//Loads image from file and makes texture
	void Texture2D::load(const char* path,  bool gen, GLint magFilter, GLint minFilter,  GLint wrapS, GLint wrapT, int width, int height) {
		if (loaded) {
			return;
		}
		try {
			IplImage* image_original = cvLoadImage(path, CV_LOAD_IMAGE_UNCHANGED);
			if (image_original == NULL) {
				throw FileNotFoundException(path);
			}
			make(image_original, path, gen, magFilter, minFilter, wrapS, wrapT, width, height);
			cvReleaseImage(&image_original);
		} catch (cv::Exception &e) {
			throw Exception("Texture2D::load() : " + e.msg);
		}
	}
	//Constructor, loads from filea and makes texture
	Texture2D::Texture2D(const char* path, bool gen, GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT, int width, int height) {
		loaded=generated=false;
		load(path, gen, magFilter, minFilter, wrapS, wrapT, width, height);
	}
	//Returns color color format as GLenum according to image format
	GLenum Texture2D::getColorFormat() const{
		return (nChannels == 1) ? GL_INTENSITY : (nChannels == 3) ? GL_BGR : GL_BGRA;
	}
	//Returns color component as GLenum according to image channel
	GLenum Texture2D::getColorComponent() const {
		return (nChannels == 1) ? GL_INTENSITY8 : (nChannels == 3) ? GL_RGB8 : GL_RGBA8;
	}
	//generates opengl texture from loaded iplImage::imageData
	void Texture2D::generate(GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT) {
		if (loaded && !generated) {
			if (_textureLibrary.has(path)) {
				_textureLibrary.add(path);
				texture = _textureLibrary.get(path);
			} else {
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
				GLint components = getColorComponent();
				GLenum format = getColorFormat();
				if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST)
					gluBuild2DMipmaps(GL_TEXTURE_2D, components, width, height, format, GL_UNSIGNED_BYTE, data);
				else
					glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				_textureLibrary.add(path, texture);
			}
			if (data != NULL) {
				delete[] data;
			}
			generated=true;
		}
	}
	//binds texture
	void Texture2D::bind() {
		if(generated) {
			glBindTexture(GL_TEXTURE_2D, texture);
		}
	}
	void Texture2D::bind(GLint magFilter, GLint minFilter, GLint wrapS, GLint wrapT) {
		if(generated) {
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		}
	}
	//unbinds texture
	void Texture2D::bindNone() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//Return true if any texture is loaded into the class
	bool Texture2D::isLoaded() {
		return loaded;
	}
	//Returns texture name returned by glGenTextures()
	GLuint Texture2D::getID() {
		return texture;
	}
	//Unload
	void Texture2D::unload() {
		if(loaded) {
			if (generated) {
				_textureLibrary.remove(path);
			} else {
				delete[] data;
			}
			loaded=false;
			generated=false;
		}
	}
	//Destructor
	Texture2D::~Texture2D() {
		unload();
	}
	//return color at x, y point
	Color Texture2D::getColorAt(float x, float y) const {
		if(!loaded)
			return Color();
		int c=x*width, r=y*height;
		int offset = (r*this->width + c)*nChannels;
		unsigned char *pixels = new unsigned char[this->height*this->width*nChannels];
		GLenum format = getColorFormat();
		glBindTexture(GL_TEXTURE_2D, texture);
		glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels);
		Color color;
		switch (nChannels) {
		case 4:
			color.set(float(pixels[offset + 2]) / 256, float(pixels[offset + 1]) / 256, float(pixels[offset]) / 256, float(pixels[offset + 3]) / 256);
			break;
		case 3:
			color.set(float(pixels[offset + 2]) / 256, float(pixels[offset + 1]) / 256, float(pixels[offset]) / 256);
			break;
		case 1:
			color.set(float(pixels[offset]) / 256);
			break;
		}
		delete[] pixels;
		return color;
	}
	int Texture2D::getWidth() const {
		return width;
	}
	int Texture2D::getHeight() const {
		return height;
	}
	int Texture2D::getActualWidth() const {
		return actualWidth;
	}
	int Texture2D::getActualHeight() const {
		return actualHeight;
	}
};
