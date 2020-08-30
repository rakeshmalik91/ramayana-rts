#ifndef __MATH_H
#define __MATH_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

using namespace std;

namespace math {

	//Boundary Check
	template<typename DT> bool isNAN(DT n) {
		return n!=n;
	}

	//Template Classes
	template<class DT1, class DT2> struct Tuple {
		DT1 e1;
		DT2 e2;
		Tuple(DT1 e1, DT2 e2) : e1(e1), e2(e2) {}
	};
	template<class DT1, class DT2, class DT3> struct Triplet : public Tuple<DT1, DT2> {
		DT3 e3;
		Triplet(DT1 e1, DT2 e2, DT3 e3) : Tuple<DT1, DT2>(e1, e2), e3(e3) {}
		Triplet(Tuple<DT1, DT2> e12, DT3 e3) : Tuple<DT1, DT2>(e12), e3(e3) {}
	};
	template<class DT> struct Range {
		DT low, high;
		Range() {}
		Range(DT low) : low(low), high(low) {}
		Range(DT low, DT high) : low(low), high(high) {}
	};
	
	//constants
	const float PI=3.1415926535897932384626433832795f;
	const float EULER=2.71828182845904523536028747135266249775724709369995f;
	const float SQRT2=1.4142135623730950488016887242097f;
	
