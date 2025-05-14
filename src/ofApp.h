#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "Emitter.h"
#include "Shape.h"

// added inline to fix linker
inline float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

class Agent : public Particle {
public:
	Agent() : Particle(){
//		Particle::Particle();
		//		cout << "in Agent Constuctor" << endl;
	}
};

class AgentEmitter : public Emitter {
public:
    void spawnSprite(){
//	void AgentEmitter::spawnSprite() {
		//		cout << "in AgentEmitter::spawnSprite" << endl;
		Agent particle;
		particle.velocity = velocity;
		particle.lifespan = lifespan;
		particle.pos = pos;
		particle.birthtime = ofGetElapsedTimeMillis();
		sys->add(particle);
	}
	void moveSprite(Particle* particle) {
		Emitter::moveParticle(particle);

		// Align path of travel to velocity 
		//
		glm::vec3 v = glm::normalize(particle->velocity);
		glm::vec3 u = glm::vec3(0, -1, 0);
		float a = glm::orientedAngle(u, v, glm::vec3(0, 0, 1));
		particle->rot = glm::degrees(a);
	}
};


class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
        void drawStarfield();
        void restartGame();
    
        enum CamMode { FREE_CAM, TRACK_CAM, COCKPIT_CAM };
        CamMode currentCam = FREE_CAM;
        CamMode lastFixedCam = TRACK_CAM;

		ofEasyCam cam;
		ofxAssimpModelLoader mars, lander;
		ofLight light;
		Box boundingBox, landerBounds;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;
        
        ofxPanel gui;
        ofxLabel altitudeLabel;
    ofxLabel fuelLabel;
//		ofxIntSlider numLevels;
//		ofxToggle timingInfo;
    
        bool gameOver = false;
        bool showGameOverText = false;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
        bool bShowTelemetry = false;
        bool bShipLightOn = false;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;
        ofLight shipLight;

		Emitter* shooter = NULL;

		vector<Box> bboxList;

		const float selectionRange = 4.0;
        
		bool bResolveCollision = false;
		glm::vec3 collisionDirection = glm::vec3(0, 0, 0);
		float collisionSpeed = 0.1;
    
        glm::vec3 explosionVelocity;
        bool explosionActive = false;

		float shipVelocity = 0.0;
		float shipAcceleration = 0.0;
		float shipVelocityX = 0.0;
		float shipAccelerationX = 0.0;
		float shipVelocityZ = 0.0;
		float shipAccelerationZ = 0.0;
        float fuel = 120.0f;
        float fuelTimer = 0.0f;
    
		bool landingStarted = false;
    
        vector<ofPoint> stars;

		map<int, bool> keymap;
};
