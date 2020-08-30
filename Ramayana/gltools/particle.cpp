/*****************************************************************************************************
 * Subject                   : Particle Engine Class                                                 *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"

#include "random.h"
#include "particle.h"
#include "algorithm.h"

using namespace algorithm;
using namespace graphics;

namespace physics {

	Particle::Particle() : texture(NULL), color(0, 0, 0, 0), age(0) {}
	Particle::Particle(Point3D pos, float size, float dSize, int lifespan, float hAngle, float vAngle, float velocity, float weight, float rotation, bool fade, Color color, Texture2D *texture) : size(size), dSize(dSize), lifespan(lifespan), color(color), initAlpha(color.a()), texture(texture), rotation(rotation), fade(fade), motion(pos, hAngle, vAngle, velocity, weight, false), angle(0), age(0) {}
	void Particle::bindTexture() {
		if(texture!=NULL) {
			texture->bind();
		} else
			glBindTexture(GL_TEXTURE_2D, 0);
	}

	int __particle_comparator(const Particle &p1, const Particle &p2, void *unused) {
		if(p1.transformed_pos.z<p2.transformed_pos.z)
			return -1;
		else if(p1.transformed_pos.z>p2.transformed_pos.z)
			return 1;
		else
			return 0;
	}
	
	ParticleRenderer::ParticleRenderer() {
		mutex=SDL_CreateMutex();
	}
	ParticleRenderer::~ParticleRenderer() {
		SDL_DestroyMutex(mutex);
	}
	int ParticleRenderer::MAX_PARTICLES=10000;
	void ParticleRenderer::add(Particle p) {
		if(particles.size()>MAX_PARTICLES)
			return;
		SDL_mutexP(mutex);
		particles.push_back(p);
		SDL_mutexV(mutex);
	}
	void ParticleRenderer::clear() {
		SDL_mutexP(mutex);
		particles.clear();
		SDL_mutexV(mutex);
	}
	void ParticleRenderer::render(Frustum &frustum) {
		if(particles.empty())
			return;

		SDL_mutexP(mutex);

		//Transformation
		GLfloat trans_mat[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, trans_mat);
		for(vector<Particle>::iterator p=particles.begin(); p!=particles.end(); p++) {
			float pArr[4]={0, 0, 0, 1};
			p->motion.pos.toArray(pArr);
			vect4Mat16Mult(trans_mat, pArr);
			p->transformed_pos=Point3D(pArr);
		}

		//sort
		quicksort(particles.data(), particles.size(), &__particle_comparator);

		//render, update
		float colorBuffer[4];
		glPushMatrix();
		glLoadIdentity();
		Texture2D* boundTexture=NULL;
		glBegin(GL_QUADS);
		for(vector<Particle>::iterator p=particles.begin(); p!=particles.end(); p++) {
			if(!frustum.sphereInFrustum(p->motion.pos, p->size))
				continue;

			//Texture
			if(boundTexture!=p->texture) {
				glEnd();
				p->bindTexture();
				boundTexture=p->texture;
				glBegin(GL_QUADS);
			}

			//render
			glColor4fv(p->color.rgba().toArray(colorBuffer));
			glTexCoord2f(0, 1);		glVertex3f(p->transformed_pos.x-p->size, p->transformed_pos.y+p->size, p->transformed_pos.z);
			glTexCoord2f(0, 0);		glVertex3f(p->transformed_pos.x-p->size, p->transformed_pos.y-p->size, p->transformed_pos.z);
			glTexCoord2f(1, 0);		glVertex3f(p->transformed_pos.x+p->size, p->transformed_pos.y-p->size, p->transformed_pos.z);
			glTexCoord2f(1, 1);		glVertex3f(p->transformed_pos.x+p->size, p->transformed_pos.y+p->size, p->transformed_pos.z);
		}
		glEnd();
		glPopMatrix();
		
		SDL_mutexV(mutex);
	}
	void ParticleRenderer::update() {
		if(particles.empty())
			return;

		SDL_mutexP(mutex);

		for(vector<Particle>::iterator p=particles.begin(); p!=particles.end(); ) {
			if(p->age >= p->lifespan)
				p=particles.erase(p);
			else {
				if(p->motion.v!=0) {
					p->motion.move();
					p->motion.pos=rotatePointAlongZ(p->angle, p->motion.pos, p->motion.ipos);
					p->angle=modulo(p->angle+p->rotation, 360);
				}
				p->size+=p->dSize;
				if(p->fade)
					p->color.a(p->initAlpha*(float)(p->lifespan-p->motion.s)/(float)p->lifespan);
				p->age++;
				p++;
			}
		}

		SDL_mutexV(mutex);
	}

	ParticleEngine::ParticleEngine() : pl(NULL), texture(NULL), nTexture(0), waiting(0), fade(true) {}
	void ParticleEngine::bind(ParticleRenderer *pl) {
		this->pl=pl;
	}
	void ParticleEngine::set(Point3D pos, int lifespan, Texture2D* texture, int nTexture, Range<float> hAngle, Range<float>vAngle, Range<float>size, Range<float> dSize, Range<float> velocity, Range<float> weight, float rotation, Color color, float density) {
		this->pos=pos;
		this->lifespan=lifespan;
		this->texture=texture;
		this->nTexture=nTexture;
		this->hAngle=hAngle;
		this->vAngle=vAngle;
		this->size=size;
		this->dSize=dSize;
		this->velocity=velocity;
		this->weight=weight;
		this->rotation=rotation;
		this->color=color;
		if(density>=1) {
			this->density=density;
			this->delay=0;
		} else {
			this->density=1;
			this->delay=1.0/density;
		}
		waiting=choice(0, delay);
	}
	void ParticleEngine::setDensity(float density) {
		if(density>=1) {
			this->density=density;
			this->delay=0;
		} else {
			this->density=1;
			this->delay=1.0/density;
		}
		waiting=choice(0, delay);
	}
	float ParticleEngine::getDensity() const {
		return density;
	}
	void ParticleEngine::emit(float dim) {
		if(pl==NULL)
			throw Exception("Particle Engine unbound");
		if(!waiting) {
			for(int i=0; i<density; i++) {
				pl->add(Particle(
					pos, 
					choice(size.low, size.high), choice(dSize.low, dSize.high), 
					lifespan, 
					choice(hAngle.low, hAngle.high), choice(vAngle.low, vAngle.high), 
					choice(velocity.low, velocity.high), 
					choice(weight.low, weight.high), 
					rotation,
					fade,
					Color(color.r(), color.g(), color.b(), color.a()*dim), 
					texture+roundInt(choice(0, nTexture-1))));
			}
			waiting=choice(0, delay);
		} else {
			waiting--;
		}
	}
	void ParticleEngine::emitTrail(Point3D p2, int n) {
		Point3D initpos=pos;
		Point3D p1=pos;
		for(int d=0; d<n; d++) {
			pos=(p1*d+p2*(n-d))/n;
			emit(1.0-((float)(n-d)/(float)(n*lifespan)));
		}
		pos=initpos;
	}
	void ParticleEngine::setFade(bool fade) {
		this->fade=fade;
	}
};
