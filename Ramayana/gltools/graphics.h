#ifndef __GRAPHICS_H
#define __GRAPHICS_H

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
	vector<Point2Di> lineBresenham(Point2Di, Point2Di);
	void fillCircle(bool**, int, int, bool, Point2Di, int);
	template<class DT> void fillCircle(DT **matrix, int w, int h, DT value, Point2Di centre, int radius) {
		int d = 2 * radius + 1;
		bool **circle = allocate<bool>(d, d);
		setAll(circle, d, d, false);

		fillCircle(circle, d, d, true, Point2Di(radius, radius), radius);

		for (int r = 0; r<d; r++)
		for (int c = 0; c<d; c++)
		if (circle[r][c]) {
			Point2Di p(centre.x - radius + r, centre.y - radius + c);
			if (p.in(0, 0, w - 1, h - 1))
				matrix[p.y][p.x] += value;
		}

		deallocate(circle, d, d);
	}

	typedef Point2D TexCoord2D;
	typedef Point3D TexCoord, TexCoord3D;

	class Color : public Point4D {
	public:
		Color(float v = 0.0);
		Color(float r, float g, float b, float a = 1.0);
		Color(const Color&);
		void set(float v = 0.0);
		void set(float r, float g, float b, float a = 1.0);
		float r() const;
		float g() const;
		float b() const;
		float a() const;
		Point3D rgb() const;
		Point4D rgba() const;
		void r(float);
		void g(float);
		void b(float);
		void a(float);
		Color& operator=(const Color&);
		bool operator==(const Color&);
	};

	static const Color COLOR_WHITE(1.0f, 1.0f, 1.0f);
	static const Color COLOR_GRAY75(0.75f, 0.75f, 0.75f);
	static const Color COLOR_GRAY50(0.5f, 0.5f, 0.5f);
	static const Color COLOR_GRAY25(0.25f, 0.25f, 0.25f);
	static const Color COLOR_BLACK(0.0f, 0.0f, 0.0f);

	static const Color COLOR_RED(1.0f, 0.0f, 0.0f);
	static const Color COLOR_GREEN(0.0f, 1.0f, 0.0f);
	static const Color COLOR_BLUE(0.0f, 0.0f, 1.0f);

	static const Color COLOR_YELLOW(1.0f, 1.0f, 0.0f);
	static const Color COLOR_CYAN(0.0f, 1.0f, 1.0f);
	static const Color COLOR_MAGENTA(1.0f, 0.0f, 1.0f);

	static const Color COLOR_PURPLE(1.0f, 0.0f, 0.5f);
	static const Color COLOR_BROWN(0.5f, 0.0f, 0.0f);
	static const Color COLOR_PINK(1.0f, 0.5f, 1.0f);
	static const Color COLOR_TURQUOISE(0.00f, 0.60f, 0.90f);
	static const Color COLOR_SCARLET(0.90f, 0.10f, 0.10f);
	static const Color COLOR_SAP_GREEN(0.15f, 0.70f, 0.30f);
	static const Color COLOR_LIGHT_TURQUOISE(0.60f, 0.85f, 0.90f);
	static const Color COLOR_ROSE(1.00f, 0.70f, 0.80f);
	static const Color COLOR_VIOLET(0.60f, 0.30f, 0.65f);
	static const Color COLOR_ORANGE(1.00f, 0.50f, 0.15f);
	static const Color COLOR_LIME(0.70f, 0.90f, 0.10f);
	static const Color COLOR_GOLD(1.00f, 0.78f, 0.05f);
	static const Color COLOR_LIGHT_YELLOW(0.93f, 0.89f, 0.69f);
	static const Color COLOR_VIRIDIAN(0.25f, 0.50f, 0.50f);
	static const Color COLOR_DARK_VIRIDIAN(0.00f, 0.25f, 0.25f);
	static const Color COLOR_DARK_GREY_GREEN(0.25f, 0.50f, 0.25f);
	static const Color COLOR_LIGHT_GREY_GREEN(0.50f, 1.00f, 0.50f);

	struct Quad {
		float x1, y1, x2, y2;
		Quad(float x1 = 0.0, float y1 = 0.0, float x2 = 0.0, float y2 = 0.0);
		float area();
		void clear();
	};

	struct Box {
		float xMin, xMax, yMin, yMax, zMin, zMax;
		Box(float xMin = 0.0, float yMin = 0.0, float zMin = 0.0, float xMax = 0.0, float yMax = 0.0, float zMax = 0.0);
		float volume();
		bool intersectRay(Line3D);
		bool intersect(Line3D);
	};

	struct Sphere {
		float radius;
		Point3D centre;
		Sphere(Point3D centre, float radius) : radius(radius), centre(centre) {}
		bool intersect(Point3D);
		bool intersect(Line3D);
	};

	struct Plane {
		float a, b, c, d;
		Plane(float x1 = 0.0, float y1 = 0.0, float x2 = 0.0, float y2 = 0.0);
	};

	class Face {
	public:
		vector<int> v, vt, vn;
		unsigned char mtl;
		Face();
		virtual void add(int v, int vt = 0, int vn = 0);
		int numberOfVertices();
	};

	class Triangle {
		int nVertices;
	public:
		int v[3], vt[3], vn[3];
		unsigned char mtl;
		int neighbour[3];
		Plane p;
		bool visible;
		Triangle();
		virtual void add(int v, int vt = 0, int vn = 0);
		void calculatePlane(vector<Point4D>);
		void setVisibility(float[]);
	};

	Vector3D getNormal(Point3D, Point3D, Point3D);
	Vector4D getTangent(Point3D, Point3D, Point3D, TexCoord2D, TexCoord2D, TexCoord2D, Vector3D);
}

#endif
