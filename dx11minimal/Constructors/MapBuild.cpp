#include "MapBuild.h"

#include "../Engine/Lib/singleton.h"
#include "../Engine/ECS_Base/entityStorage.h"

#include "../Engine/Render/Star.h"
#include "../Engine/Physic/Collision/SphereCollider.h"
#include "../Engine/Physic/Gravitation/GravityPoint.h"
#include "../Engine/Physic/Movement/RotatingSystem.h"
#include "../Engine/Compute/Combat/Health.h"


void MapBuild::BuildMaze() {
	EntityStorage* entityStorage = Singleton::GetInstance<EntityStorage>();
	Entity* worldFolder = entityStorage->GetEntityByName("World");

	Transform t = Transform();
	t.position = point3d(0, 10, 0);

	CreateRotatingStar(t, 6.0f, point3d(0, 1, 0), worldFolder);

	CreateRotatingStar(t, 10.0f, point3d(1, 0, 0), worldFolder);

	t.position = point3d(10, 10, 0);

	CreateRotatingStar(t, 2.0f, point3d(1, 0, 0), worldFolder);

	t.position = point3d(20, 10, 0);

	CreateRotatingStar(t, 12.0f, point3d(1, 0, 0), worldFolder);

	t.position = point3d(30, 10, 0);

	CreateRotatingStar(t, 4.0f, point3d(1, 0, 0), worldFolder);

	t.position = point3d(40, 10, 0);

	CreateRotatingStar(t, 0.0f, point3d(1, 0, 0), worldFolder);
}


void MapBuild::CreateRotatingStar(const Transform& onTransform, float rotateSpeed, const point3d& axis, Entity* parent) {
	EntityStorage* entityStorage = Singleton::GetInstance<EntityStorage>();

	///////////////////////////////////

	Entity* starHolder = entityStorage->CreateEntity("RotatingStarHolder", parent);

	Transform* transform = starHolder->AddComponent<Transform>();
	*transform = onTransform;

	RotatingBody* rotatingBody = starHolder->AddComponent<RotatingBody>();
	rotatingBody->axis = axis;
	rotatingBody->rotateSpeed = rotateSpeed;

	///////////////////////////////////

	Entity* starEntity = entityStorage->CreateEntity("RotatingStar", starHolder);

	transform = starEntity->AddComponent<Transform>();
	transform->position = point3d(0, 0, 50);

	rotatingBody = starEntity->AddComponent<RotatingBody>();
	rotatingBody->axis = axis;
	rotatingBody->rotateSpeed = -rotateSpeed;

	Star* star = starEntity->AddComponent<Star>();
	star->radius = 2.0f;

	SphereCollider* sphereCollider = starEntity->AddComponent<SphereCollider>();
	sphereCollider->radius = 2.0f;
	sphereCollider->collisionGroup = CollisionFilter::Group::HardBody;

	Health* health = starEntity->AddComponent<Health>();
	health->fraction = Fraction::Enemy;

	/*GravityPoint* gravityPoint = starEntity->AddComponent<GravityPoint>();
	gravityPoint->radius = 25.0f;
	gravityPoint->mass = 50.0f;*/
}