	//trigonometric function for faster calculation only for integer angles
	const float _sin[360]={0.000000f, 0.017452f, 0.034899f, 0.052336f, 0.069756f, 0.087156f, 0.104528f, 0.121869f, 0.139173f, 0.156434f, 0.173648f, 0.190809f, 0.207912f, 0.224951f, 0.241922f, 0.258819f, 0.275637f, 0.292372f, 0.309017f, 0.325568f, 0.342020f, 0.358368f, 0.374607f, 0.390731f, 0.406737f, 0.422618f, 0.438371f, 0.453990f, 0.469472f, 0.484810f, 0.500000f, 0.515038f, 0.529919f, 0.544639f, 0.559193f, 0.573576f, 0.587785f, 0.601815f, 0.615661f, 0.629320f, 0.642788f, 0.656059f, 0.669131f, 0.681998f, 0.694658f, 0.707107f, 0.719340f, 0.731354f, 0.743145f, 0.754710f, 0.766044f, 0.777146f, 0.788011f, 0.798636f, 0.809017f, 0.819152f, 0.829038f, 0.838671f, 0.848048f, 0.857167f, 0.866025f, 0.874620f, 0.882948f, 0.891007f, 0.898794f, 0.906308f, 0.913545f, 0.920505f, 0.927184f, 0.933580f, 0.939693f, 0.945519f, 0.951057f, 0.956305f, 0.961262f, 0.965926f, 0.970296f, 0.974370f, 0.978148f, 0.981627f, 0.984808f, 0.987688f, 0.990268f, 0.992546f, 0.994522f, 0.996195f, 0.997564f, 0.998630f, 0.999391f, 0.999848f, 1.000000f, 0.999848f, 0.999391f, 0.998630f, 0.997564f, 0.996195f, 0.994522f, 0.992546f, 0.990268f, 0.987688f, 0.984808f, 0.981627f, 0.978148f, 0.974370f, 0.970296f, 0.965926f, 0.961262f, 0.956305f, 0.951057f, 0.945519f, 0.939693f, 0.933580f, 0.927184f, 0.920505f, 0.913545f, 0.906308f, 0.898794f, 0.891007f, 0.882948f, 0.874620f, 0.866025f, 0.857167f, 0.848048f, 0.838671f, 0.829038f, 0.819152f, 0.809017f, 0.798636f, 0.788011f, 0.777146f, 0.766044f, 0.754710f, 0.743145f, 0.731354f, 0.719340f, 0.707107f, 0.694658f, 0.681998f, 0.669131f, 0.656059f, 0.642788f, 0.629320f, 0.615661f, 0.601815f, 0.587785f, 0.573576f, 0.559193f, 0.544639f, 0.529919f, 0.515038f, 0.500000f, 0.484810f, 0.469472f, 0.453990f, 0.438371f, 0.422618f, 0.406737f, 0.390731f, 0.374607f, 0.358368f, 0.342020f, 0.325568f, 0.309017f, 0.292372f, 0.275637f, 0.258819f, 0.241922f, 0.224951f, 0.207912f, 0.190809f, 0.173648f, 0.156434f, 0.139173f, 0.121869f, 0.104528f, 0.087156f, 0.069756f, 0.052336f, 0.034899f, 0.017452f, 0.000000f, -0.017452f, -0.034899f, -0.052336f, -0.069756f, -0.087156f, -0.104528f, -0.121869f, -0.139173f, -0.156434f, -0.173648f, -0.190809f, -0.207912f, -0.224951f, -0.241922f, -0.258819f, -0.275637f, -0.292372f, -0.309017f, -0.325568f, -0.342020f, -0.358368f, -0.374607f, -0.390731f, -0.406737f, -0.422618f, -0.438371f, -0.453990f, -0.469472f, -0.484810f, -0.500000f, -0.515038f, -0.529919f, -0.544639f, -0.559193f, -0.573576f, -0.587785f, -0.601815f, -0.615661f, -0.629320f, -0.642788f, -0.656059f, -0.669131f, -0.681998f, -0.694658f, -0.707107f, -0.719340f, -0.731354f, -0.743145f, -0.754710f, -0.766044f, -0.777146f, -0.788011f, -0.798636f, -0.809017f, -0.819152f, -0.829038f, -0.838671f, -0.848048f, -0.857167f, -0.866025f, -0.874620f, -0.882948f, -0.891007f, -0.898794f, -0.906308f, -0.913545f, -0.920505f, -0.927184f, -0.933580f, -0.939693f, -0.945519f, -0.951057f, -0.956305f, -0.961262f, -0.965926f, -0.970296f, -0.974370f, -0.978148f, -0.981627f, -0.984808f, -0.987688f, -0.990268f, -0.992546f, -0.994522f, -0.996195f, -0.997564f, -0.998630f, -0.999391f, -0.999848f, -1.000000f, -0.999848f, -0.999391f, -0.998630f, -0.997564f, -0.996195f, -0.994522f, -0.992546f, -0.990268f, -0.987688f, -0.984808f, -0.981627f, -0.978148f, -0.974370f, -0.970296f, -0.965926f, -0.961262f, -0.956305f, -0.951057f, -0.945519f, -0.939693f, -0.933580f, -0.927184f, -0.920505f, -0.913545f, -0.906308f, -0.898794f, -0.891007f, -0.882948f, -0.874620f, -0.866025f, -0.857167f, -0.848048f, -0.838671f, -0.829038f, -0.819152f, -0.809017f, -0.798636f, -0.788011f, -0.777146f, -0.766044f, -0.754710f, -0.743145f, -0.731354f, -0.719340f, -0.707107f, -0.694658f, -0.681998f, -0.669131f, -0.656059f, -0.642788f, -0.629320f, -0.615661f, -0.601815f, -0.587785f, -0.573576f, -0.559193f, -0.544639f, -0.529919f, -0.515038f, -0.500000f, -0.484810f, -0.469472f, -0.453990f, -0.438371f, -0.422618f, -0.406737f, -0.390731f, -0.374607f, -0.358368f, -0.342020f, -0.325568f, -0.309017f, -0.292372f, -0.275637f, -0.258819f, -0.241922f, -0.224951f, -0.207912f, -0.190809f, -0.173648f, -0.156434f, -0.139173f, -0.121869f, -0.104528f, -0.087156f, -0.069756f, -0.052336f, -0.034899f, -0.017452f};
	const float _cos[360]={1.000000f, 0.999848f, 0.999391f, 0.998630f, 0.997564f, 0.996195f, 0.994522f, 0.992546f, 0.990268f, 0.987688f, 0.984808f, 0.981627f, 0.978148f, 0.974370f, 0.970296f, 0.965926f, 0.961262f, 0.956305f, 0.951057f, 0.945519f, 0.939693f, 0.933580f, 0.927184f, 0.920505f, 0.913545f, 0.906308f, 0.898794f, 0.891007f, 0.882948f, 0.874620f, 0.866025f, 0.857167f, 0.848048f, 0.838671f, 0.829038f, 0.819152f, 0.809017f, 0.798636f, 0.788011f, 0.777146f, 0.766044f, 0.754710f, 0.743145f, 0.731354f, 0.719340f, 0.707107f, 0.694658f, 0.681998f, 0.669131f, 0.656059f, 0.642788f, 0.629320f, 0.615661f, 0.601815f, 0.587785f, 0.573576f, 0.559193f, 0.544639f, 0.529919f, 0.515038f, 0.500000f, 0.484810f, 0.469472f, 0.453990f, 0.438371f, 0.422618f, 0.406737f, 0.390731f, 0.374607f, 0.358368f, 0.342020f, 0.325568f, 0.309017f, 0.292372f, 0.275637f, 0.258819f, 0.241922f, 0.224951f, 0.207912f, 0.190809f, 0.173648f, 0.156434f, 0.139173f, 0.121869f, 0.104528f, 0.087156f, 0.069756f, 0.052336f, 0.034899f, 0.017452f, 0.000000f, -0.017452f, -0.034899f, -0.052336f, -0.069756f, -0.087156f, -0.104528f, -0.121869f, -0.139173f, -0.156434f, -0.173648f, -0.190809f, -0.207912f, -0.224951f, -0.241922f, -0.258819f, -0.275637f, -0.292372f, -0.309017f, -0.325568f, -0.342020f, -0.358368f, -0.374607f, -0.390731f, -0.406737f, -0.422618f, -0.438371f, -0.453990f, -0.469472f, -0.484810f, -0.500000f, -0.515038f, -0.529919f, -0.544639f, -0.559193f, -0.573576f, -0.587785f, -0.601815f, -0.615661f, -0.629320f, -0.642788f, -0.656059f, -0.669131f, -0.681998f, -0.694658f, -0.707107f, -0.719340f, -0.731354f, -0.743145f, -0.754710f, -0.766044f, -0.777146f, -0.788011f, -0.798636f, -0.809017f, -0.819152f, -0.829038f, -0.838671f, -0.848048f, -0.857167f, -0.866025f, -0.874620f, -0.882948f, -0.891007f, -0.898794f, -0.906308f, -0.913545f, -0.920505f, -0.927184f, -0.933580f, -0.939693f, -0.945519f, -0.951057f, -0.956305f, -0.961262f, -0.965926f, -0.970296f, -0.974370f, -0.978148f, -0.981627f, -0.984808f, -0.987688f, -0.990268f, -0.992546f, -0.994522f, -0.996195f, -0.997564f, -0.998630f, -0.999391f, -0.999848f, -1.000000f, -0.999848f, -0.999391f, -0.998630f, -0.997564f, -0.996195f, -0.994522f, -0.992546f, -0.990268f, -0.987688f, -0.984808f, -0.981627f, -0.978148f, -0.974370f, -0.970296f, -0.965926f, -0.961262f, -0.956305f, -0.951057f, -0.945519f, -0.939693f, -0.933580f, -0.927184f, -0.920505f, -0.913545f, -0.906308f, -0.898794f, -0.891007f, -0.882948f, -0.874620f, -0.866025f, -0.857167f, -0.848048f, -0.838671f, -0.829038f, -0.819152f, -0.809017f, -0.798636f, -0.788011f, -0.777146f, -0.766044f, -0.754710f, -0.743145f, -0.731354f, -0.719340f, -0.707107f, -0.694658f, -0.681998f, -0.669131f, -0.656059f, -0.642788f, -0.629320f, -0.615661f, -0.601815f, -0.587785f, -0.573576f, -0.559193f, -0.544639f, -0.529919f, -0.515038f, -0.500000f, -0.484810f, -0.469472f, -0.453990f, -0.438371f, -0.422618f, -0.406737f, -0.390731f, -0.374607f, -0.358368f, -0.342020f, -0.325568f, -0.309017f, -0.292372f, -0.275637f, -0.258819f, -0.241922f, -0.224951f, -0.207912f, -0.190809f, -0.173648f, -0.156434f, -0.139173f, -0.121869f, -0.104528f, -0.087156f, -0.069756f, -0.052336f, -0.034899f, -0.017452f, -0.000000f, 0.017452f, 0.034899f, 0.052336f, 0.069756f, 0.087156f, 0.104528f, 0.121869f, 0.139173f, 0.156434f, 0.173648f, 0.190809f, 0.207912f, 0.224951f, 0.241922f, 0.258819f, 0.275637f, 0.292372f, 0.309017f, 0.325568f, 0.342020f, 0.358368f, 0.374607f, 0.390731f, 0.406737f, 0.422618f, 0.438371f, 0.453990f, 0.469472f, 0.484810f, 0.500000f, 0.515038f, 0.529919f, 0.544639f, 0.559193f, 0.573576f, 0.587785f, 0.601815f, 0.615661f, 0.629320f, 0.642788f, 0.656059f, 0.669131f, 0.681998f, 0.694658f, 0.707107f, 0.719340f, 0.731354f, 0.743145f, 0.754710f, 0.766044f, 0.777146f, 0.788011f, 0.798636f, 0.809017f, 0.819152f, 0.829038f, 0.838671f, 0.848048f, 0.857167f, 0.866025f, 0.874620f, 0.882948f, 0.891007f, 0.898794f, 0.906308f, 0.913545f, 0.920505f, 0.927184f, 0.933580f, 0.939693f, 0.945519f, 0.951057f, 0.956305f, 0.961262f, 0.965926f, 0.970296f, 0.974370f, 0.978148f, 0.981627f, 0.984808f, 0.987688f, 0.990268f, 0.992546f, 0.994522f, 0.996195f, 0.997564f, 0.998630f, 0.999391f, 0.999848f};
	const float _tan[360]={0.000000f, 0.017455f, 0.034921f, 0.052408f, 0.069927f, 0.087489f, 0.105104f, 0.122785f, 0.140541f, 0.158384f, 0.176327f, 0.194380f, 0.212557f, 0.230868f, 0.249328f, 0.267949f, 0.286745f, 0.305731f, 0.324920f, 0.344328f, 0.363970f, 0.383864f, 0.404026f, 0.424475f, 0.445229f, 0.466308f, 0.487733f, 0.509525f, 0.531709f, 0.554309f, 0.577350f, 0.600861f, 0.624869f, 0.649408f, 0.674509f, 0.700208f, 0.726543f, 0.753554f, 0.781286f, 0.809784f, 0.839100f, 0.869287f, 0.900404f, 0.932515f, 0.965689f, 1.000000f, 1.035530f, 1.072369f, 1.110613f, 1.150368f, 1.191754f, 1.234897f, 1.279942f, 1.327045f, 1.376382f, 1.428148f, 1.482561f, 1.539865f, 1.600335f, 1.664279f, 1.732051f, 1.804048f, 1.880726f, 1.962611f, 2.050304f, 2.144507f, 2.246037f, 2.355852f, 2.475087f, 2.605089f, 2.747477f, 2.904211f, 3.077684f, 3.270853f, 3.487414f, 3.732051f, 4.010781f, 4.331476f, 4.704630f, 5.144554f, 5.671282f, 6.313752f, 7.115370f, 8.144346f, 9.514364f, 11.430052f, 14.300666f, 19.081137f, 28.636253f, 57.289962f, 16331778728383844.000000f, -57.289962f, -28.636253f, -19.081137f, -14.300666f, -11.430052f, -9.514364f, -8.144346f, -7.115370f, -6.313752f, -5.671282f, -5.144554f, -4.704630f, -4.331476f, -4.010781f, -3.732051f, -3.487414f, -3.270853f, -3.077684f, -2.904211f, -2.747477f, -2.605089f, -2.475087f, -2.355852f, -2.246037f, -2.144507f, -2.050304f, -1.962611f, -1.880726f, -1.804048f, -1.732051f, -1.664279f, -1.600335f, -1.539865f, -1.482561f, -1.428148f, -1.376382f, -1.327045f, -1.279942f, -1.234897f, -1.191754f, -1.150368f, -1.110613f, -1.072369f, -1.035530f, -1.000000f, -0.965689f, -0.932515f, -0.900404f, -0.869287f, -0.839100f, -0.809784f, -0.781286f, -0.753554f, -0.726543f, -0.700208f, -0.674509f, -0.649408f, -0.624869f, -0.600861f, -0.577350f, -0.554309f, -0.531709f, -0.509525f, -0.487733f, -0.466308f, -0.445229f, -0.424475f, -0.404026f, -0.383864f, -0.363970f, -0.344328f, -0.324920f, -0.305731f, -0.286745f, -0.267949f, -0.249328f, -0.230868f, -0.212557f, -0.194380f, -0.176327f, -0.158384f, -0.140541f, -0.122785f, -0.105104f, -0.087489f, -0.069927f, -0.052408f, -0.034921f, -0.017455f, -0.000000f, 0.017455f, 0.034921f, 0.052408f, 0.069927f, 0.087489f, 0.105104f, 0.122785f, 0.140541f, 0.158384f, 0.176327f, 0.194380f, 0.212557f, 0.230868f, 0.249328f, 0.267949f, 0.286745f, 0.305731f, 0.324920f, 0.344328f, 0.363970f, 0.383864f, 0.404026f, 0.424475f, 0.445229f, 0.466308f, 0.487733f, 0.509525f, 0.531709f, 0.554309f, 0.577350f, 0.600861f, 0.624869f, 0.649408f, 0.674509f, 0.700208f, 0.726543f, 0.753554f, 0.781286f, 0.809784f, 0.839100f, 0.869287f, 0.900404f, 0.932515f, 0.965689f, 1.000000f, 1.035530f, 1.072369f, 1.110613f, 1.150368f, 1.191754f, 1.234897f, 1.279942f, 1.327045f, 1.376382f, 1.428148f, 1.482561f, 1.539865f, 1.600335f, 1.664279f, 1.732051f, 1.804048f, 1.880726f, 1.962611f, 2.050304f, 2.144507f, 2.246037f, 2.355852f, 2.475087f, 2.605089f, 2.747477f, 2.904211f, 3.077684f, 3.270853f, 3.487414f, 3.732051f, 4.010781f, 4.331476f, 4.704630f, 5.144554f, 5.671282f, 6.313752f, 7.115370f, 8.144346f, 9.514364f, 11.430052f, 14.300666f, 19.081137f, 28.636253f, 57.289962f, 5443926242794615.000000f, -57.289962f, -28.636253f, -19.081137f, -14.300666f, -11.430052f, -9.514364f, -8.144346f, -7.115370f, -6.313752f, -5.671282f, -5.144554f, -4.704630f, -4.331476f, -4.010781f, -3.732051f, -3.487414f, -3.270853f, -3.077684f, -2.904211f, -2.747477f, -2.605089f, -2.475087f, -2.355852f, -2.246037f, -2.144507f, -2.050304f, -1.962611f, -1.880726f, -1.804048f, -1.732051f, -1.664279f, -1.600335f, -1.539865f, -1.482561f, -1.428148f, -1.376382f, -1.327045f, -1.279942f, -1.234897f, -1.191754f, -1.150368f, -1.110613f, -1.072369f, -1.035530f, -1.000000f, -0.965689f, -0.932515f, -0.900404f, -0.869287f, -0.839100f, -0.809784f, -0.781286f, -0.753554f, -0.726543f, -0.700208f, -0.674509f, -0.649408f, -0.624869f, -0.600861f, -0.577350f, -0.554309f, -0.531709f, -0.509525f, -0.487733f, -0.466308f, -0.445229f, -0.424475f, -0.404026f, -0.383864f, -0.363970f, -0.344328f, -0.324920f, -0.305731f, -0.286745f, -0.267949f, -0.249328f, -0.230868f, -0.212557f, -0.194380f, -0.176327f, -0.158384f, -0.140541f, -0.122785f, -0.105104f, -0.087489f, -0.069927f, -0.052408f, -0.034921f, -0.017455f};
	
