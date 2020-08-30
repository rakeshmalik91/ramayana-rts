#ifndef __PHYSICS_H
#define __PHYSICS_H

#include <vector>
#include <SDL/SDL_mutex.h>
#include "graphics.h"
#include "math.h"

using namespace std;
using namespace graphics;
using namespace math;

namespace physics {
	struct Projectile {
		int s, timeOfFlight;
		Point3D ipos, lastpos, pos, dst;
		float v, weight;
		float hAngle, vAngle, angle;
		bool approx, overHorizon;
		Projectile();
		Projectile(Point3D ipos, float hAngle, float vAngle, float v, float weight, bool approx=false);
		Projectile(Point3D ipos, Point3D dst, float hAngle, float v, float weight, bool overHorizon, bool approx=false);
		void move();
		bool hits(Point3D);
		bool isPossible();
	};
	
	struct Particle {
		int age;
		Point3D transformed_pos;
		float size, dSize;
		int lifespan;
		float rotation, angle;
		Texture2D *texture;
		Color color;
		float initAlpha;
		Projectile motion;
		bool fade;
		Particle();
		Particle(Point3D pos, float size, float dSize, int lifespan, float hAngle, float vAngle, float velocity, float weight, float rotation, bool fade, Color color, Texture2D *texture=NULL);
		void bindTexture();
	};
	
	class ParticleRenderer {
		SDL_mutex *mutex;
		vector<Particle> particles;
	public:
		static int MAX_PARTICLES;
		ParticleRenderer();
		~ParticleRenderer();
		void add(Particle);
		void render(Frustum&);
		void update();
		void clear();
		int getNumberOfParticles() {return particles.size();}
	};
	
	class ParticleEngine {
		Point3D pos;
		ParticleRenderer *pl;
		Texture2D *texture;
		int nTexture;
		int lifespan;
		float rotation;
		Color color;
		Range<float> hAngle, vAngle, size, dSize, velocity, weight;
		int density, delay, waiting;
		bool fade;
	public:
		ParticleEngine();
		void bind(ParticleRenderer*);
		void set(Point3D pos, int lifespan, Texture2D* texture, int nTexture, Range<float> hAngle, Range<float>vAngle, Range<float>size, Range<float> dSize, Range<float> velocity, Range<float> weight, float rotation, Color color, float density);
		void setLifeSpan(int lifespan) {this->lifespan=lifespan;}
		void setTexture(Texture2D* texture, int nTexture) {this->texture=texture; this->nTexture=nTexture;}
		void setPosition(Point3D pos) {this->pos=pos;}
		void setDensity(float);
		float getDensity() const;
		void emit(float dim=1.0);
		void emitTrail(Point3D, int);
		void setFade(bool);
	};
	
}

#endif
