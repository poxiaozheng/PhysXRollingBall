#include <ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include <PxPhysicsAPI.h>
#include <extensions/PxDefaultAllocator.h>
#include "Header/Utils/SnippetPVD.h"

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;

PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;

PxMaterial* gMaterial = NULL;

PxPvd* gPvd = NULL;

PxReal stackZ = 12.0f;

PxRigidBody* actor0 = NULL;

bool GAME_OVER = false;

bool GAME_START = false;

extern void renderLoop();

int obstaclePosition[2] = { 0,-5 };

//PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
//{
//	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
//	dynamic->setAngularDamping(0.5f);
//	dynamic->setLinearVelocity(velocity);
//	gScene->addActor(*dynamic);
//	return dynamic;
//}

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
	//PxRigidBodyExt::updateMassAndInertia(*body, 300.0f);
	gScene->addActor(*body);
	shape->release();
}

static PxRigidBody* createBall(const PxTransform& t, PxReal halfExtent) //创建小球
{

	PxShape* shape = gPhysics->createShape(PxSphereGeometry(halfExtent), *gMaterial);
	PxTransform localTm(PxVec3(PxReal(0) - PxReal(4), PxReal(1), 0) * halfExtent);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
	body->setLinearVelocity(PxVec3(0, 0, -10.0f));
	body->attachShape(*shape);
	/*body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	body->setAngularVelocity(PxVec3(0.f, 0.f, 5.f));
	body->setAngularDamping(0.f);*/
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
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

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
		int obstacleP = obstaclePosition[rand() % 2];
		int obstacleDistance = rand() % 50 + 15;
		createObstacle(PxTransform(PxVec3(obstacleP, 0, stackZ -= obstacleDistance)), 2.0f);
	}

}

void stepPhysics(bool interactive)
{
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

}

void keyPress(unsigned char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
	
	case 'B':createBall(PxTransform(PxVec3(-12.5, 2, -0.1f)), 1.0f);
		break;
	case 'R':
		stackZ = 12.0f;
		cleanupPhysics(true);
		initPhysics(true);
		break;
		//left
	//case 'J':
	//	actor0->addForce(PxVec3(-10.0f, 10.0f, 0), PxForceMode::eIMPULSE, true);
	//	break;
	//	//right
	//case 'K':
	//	actor0->addForce(PxVec3(10.0f, 10.0f, 0), PxForceMode::eIMPULSE, true);
	//	break;
	case 'E':
		GAME_OVER = true;
		GAME_START = false;
		break;
	case 'G':
		GAME_START = true;
		break;
	}
}
int run(int argc, const char** argv)
{
	renderLoop();
	return 0;
}