	void vect4Mat16Mult(float M[16], float v[4]);
	void mat16Inverse(float M[16]);
	
	float positiveSubtract(float x, float y);
	float clamp(float n, float lo, float hi);
	float clampLow(float n, float lo);
	float clampHigh(float n, float hi);
	float modulo(float x, float y);
	int roundInt(float n);
	int log2(int n);
	int absolute(int n);
	float sign(float n);

	int factorial(int n);
	int combination(int n, int r);
	
	struct Point2Di {
		int x, y;
		Point2Di(int x=0, int y=0) : x(x), y(y) {}
		bool operator==(const Point2Di&) const;
		bool operator!=(const Point2Di&) const;
		Point2Di operator+(const Point2Di&) const;
		Point2Di operator-(const Point2Di&) const;
		Point2Di operator/(float) const;
		Point2Di operator*(float) const;
		Point2Di& operator+=(const Point2Di&);
		Point2Di& operator-=(const Point2Di&);
		Point2Di& operator/=(float);
		Point2Di& operator*=(float);
		bool in(int, int, int, int) const;
	};
	struct Point3Di : Point2Di {
		int z;
		Point3Di(int x=0, int y=0, int z=0) : Point2Di(x, y), z(z) {}
		Point3Di(const Point2Di& p, int z=0) : Point2Di(p), z(z) {}
		bool operator==(const Point3Di&) const;
		bool operator!=(const Point3Di&) const;
		Point3Di operator+(const Point3Di&) const;
		Point3Di operator-(const Point3Di&) const;
		Point3Di operator/(float) const;
		Point3Di operator*(float) const;
		Point2Di xy() const;
	};
	
