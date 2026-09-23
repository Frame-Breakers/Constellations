#ifndef _MESH_H_
#define _MESH_H_

#include "../../ECS_Base/component.h"
#include "../../dx11.h"


struct Mesh : Component
{
	point3d position = point3d();
	point3d scale = point3d(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX mRotation = DirectX::XMMatrixIdentity();

	point3d color = point3d(1.f, 1.f, 1.f);

	Rasterizer::cullmode cullMode = Rasterizer::cullmode::front;
	int index = 0;
	std::string textureName = "";

	int vShader = 15;
	int pShader = 15;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Mesh,
	active,
	position,
	scale,
	mRotation,
	color,
	cullMode,
	index,
	textureName,
	vShader,
	pShader)

#endif