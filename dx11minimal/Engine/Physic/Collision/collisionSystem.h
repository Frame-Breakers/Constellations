#ifndef _COLLISION_SYSTEM_H_
#define _COLLISION_SYSTEM_H_

//////////////
// INCLUDES //
//////////////
#include <map>
#include <unordered_map>
#include <typeindex>
#include <functional>

#include<cmath>
#include "../../ECS_Base/system.h"
#include "../../BasicComponents/Transform.h"
#include "../Movement/PhysicBody.h"

#include "../../BasicComponents/collider.h"
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include "SurfaceCollider.h"
#include "MeshCollider.h"

#include "../../Compute/Combat/Health.h"
#include "../../Compute/Combat/SingleDamager.h"
#include "../../Compute/Combat/MultiDamager.h"

#include "MethodOfClosest.h"
#include "CollisionManagerClass.h"


class CollisionSystem : public System
{
public:
	CollisionSystem();
	void Initialize();
	void Shutdown();

	//float f(float x);

	void Update(EntityStorage&, float);

private:
	CollisionManagerClass* collisionManager;
};

#endif