	struct Point2D {
		float x, y;
		Point2D(const Point2Di& p) : x(p.x), y(p.y) {}
		Point2D(float pArr[]) : x(pArr[0]), y(pArr[1]) {}
		Point2D(float x=0.0, float y=0.0) : x(x), y(y) {}
		Point2D operator-() const;
		Point2D operator+(const Point2D&) const;
		Point2D operator-(const Point2D&) const;
		Point2D operator/(float) const;
		Point2D operator*(float) const;
		Point2D operator*(const Point2D&) const;
		bool operator==(const Point2D&) const;
		bool operator!=(const Point2D&) const;
		Point2D& operator+=(const Point2D&);
		Point2D& operator-=(const Point2D&);
		Point2D& operator/=(float);
		Point2D& operator*=(float);
		bool in(float, float, float, float) const;
		float hypot() const;
		operator Point2Di() const;
		Point2D& normalize();
	};
	struct Point3D : Point2D {
		float z;
		Point3D(float pArr[]) : Point2D(pArr), z(pArr[2]) {}
		Point3D(float x=0.0, float y=0.0, float z=0.0) : Point2D(x, y), z(z) {}
		Point3D(const Point2D& p, float z=0.0) : Point2D(p), z(z) {}
		void set(float x=0.0, float y=0.0, float z=0.0);
		float* toArray(float[]) const;
		Point3D operator-() const;
		Point3D operator+(const Point3D&) const;
		Point3D operator-(const Point3D&) const;
		Point3D operator/(float) const;
		Point3D operator*(float) const;
		Point3D operator*(const Point3D&) const;
		bool operator==(const Point3D&) const;
		bool operator!=(const Point3D&) const;
		Point3D& operator+=(const Point3D&);
		Point3D& operator-=(const Point3D&);
		Point3D& operator/=(float);
		Point3D& operator*=(float);
		float hypot() const;
		Point3D& normalize();
		operator Point3Di();
		Point2D xy() const {return Point2D(x, y);}
		Point2D yz() const {return Point2D(y, z);}
		Point2D zx() const {return Point2D(z, x);}
		Point2D yx() const {return Point2D(y, x);}
		Point2D zy() const {return Point2D(z, y);}
		Point2D xz() const {return Point2D(x, z);}
	};
	struct Point4D : public Point3D {
		float w;
		float* toArray(float[]) const;
		Point4D(float pArr[]) : Point3D(pArr), w(pArr[3]) {}
		Point4D(float x=0.0, float y=0.0, float z=0.0, float w=1.0) : Point3D(x, y, z), w(w) {}
		Point4D(const Point2D& p, float z=0.0, float w=1.0) : Point3D(p, z), w(w) {}
		Point4D(const Point3D& p, float w=1.0) : Point3D(p), w(w) {}
		void set(float x=0.0, float y=0.0, float z=0.0, float w=1.0);
		Point4D operator-() const;
		Point4D operator+(const Point4D&) const;
		Point4D operator-(const Point4D&) const;
		Point4D operator/(float) const;
		Point4D operator*(float) const;
		Point4D operator*(const Point4D&) const;
		bool operator==(const Point4D&) const;
		bool operator!=(const Point4D&) const;
		float hypot() const;
		Point4D& normalize();
		bool in(const Point4D&, const Point4D&) const;
		Point3D xyz() const {return Point3D(x, y, z);}
	};
	
