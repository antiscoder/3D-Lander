#pragma once

#include "ofMain.h"


// Basic Shape class supporting matrix transformations and drawing.
// 
//
class Shape {
public:
	virtual void draw() {

		// draw a box by defaultd if not overridden
		//
		ofPushMatrix();
		ofMultMatrix(getTransform());
		ofDrawBox(defaultSize);
		ofPopMatrix();
	}

	glm::mat4 getTransform() {
		glm::mat4 modelMatrix = glm::identity<glm::mat4>();
		glm::mat4 T = glm::translate(modelMatrix, glm::vec3(pos));
		glm::mat4 R = glm::rotate(modelMatrix, glm::radians(rot), glm::vec3(0, 0, 1));
		glm::mat4 S = glm::scale(modelMatrix, scale);      // added this scale if you want to change size of object
		return T*R*S;
	}

	glm::vec3 pos;
	float rot = 0.0;    // degrees 
	glm::vec3 scale = glm::vec3(1, 1, 1);
	float defaultSize = 20.0;

	float velocity;
	float accelerationF;
	float accelerationB;
	float damping;
	
};