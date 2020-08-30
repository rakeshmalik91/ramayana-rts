/*****************************************************************************************************
 * Subject                   : Vector classes & functions                                            *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
 #include "math.h"

namespace math {
	
	Vector2D directionVector(Point2D A, Point2D B) {
		return Vector2D(B.x-A.x, B.y-A.y);
	}
	Vector2D unitVector(Point2D A, Point2D B) {
		Vector2D dirVect=directionVector(A, B);
		return dirVect.normalize();
	}
	float dotProduct(Vector2D A, Vector2D B) {
		return A.x*B.x+A.y*B.y;
	}
	
	Vector3D directionVector(Point3D A, Point3D B) {
		return Vector3D(B.x-A.x, B.y-A.y, B.z-A.z);
	}
	Vector3D unitVector(Point3D A, Point3D B) {
		Vector3D dirVect=directionVector(A, B);
		return dirVect.normalize();
	}
	float dotProduct(Vector3D A, Vector3D B) {
		return A.x*B.x+A.y*B.y+A.z*B.z;
	}
	Vector3D crossProduct(Vector3D A, Vector3D B) {
		return Vector3D((A.y*B.z)-(A.z*B.y), (A.z*B.x)-(A.x*B.z), (A.x*B.y)-(A.y*B.x));
	}
	
	Vector4D directionVector(Point4D A, Point4D B) {
		return Vector4D(B.x-A.x, B.y-A.y, B.z-A.z, B.w-A.w);
	}
	Vector4D unitVector(Point4D A, Point4D B) {
		Vector4D dirVect=directionVector(A, B);
		return dirVect.normalize();
	}
	float dotProduct(Point4D A, Point4D B) {
		return A.x*B.x+A.y*B.y+A.z*B.z+A.w*B.w;
	}
}
