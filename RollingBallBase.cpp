#include <ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include <PxPhysicsAPI.h>
#include <extensions/PxDefaultAllocator.h>
#include "Header/Utils/SnippetPVD.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

typedef struct timeval {
	long tv_sec;
	long tv_usec;
} timeval;

int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;

PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;

PxMaterial* gMaterial = NULL;
PxMaterial* ballMaterial = NULL;

PxPvd* gPvd = NULL;

PxReal stackZ = 12.0f;

bool GAME_OVER = false;

bool GAME_START = false;

int scoreValue = 0; //游戏得分

timeval last;
timeval current;

PxReal runSpeed = 0.6;

extern void renderLoop();

int obstaclePosition[2] = { 0,-5 };

PxRigidDynamic* ballReference = NULL;

unsigned long long MoveFrontDistance = 0;

static const PxFilterData collisionGroupBall(1, 1, 1, 1);
static const PxFilterData collisionGroupObstacle(1, 1, 1, 1);

//碰撞过滤
PxFilterFlags BallFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	// trigger the contact callback for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		//如果球和障碍物碰撞的话游戏结束
		GAME_OVER = true;
		GAME_START = false;
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}
	return PxFilterFlag::eDEFAULT;
}

void createTrack(const PxTransform& t, PxReal halfExtent)  //创建轨道
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, 0.1, halfExtent * 3), *gMaterial);
	PxTransform localTm(PxVec3(PxReal(0) - PxReal(8.2), PxReal(1), 0) * halfExtent);
	PxRigidStatic* body = gPhysics->createRigidStatic(t.transform(localTm));
	body->attachShape(*shape);
	gScene->addActor(*body);
	shape->release();
}

void createObstacle(const PxTransform& t, PxReal halfExtent)//创建障碍物
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent * 0.6, halfExtent * 1.1, halfExtent * 0.6), *gMaterial);
	shape->setSimulationFilterData(collisionGroupObstacle);//障碍物碰撞标识
	PxTransform localTm(PxVec3(PxReal(0) - PxReal(8.2), PxReal(1), 0) * halfExtent);
	PxRigidStatic* body = gPhysics->createRigidStatic(t.transform(localTm));
	body->attachShape(*shape);
	gScene->addActor(*body);
	shape->release();
}

void createRailing(const PxTransform& t, PxReal halfExtent)//创建轨道两边栏杆
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.6, halfExtent * 0.6, 6), *gMaterial);
	PxTransform localTm(PxVec3(PxReal(0) - PxReal(8.2), PxReal(1), 0) * halfExtent);
	PxRigidStatic* body = gPhysics->createRigidStatic(t.transform(localTm));
	body->attachShape(*shape);
	gScene->addActor(*body);
	shape->release();
}

static PxRigidBody* createBall(const PxTransform& t, PxReal halfExtent) //创建小球
{
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(halfExtent), *ballMaterial);
	shape->setSimulationFilterData(collisionGroupBall);//球的碰撞标识
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
	ballReference = body;

	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	gScene->addActor(*body);

	shape->release();
	return body;
}

void initPhysics(bool interactive)
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(4);
	sceneDesc.cpuDispatcher = gDispatcher;
	//sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = BallFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	ballMaterial = gPhysics->createMaterial(0.1f, 0.1f, 0.0f);


	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	gScene->addActor(*groundPlane);

	//创建两条轨道
	for (PxU32 i = 0; i < 2000; i++) {
		createTrack(PxTransform(PxVec3(0, 0, stackZ -= 12.0f)), 2.0f);
	}
	stackZ = 12.0f;
	for (PxU32 i = 0; i < 2000; i++) {
		createTrack(PxTransform(PxVec3(-5, 0, stackZ -= 12.0f)), 2.0f);
	}

	//生成轨道两边栏杆
	stackZ = 12.0f;
	for (PxU32 i = 0; i < 2000; i++) {
		createRailing(PxTransform(PxVec3(-7.6, 0, stackZ -= 12.0f)), 2.0f);

	}
	stackZ = 12.0f;
	for (PxU32 i = 0; i < 2000; i++) {
		createRailing(PxTransform(PxVec3(2.6, 0, stackZ -= 12.0f)), 2.0f);
	}

	//障碍物随机生成
	stackZ = 24.0f;
	for (PxU32 i = 0; i < 1000; i++) {
		int obstacleP = obstaclePosition[rand()%2]; 
		int obstacleDistance = rand() % 40 + 8;   //这里可以调节游戏难度
		createObstacle(PxTransform(PxVec3(obstacleP, 0, stackZ -= obstacleDistance)), 2.0f);
	}

	createBall(PxTransform(PxVec3(-16.5, 3, -0.1f)), 1.0f);

}
// Mode 0: Place left.
// Mode 1: Place right
void MoveBallLeftRight(int mode)
{
	static float LeftRightTrackDistance = 4.5f;
	if (ballReference != NULL)
	{
		auto BallPosition = ballReference->getGlobalPose();
		BallPosition.p.x = mode ? -16.5f : -21.0f;
		ballReference->setGlobalPose(BallPosition);
	}
}
void MoveBallToFrontPoisiton(PxReal speed)
{
	scoreValue += 1; //球每次移动一单位加一分
	if (ballReference != NULL)
	{
		auto BallPosition = ballReference->getGlobalPose();
		BallPosition.p.z -= speed;
		ballReference->setGlobalPose(BallPosition);
	}
}

void tryIncreaseSpeed() 
{
	if (last.tv_sec == 0)
	{
		gettimeofday(&last,NULL);
		gettimeofday(&current, NULL);
		return;
	}
	gettimeofday(&current,NULL);
	if (current.tv_sec - last.tv_sec > 10)
	{
		runSpeed += 0.2;
		last = current;
	}
}
void stepPhysics(bool interactive)
{
	MoveFrontDistance++;
	PX_UNUSED(interactive);
	gScene->simulate(1.0f / 60.0f);
	gScene->fetchResults(true);
}

void cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->release();
	gDispatcher->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	gFoundation->release();
	ballReference = NULL;

	last = timeval();
	current = timeval();
}

void keyPress(unsigned char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
	case 'G':
		GAME_START = true;
		break;
	case 'A':
		MoveBallLeftRight(0);
		break;
	case 'D':
		MoveBallLeftRight(1);
		break;
	}
}
int run(int argc, const char** argv)
{
	renderLoop();
	return 0;
}