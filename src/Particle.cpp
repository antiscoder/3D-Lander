#include "Particle.h"


//
	// inside - check if point is inside player (can be an image or TriangleShape if no image)
	//

void Particle::draw() {
		ofSetColor(ofColor::white);
		ofPushMatrix();
		ofMultMatrix(getTransform());
		ofDrawSphere(glm::vec3(0, 0, 0), radius);
		ofPopMatrix();
}

