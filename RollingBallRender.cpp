#include <vector>

#include "PxPhysicsAPI.h"

#include "Header/Utils/SnippetCamera.h"
#include "Header/Utils/SnippetRender.h"
#include "glut.h"

using namespace physx;

extern void initPhysics(bool interactive);
extern void stepPhysics(bool interactive);
extern void cleanupPhysics(bool interactive);
extern void keyPress(unsigned char key, const PxTransform& camera);


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

		Snippets::startRender(sCamera->getEye(), sCamera->getDir());

		PxScene* scene;
		PxGetPhysics().getScenes(&scene, 1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]),nbActors);
			Snippets::renderActors(&actors[0], static_cast<PxU32>(1), true,PxVec3(1.0f,0.0f,1.0f));
			Snippets::renderActors(&actors[1], static_cast<PxU32>(1), true, PxVec3(1.0f, 1.0f, 1.0f));
			Snippets::renderActors(&actors[2], static_cast<PxU32>(1), true, PxVec3(0.7f, 0.9f, 0.9f));
			Snippets::renderActors(&actors[3], static_cast<PxU32>(actors.size()-3), true, PxVec3(0.5f, 0.5f, 0.5f));
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
	sCamera = new Snippets::Camera(PxVec3(-18.0f, 8.0f, 9.0f), PxVec3(-0.2f, -0.6f, -1.3f));
	//Camera eye:左右、上下、前后

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