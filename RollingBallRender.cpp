#include <vector>
#include <iostream>
#include <PxPhysicsAPI.h>

#include "Header/Utils/SnippetCamera.h"
#include "Header/Utils/SnippetRender.h"
#include "glut.h"

using namespace physx;

extern void initPhysics(bool interactive);
extern void stepPhysics(bool interactive);
extern void cleanupPhysics(bool interactive);
extern void keyPress(unsigned char key, const PxTransform& camera);
extern void MoveBallToFrontPoisiton();


extern bool GAME_OVER;
extern bool GAME_START;
const char* GAME_OVER_TEXT = "GAME OVER";
const int GAME_OVER_LENGTH = strlen(GAME_OVER_TEXT);

namespace
{
	Snippets::Camera* sCamera;

	void motionCallback(int x, int y)
	{
		sCamera->handleMotion(x, y);
	}

	void keyboardCallback(unsigned char key, int x, int y)
	{
		if (key == 27)
			exit(0);

		if (!sCamera->handleKey(key, x, y))
			keyPress(key, sCamera->getTransform());
	}

	void mouseCallback(int button, int state, int x, int y)
	{
		sCamera->handleMouse(button, state, x, y);
	}

	void idleCallback()
	{
		glutPostRedisplay();
	}

	void renderCallback()
	{
		stepPhysics(true);
		if (GAME_START)
		{
			sCamera->goFront();
			MoveBallToFrontPoisiton();
		}
		Snippets::startRender(sCamera->getEye(), sCamera->getDir());
		Snippets::renderText(10, 10, "Press B to build the ball,Press G to go", 40);
		PxScene* scene;
		PxGetPhysics().getScenes(&scene, 1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
			//ÑÕÉ«¸Ä±ä
			Snippets::renderActors(&actors[0], static_cast<PxU32>(1), true, PxVec3(0.5f, 0.5f, 0.7f)); 
			Snippets::renderActors(&actors[1], static_cast<PxU32>(actors.size() - 1), true, PxVec3(0.8f, 0.8f, 0.5f));
		}
		if (GAME_OVER)
		{
			Snippets::renderGameOver(GAME_OVER_TEXT, GAME_OVER_LENGTH);
		}
		Snippets::finishRender();
	}

	void exitCallback(void)
	{
		delete sCamera;
		cleanupPhysics(true);
	}
}

void renderLoop()
{
	sCamera = new Snippets::Camera(PxVec3(-19.0f, 8.0f, 9.3f), PxVec3(0.0f, -0.6f, -1.3f));

	Snippets::setupDefaultWindow("RollingBall");
	Snippets::setupDefaultRenderState();

	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);
	glutKeyboardFunc(keyboardCallback);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	motionCallback(0, 0);

	atexit(exitCallback);

	initPhysics(true);
	glutMainLoop();
}