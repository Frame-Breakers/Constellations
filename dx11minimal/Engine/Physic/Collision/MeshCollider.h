#ifndef _MESH_COLLIDER_H_
#define _MESH_COLLIDER_H_

#include "../../BasicComponents/collider.h"

struct MeshCollider : Collider
{
	float radius = 1.0f;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MeshCollider,
	active,
	friction,
	softness,
	isTouchable,
	anti,
	collisionGroup,
	collisions,
	radius)

#endif