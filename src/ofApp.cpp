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
	bLanderLoaded = false;
	bTerrainSelected = true;
	cam.setDistance(60);
	cam.setNearClip(.1);
	cam.setFov(90);
	ofSetVerticalSync(true);
    cam.enableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();
	ofSetFrameRate(60);

	bumpS.load("sounds/bump.wav");
	crashS.load("sounds/crash.wav");
	shootS.load("sounds/shoot.wav");
	thrustS.load("sounds/thrust.wav");

    initLightingAndMaterials();

	gui.setup();
    gui.add(altitudeLabel.setup("Altitude AGL", "0.00"));
    gui.add(fuelLabel.setup("Fuel (s)", ofToString((int)fuel)));

	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);
    
    stars.reserve(200);
      for(int i = 0; i < 200; ++i){
        stars.emplace_back(ofRandomWidth(), ofRandomHeight());
      }
    
    lander.loadModel("geo/rocket.obj");
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
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    glm::vec3 landerPos = lander.getPosition();
    ofVec3f min = lander.getSceneMin() + landerPos;
    ofVec3f max = lander.getSceneMax() + landerPos;
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	vector<Box> collisions;
	octree.intersect(bounds, octree.root, collisions);
    
	shooter->update();
    
	if (landingStarted) {
		if (collisions.size() < 10) {
			
            bool anyKeyPressed = false;
            
			if (keymap[32] && fuel > 0.0f) {
				shipVelocity += (10.0 / std::pow(ofGetFrameRate(), 2));
				thrust(landerPos, shooter);
				thrustS.setLoop(true);
				if (!thrustS.isPlaying())
					thrustS.play();
				fuelTimer += 1.0f / ofGetFrameRate();
				if (fuelTimer >= 1.0f) {
					fuelTimer -= 1.0f;
					fuel = std::max(0.0f, fuel - 1.0f);
					fuelLabel = ofToString((int)fuel);
				}
                bool anyKeyPressed = true;
			}
            
			if (keymap[OF_KEY_LEFT]) {
				shipVelocityX -= shipAccelerationX;
                bool anyKeyPressed = true;
			}
			if (keymap[OF_KEY_RIGHT]) {
				shipVelocityX += shipAccelerationX;
                bool anyKeyPressed = true;
            }
			if (keymap[OF_KEY_UP]) {
				shipVelocityZ -= shipAccelerationZ;
                bool anyKeyPressed = true;
            }
			if (keymap[OF_KEY_DOWN]) {
				shipVelocityZ += shipAccelerationZ;
                bool anyKeyPressed = true;
            }
			
            if (anyKeyPressed) {
                thrustS.setLoop(true);
                if (!thrustS.isPlaying()) {
                    thrustS.play();
                }
            }
            else {
                thrustS.setLoop(false);
            }
            
			shipVelocity += shipAcceleration;
			
            float turbulenceX = ofRandom(-0.05f, 0.05f);
            float turbulenceZ = ofRandom(-0.05f, 0.05f);
            lander.setPosition(
                landerPos.x + shipVelocityX + turbulenceX,
                landerPos.y + shipVelocity,
                landerPos.z + shipVelocityZ + turbulenceZ
            );

            shooter->pos = landerPos;
		}
		else {
            if (std::abs(shipVelocity) > 0.08) {
				explode(landerPos, shooter);
				crashS.play();
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
        explosionVelocity += glm::vec3(0, -0.2, 0);
        landerPos += explosionVelocity * ofGetLastFrameTime();
        lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
        lander.setRotation(0, ofRandom(-5, 5), 0, 1, 0);
    }

    if (bResolveCollision) {
        landerPos += collisionDirection * collisionSpeed;
        lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
       
        
        if (collisions.size() < 10) {
            bResolveCollision = false;
        }
    }
    else if (!bResolveCollision && collisions.size() >= 10) {
        float impactForce = std::abs(shipVelocity);
        
        if (impactForce <= 0.015f) {
            for (auto& zone : landingZones) {
                if (glm::distance(landerPos, zone) < landingZoneSize) {
                    gameWin = true;
                    landingStarted = false;
                    return;
                }
            }
        }
        collisionDirection = glm::vec3(0, impactForce * 1.2f, 0);
		bumpS.play();
        bResolveCollision = true;
        shipVelocity = 0;
        shipVelocityX = 0;
        shipVelocityZ = 0;
    }
    
    if (bShowTelemetry) {
        Ray downRay(Vector3(landerPos.x, landerPos.y, landerPos.z), Vector3(0, -1, 0));
        TreeNode hitNode;
        float altitude = 0;
        if (octree.intersect(downRay, octree.root, hitNode)) {
            auto v = octree.mesh.getVertex(hitNode.points[0]);
            altitude = landerPos.y - v.y;
        }
        altitudeLabel = ofToString(altitude, 2);
    } else {
        altitudeLabel = "OFF";
    }
        
    switch (currentCam) {
        case FREE_CAM:
            break;

        case TRACK_CAM:
            cam.disableMouseInput();
            cam.setPosition(landerPos + glm::vec3(0, 5, 10));
            cam.lookAt(landerPos);
            break;

        case BOTTOM_CAM: {
            cam.disableMouseInput();
            bDisplayOctree = false;

            glm::vec3 min = glm::vec3(lander.getSceneMin());
            glm::vec3 rocketBottom = landerPos + glm::vec3(0, min.y, 0);
            glm::vec3 cameraPos = rocketBottom + glm::vec3(0, 0.5f, 0);
            glm::vec3 lookAtPos = rocketBottom + glm::vec3(0, -10.0f, 0);

            cam.setPosition(cameraPos);
            cam.lookAt(lookAtPos);
            break;
        }

        case TOP_CAM:
            cam.disableMouseInput();
            cam.setPosition(landerPos + glm::vec3(0, 25, 0));
            cam.lookAt(landerPos);
            break;
    }
    lander.setRotation(0, landerRotation, 0, 1, 0);
    
    if(bShipLightOn) {
        shipLight.setPosition(landerPos.x, landerPos.y - 2, landerPos.z);
        shipLight.enable();
    } else {
        shipLight.disable();
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(ofColor::black);
    
    ofDisableDepthTest();
    glDepthMask(false);
    drawStarfield();
    gui.draw();
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

        }
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	// recursively draw octree
	//
	ofDisableLighting();
	
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
    case 'a':
        landerRotation -= rotationSpeed;
        break;
    case 'd':
        landerRotation += rotationSpeed;
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
            currentCam = BOTTOM_CAM;
        }
		else if (currentCam == BOTTOM_CAM) {
			currentCam = TOP_CAM;
		}
        else {
            currentCam = TRACK_CAM;
        }
        lastFixedCam = currentCam;
        cam.disableMouseInput();
        break;
    case 'r':
        if (gameOver || gameWin) {
            restartGame();
        }
        break;
	case 'f':
		ofToggleFullscreen();
		break;
	case 'l':
        bShipLightOn = !bShipLightOn;
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
    case 'w':
		toggleWireframeMode();
		break;
    case 'g':
        bShowTelemetry = !bShowTelemetry;
        break;
	case '1':
		landingStarted=true;
		break;
	default:
		break;
	}

	keymap[key] = true;
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::keyReleased(int key) {
	keymap[key] = false;
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

		ofVec3f min = lander.getSceneMin() + landerPos;
		ofVec3f max = lander.getSceneMax() + landerPos;

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
    
    ofEnableLighting();

    // Key Light - white light from above
    keyLight.setup();
    keyLight.enable();
    keyLight.setPosition(100, 200, 100);
    keyLight.setDiffuseColor(ofFloatColor(1.0, 1.0, 1.0));
    keyLight.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0));
    keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));

    // Fill Light - softer light from side
    fillLight.setup();
    fillLight.enable();
    fillLight.setPosition(-200, 100, 0);
    fillLight.setDiffuseColor(ofFloatColor(0.3, 0.3, 0.4));
    fillLight.setAmbientColor(ofFloatColor(0.05, 0.05, 0.05));

    // Back Light - white backlight
    backLight.setup();
    backLight.enable();
    backLight.setPosition(0, 100, -200);
    backLight.setDiffuseColor(ofFloatColor(0.2, 0.2, 0.3));
    backLight.setAmbientColor(ofFloatColor(0.05, 0.05, 0.05));

    // Ship Light (off by default)
    shipLight.setup();
    shipLight.setPointLight();
    shipLight.setDiffuseColor(ofFloatColor(1.0, 1.0, 0.9));
    shipLight.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0));
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
    lander.setPosition(0, 50, 0);
    lander.setScale(1, 1, 1);
    shooter->sys->particles.clear();
    landingStarted = false;
}
