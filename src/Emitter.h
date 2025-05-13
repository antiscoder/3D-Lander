#pragma once

#include "ofMain.h"
#include "Shape.h"
#include "Particle.h"

//
//  Manages all Sprites in a system.  You can create multiple systems
//
class ParticleList {
public:
	void add(Particle);
	void remove(int);
	void update();
	void draw();
	vector<Particle> particles;
};


//  General purpose Emitter class for emitting sprites
//  This works similar to a Particle emitter
//
class Emitter : public Shape {
public:
	Emitter();
	void init();
	void draw();
	void start();
	void stop();
	void setLifespan(float);
	void setVelocity(const glm::vec3 v);
	void setRate(float);
	void update();
	

	// virtuals - can overloaded
	virtual void moveParticle(Particle*);
	virtual void spawnParticle();
	virtual bool inside(glm::vec3 p) {
		glm::vec3 s = glm::inverse(getTransform()) * glm::vec4(p, 1);
		return (s.x > -width / 2 && s.x < width / 2 && s.y > -height / 2 && s.y < height / 2);
	}


	ParticleList *sys;
	float rate;
	glm::vec3 velocity;
	float lifespan;
	bool started;
	bool drawable;
	float width, height;
	float emitterVelocity;
	float emitterAcceleration;
	float emitterDamping;
};

