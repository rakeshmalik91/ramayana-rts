/*****************************************************************************************************
 * Subject                   : Graphics Componenets                                                  *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
 
#include "math.h"
#include "graphics.h"

using namespace std;
using namespace math;

namespace graphics {

	vector<Point2Di> lineBresenham(Point2Di A, Point2Di B) {
		vector<Point2Di> line;
		line.clear();
		int dx=abs(A.x-B.x), dy=abs(A.y-B.y);
		if(dx==0) {
			if(A.y<B.y)
				for(int y=A.y; y<=B.y; y++)
					line.insert(line.end(), Point2Di(A.x, y));
			else
				for(int y=A.y; y>=B.y; y--)
					line.insert(line.end(), Point2Di(A.x, y));
		} else if(dy==0) {
			if(A.x<B.x)
				for(int x=A.x; x<=B.x; x++)
					line.insert(line.end(), Point2Di(x, A.y));
			else
				for(int x=A.x; x>=B.x; x--)
					line.insert(line.end(), Point2Di(x, A.y));
		} else {
			float slope=(float)(B.y-A.y)/(B.x-A.x);
			int d=(slope>0)?1:(-1);
			if(abs(slope)<=1) {
				int p=2*dy-dx;
				int twoDy=2*dy, twoDyDx=2*(dy-dx);
				int x, y, xEnd;
				if(A.x>B.x) {
					x=B.x;
					y=B.y;
					xEnd=A.x;
				} else {
					x=A.x;
					y=A.y;
					xEnd=B.x;
				}
				line.insert(line.end(), Point2Di(x, y));
				while(x<xEnd) {
					x++;
					if(p<0) {
						p+=twoDy;
					} else {
						y+=d;
						p+=twoDyDx;
					}
					if(A.x>B.x) line.insert(line.begin(), Point2Di(x, y));
					else line.insert(line.end(), Point2Di(x, y));
				}
			} else {
				int p=2*dy-dx;
				int twoDx=2*dx, twoDyDx=2*(dx-dy);
				int x, y, yEnd;
				if(A.y>B.y) {
					x=B.x;
					y=B.y;
					yEnd=A.y;
				} else {
					x=A.x;
					y=A.y;
					yEnd=B.y;
				}
				line.insert(line.end(), Point2Di(x, y));
				while(y<yEnd) {
					y++;
					if(p<0) {
						p+=twoDx;
					} else {
						x+=d;
						p+=twoDyDx;
					}
					if(A.y>B.y) line.insert(line.begin(), Point2Di(x, y));
					else line.insert(line.end(), Point2Di(x, y));
				}
			}
		}
		return line;
	}
	
	void fillCircle(bool **matrix, int w, int h, bool value, Point2Di centre, int radius) {
		int x0=centre.x, y0=centre.y;
		int d=1-radius;
		int ddx=1;
		int ddy=-2*radius;
		int x=0;
		int y=radius;
		while(x<=y) {
			if(x0+x<w)		for(int r=(y0-y<0)?0:(y0-y), c=x0+x; r<=y0+y && r<h; r++)	matrix[r][c]=value;
			if(x0+y<w)		for(int r=(y0-x<0)?0:(y0-x), c=x0+y; r<=y0+x && r<h; r++)	matrix[r][c]=value;
			if(x0-y>=0)     for(int r=(y0-x<0)?0:(y0-x), c=x0-y; r<=y0+x && r<h; r++)	matrix[r][c]=value;
			if(x0-x>=0)     for(int r=(y0-y<0)?0:(y0-y), c=x0-x; r<=y0+y && r<h; r++)	matrix[r][c]=value;
			if(d>=0) {
				y--;
				ddy+=2;
				d+=ddy;
			}
			x++;
			ddx+=2;
			d+=ddx;
		}
	}

	Color::Color(float v) 
		: Point4D(clamp(v, 0.0, 1.0), 1.0) {
	}
	Color::Color(float r, float g, float b, float a) 
		: Point4D(clamp(r, 0.0, 1.0), clamp(g, 0.0, 1.0), clamp(b, 0.0, 1.0), clamp(a, 0.0, 1.0)) {
	}
	Color::Color(const Color &c)
		: Point4D(c) {
	}
	void Color::set(float v) {
		x = y = z = v;
		z = 1.0;
	}
	void Color::set(float r, float g, float b, float a) {
		x = r;
		y = g;
		z = b;
		w = a;
	}
	float Color::r() const {
		return x;
	}
	float Color::g() const {
		return y;
	}
	float Color::b() const {
		return z;
	}
	float Color::a() const {
		return w;
	}
	Point3D Color::rgb() const {
		return *this;
	}
	Point4D Color::rgba() const {
		return *this;
	}
	void Color::r(float r) {
		x = clamp(r, 0.0, 1.0);
	}
	void Color::g(float g) {
		y = clamp(g, 0.0, 1.0);
	}
	void Color::b(float b) {
		z = clamp(b, 0.0, 1.0);
	}
	void Color::a(float a) {
		w = clamp(a, 0.0, 1.0);
	}
	Color& Color::operator=(const Color &color) {
		x = color.x;
		y = color.y;
		z = color.z;
		w = color.w;
		return *this;
	}
	bool Color::operator==(const Color &color) {
		float t=0.05;
		return abs(x - color.x)<t && abs(y - color.y)<t && abs(z - color.z)<t && abs(w - color.w)<t && abs(hypot() - color.hypot())<t;
	}
	
	Quad::Quad(float x1, float y1, float x2, float y2) {
		this->x1=x1;
		this->y1=y1;
		this->x2=x2;
		this->y2=y2;
	}
	float Quad::area() {
		return abs((x1-x2)*(y1-y2));
	}
	void Quad::clear() {
		x1=x2=y1=y2=0;
	}

	Box::Box(float x1,float y1,float z1,float x2,float y2,float z2) {
		xMin=min(x1, x2);
		xMax=max(x1, x2);
		yMin=min(y1, y2);
		yMax=max(y1, y2);
		zMin=min(z1, z2);
		zMax=max(z1, z2);
	}
	float Box::volume() {
		return abs((xMax-xMin)*(yMax-yMin)*(zMax-zMin));
	}
	bool Box::intersectRay(Line3D l) {
		//Ray-Box Intersection
		float tmin = (xMin - l.A.x) / l.B.x;
		float tmax = (xMax - l.A.x) / l.B.x;
		if(tmin > tmax) swap(tmin, tmax);
		float tymin = (yMin - l.A.y) / l.B.y;
		float tymax = (yMax - l.A.y) / l.B.y;
		if(tymin > tymax) swap(tymin, tymax);
		if((tmin > tymax) || (tymin > tmax))
		    return false;
		if(tymin > tmin)
		    tmin = tymin;
		if(tymax < tmax)
		    tmax = tymax;
		float tzmin = (zMin - l.A.z) / l.B.z;
		float tzmax = (zMax - l.A.z) / l.B.z;
		if(tzmin > tzmax) 
			swap(tzmin, tzmax);
		if((tmin > tzmax) || (tzmin > tmax))
		    return false;
		if(tzmin > tmin)
		    tmin = tzmin;
		if(tzmax < tmax)
		    tmax = tzmax;
		//if((tmin > r.tmax) || (tmax < r.tmin)) 
		//	return false;
		//if(r.tmin < tmin) r.tmin = tmin;
		//if(r.tmax > tmax) r.tmax = tmax;
		return true;
	}
	bool Box::intersect(Line3D l) {
		//Cohen-Sutherland Clipping Algorithm
		unsigned char codeA=0, codeB=0;
		codeA |= (unsigned char)(l.A.x<=xMin) & (unsigned char)0x01;
		codeA |= (unsigned char)(l.A.x>=xMax) & (unsigned char)0x02;
		codeA |= (unsigned char)(l.A.y<=yMin) & (unsigned char)0x04;
		codeA |= (unsigned char)(l.A.y>=yMax) & (unsigned char)0x08;
		codeA |= (unsigned char)(l.A.z<=zMin) & (unsigned char)0x10;
		codeA |= (unsigned char)(l.A.z>=zMax) & (unsigned char)0x20;
		codeB |= (unsigned char)(l.B.x<=xMin) & (unsigned char)0x01;
		codeB |= (unsigned char)(l.B.x>=xMax) & (unsigned char)0x02;
		codeB |= (unsigned char)(l.B.y<=yMin) & (unsigned char)0x04;
		codeB |= (unsigned char)(l.B.y>=yMax) & (unsigned char)0x08;
		codeB |= (unsigned char)(l.B.z<=zMin) & (unsigned char)0x10;
		codeB |= (unsigned char)(l.B.z>=zMax) & (unsigned char)0x20;
		unsigned char code = codeA & codeB;
		return (codeA==0 || codeB==0 || code!=0) && intersectRay(l);
	}

	bool Sphere::intersect(Point3D p) {
		return squareDist(p, centre) < radius*radius;
	}
	
	Plane::Plane(float a, float b, float c, float d) {
		this->a=a;
		this->b=b;
		this->c=c;
		this->d=d;
	}
	
	Face::Face() {
	}
	void Face::add(int v, int vt, int vn) {
		this->v.push_back(v);
		this->vt.push_back(vt);
		this->vn.push_back(vn);
	}
	int Face::numberOfVertices() {
		return this->v.size();
	}
	
	Triangle::Triangle() {
		nVertices=0;
		for(int i=0; i<3; i++)
			this->neighbour[i]=-1;
		visible=true;
	}
	void Triangle::add(int v, int vt, int vn) {
		this->v[nVertices]=v;
		this->vt[nVertices]=vt;
		this->vn[nVertices]=vn;
		if(nVertices<3) nVertices++;
	}
	void Triangle::calculatePlane(vector<Point4D> vref) {
		Point4D v1=vref[this->v[0]], v2=vref[this->v[1]], v3=vref[this->v[2]];
		this->p.a = v1.y*(v2.z-v3.z) + v2.y*(v3.z-v1.z) + v3.y*(v1.z-v2.z);
		this->p.b = v1.z*(v2.x-v3.x) + v2.z*(v3.x-v1.x) + v3.z*(v1.x-v2.x);
		this->p.c = v1.x*(v2.y-v3.y) + v2.x*(v3.y-v1.y) + v3.x*(v1.y-v2.y);
		this->p.d = -( v1.x*( v2.y*v3.z - v3.y*v2.z ) + v2.x*(v3.y*v1.z - v1.y*v3.z) + v3.x*(v1.y*v2.z - v2.y*v1.z) );
	}
	void Triangle::setVisibility(float lightpos[]) {
		float side = this->p.a*lightpos[0] + this->p.b*lightpos[1] + this->p.c*lightpos[2] + this->p.d*lightpos[3];
		if(side>0) visible=true;
		else visible=false;
	}
	
	Vector3D getNormal(Point3D p1, Point3D p2, Point3D p3) {
		Vector3D u=unitVector(p1, p2);
		Vector3D v=unitVector(p1, p3);
		Vector3D normal=Vector3D(u.y*v.z-u.z*v.y, u.z*v.x-u.x*v.z, u.x*v.y-u.y*v.x);
		return normal;
	}
	Vector4D getTangent(Point3D pos1, Point3D pos2, Point3D pos3, TexCoord2D texcoord1, TexCoord2D texcoord2, TexCoord2D texcoord3, Vector3D normal) {
		Vector3D e1=unitVector(pos1, pos2);
		Vector3D e2=unitVector(pos1, pos3);
		Vector2D te1=unitVector(texcoord1, texcoord2);
		Vector2D te2=unitVector(texcoord1, texcoord3);
		Vector3D t, b;
		float det=(te1.x*te2.y)-(te1.y*te2.x);
		if(det==0.0) {
			t=Vector3D(1, 0, 0);
			b=Vector3D(0, 1, 0);
		} else {
			det=1.0/det;
			t=Vector3D(te2.y*e1.x-te1.y*e2.x, te2.y*e1.y-te1.y-e2.y, te2.y*e1.z-te1.y*e2.z)*det;
			b=Vector3D(-te2.x*e1.x+te1.x*e2.x, -te2.x*e1.y+te1.x-e2.y, -te2.x*e1.z+te1.x*e2.z)*det;
			t=t/t.hypot();
			b=b/b.hypot();
		}
		Vector3D bitangent=crossProduct(normal, t);
		float handedness=(dotProduct(bitangent, b)<0.0)?-1.0:1.0;
		Vector4D tangent=Vector4D(t.x, t.y, t.z, handedness);
		if(tangent.in(Vector4D(-1, -1, -1, 0), Vector4D(1, 1, 1, 1)))
			return tangent;
		else 
			return Vector4D(1, 0, 0, 1);
	}

};
