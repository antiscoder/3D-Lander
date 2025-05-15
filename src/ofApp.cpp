#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>

static void explode(glm::vec3 pos, Emitter* em) {
	for (int x = 0; x < 300; x++) {
		Particle child;
		child.birthtime = ofGetElapsedTimeMillis();
		child.lifespan = 1000;
		child.pos = pos;
		child.velocity = glm::vec3(RandomFloat(-3000, 3000), RandomFloat(-3000, 3000), RandomFloat(-3000, 3000));
		child.scale = glm::vec3(0.15, 0.15, 0.15);
		em->sys->add(child);
	}
}

// (3) Rocket exhaust uses particle emitter
static void thrust(glm::vec3 pos, Emitter* em) {
	for (int x = 0; x < 10; x++) {
		Particle child;
		child.radius = 1;
		child.birthtime = ofGetElapsedTimeMillis();
		child.lifespan = 50;
		child.pos = pos;
		child.velocity = glm::vec3(RandomFloat(-30, 30), RandomFloat(-300, 0), RandomFloat(-30, 30));
		child.scale = glm::vec3(0.1, 0.1, 0.1);
		em->sys->add(child);
	}
}

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);
	ofSetVerticalSync(true);
    cam.enableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	// create sliders for testing
	//
	gui.setup();
    
    // Right now no toggle, have to add later
    gui.add(altitudeLabel.setup("Altitude AGL", "0.00"));
    gui.add(fuelLabel.setup("Fuel (s)", ofToString((int)fuel)));
    
//	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	bHide = false;

	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);
    
    stars.reserve(200);
      for(int i = 0; i < 200; ++i){
        stars.emplace_back(ofRandomWidth(), ofRandomHeight());
      }
    
    lander.loadModel("geo/lander.obj");
    lander.setScaleNormalization(false);
    lander.setPosition(0,50, 0);
	shipVelocity = 0;
	shipAcceleration = -(1.625 / std::pow(ofGetFrameRate(), 2));
	shipVelocityX = 0;
	shipAccelerationX = (1 / std::pow(ofGetFrameRate(), 2));
	shipVelocityZ = 0;
	shipAccelerationZ = (1 / std::pow(ofGetFrameRate(), 2));
    bLanderLoaded = true;
    
    //mountains
    landingZones.push_back(glm::vec3(50, 0.2, -179));
    //behind mountains
    landingZones.push_back(glm::vec3(-180, 0.2, 154));
    //middle
    landingZones.push_back(glm::vec3(0, 0.2, 20));
    
	shooter = new AgentEmitter();
	shooter->emitterVelocity = shipVelocity;
	shooter->emitterAcceleration = shipAcceleration;
	shooter->drawable = true;

	shooter->pos = lander.getPosition();

	shooter->start();

	//  Create Octree for testing.
	//
	octree.create(mars.getMesh(0), 20);
	