	float squareDist(Point2D A, Point2D B);
	float squareDist(Point3D A, Point3D B);
	float dist(Point2D A, Point2D B);
	float dist(Point3D A, Point3D B);
	int manhattanDist(Point2Di A, Point2Di B);
	int manhattanDist(Point3Di A, Point3Di B);
	
	Point2D rotatePoint(int theta, Point2D P, Point2D O=Point2D(0, 0));
	Point3D rotatePointAlongX(int theta, Point3D P, Point3D O=Point3D(0, 0, 0));
	Point3D rotatePointAlongY(int theta, Point3D P, Point3D O=Point3D(0, 0, 0));
	Point3D rotatePointAlongZ(int theta, Point3D P, Point3D O=Point3D(0, 0, 0));
		
	struct Line2D {
		Point2D A, B;
		Line2D(Point2D A, Point2D B) : A(A), B(B) {}
		float slope();
		float tangent();
	};
	struct Line3D {
		Point3D A, B;
		Line3D(Point3D A, Point3D B) : A(A), B(B) {}
	};
	
	float dist(Line2D L, Point2D P);
	float dist(Line3D L, Point3D P);

	typedef Point2D Vector2D;
	typedef Point3D Vector3D;
	typedef Point4D Vector4D;
	
	Vector2D directionVector(Point2D A, Point2D B);
	Vector2D unitVector(Point2D A, Point2D B);
	
	Vector3D directionVector(Point3D A, Point3D B);
	Vector3D unitVector(Point3D A, Point3D B);
	float dotProduct(Vector3D A, Vector3D B);
	Vector3D crossProduct(Vector3D A, Vector3D B);
	
	Vector4D directionVector(Point4D A, Point4D B);
	Vector4D unitVector(Point4D A, Point4D B);
	float dotProduct(Point4D A, Point4D B);

	typedef vector<Point2Di> Path;
};

#endif