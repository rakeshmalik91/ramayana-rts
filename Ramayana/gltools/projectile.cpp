/*****************************************************************************************************
 * Subject                   : Projectile Motion Class                                               *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
#include "particle.h"

using namespace graphics;

#define GRAVITY 0.05

namespace physics {
	
	Projectile::Projectile() {}
	Projectile::Projectile(Point3D ipos, float hAngle, float vAngle, float v, float weight, bool approx) : 
		pos(ipos), ipos(ipos), lastpos(ipos), dst(ipos), hAngle(modulo(hAngle, 360)), vAngle(modulo(vAngle, 360)), v(v), weight(weight), s(0), overHorizon(false), approx(approx) {
		float vsintheta=v*sin(vAngle*PI/180);
		float g=GRAVITY*weight;
		timeOfFlight=roundInt((vsintheta+sqrt(vsintheta*vsintheta+2*g*(ipos.z-dst.z)))/g);
	}
	Projectile::Projectile(Point3D ipos, Point3D dst, float hAngle, float v, float weight, bool overHorizon, bool approx) :
		pos(ipos), ipos(ipos), lastpos(ipos), dst(dst), hAngle(modulo(hAngle, 360)), vAngle(0), v(v), weight(weight), s(0), overHorizon(overHorizon), approx(approx) {
		float x=dist(Point2D(ipos.x, ipos.y), Point2D(dst.x, dst.y));
		float y=dst.z-ipos.z;
		float g=GRAVITY*weight;
		float m=g*x*x/(v*v);
		float t;
		if(overHorizon)
			t=(x+sqrt(x*x-2*m*(y+m/2)))/m;							// Equation of trajectory : y = x tan (theta) + ( (g * x^2) / (2 v^2 cos^2(theta) ) 
		else
			t=(x-sqrt(x*x-2*m*(y+m/2)))/m;							// Equation of trajectory : y = x tan (theta) - ( (g * x^2) / (2 v^2 cos^2(theta) ) 
		vAngle=atan(t)*180/PI;
		if(vAngle<0) vAngle+=360;
		if(isNAN(vAngle)) vAngle=45;

		float vsintheta=v*sin(vAngle*PI/180);
		timeOfFlight=roundInt((vsintheta+sqrt(vsintheta*vsintheta+2*g*(ipos.z-dst.z)))/g);
	}
	void Projectile::move() {
		lastpos=pos;
		s++;

		float g=GRAVITY*weight;
		if(approx) {
			float x=v*s*_cos[(int)vAngle];									// x = v * t * cos(thata)
			float y=v*s*_sin[(int)vAngle]-g*(s*s)/2;						// y = v * t * sin(thata) - 0.5 * g * t^2
			pos.x=ipos.x+x*_cos[(int)hAngle];
			pos.y=ipos.y+x*_sin[(int)hAngle];
			pos.z=ipos.z+y;
		} else {
			float vAngleRadian=vAngle*PI/180, hAngleRadian=hAngle*PI/180;
			float x=v*s*cos(vAngleRadian);									// x = v * t * cos(thata)
			float y=v*s*sin(vAngleRadian)-g*(s*s)/2;						// y = v * t * sin(thata) - 0.5 * g * t^2
			pos.x=ipos.x+x*cos(hAngleRadian);
			pos.y=ipos.y+x*sin(hAngleRadian);
			pos.z=ipos.z+y;
		}
	
		float d=dist(Point2D(pos.x, pos.y), Point2D(lastpos.x, lastpos.y));
		angle=-atan((pos.z-lastpos.z)/d)*180/PI;
	}
	bool Projectile::hits(Point3D p) {
		return dist(p, lastpos)<=dist(pos, lastpos) || s>timeOfFlight;
	}
	bool Projectile::isPossible() {
		return vAngle>=0 && vAngle<360;
	}
}