#include <vector>
#include <iostream>
#include <PxPhysicsAPI.h>
#include <fstream>

#include "Header/Utils/SnippetCamera.h"
#include "Header/Utils/SnippetRender.h"
#include "glut.h"
#include "SoundUtils.h"
using namespace physx;

extern void initPhysics(bool interactive);
extern void stepPhysics(bool interactive);
extern void cleanupPhysics(bool interactive);
extern void keyPress(unsigned char key, const PxTransform& camera);
extern void MoveBallToFrontPoisiton(PxReal speed);
extern void tryIncreaseSpeed();

extern PxReal runSpeed;
extern bool GAME_OVER;
extern bool GAME_START;
const char* GAME_OVER_TEXT = "GAME OVER";
const int GAME_OVER_LENGTH = strlen(GAME_OVER_TEXT);
extern int scoreValue; //游戏得分
int lastHighestScore;//排行榜分数
char s[100]; //存放游戏得分数的char数组
char s1[100];

namespace
{
	Snippets::Camera* sCamera;

	/*void motionCallback(int x, int y)
	{
		sCamera->handleMotion(x, y);
	}*/

	void keyboardCallback(unsigned char key, int x, int y)
	{
		if (key == 27)
			exit(0);

		if (!sCamera->handleKey(key, x, y))
			keyPress(key, sCamera->getTransform());
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
			sCamera->goFront(runSpeed);
			MoveBallToFrontPoisiton(runSpeed);
			tryIncreaseSpeed();
		}
		Snippets::startRender(sCamera->getEye(), sCamera->getDir());

		if (scoreValue > lastHighestScore)
		{
			lastHighestScore = scoreValue;
		}

		//历史最高分
		Snippets::renderText(10, 475, "HighestScore: ", 15);
		snprintf(s1, sizeof(s), "%d", lastHighestScore);
		Snippets::renderText(200, 475, s1, strlen(s1));

		//按键说明
		Snippets::renderText(10, 10, "Press G to go,Press A to left,Press D to right", 48);
	
		//当前游戏得分
		Snippets::renderText(10, 450,"Score: ", 8);
		snprintf(s, sizeof(s), "%d", scoreValue);
		Snippets::renderText(100, 450, s, strlen(s));
		
		PxScene* scene;
		PxGetPhysics().getScenes(&scene, 1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
			//颜色改变
			Snippets::renderActors(&actors[0], static_cast<PxU32>(1), true, PxVec3(0.5f, 0.5f, 0.7f)); 
			Snippets::renderActors(&actors[1], static_cast<PxU32>(actors.size() - 1), true, PxVec3(0.8f, 0.8f, 0.5f));
		}
		if (GAME_OVER)
		{

			Snippets::renderGameOver(GAME_OVER_TEXT, GAME_OVER_LENGTH);

			ofstream out("score.txt");//输出分数到文件
			out << lastHighestScore;
			out.close();
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
	
	ifstream in("score.txt");//读取历史最高分
	in >> lastHighestScore;
	in.close();

	Snippets::setupDefaultWindow("RollingBall");
	Snippets::setupDefaultRenderState();

	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);
	glutKeyboardFunc(keyboardCallback);
	//glutMouseFunc(mouseCallback);
	//glutMotionFunc(motionCallback);
	//motionCallback(0, 0);

	atexit(exitCallback);
	Sound s;
	s.playSound();//播放音乐
	initPhysics(true);
	glutMainLoop();
}