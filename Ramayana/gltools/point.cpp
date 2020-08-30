/*****************************************************************************************************
 * Subject                   : 2D/3D/4D Point Classes/Functions                                      *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
#include "math.h"

namespace math {

	bool Point2Di::operator==(const Point2Di &p) const {
		return x==p.x && y==p.y;
	}
	bool Point2Di::operator!=(const Point2Di &p) const {
		return !(*this==p);
	}
	Point2Di Point2Di::operator+(const Point2Di &p) const {
		return Point2Di(x+p.x, y+p.y);
	}
	Point2Di Point2Di::operator-(const Point2Di &p) const {
		return Point2Di(x-p.x, y-p.y);
	}
	Point2Di Point2Di::operator/(float f) const {
		return Point2Di(x/f, y/f);
	}
	Point2Di Point2Di::operator*(float f) const {
		return Point2Di(x*f, y*f);
	}
	Point2Di& Point2Di::operator+=(const Point2Di &p) {
		*this=*this+p;
		return *this;
	}
	Point2Di& Point2Di::operator-=(const Point2Di &p) {
		*this=*this-p;
		return *this;
	}
	Point2Di& Point2Di::operator/=(float f) {
		*this=*this/f;
		return *this;
	}
	Point2Di& Point2Di::operator*=(float f) {
		*this=*this*f;
		return *this;
	}
	bool Point2Di::in(int xmin, int ymin, int xmax, int ymax) const {
		return x>=xmin && y>=ymin && x<=xmax && y<=ymax;
	}
	
	Point2D Point2D::operator-() const {
		return Point2D(-x, -y);
	}
	Point2D Point2D::operator+(const Point2D &p) const {
		Point2D p1(x+p.x, y+p.y);
		return p1;
	}
	Point2D Point2D::operator-(const Point2D &p) const {
		Point2D p1(x-p.x, y-p.y);
		return p1;
	}
	Point2D Point2D::operator/(float f) const {
		Point2D p1(x/f, y/f);
		return p1;
	}
	Point2D Point2D::operator*(float f) const {
		Point2D p1(x*f, y*f);
		return p1;
	}
	Point2D Point2D::operator*(const Point2D &p) const {
		Point2D p1(x*p.x, y*p.y);
		return p1;
	}
	bool Point2D::operator==(const Point2D &p) const {
		return x==p.x && y==p.y;
	}
	bool Point2D::operator!=(const Point2D &p) const {
		return x!=p.x || y!=p.y;
	}
	Point2D& Point2D::operator+=(const Point2D &p) {
		*this = *this + p;
		return *this;
	}
	Point2D& Point2D::operator-=(const Point2D &p) {
		*this = *this - p;
		return *this;
	}
	Point2D& Point2D::operator/=(float f) {
		*this = *this / f;
		return *this;
	}
	Point2D& Point2D::operator*=(float f) {
		*this = *this * f;
		return *this;
	}
	bool Point2D::in(float xmin, float ymin, float xmax, float ymax) const {
		return x>=xmin && y>=ymin && x<=xmax && y<=ymax;
	}
	float Point2D::hypot() const {
		return sqrt(x*x+y*y);
	}
	Point2D::operator Point2Di() const {
		return Point2Di(roundInt(x), roundInt(y));
	}
	Point2D& Point2D::normalize() {
		*this=*this/hypot();
		return *this;
	}
	
	bool Point3Di::operator==(const Point3Di &p) const {
		return x==p.x && y==p.y && z==p.z;
	}
	bool Point3Di::operator!=(const Point3Di &p) const {
		return !(*this==p);
	}
	Point3Di Point3Di::operator+(const Point3Di &p) const {
		return Point3Di(x+p.x, y+p.y, z+p.z);
	}
	Point3Di Point3Di::operator-(const Point3Di &p) const {
		return Point3Di(x-p.x, y-p.y, z-p.z);
	}
	Point3Di Point3Di::operator/(float f) const {
		return Point3Di(x/f, y/f, z/f);
	}
	Point3Di Point3Di::operator*(float f) const {
		return Point3Di(x*f, y*f, z*f);
	}
	Point2Di Point3Di::xy() const {
		return Point2Di(x, y);
	}
	
	Point3D Point3D::operator-() const {
		return Point3D(-x, -y, -z);
	}
	void Point3D::set(float x, float y, float z) {
		this->x=x;
		this->y=y;
		this->z=z;
	}
	float* Point3D::toArray(float arr[]) const {
		arr[0]=x;
		arr[1]=y;
		arr[2]=z;
		return arr;
	}
	Point3D Point3D::operator+(const Point3D &p) const {
		Point3D p1(x+p.x, y+p.y, z+p.z);
		return p1;
	}
	Point3D Point3D::operator-(const Point3D &p) const {
		Point3D p1(x-p.x, y-p.y, z-p.z);
		return p1;
	}
	Point3D Point3D::operator/(float f) const {
		Point3D p1(x/f, y/f, z/f);
		return p1;
	}
	Point3D Point3D::operator*(float f) const {
		Point3D p1(x*f, y*f, z*f);
		return p1;
	}
	Point3D Point3D::operator*(const Point3D &p) const {
		Point3D p1(x*p.x, y*p.y, z*p.z);
		return p1;
	}
	bool Point3D::operator==(const Point3D &p) const {
		return x==p.x && y==p.y && z==p.z;
	}
	bool Point3D::operator!=(const Point3D &p) const {
		return x!=p.x || y!=p.y || z!=p.z;
	}
	Point3D& Point3D::operator+=(const Point3D &p) {
		*this = *this + p;
		return *this;
	}
	Point3D& Point3D::operator-=(const Point3D &p) {
		*this = *this - p;
		return *this;
	}
	Point3D& Point3D::operator/=(float f) {
		*this = *this / f;
		return *this;
	}
	Point3D& Point3D::operator*=(float f) {
		*this = *this * f;
		return *this;
	}
	float Point3D::hypot() const {
		return sqrt(x*x+y*y+z*z);
	}
	Point3D& Point3D::normalize() {
		*this=*this/hypot();
		return *this;
	}
	Point3D::operator Point3Di() {
		return Point3Di(roundInt(x), roundInt(y), roundInt(z));
	}
	
	Point4D Point4D::operator-() const {
		return Point4D(-x, -y, -z, -w);
	}
	void Point4D::set(float x, float y, float z, float w) {
		this->x=x;
		this->y=y;
		this->z=z;
		this->w=w;
	}
	float* Point4D::toArray(float arr[]) const {
		arr[0]=x;
		arr[1]=y;
		arr[2]=z;
		arr[3]=w;
		return arr;
	}
	Point4D Point4D::operator+(const Point4D &p) const {
		Point4D p1(x+p.x, y+p.y, z+p.z, w+p.w);
		return p1;
	}
	Point4D Point4D::operator-(const Point4D &p) const {
		Point4D p1(x-p.x, y-p.y, z-p.z, w-p.w);
		return p1;
	}
	Point4D Point4D::operator/(float f) const {
		Point4D p1(x/f, y/f, z/f, w/f);
		return p1;
	}
	Point4D Point4D::operator*(float f) const {
		Point4D p1(x*f, y*f, z*f, w*f);
		return p1;
	}
	Point4D Point4D::operator*(const Point4D &p) const {
		Point4D p1(x*p.x, y*p.y, z*p.z, w*p.w);
		return p1;
	}
	bool Point4D::operator==(const Point4D &p) const {
		return x==p.x && y==p.y && z==p.z && w==p.w;
	}
	bool Point4D::operator!=(const Point4D &p) const {
		return x!=p.x || y!=p.y || z!=p.z || w!=p.w;
	}
	float Point4D::hypot() const {
		return sqrt(x*x+y*y+z*z+w*w);
	}
	Point4D& Point4D::normalize() {
		*this=*this/hypot();
		return *this;
	}
	bool Point4D::in(const Point4D &p1, const Point4D &p2) const {
		return x>=p1.x && x<=p2.x && y>=p1.y && y<=p2.y && z>=p1.z && z<=p2.z && w>=p1.w && w<=p2.w;
	}

	float squareDist(Point2D A, Point2D B) {
		float dx=A.x-B.x, dy=A.y-B.y;
		return dx*dx+dy*dy;
	}
	float squareDist(Point3D A, Point3D B) {
		float dx=A.x-B.x, dy=A.y-B.y, dz=A.z-B.z;
		return dx*dx+dy*dy+dz*dz;
	}
	float dist(Point2D A, Point2D B) {
		return sqrt(squareDist(A, B));
	}
	float dist(Point3D A, Point3D B) {
		return sqrt(squareDist(A, B));
	}
	int manhattanDist(Point2Di A, Point2Di B) {
		return abs(A.x-B.x)+abs(A.y-B.y);
	}
	int manhattanDist(Point3Di A, Point3Di B) {
		return abs(A.x-B.x)+abs(A.y-B.y)+abs(A.z-B.z);
	}
	
	Point2D rotatePoint(int theta, Point2D P, Point2D O) {
		P=P-O;
		theta=(theta+3600000)%360;
		Point2D P1=Point2D(P.x*_cos[theta]-P.y*_sin[theta], P.x*_sin[theta]+P.y*_cos[theta]);
		P=P1+O;
		return P;
	}
	Point3D rotatePointAlongX(int theta, Point3D P, Point3D O) {
		P=P-O;
		theta=(theta+3600000)%360;
		Point3D P1=Point3D(P.x, P.y*_cos[theta]-P.z*_sin[theta], P.y*_sin[theta]+P.z*_cos[theta]);
		P=P1+O;
		return P;
	}
	Point3D rotatePointAlongY(int theta, Point3D P, Point3D O) {
		P=P-O;
		theta=(theta+3600000)%360;
		Point3D P1=Point3D(P.z*_sin[theta]+P.x*_cos[theta], P.y, P.z*_cos[theta]-P.x*_sin[theta]);
		P=P1+O;
		return P;
	}
	Point3D rotatePointAlongZ(int theta, Point3D P, Point3D O) {
		P=P-O;
		theta=(theta+3600000)%360;
		Point3D P1=Point3D(P.x*_cos[theta]-P.y*_sin[theta], P.x*_sin[theta]+P.y*_cos[theta], P.z);
		P=P1+O;
		return P;
	}
}
