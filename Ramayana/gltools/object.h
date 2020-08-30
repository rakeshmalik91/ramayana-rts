#ifndef __OBJECT_H
#define __OBJECT_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <cmath>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <dirent.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include "math.h"
#include "array.h"

using namespace std;
using namespace math;

namespace graphics {

	struct Material;
	class MaterialLibrary;

	class WaveFrontObj {
		bool loaded, compiled;
		string fname, mtlLibName;
		MaterialLibrary* mtlLib;
		struct Group {
			GLuint vbName;
			int nVert;
			struct Vertex {
				Point3D v, vn;
				Point2D vt;
				Point4D tangent;
				Vertex() {}
				Vertex(Point3D v, Point3D vn, Point2D vt) : v(v), vn(vn), vt(vt) {}
			};
			vector<WaveFrontObj::Group::Vertex> vert;
		};
		struct Triangle {
			WaveFrontObj::Group::Vertex v[3];
		};
		vector<WaveFrontObj::Group> group;
		float radiusX, radiusY, radiusZ, radius;
		float xMax, xMin, yMax, yMin, zMax, zMin, xMid, yMid, zMid;
		bool ownMaterialLibrary;
		Point4D lightpos;
		void readOBJ();
		void readMTL();
		void buildVBO();
		void copyStatFrom(WaveFrontObj&);
	public:
		WaveFrontObj();
		void render(Color varColor = COLOR_BLACK, Color tint = Color(1.0, 1.0, 1.0, 1.0));
		void load(string fname, bool toBeCompiled = true, MaterialLibrary *mtlLib = NULL);
		void write(ostream&);
		void read(istream&, MaterialLibrary *mtlLib = NULL, string texturePath = ".");
		void compile();
		void info();
		float getRadius();
		float getRadiusAcrossXPlane();
		float getRadiusAcrossYPlane();
		float getRadiusAcrossZPlane();
		float getXMax();
		float getXMid();
		float getXMin();
		float getYMax();
		float getYMid();
		float getYMin();
		float getZMax();
		float getZMid();
		float getZMin();
		~WaveFrontObj();
		friend ostream& operator<<(ostream&, WaveFrontObj&);
		friend int triangle_comparator(const WaveFrontObj::Triangle&, const WaveFrontObj::Triangle&, void*);
		friend class WaveFrontObjSequence;
	};

	class WaveFrontObjSequence {
		bool loaded, compiled;
		string path;
		vector<WaveFrontObj> frame;
	public:
		WaveFrontObjSequence();
		WaveFrontObj& getFrame(int);
		void load(string path, bool toBeCompiled = true, bool loadAnimation = true);
		void write(ostream&);
		void read(istream&, string texturePath, bool loadAnimation);
		void compile();
		void render(int frame_number, Color varColor = COLOR_BLACK, Color tint = Color(1.0, 1.0, 1.0, 1.0));
		int length();
		~WaveFrontObjSequence();
	};

	class ColladaDAE {
		bool loaded, compiled;
		string path;
		vector<Texture2D*> images;
		void buildVBO();
	public:
		ColladaDAE();
		void load(string path, bool toBeCompiled = true, bool loadAnimation = true);
		void compile();
		void render(int frame_number, Color varColor = COLOR_BLACK);
		~ColladaDAE();
	};

	class Frustum {
		Plane m_planes[6];
		Plane extractPlane(float, float, float, float);
	public:
		Frustum() {}
		enum PlaneDir{ PLANE_LEFT = 0, PLANE_RIGHT, PLANE_TOP, PLANE_BOTTOM, PLANE_FAR, PLANE_NEAR };
		void updateFrustum();
		bool sphereInFrustum(Point3D, float) const;
		bool pointInFrustum(Point3D) const;
		Plane getPlane(PlaneDir d) { return m_planes[d]; }
	};
}

#endif