/*****************************************************************************************************
* Subject                   : Random Functions                                                      *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include "math.h"
#include "random.h"

namespace math {

	int seed() {
		return SDL_GetTicks();
	}
	float randomFloat() {
		return float((seed() + rand()) % RAND_MAX) / float(RAND_MAX);
	}

	float choice(float min, float max) {
		float d = max - min;
		return randomFloat() * d + min;
	}
	unsigned int choice() {
		return randomFloat() * RAND_MAX;
	}

	bool satisfiesInProbability(float p) {
		p = clamp(p, 0.000000001, 1.0);
		float n = choice(0, 1.0);
		return n <= p;
	}


	RandomNumberGenerator::RandomNumberGenerator() {
		m_z = rand();
		m_w = SDL_GetTicks();
	}

	RandomNumberGenerator::RandomNumberGenerator(int seed) {
		m_z = seed;
		m_w = seed + 13;
	}
	
	int RandomNumberGenerator::nextInt() {
		m_z = 36969 * (m_z & 65535) + (m_z >> 16);
		m_w = 18000 * (m_w & 65535) + (m_w >> 16);
		return (m_z << 16) + m_w;
	}
}
