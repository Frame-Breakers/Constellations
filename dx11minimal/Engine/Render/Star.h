#ifndef _STAR_H_
#define _STAR_H_

#include "../ECS_Base/component.h"
#include "../Types/Point3d.h"


struct Star : Component
{
	float radius = 0.5f;
	float crownRadius = 0.9f;

	point3d color1 = point3d(1.f, 0.95f, 0.25f);
	point3d color2 = point3d(0.75f, 0.015f, 0.f);
	point3d crownColor = point3d(1.f, 0.95f, 0.3f);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Star,
	active,
	radius,
	crownRadius,
	color1,
	color2,
	crownColor)

#endif