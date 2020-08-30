/*****************************************************************************************************
 * Subject                   : Math Functions                                                        *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
#include "math.h"

namespace math {

	void vect4Mat16Mult(float M[16], float v[4]) {
		float res[4];
		res[0]=M[ 0]*v[0]+M[ 4]*v[1]+M[ 8]*v[2]+M[12]*v[3];
		res[1]=M[ 1]*v[0]+M[ 5]*v[1]+M[ 9]*v[2]+M[13]*v[3];
		res[2]=M[ 2]*v[0]+M[ 6]*v[1]+M[10]*v[2]+M[14]*v[3];
		res[3]=M[ 3]*v[0]+M[ 7]*v[1]+M[11]*v[2]+M[15]*v[3];
		memcpy(v, res, 4*sizeof(float));
	}
	void mat16Inverse(float M[16]) {
		float minor[16], inv[16], det;
		minor[ 0]=M[ 5]*(M[10]*M[15]-M[11]*M[14])-M[ 6]*(M[ 9]*M[15]-M[11]*M[13])+M[ 7]*(M[ 9]*M[14]-M[10]*M[13]);
		minor[ 1]=M[ 4]*(M[10]*M[15]-M[11]*M[14])-M[ 6]*(M[ 8]*M[15]-M[11]*M[12])+M[ 7]*(M[ 8]*M[14]-M[10]*M[12]);
		minor[ 2]=M[ 4]*(M[ 9]*M[15]-M[11]*M[13])-M[ 5]*(M[ 8]*M[15]-M[11]*M[12])+M[ 7]*(M[ 8]*M[13]-M[ 9]*M[12]);
		minor[ 3]=M[ 4]*(M[ 9]*M[14]-M[10]*M[13])-M[ 5]*(M[ 8]*M[14]-M[10]*M[12])+M[ 6]*(M[ 8]*M[13]-M[ 9]*M[12]);
		minor[ 4]=M[ 1]*(M[10]*M[15]-M[11]*M[14])-M[ 2]*(M[ 9]*M[15]-M[11]*M[13])+M[ 3]*(M[ 9]*M[14]-M[10]*M[13]);
		minor[ 5]=M[ 0]*(M[10]*M[15]-M[11]*M[14])-M[ 2]*(M[ 8]*M[15]-M[11]*M[12])+M[ 3]*(M[ 8]*M[14]-M[10]*M[12]);
		minor[ 6]=M[ 0]*(M[ 9]*M[15]-M[11]*M[13])-M[ 1]*(M[ 8]*M[15]-M[11]*M[12])+M[ 3]*(M[ 8]*M[13]-M[ 9]*M[12]);
		minor[ 7]=M[ 0]*(M[ 9]*M[14]-M[10]*M[13])-M[ 1]*(M[ 8]*M[14]-M[10]*M[12])+M[ 2]*(M[ 8]*M[13]-M[ 9]*M[12]);
		minor[ 8]=M[ 1]*(M[ 6]*M[15]-M[ 7]*M[14])-M[ 2]*(M[ 5]*M[15]-M[ 7]*M[13])+M[ 3]*(M[ 5]*M[14]-M[ 6]*M[13]);
		minor[ 9]=M[ 0]*(M[ 6]*M[15]-M[ 7]*M[14])-M[ 2]*(M[ 4]*M[15]-M[ 7]*M[12])+M[ 3]*(M[ 4]*M[14]-M[ 6]*M[12]);
		minor[10]=M[ 0]*(M[ 5]*M[15]-M[ 7]*M[13])-M[ 1]*(M[ 4]*M[15]-M[ 7]*M[12])+M[ 3]*(M[ 4]*M[13]-M[ 5]*M[12]);
		minor[11]=M[ 0]*(M[ 5]*M[14]-M[ 6]*M[13])-M[ 1]*(M[ 4]*M[14]-M[ 6]*M[12])+M[ 2]*(M[ 4]*M[13]-M[ 5]*M[12]);
		minor[12]=M[ 1]*(M[ 6]*M[11]-M[ 7]*M[10])-M[ 2]*(M[ 5]*M[11]-M[ 7]*M[ 9])+M[ 3]*(M[ 5]*M[10]-M[ 6]*M[ 9]);
		minor[13]=M[ 0]*(M[ 6]*M[11]-M[ 7]*M[10])-M[ 2]*(M[ 4]*M[11]-M[ 7]*M[ 8])+M[ 3]*(M[ 4]*M[10]-M[ 6]*M[ 8]);
		minor[14]=M[ 0]*(M[ 5]*M[11]-M[ 7]*M[ 9])-M[ 1]*(M[ 4]*M[11]-M[ 7]*M[ 8])+M[ 3]*(M[ 4]*M[ 9]-M[ 5]*M[ 8]);
		minor[15]=M[ 0]*(M[ 5]*M[10]-M[ 6]*M[ 9])-M[ 1]*(M[ 4]*M[10]-M[ 6]*M[ 8])+M[ 2]*(M[ 4]*M[ 9]-M[ 5]*M[ 8]);
		det=M[0]*minor[0]-M[1]*minor[1]+M[2]*minor[2]-M[3]*minor[3];
		inv[ 0]= minor[ 0]/det; inv[ 1]=-minor[ 4]/det; inv[ 2]= minor[ 8]/det; inv[ 3]=-minor[12]/det;
		inv[ 4]=-minor[ 1]/det; inv[ 5]= minor[ 5]/det; inv[ 6]=-minor[ 9]/det; inv[ 7]= minor[13]/det;
		inv[ 8]= minor[ 2]/det; inv[ 9]=-minor[ 6]/det; inv[10]= minor[10]/det; inv[11]=-minor[14]/det;
		inv[12]=-minor[ 3]/det; inv[13]= minor[ 7]/det; inv[14]=-minor[11]/det; inv[15]= minor[15]/det;
		memcpy(M, inv, 16*sizeof(float));
	}

	float positiveSubtract(float x, float y) {
		float z=x-y;
		return (z<0)?0:z;
	}
	float clamp(float n, float lo, float hi) {
		return (n<=lo)?lo:(n>=hi)?hi:n;
	}
	float clampLow(float n, float lo) {
		return (n<lo)?lo:n;
	}
	float clampHigh(float n, float hi) {
		return (n>hi)?hi:n;
	}
	float modulo(float x, float y) {
		return x-y*(x/y<0?-1:1)*abs(floor(x/y));
	}
	int roundInt(float n) {
		return floor(n+0.5);
	}
	int log2(int n) {
		int l=0;
		if(n>0) {
			while(n!=1) {
				n/=2;
				l++;
			}
		}
		return l;
	}
	float sign(float n) {
		return n<0.0?-1.0:n>0.0?1.0:0.0;
	}
	int absolute(int n) {
		return n<0?-n:n;
	}

	int factorial(int n) {
		int f = 1;
		while (n > 1) {
			f *= n;
			f--;
		}
		return f;
	}
	int combination(int n, int r) {
		return factorial(n) / (factorial(r) * factorial(n - r));
	}
};