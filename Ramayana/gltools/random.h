#ifndef __RANDOM_H
#define __RANDOM_H

#include <cmath>
#include <vector>

using namespace std;

namespace math {

	unsigned int choice();
	float choice(float min, float max);
	template <typename DT> DT& choice(DT list[], int length) {
		return list[choice() % length];
	}
	template <typename DT> DT choice(vector<DT> vec) {
		return vec[choice() % vec.size()];
	}
	template <typename DT> DT& choice(DT list[], float prob[], int length) {
		float h = 0.0;
		for (int i = 0; i<length; i++)
			h += prob[i];
		for (int i = 0; i<length; i++)
			prob[i] /= h;
		float c = choice(0.0, 1.0);
		for (int i = 0; i<length; i++) {
			c -= prob[i];
			if (c <= 0.0)
				return list[i];
		}
		return list[0];
	}
	template <typename DT> DT choice(vector<DT> vec, float prob[]) {
		return choice(vec.data(), prob, vec.size());
	}
	template <typename DT> DT choice(vector<DT> vec, vector<float> prob) {
		return choice(vec.data(), prob.data(), vec.size());
	}

	bool satisfiesInProbability(float p);

#define randomArrayElement(a) (a[choice()%arrayLength(a)])
#define randomVectorElement(v) (v[choice()%v.size()])


	class RandomNumberGenerator {
		int m_w, m_z;
		RandomNumberGenerator();
		RandomNumberGenerator(int seed);
		int nextInt();
		float nextFloat();
	};
}

#endif
