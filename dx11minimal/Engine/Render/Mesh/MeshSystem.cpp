#include "meshSystem.h"

using namespace std;



MeshSystem::MeshSystem()
{
	frustum = Singleton::GetInstance<FrustumClass>();
	camera = Singleton::GetInstance<CameraClass>();
}


void MeshSystem::Initialize()
{
}


void MeshSystem::Shutdown()
{
	if (frustum) {
		frustum = 0;
	}

	if (camera) {
		camera = 0;
	}
}


void MeshSystem::Update(EntityStorage& entityStorage, float deltaTime)
{
	// Clear the buffers to begin the scene.
	/*Draw::Clear({ 0.0f, 0.0588f, 0.1176f, 1.0f });
	Draw::ClearDepth();*/

	Blend::Blending(Blend::blendmode::off, Blend::blendop::add);
	//Rasterizer::Cull(Rasterizer::cullmode::off);
	Depth::Depth(Depth::depthmode::on);

	const std::vector<Entity*>& entities = entityStorage.GetEntitiesWithComponent<Transform>();
	size_t size = entities.size();
	for (int i = 0; i < size; i++)
	{
		Entity* entity = entities[i];
		if (IsEntityValid(entity))
		{
			Shaders::gShader(0);
			InputAssembler::IA(InputAssembler::topology::triList);

			Transform* transform = entity->GetComponent<Transform>();

			if (transform != nullptr)
			{
				Transform worldTransform = GetWorldTransform(entity);
				float scaler = worldTransform.scale.magnitude();

				Mesh* mesh = entity->GetComponent<Mesh>();
				if (mesh != nullptr && mesh->active) {
					/*if (entity->parent != nullptr && !entity->parent->HasComponent<Transform>()) {
						transform->mRotation = XMMatrixRotationAxis(XMVectorSet(0, 1, 0, 0), -2 * RAD) * transform->mRotation;
					}*/

					Transform meshTransform = worldTransform;

					meshTransform.position += (meshTransform.GetRightVector() * mesh->position.x +
						meshTransform.GetUpVector() * mesh->position.y + meshTransform.GetLookVector() *
						mesh->position.z) * meshTransform.scale;
					meshTransform.scale *= mesh->scale;
					meshTransform.mRotation = mesh->mRotation * meshTransform.mRotation;

					if (frustum->CheckSphere(meshTransform.position, meshTransform.scale.magnitude() * scaler)) {
						//ConstBuf::CreateVertexBuffer(15);

						UpdateWorldMatrix(meshTransform);
						ConstBuf::global[0] = mesh->color.toXMFloat4();
						ConstBuf::Update(5, ConstBuf::global);
						ConstBuf::ConstToPixel(5);

						Rasterizer::Cull(SHOW_GRID ? Rasterizer::cullmode::wireframe : mesh->cullMode);

						Shaders::vShader(mesh->vShader);
						Shaders::pShader(mesh->pShader);

						InputAssembler::IA(InputAssembler::topology::triList);
						InputAssembler::vBuffer(mesh->index);

						context->DrawIndexed(Models::Model[mesh->index].indexes, 0, 0);
					}

				}

				Constellation* constellation = entity->GetComponent<Constellation>();
				if (constellation != nullptr && constellation->active) {
					point3d transformPos = transform->position;
					vector<point3d> transformedStars;

					for (int a = 0; a < constellation->stars.size(); a++) {
						point3d star = constellation->stars[a];
						star = transformPos + transform->GetLookVector() * star.z + transform->GetRightVector() * star.x + transform->GetUpVector() * star.y;

						transformedStars.push_back(star);
					}

					/*int n = 33;
					ConstBuf::drawerV[0] = n;
					ConstBuf::Update(0, ConstBuf::drawerV);
					ConstBuf::ConstToVertex(0);

					int count = 0;
					for (int a = 0; a < transformedStars.size() && count < constCount - 1; a++) {
						point3d star = transformedStars[a];
						if (frustum->CheckSphere(star, constellation->starSize)) {
							worldTransform.position = star;
							ConstBuf::drawerMatrix[count] = GetWorldMatrix(worldTransform);
							ConstBuf::global[count].w = constellation->starSize;
							count++;
						}
					}

					if (count > 0) {
						Rasterizer::Cull(Rasterizer::cullmode::front);

						Shaders::vShader(19);
						Shaders::pShader(19);

						ConstBuf::Update(5, ConstBuf::global);
						ConstBuf::ConstToVertex(5);
						ConstBuf::Update(8, ConstBuf::drawerMatrix);
						ConstBuf::ConstToVertex(8);

						context->DrawInstanced(n * n * 6, min(count, constCount - 1), 0, 0);
					}*/

					Rasterizer::Cull(SHOW_GRID ? Rasterizer::cullmode::wireframe : Rasterizer::cullmode::front);

					int n = GetVertexCount(worldTransform.position, 5, 33, constellation->starSize * scaler);

					float shaderId = n > 8 ? 19 : 30;
					Shaders::vShader(shaderId);
					Shaders::pShader(shaderId);

					ConstBuf::drawerInt[0] = n;
					ConstBuf::Update(7, ConstBuf::drawerInt);
					ConstBuf::ConstToVertex(7);
					ConstBuf::ConstToPixel(7);

					ConstBuf::drawerV[0] = (float)entity->localTime * 0.01f;
					ConstBuf::Update(0, ConstBuf::drawerV);
					ConstBuf::ConstToVertex(0);
					ConstBuf::ConstToPixel(0);

					int count = 0;
					for (int a = 0; a < transformedStars.size() && count < constCount; a++) {
						point3d star = transformedStars[a];
						if (frustum->CheckSphere(star, constellation->starSize * scaler)) {
							worldTransform.position = star;

							ConstBuf::global[0] = XMFLOAT4(constellation->starColor1.x, constellation->starColor1.y, constellation->starColor1.z, 0);
							ConstBuf::global[1] = XMFLOAT4(constellation->starColor2.x, constellation->starColor2.y, constellation->starColor2.z, 0);

							ConstBuf::global[0].w = constellation->starSize;
							ConstBuf::Update(5, ConstBuf::global);
							ConstBuf::ConstToVertex(5);

							ConstBuf::drawerMatrix[0] = GetWorldMatrix(worldTransform);
							ConstBuf::Update(8, ConstBuf::drawerMatrix);
							ConstBuf::ConstToVertex(8);

							context->Draw(n * n * 6, 0);

							count++;
						}
					}
				}

				Star* star = entity->GetComponent<Star>();
				if (star != nullptr && star->active) {
					if (frustum->CheckSphere(worldTransform.position, star->radius * scaler)) {
						ConstBuf::drawerMatrix[0] = GetWorldMatrix(worldTransform);
						ConstBuf::Update(8, ConstBuf::drawerMatrix);
						ConstBuf::ConstToVertex(8);

						ConstBuf::global[0] = XMFLOAT4(star->color1.x, star->color1.y, star->color1.z, 0);
						ConstBuf::global[1] = XMFLOAT4(star->color2.x, star->color2.y, star->color2.z, 0);

						ConstBuf::global[0].w = star->radius;
						ConstBuf::Update(5, ConstBuf::global);
						ConstBuf::ConstToVertex(5);

						Rasterizer::Cull(SHOW_GRID ? Rasterizer::cullmode::wireframe : Rasterizer::cullmode::front);

						int n = GetVertexCount(worldTransform.position, 5, 255, star->radius * scaler);

						float shaderId = n > 20 ? 19 : 30;
						Shaders::vShader(shaderId);
						Shaders::pShader(shaderId);

						ConstBuf::drawerInt[0] = n;
						ConstBuf::Update(7, ConstBuf::drawerInt);
						ConstBuf::ConstToVertex(7);
						ConstBuf::ConstToPixel(7);

						ConstBuf::drawerV[0] = (float)entity->localTime * 0.01f;
						ConstBuf::Update(0, ConstBuf::drawerV);
						ConstBuf::ConstToVertex(0);
						ConstBuf::ConstToPixel(0);

						Draw::Drawer(n * n);
					}
				}

			}
		}
	}
}


void MeshSystem::UpdateWorldMatrix(Transform worldTransform) {
	ConstBuf::camera.world = GetWorldMatrix(worldTransform);
	ConstBuf::UpdateCamera();
	ConstBuf::ConstToVertex(3);
	ConstBuf::ConstToPixel(3);
}

int MeshSystem::GetVertexCount(point3d position, int min, int max, float size) {
	size = max(size, 1);
	float dist = (camera->position - position).magnitude();

	float a = dist - HIGH_RENDER_DISTANCE * size;
	a /= RENDER_DISTANCE_DELTA * size;
	a = clamp(a, 0, 1);

	int n = (int)lerp((float)max, (float)min, a);
	n += 1 - n % 2;

	return n;
}