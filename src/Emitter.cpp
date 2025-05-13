#include "ofApp.h"
//----------------------------------------------------------------------------------
//
// This example code demonstrates the use of an "Emitter" class to emit Sprites
// and set them in motion. The concept of an "Emitter" is taken from particle
// systems (which we will cover next week).
//
// The Sprite class has also been upgraded to include lifespan, velocity and age
// members.   The emitter can control rate of emission and the current velocity
// of the particles. In this example, there is no acceleration or physics, the
// sprites just move simple frame-based animation.
//
// The code shows a way to attach images to the sprites and optional the
// emitter (which is a point source) can also have an image.  If there are
// no images attached, a placeholder rectangle is drawn.
// Emitters  can be placed anywhere in the window. In this example, you can drag
// it around with the mouse.
//
// OF has an add-in called ofxGUI which is a very simple UI that is useful for
// creating sliders, buttons and fields. It is not recommended for commercial 
// game development, but it is useful for testing.  The "h" key will hide the GUI
// 
// If you want to run this example, you need to use the ofxGUI add-in in your
// setup.
//
//
//  Kevin M. Smith - CS 134 SJSU




//  Add a Sprite to the Sprite System
//
void ParticleList::add(Particle s) {
	particles.push_back(s);
}

// Remove a sprite from the sprite system. Note that this function is not currently
// used. The typical case is that sprites automatically get removed when the reach
// their lifespan.
//
void ParticleList::remove(int i) {
	particles.erase(particles.begin() + i);
}


//  Update the SpriteSystem by checking which sprites have exceeded their
//  lifespan (and deleting).  Also the sprite is moved to it's next
//  location based on velocity and direction.
//
void ParticleList::update() {

	if (particles.size() == 0) return;
	vector<Particle>::iterator s = particles.begin();
	vector<Particle>::iterator tmp;

	// check which sprites have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, use an iterator.
	//
	while (s != particles.end()) {
		if (s->lifespan != -1 && s->age() > s->lifespan || s->lifespan == -2) {
			//			cout << "deleting sprite: " << s->name << endl;
			tmp = particles.erase(s);
			s = tmp;
		}
		else s++;
	}

	//  Move sprite
	//
	for (int i = 0; i < particles.size(); i++) {
		particles[i].pos += particles[i].velocity / ofGetFrameRate();
	}
}

//  Render all the sprites
//
void ParticleList::draw() {
	for (int i = 0; i < particles.size(); i++) {
		particles[i].draw();
	}
}



Emitter::Emitter() {
	sys = new ParticleList();
	init();
}


void Emitter::init() {
	lifespan = 60000;    // default milliseconds
	started = true;

	//lastSpawned = 0;
	rate = 1;    // sprites/sec
	velocity = ofVec3f(100, 100, 0);
	drawable = false;
	width = 50;
	height = 50;
}



//  Draw the Emitter if it is drawable. In many cases you would want a hidden emitter
//
//
void Emitter::draw() {
	sys->draw();
}

//  Update the Emitter. If it has been started, spawn new sprites with
//  initial velocity, lifespan, birthtime.
//  initial velocity, lifespan, birthtime.
//
void Emitter::update() {
	if (!started) return;

	/*
	float time = ofGetElapsedTimeMillis();

	if ((time - lastSpawned) > (1000.0 / rate)) {

		// call virtual to spawn a new sprite
		//
		spawnSprite();
		lastSpawned = time;
	}
	*/


	// update sprite list
	//
	if (sys->particles.size() == 0) return;
	vector<Particle>::iterator s = sys->particles.begin();
	vector<Particle>::iterator tmp;

	// check which sprites have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, use an iterator.
	//
	while (s != sys->particles.end()) {
		if (s->lifespan != -1 && s->age() > s->lifespan) {
			//			cout << "deleting sprite: " << s->name << endl;
			tmp = sys->particles.erase(s);
			s = tmp;
		}
		else s++;
	}

	
	for (int i = 0; i < sys->particles.size(); i++) {
		sys->particles[i].rot += .1;
		sys->particles[i].pos += sys->particles[i].velocity / ofGetFrameRate();
	}
	
}

// virtual function to move sprite (can be overloaded)
//
void Emitter::moveParticle(Particle* Particle) {
	Particle->pos += Particle->velocity / ofGetFrameRate();
}


// virtual function to spawn sprite (can be overloaded)
//
void Emitter::spawnParticle() {
	Particle particle;
	particle.velocity = velocity;
	particle.lifespan = lifespan;
	particle.pos = pos;
	particle.birthtime = ofGetElapsedTimeMillis();
	sys->add(particle);
}

// Start/Stop the emitter.
//
void Emitter::start() {
	started = true;
	//lastSpawned = ofGetElapsedTimeMillis();
}

void Emitter::stop() {
	started = false;
}


void Emitter::setLifespan(float life) {
	lifespan = life;
}

void Emitter::setVelocity(const glm::vec3 v) {
	velocity = v;
}

void Emitter::setRate(float r) {
	rate = r;
}