//	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    float dt = ofGetLastFrameTime();
    
    ofVec3f min = lander.getSceneMin() + lander.getPosition();
    ofVec3f max = lander.getSceneMax() + lander.getPosition();

	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	vector<Box> collisions;
	octree.intersect(bounds, octree.root, collisions);
	shooter->update();
	if (landingStarted) {
		if (collisions.size() < 10) {
			lander.setScale(1, 1, 1);
			lander.setPosition(lander.getPosition().x, lander.getPosition().y + shipVelocity, lander.getPosition().z);
            
            // (1) Thrust upward with spacebar and fuel system
			if (keymap[32] && fuel > 0.0f) {
				shipVelocity += (20.0 / std::pow(ofGetFrameRate(), 2));
				thrust(lander.getPosition(), shooter);
				fuelTimer += 1.0f / ofGetFrameRate();
                // (1) Fuel decrementation
				if (fuelTimer >= 1.0f) {
					fuelTimer -= 1.0f;
					fuel = std::max(0.0f, fuel - 1.0f);
					fuelLabel = ofToString((int)fuel);
				}
			}
            
            // (1) Ship maneuvering
			if (keymap[OF_KEY_LEFT]) {
				shipVelocityX -= shipAccelerationX;
			}
			if (keymap[OF_KEY_RIGHT]) {
				shipVelocityX += shipAccelerationX;
			}
			if (keymap[OF_KEY_UP]) {
				shipVelocityZ -= shipAccelerationZ;
			}
			if (keymap[OF_KEY_DOWN]) {
				shipVelocityZ += shipAccelerationZ;
			}
			shipVelocity += shipAcceleration;
			
            // Turbulence
            glm::vec3 P = lander.getPosition();
            float turbulenceX = ofRandom(-0.05f, 0.05f);
            float turbulenceZ = ofRandom(-0.05f, 0.05f);
            lander.setPosition(
                P.x + shipVelocityX + turbulenceX,
                P.y + shipVelocity,
                P.z + shipVelocityZ + turbulenceZ
            );

            shooter->pos = lander.getPosition();
		}
		else {
			cout << shipVelocity << endl;
			if (std::abs(shipVelocity) > 0.015) {
				explode(lander.getPosition(), shooter);
                explosionVelocity = glm::vec3(
                        ofRandom(-150, 150),
                        ofRandom(200, 300),
                        ofRandom(-150, 150)
                    );
                explosionActive = true;
                landingStarted = false;
                gameOver = true;
                showGameOverText = true;
			}
		}
	}
    
    if (explosionActive) {
        glm::vec3 pos = lander.getPosition();
        explosionVelocity += glm::vec3(0, -0.2, 0);
        pos += explosionVelocity * ofGetLastFrameTime();
        lander.setPosition(pos.x, pos.y, pos.z);
        lander.setRotation(0, ofRandom(-5, 5), 0, 1, 0);
    }

    
    if (bResolveCollision) {
        glm::vec3 pos = lander.getPosition();
        pos += collisionDirection * collisionSpeed;
        lander.setPosition(pos.x, pos.y, pos.z);
       
        
        if (collisions.size() < 10) {
            bResolveCollision = false;
        }
    }
    
    else if (!bResolveCollision && collisions.size() >= 10) {
        float impactForce = std::abs(shipVelocity);
        
        if (impactForce <= 0.015f) {
            glm::vec3 landerPos = lander.getPosition();
            for (auto& zone : landingZones) {
                if (glm::distance(landerPos, zone) < landingZoneSize) {
                    gameWin = true;
                    landingStarted = false;
                    return;
                }
            }
        }
        // Apply a damped upward bounce impulse
        glm::vec3 bounce = glm::vec3(0, impactForce * 1.2f, 0);
        collisionDirection = bounce;
        bResolveCollision = true;
        cout << "Soft collision: impulse applied\n";

        // Reset velocities for stability
        shipVelocity = 0;
        shipVelocityX = 0;
        shipVelocityZ = 0;
    }
    
    // (2) AGL telemetry sensor and toggling
    if (bShowTelemetry) {
        glm::vec3 origin = lander.getPosition();
        Ray downRay(Vector3(origin.x, origin.y, origin.z), Vector3(0, -1, 0));
        TreeNode hitNode;
        float altitude = 0;
        if (octree.intersect(downRay, octree.root, hitNode)) {
            auto v = octree.mesh.getVertex(hitNode.points[0]);
            altitude = origin.y - v.y;
        }
        altitudeLabel = ofToString(altitude, 2);
    } else {
        altitudeLabel = "OFF";
    }
        
    // Different camera angles
    glm::vec3 L = lander.getPosition();
    switch(currentCam) {
        case FREE_CAM:
            break;
        case TRACK_CAM:
            cam.disableMouseInput();
            cam.setPosition(L + glm::vec3(0,5,10));
            cam.lookAt(L);
            break;
        case COCKPIT_CAM:
            cam.disableMouseInput();
            glm::vec3 forward = glm::normalize(
            (lander.getModelMatrix() * glm::vec4(0,0,1,0))
                                               );
            cam.setPosition(L);
            cam.lookAt(L + forward);
            break;
    }
    lander.setRotation(0, landerRotation, 0, 1, 0);
}

