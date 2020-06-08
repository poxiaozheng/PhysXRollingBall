#include <ctype.h>

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

PxRigidBody* actor0=NULL;

extern void renderLoop();

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
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 100.0f);
	gScene->addActor(*body);

	shape->release();
}

void createObstacle(const PxTransform& t, PxReal halfExtent)//创建障碍物
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent*0.6, halfExtent*0.6, 0.6), *gMaterial);
	PxTransform localTm(PxVec3(PxReal(0) - PxReal(8.2), PxReal(1), 0) * halfExtent);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 3000.0f);
	gScene->addActor(*body);

	shape->release();
}
void createRailing(const PxTransform& t, PxReal halfExtent)//创建轨道两边栏杆
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.6, halfExtent * 0.6, 6), *gMaterial);
	PxTransform localTm(PxVec3(PxReal(0) - PxReal(8.2), PxReal(1), 0) * halfExtent);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 300.0f);
	gScene->addActor(*body);

	shape->release();
}

static PxRigidBody* createBall(const PxTransform& t, PxReal halfExtent) //创建小球
{
	
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(halfExtent), *gMaterial);
	PxTransform localTm(PxVec3(PxReal(0) - PxReal(4), PxReal(1), 0) * halfExtent);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
	body->setLinearVelocity(PxVec3(0,0,-10.0f));
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

	for (PxU32 i = 0; i < 40; i++) {
		createTrack(PxTransform(PxVec3(0, 0, stackZ -= 12.0f)), 2.0f);
	}
	stackZ = 12.0f;
	for (PxU32 i = 0; i < 40; i++) {
		createTrack(PxTransform(PxVec3(-5, 0, stackZ -= 12.0f)), 2.0f);
	}
	
	/*if (!interactive)
		createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10), PxVec3(0, -50, -100));*/
	/*actor0=createBall(PxTransform(PxVec3(-12.5, 0, -0.1f)), 1.0f);*/
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
	case 'B':
				stackZ = 12.0f;
				for (PxU32 i = 0; i < 60; i++) {
					createRailing(PxTransform(PxVec3(-7.6, 0, stackZ -= 12.0f)), 2.0f);
					//createObstacle(PxTransform(PxVec3(-5.0, 0, stackZ -= 12.0f)), 2.0f);
				}
				stackZ = 12.0f;
				for (PxU32 i = 0; i < 60; i++) {
					createRailing(PxTransform(PxVec3(2.6, 0, stackZ -= 12.0f)), 2.0f);
					//createObstacle(PxTransform(PxVec3(0.0, 0, stackZ -= 12.0f)), 2.0f);
				}
				stackZ = 48.0f;
				for (PxU32 i = 0; i < 60; i++) {
					//createRailing(PxTransform(PxVec3(2.6, 0, stackZ -= 12.0f)), 2.0f);
					createObstacle(PxTransform(PxVec3(-5.0, 0, stackZ -= 48.0f)), 2.0f);
				}
				stackZ = 24.0f;
				for (PxU32 i = 0; i < 60; i++) {
					//createRailing(PxTransform(PxVec3(2.6, 0, stackZ -= 12.0f)), 2.0f);
					createObstacle(PxTransform(PxVec3(0.0, 0, stackZ -= 48.0f)), 2.0f);
				}
				break;
	case ' ':	actor0=createBall(PxTransform(PxVec3(-12.5, 0, -0.1f)), 1.0f);
				//createBall(PxTransform(PxVec3(-17.5, 0, -0.1f)), 1.0f);
				break;
	/*case 'O':
		stackZ = 12.0f;
		for (PxU32 i = 0; i < 30; i++) {
			createObstacle(PxTransform(PxVec3(-7.6, 0, stackZ -= 18.0f)), 2.0f);
		}
		stackZ = 12.0f;
		for (PxU32 i = 0; i < 30; i++) {
			createObstacle(PxTransform(PxVec3(-7.6, 0, stackZ -= 18.0f)), 2.0f);
		}
		break;*/
	case 'R':   
		stackZ = 12.0f;
		cleanupPhysics(true);
		initPhysics(true);
		break;
    //left
	case 'J':
		actor0-> addForce(PxVec3(-10.0f, 10.0f, 0), PxForceMode::eIMPULSE, true);
		break;
	//right
	case 'K':
		actor0->addForce(PxVec3(10.0f, 10.0f, 0), PxForceMode::eIMPULSE, true);
		break;
	}
}
int run(int argc, const char** argv)
{
	renderLoop();
	return 0;
}