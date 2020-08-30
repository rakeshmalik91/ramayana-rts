/*****************************************************************************************************
* Subject                   : OpenGL tools                                                          *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include <cstdio>

#include "string.h"
#include "glprint.h"

namespace gltools {

	glPrint::glPrint(int x, int y, void* font) {
		this->startX = this->x = x;
		this->y = y;
		this->font = font;
		alignment = LEFT_ALIGNMENT;
	}
	glPrint::glPrint(int x, int y, int w, int h, void* font) {
		this->startX = this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->font = font;
		alignment = CENTRE_ALIGNMENT;
	}
	glPrint& glPrint::operator<<(string s) {
		int width = 0, height = 0;
		switch (alignment) {
		case LEFT_ALIGNMENT:
			glRasterPos2i(x, y);
			for (int i = 0; i < s.length(); i++) {
				if (s[i] == '\n') {
					y += glutBitmapHeight(font);
					width = 0;
					x = startX;
					glRasterPos2i(startX, y);
				} else {
					width += glutBitmapWidth(font, s[i]);
					glutBitmapCharacter(font, s[i]);
				}
			}
			break;
		case CENTRE_ALIGNMENT:
			width = glPrintWidth(font, s);
			height = glutBitmapHeight(font);
			glRasterPos2i(x + (w - width) / 2, y + (h + height) / 2);
			for (int i = 0; s[i] != '\0'; i++) {
				if (s[i] != '\n') {
					glutBitmapCharacter(font, s[i]);
				}
			}
			break;
		}
		x += width;
		return *this;
	}
	glPrint& glPrint::operator<<(const char* s) {
		return operator<<(string(s));
	}
	glPrint& glPrint::operator<<(int i) {
		return operator<<(toString(i));
	}
	glPrint& glPrint::operator<<(float f) {
		return operator<<(toString(f));
	}
	glPrint& glPrint::operator<<(bool b) {
		return operator<<(toString(b));
	}
	glPrint& glPrint::operator<<(Point2D p) {
		return (*this) << "(" << (toString(p.x)) << "," << (toString(p.y)) << ")";
	}
	glPrint& glPrint::operator<<(Point3D p) {
		return (*this) << "(" << (toString(p.x)) << "," << (toString(p.y)) << "," << (toString(p.z)) << ")";
	}
	glPrint& glPrint::operator<<(glPrintfModiier mod) {
		switch (mod.type) {
		case glPrintfModiier::DOWN:
			y += mod.value;
			x = startX;
			break;
		case glPrintfModiier::RIGHT:
			x += mod.value;
			break;
		}
		return *this;
	}
	glPrint& glPrint::operator<<(TextAlignment alignment) {
		this->alignment = alignment;
		return *this;
	}
	glPrintfModiier glPrint::down(int value) {
		return glPrintfModiier(glPrintfModiier::DOWN, value);
	}

	int glPrintWidth(void *font, string s) {
		int width = 0;
		for (int i = 0; s[i] != '\0'; i++) {
			width += glutBitmapWidth(font, s[i]);
		}
		return width;
	}
	int glPrintHeight(void *font, string s) {
		int height = glutBitmapHeight(font);
		for (int i = 0; s[i] != '\0'; i++) {
			if (s[i] == '\n')
				height += glutBitmapHeight(font);
		}
		return height;
	}
}