//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(ofColor::black);
    
    ofDisableDepthTest();
    glDepthMask(false);
    if (!bHide) gui.draw();
    drawStarfield();
    glDepthMask(true);
    ofEnableDepthTest();
    
	ofSetColor(ofColor::white);

	cam.begin();
	ofPushMatrix();
    
    for (auto &zone : landingZones) {
        ofSetColor(ofColor::green);
        ofDrawBox(zone, landingZoneSize, 0.2, landingZoneSize);
    }
    
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}
			shooter->draw();

			if (bLanderSelected) {

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));



	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}


	// recursively draw octree
	//
	ofDisableLighting();
	//int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		//cout << "num leaf: " << octree.numLeaf << endl;
    }
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
//		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}
    
	ofPopMatrix();
	cam.end();
    
    if (showGameOverText) {
        ofSetColor(ofColor::red);
        ofDrawBitmapString("YOU LOSE!\nPress R to Restart", ofGetWidth()/2 - 60, ofGetHeight()/2);
    }
    
    if (gameWin) {
        ofSetColor(ofColor::green);
        ofDrawBitmapString("YOU WIN!\nPress R to Restart", ofGetWidth()/2 - 60, ofGetHeight()/2);
    }
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
    case 'C':
        if(currentCam == FREE_CAM) {
            currentCam  = lastFixedCam;
            cam.disableMouseInput();
        }
        else {
            lastFixedCam = currentCam;
            currentCam   = FREE_CAM;
            cam.enableMouseInput();
        }
        break;
	case 'c':
        if(currentCam == TRACK_CAM) {
            currentCam = COCKPIT_CAM;
        }
        else {
            currentCam = TRACK_CAM;
        }
        lastFixedCam = currentCam;
        cam.disableMouseInput();
        break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
        //		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'R':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
    case 'g':
        bShowTelemetry = !bShowTelemetry;
        break;
	case '1':
		landingStarted=true;
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	}

    if (key == 'a') {
        landerRotation -= rotationSpeed;
    }
    if (key == 'd') {
        landerRotation += rotationSpeed;
    }
    
    if ((gameOver || gameWin) && key == 'r') {
        restartGame();
    }

	keymap[key] = true;
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}
	keymap[key] = false;
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
    if (currentCam != FREE_CAM) return;
    
	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
            cam.disableMouseInput();
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	float startTime = ofGetElapsedTimef() * 1000;
	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
    if (currentCam != FREE_CAM) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);


	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
    if (currentCam == FREE_CAM) {
        cam.enableMouseInput();
    }
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {
    cam.reset();
    cam.setTarget(lander.getPosition());
}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (lander.loadModel(dragInfo.files[0])) {
		lander.setScaleNormalization(false);
//		lander.setScale(.1, .1, .1);
	//	lander.setPosition(point.x, point.y, point.z);
		lander.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//cout << "Mesh Count: " << lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		//cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

// Draws stars for 2D image background
void ofApp::drawStarfield() {
  ofSetColor(255);
  for(auto &p : stars) {
    ofDrawCircle(p.x, p.y, 1);
  }
}

void ofApp::restartGame() {
    gameOver = false;
    gameWin = false;
    showGameOverText = false;
    explosionActive = false;
    shipVelocity = 0;
    shipVelocityX = 0;
    shipVelocityZ = 0;
    fuel = 120.0f;
    fuelTimer = 0.0f;
    lander.setPosition(0, 50, 0); // or initial spawn point
    lander.setScale(1, 1, 1);
    shooter->sys->particles.clear(); // clear old particles
    landingStarted = false;
}
