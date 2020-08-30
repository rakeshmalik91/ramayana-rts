/*****************************************************************************************************
 * Subject                   : Line class                                                            *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
 #include "math.h"

namespace math {
	
	float Line2D::slope() {
		if(A.x==B.x)
			return 0;
		else
			return (A.y-B.y)/(A.x-B.x);
	}
	float Line2D::tangent() {
		int angle=atan(slope())*180.0/PI;
		angle=(A.x<B.x)?angle:(angle+180);
		angle=(angle+360)%360;
		return angle;
	}
	
	float dist(Line2D L, Point2D P) {
		float normalLength=dist(L.A, L.B);
		return abs((P.x-L.A.x)*(L.B.x-L.A.x)-(P.y-L.A.y)*(L.B.y-L.A.y))/normalLength;
	}
	float dist(Line3D L, Point3D P) {
		Vector3D vPB=directionVector(P, L.B);
		float dPB=vPB.hypot();
		Vector3D vAB=unitVector(L.A, L.B);
		float dot=dotProduct(vPB, vAB);
		return sqrt(dPB*dPB-dot*dot);
	}

}
