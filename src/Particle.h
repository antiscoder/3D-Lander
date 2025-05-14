#pragma once

#include "Shape.h"


// Base class for a Sprite. Can be instantiated on it's own (deafult)
// or subclassed to create a custom sprite.  Includes Shape transformations.
// If the sprite has no image set on it, then a simple triangle will be drawn.
//

class Particle : public Shape {
public:

	void draw();

	float age() {
		return (ofGetElapsedTimeMillis() - birthtime);
	}

	virtual void update() {}
	

	glm::vec3 velocity = glm::vec3(0, 0, 0);
	float birthtime = 0; // elapsed time in ms
	float lifespan = -1;  //  time in ms
	string name =  "particle";

	int radius = 2;
};

