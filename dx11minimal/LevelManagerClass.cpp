#include "LevelManagerClass.h"

#include "Constructors/MapBuild.h"

using namespace std;



LevelManagerClass::LevelManagerClass()
{
	window = 0;
	mouse = 0;

	m_World = 0;

	m_ShowGameOverMessage = false;
	m_ShowVictoryMessage = false;
	m_MessageTimer = 0.0f;
}

LevelManagerClass::LevelManagerClass(const LevelManagerClass& other)
{
}


LevelManagerClass::~LevelManagerClass()
{
}


void LevelManagerClass::InitWindow()
{
	if (window == 0) {
		window = Singleton::GetInstance<WindowClass>();
	}
}


bool LevelManagerClass::Initialize()
{
	InitWindow();

	m_World = Singleton::GetInstance<World>();
	entityStorage = m_World->entityStorage;

	collisionManager = Singleton::GetInstance<CollisionManagerClass>();
	mouse = Singleton::GetInstance<MouseClass>();
	questManager = Singleton::GetInstance<QuestManager>();

	Dx11Init(window->hWnd, window->width, window->height);
	std::thread modelsLoadingThread(&LevelManagerClass::LoadModels, this);

	D3D11_BUFFER_DESC boneDesc = {};
	boneDesc.Usage = D3D11_USAGE_DEFAULT;
	boneDesc.ByteWidth = sizeof(XMMATRIX) * 128;
	boneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	device->CreateBuffer(&boneDesc, nullptr, &m_BoneBuffer);

	// window params into const buffer
	ConstBuf::frame.aspect = XMFLOAT4(window->aspect, window->iaspect, float(window->width), float(window->height));

	ConstBuf::factors.AriesNebulaLerpFactor = 0;
	ConstBuf::UpdateFactors();

	if (modelsLoadingThread.joinable()) {
		modelsLoadingThread.join();
	}
	else {
		modelsLoadingThread.detach();
	}

	LoadTextures();
	LoadSounds();

	//////////////////////////////////////////////////////////////////////////////////////////////
	// WORLD CREATING START //
	//////////////////////////////////////////////////////////////////////////////////////////////

	Entity* entity;
	Constellation* constellation;
	Transform* transform;
	PhysicBody* physicBody;
	AIComponent* aiComponent;

	SphereCollider* sphereCollider;

	SpriteCluster* spriteCluster;
	PlaneCollider* planeCollider;
	ParticleEmitter* particleEmitter;

	Mesh* mesh;
	PointCloud* pointCloud;
	DelayedDestroy* delayedDestroy;
	Star* star;
	Health* health;
	SingleDamager* singleDamager;


	//worldFolder = entityStorage->CreateEntity("World"); // COMMENTED RECENTLY

	Entity* player = CreatePlayer();

	worldFolder = entityStorage->LoadEntityFromFile("World");
	//MapBuild::BuildMaze(); // COMMENTED RECENTLY



	Entity* dragger = entityStorage->CreateEntity("Dragger");
	transform = dragger->AddComponent<Transform>();

	entity = entityStorage->CreateEntity("arrow", dragger);
	transform = entity->AddComponent<Transform>();
	mesh = entity->AddComponent<Mesh>();
	mesh->index = 9;

	entity = entityStorage->CreateEntity("arrow", dragger);
	transform = entity->AddComponent<Transform>();
	transform->mRotation = XMMatrixRotationAxis(XMVectorSet(1, 0, 0, 0), PI / 2);
	mesh = entity->AddComponent<Mesh>();
	mesh->index = 9;

	entity = entityStorage->CreateEntity("arrow", dragger);
	transform = entity->AddComponent<Transform>();
	transform->mRotation = XMMatrixRotationAxis(XMVectorSet(0, 0, 1, 0), PI / 2);
	mesh = entity->AddComponent<Mesh>();
	mesh->index = 9;


	/////////////////////////

	/*entity = entityStorage->CreateEntity("Ray", worldFolder);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(20, 20, 0);
	Beam* beam = entity->AddComponent<Beam>();
	beam->point1 = point3d();
	beam->point2 = point3d(0, 0, 25);
	RayDamager* rayDamager = entity->AddComponent<RayDamager>();
	rayDamager->direction = point3d(0, 0, 25);
	rayDamager->repeats = -1;
	rayDamager->damage = 0.1f;*/

	/////////////////////////
	
	/*CreateSpaceBackground(worldFolder, 1); // COMMENTED RECENTLY
	CreateAries(worldFolder);
	CreateZenithLocation(worldFolder, 2);*/

	//CreateNebula(worldFolder,2);
	//CreateStarQuestLoc(worldFolder, 2);

	/////////////////////////

	/*Entity* holder = entityStorage->CreateEntity("Holder", folder);
	transform = holder->AddComponent<Transform>();
	transform->scale = point3d(10, 10, 10);
	transform->position = point3d(0.0f, 0.0f, -50.0f);
	mesh = holder->AddComponent<Mesh>();
	mesh->index = 1;*/

#ifndef _EDITOR
	CreateUI();
#endif

	//////////////////////////////////////////////////////////////////////////////////////////////
	// WORLD CREATING END //
	//////////////////////////////////////////////////////////////////////////////////////////////

	InitSystems();

#ifdef _EDITOR
	editor = Singleton::GetInstance<EditorClass>();
	editorUI = Singleton::GetInstance<EditorUI>();
#else
	playerController = new PlayerController();
	playerController->Initialize(player);
#endif

	auto spawnSkinned = [this](
		const char* name,
		const point3d& pos,
		const point3d& scale,
		const SkinnedMesh& srcMesh,
		Skeleton* skeleton,
		std::vector<AnimationClip>* animations,
		const char* preferredClip = nullptr,
		bool autoPlay = true) -> Entity*
	{
		Entity* e = entityStorage->CreateEntity(name, worldFolder);
		Transform* t = e->AddComponent<Transform>();
		t->position = pos;
		t->scale = scale;

		SkinnedMesh* meshComp = e->AddComponent<SkinnedMesh>();
		meshComp->vertices = srcMesh.vertices;
		meshComp->indices = srcMesh.indices;
		meshComp->gpuModelIndex = srcMesh.gpuModelIndex;
		meshComp->active = false;

		PointCloud* pointCloud = e->AddComponent<PointCloud>();
		pointCloud->index = srcMesh.gpuModelIndex;
		pointCloud->vShader = 28;
		pointCloud->gShader = 17;
		pointCloud->pShader = 17;
		pointCloud->pointSize = 1.0f;
		pointCloud->brightness = 1.25f;
		pointCloud->color = point3d(1.0f, 0.95f, 0.85f);
		pointCloud->frustumRadius = 12.0f;
		pointCloud->compress = RenderCompress::x2;

		SkeletalAnimationComponent* animComp = e->AddComponent<SkeletalAnimationComponent>();
		animComp->skeleton = skeleton;
		animComp->animationClips = animations;
		animComp->rootMotionMode = RootMotionMode::Accumulate;
		animComp->CaptureBindPose();
		if (animations && !animations->empty())
		{
			animComp->SetAnimationByIndex(0, true);
			if (preferredClip && preferredClip[0] != '\0')
			{
				animComp->SetAnimationByName(preferredClip, true);
			}
		}
		animComp->isPlaying = autoPlay && animComp->animationClip != nullptr;
		animComp->isLooping = true;
		animComp->currentTime = 0.0f;
		return e;
	};

	//spawnSkinned("Fox", point3d(0.0f, 0.0f, 10.0f), point3d(0.2f, 0.2f, 0.2f), m_FoxMesh, &m_FoxSkeleton, &m_FoxAnimations);
	//spawnSkinned("CesiumMan", point3d(3.0f, 0.0f, 10.0f), point3d(1.0f, 1.0f, 1.0f), m_CesiumMesh, &m_CesiumSkeleton, &m_CesiumAnimations);
	//spawnSkinned("PunchComboNew", point3d(3.0f, 0.0f, 10.0f), point3d(100.0f, 100.0f, 100.0f), m_PunchComboNewMesh, &m_PunchComboNewSkeleton, &m_PunchComboNewAnimations);
	//m_TestAnimEntity = spawnSkinned("TestMannequin", point3d(6.0f, 0.0f, 10.0f), point3d(1.0f, 1.0f, 1.0f), m_TestAnimMesh, &m_TestAnimSkeleton, &m_TestAnimAnimations, nullptr, false);
	m_TestAnimCycleIndex = 0;
	if (m_TestAnimEntity)
	{
		if (SkeletalAnimationComponent* animComp = m_TestAnimEntity->GetComponent<SkeletalAnimationComponent>())
		{
			animComp->ResetToBindPose();
			animComp->animationClip = nullptr;
			animComp->isPlaying = false;
			animComp->currentTime = 0.0f;
		}
	}

	return true;
}


void LevelManagerClass::Shutdown()
{
#ifdef _EDITOR
	if (editor)
	{
		editor->Shutdown();
		delete editor;
		editor = 0;
	}
	if (editorUI)
	{
		editorUI->Shutdown();
		delete editorUI;
		editorUI = 0;
	}
#else
	if (playerController)
	{
		playerController->Shutdown();
		delete playerController;
		playerController = 0;
	}
#endif

	if (m_World)
	{
		m_World->Shutdown();
		delete m_World;
		m_World = 0;
	}

	if (collisionManager)
	{
		collisionManager->Shutdown();
		delete collisionManager;
		collisionManager = 0;
	}

	if (questManager)
	{
		questManager->Shutdown();
		delete questManager;
		questManager = 0;
	}

	if (mouse)
	{
		delete mouse;
		mouse = 0;
	}

	if (window)
	{
		delete window;
		window = 0;
	}

	if (m_BoneBuffer)
	{
		m_BoneBuffer->Release();
		m_BoneBuffer = nullptr;
	}
}

void LevelManagerClass::Frame()
{
	mouse->UpdateSystemCursorVisibility();
	if (!window->IsActive())
		return;

	mouse->Update();

	UpdateTestAnimationToggle();

	interp::UpdateTweens();

	m_Transform2DDebugUI.UpdateToggle();

#ifdef _EDITOR
	editor->Update();
#else
	playerController->ProcessInput();
	playerController->ProcessMouse();
	playerController->abilities->Update();
	playerController->ProcessUI();

	questManager->UpdateQuests();

	// DEBUG
	Entity* player = entityStorage->GetEntityByName("Player");
	if (IsEntityValid(player) && !m_ShowVictoryMessage)
	{
		Health* playerHealth = player->GetComponent<Health>();
		if (playerHealth && playerHealth->hp <= 0 && !m_ShowGameOverMessage)
		{
			m_ShowGameOverMessage = true;
			m_MessageTimer = 3.0f;

			// Создаём сообщение
			Entity* msgContainer = entityStorage->CreateEntity("GameOverMsg", nullptr);
			Transform2D* msgTransform = msgContainer->AddComponent<Transform2D>();
			msgTransform->anchorPoint = point3d(0, 0, 0);
			msgTransform->ratio = ScreenAspectRatio::XY;
			msgTransform->position = point3d(-0.35f, -0.1f, 0);

			TextLabel* text = msgContainer->AddComponent<TextLabel>();
			text->textW = L"ВЫ ПОГИБЛИ";
			text->fontFamilyW = L"Impact";
			text->fontFilePathW = L"..\\dx11minimal\\Resourses\\Fonts\\Impact.ttf";
			text->fontWeight = 900;
			text->fontSizePx = 70;
			text->fontScale = 1.2f;
			text->color = point3d(1.0f, 0.2f, 0.2f);

			// Автоудаление через 3 секунды
			DelayedDestroy* delayed = msgContainer->AddComponent<DelayedDestroy>();
			delayed->lifeTime = 3000;

			// Отключаем управление игроком
			if (playerController)
			{
				//playerController->SetEnabled(false);
			}
		}
	}

	// Обновляем таймер сообщений
	if (m_ShowGameOverMessage || m_ShowVictoryMessage)
	{
		m_MessageTimer -= 1.0f / 60.0f;  // Примерно 1 кадр
		if (m_MessageTimer <= 0)
		{
			m_ShowGameOverMessage = false;
			m_ShowVictoryMessage = false;
		}
	}
#endif

	// DEBUG

	ConstBuf::frame.aspect = XMFLOAT4{ float(window->aspect), float(window->iaspect), float(window->width), float(window->height) };

	m_World->UpdateCompute();
#ifndef _EDITOR
	m_World->UpdatePhysic();
	m_World->UpdateAudio();
#endif

	entityStorage->CleanMem();

#ifndef _EDITOR
	playerController->ProcessCamera();
#endif

	m_World->UpdateRender();

#ifdef _EDITOR
	editorUI->Update();
#endif


	mouse->RenderCursor();
	Draw::Present();
}


////////////////////////////////////////////////////////////////////////////////////////////////


void LevelManagerClass::InitSystems()
{
	m_World->AddComputeSystem<TimeSystem>();
#ifndef _EDITOR
	m_World->AddComputeSystem<DelayedDestroySystem>();
	m_World->AddComputeSystem<AISystem>();
	m_World->AddComputeSystem<QuestSystem>();
	m_World->AddComputeSystem<RayDamageSystem>();
#endif

	m_World->AddComputeSystem<MazeLinkSystem>();

#ifndef _EDITOR
	m_World->AddPhysicSystem<GravitySystem>();
	m_World->AddPhysicSystem<RotatingSystem>();
	m_World->AddPhysicSystem<PhysicSystem>();
	m_World->AddPhysicSystem<CollisionSystem>();
	m_World->AddPhysicSystem<CombatSystem>();
	m_World->AddPhysicSystem<SkeletalAnimationSystem>(context, m_BoneBuffer);
#endif

	m_World->AddAudioSystem<SoundSystem>();

	m_World->AddRenderSystem<MeshSystem>();
	m_World->AddRenderSystem<StarClaySystem>();
	m_World->AddRenderSystem<SkinnedMeshSystem>(m_World->m_Camera->frustum, m_World->m_Camera, m_BoneBuffer);

	if (SHOW_COLLIDERS) {
		m_World->AddRenderSystem<CollisionDrawSystem>();
	}
	if (SHOW_GRAVITY) {
		m_World->AddRenderSystem<GravityDrawSystem>();
	}

	m_World->AddRenderSystem<SpriteSystem>(m_World->m_Camera->frustum, m_BoneBuffer);
	m_World->AddRenderSystem<NebulaSystem>();

#ifdef _EDITOR
	m_World->AddRenderSystem<PivotDrawSystem>();
	m_World->AddRenderSystem<WayDrawSystem>();
	m_World->AddRenderSystem<LinkRadiusDrawSystem>();
#else
	if (SHOW_PIVOTS) {
		m_World->AddRenderSystem<PivotDrawSystem>();
	}
	if (SHOW_LINK_RADIUS) {
		m_World->AddRenderSystem<LinkRadiusDrawSystem>();
	}
#endif

	m_World->AddRenderSystem<UISystem>();
	m_World->AddRenderSystem<UITextSystem>();
}


void LevelManagerClass::LoadModels()
{
	Models::LoadObjModel("..\\dx11minimal\\Resourses\\Models\\FistBasePos.obj");
	Models::LoadObjModel("..\\dx11minimal\\Resourses\\Models\\Arrow.obj");

	Models::LoadObjModel("..\\dx11minimal\\Resourses\\Models\\AriesBody.obj");
	Models::LoadObjModel("..\\dx11minimal\\Resourses\\Models\\AriesArmor.obj");
	Models::LoadSkinnedModel("..\\dx11minimal\\Resourses\\Models\\Fox.glb", m_FoxMesh, m_FoxSkeleton, m_FoxAnimations);
	Models::LoadSkinnedModel("..\\dx11minimal\\Resourses\\Models\\CesiumMan.glb", m_CesiumMesh, m_CesiumSkeleton, m_CesiumAnimations);
	Models::LoadSkinnedModel("..\\dx11minimal\\Resourses\\Models\\PunchComboNew.glb", m_PunchComboNewMesh, m_PunchComboNewSkeleton, m_PunchComboNewAnimations);

	if (Models::LoadSkinnedModel("..\\dx11minimal\\Resourses\\Models\\TestAnims\\1.glb", m_TestAnimMesh, m_TestAnimSkeleton, m_TestAnimAnimations))
	{
		Models::LoadAndRemapAnimations("..\\dx11minimal\\Resourses\\Models\\TestAnims\\2.glb", m_TestAnimSkeleton, m_TestAnimAnimations, false);
		Models::LoadAndRemapAnimations("..\\dx11minimal\\Resourses\\Models\\TestAnims\\3.glb", m_TestAnimSkeleton, m_TestAnimAnimations, true);
		Models::LoadAndRemapAnimations("..\\dx11minimal\\Resourses\\Models\\TestAnims\\4.glb", m_TestAnimSkeleton, m_TestAnimAnimations, true);
	}

	Models::LoadObjModel("..\\dx11minimal\\Resourses\\Models\\SnakeModel.obj");

	Models::LoadObjModel("..\\dx11minimal\\Resourses\\Models\\drag_arrow.obj");
}

void LevelManagerClass::LoadTextures()
{
	//Textures::LoadTexture("..\\dx11minimal\\Resourses\\Textures\\testTexture.tga");
	Textures::LoadDDSTexture("gta", L"..\\dx11minimal\\Resourses\\Textures\\gta.dds");
	Textures::LoadDDSTexture("aperture", L"..\\dx11minimal\\Resourses\\Textures\\aperture.dds");
	Textures::LoadPNGTexture("comicsSpot", L"..\\dx11minimal\\Resourses\\Textures\\comicsSpot.png");

	Textures::LoadPNGTexture("itemArrow", L"..\\dx11minimal\\Resourses\\Textures\\Editor\\itemArrow.png");
}

void LevelManagerClass::LoadSounds()
{
	Audio::LoadWavFile("lucky", "..\\dx11minimal\\Resourses\\Sounds\\lucky.wav");
	Audio::LoadOggFile("demotivation", "..\\dx11minimal\\Resourses\\Sounds\\demotivation.ogg");

	Audio::LoadWavFile("punch-swing", "..\\dx11minimal\\Resourses\\Sounds\\punch-swing.wav");
}


// TODO: Remove, only for test animation change
void LevelManagerClass::UpdateTestAnimationToggle()
{
	const bool isTogglePressed = input::IsKeyDown('Y');
	if (!isTogglePressed)
	{
		m_WasToggleAnimationPressed = false;
		return;
	}

	if (m_WasToggleAnimationPressed)
	{
		return;
	}

	m_WasToggleAnimationPressed = true;

	if (!m_TestAnimEntity)
	{
		return;
	}

	SkeletalAnimationComponent* animComp = m_TestAnimEntity->GetComponent<SkeletalAnimationComponent>();
	if (!animComp)
	{
		return;
	}

	const int clipCount = animComp->animationClips ? static_cast<int>(animComp->animationClips->size()) : 0;
	if (clipCount <= 0)
	{
		animComp->ResetToBindPose();
		animComp->animationClip = nullptr;
		animComp->isPlaying = false;
		animComp->currentTime = 0.0f;
		m_TestAnimCycleIndex = 0;
		return;
	}

	m_TestAnimCycleIndex = (m_TestAnimCycleIndex + 1) % (clipCount + 1);
	if (m_TestAnimCycleIndex == 0)
	{
		animComp->ResetToBindPose();
		animComp->animationClip = nullptr;
		animComp->isPlaying = false;
		animComp->currentTime = 0.0f;
		return;
	}

	animComp->SetAnimationByIndex(static_cast<size_t>(m_TestAnimCycleIndex - 1), true);
	animComp->isPlaying = animComp->animationClip != nullptr;
	animComp->isLooping = true;
	animComp->currentTime = 0.0f;
}

Entity* LevelManagerClass::CreatePlayer(Entity* folder)
{
	/*Entity* player = entityStorage->CreateEntity("Player", folder);

	Transform* transform = player->AddComponent<Transform>();
	transform->position = point3d(0.0f, 0.0f, 0.0f);

	PhysicBody* physicBody = player->AddComponent<PhysicBody>();
	physicBody->preciseMovement = true;

	SphereCollider* sphereCollider = player->AddComponent<SphereCollider>();
	sphereCollider->radius = 0.75f;
	sphereCollider->collisionGroup = CollisionFilter::Group::Player;

	Health* health = player->AddComponent<Health>();
	health->fraction = Fraction::Player;
	health->destroyOnDeath = false;

	StarClay* starClay = player->AddComponent<StarClay>();
	starClay->blobsRadius = { 0.2f, 0.4f };
	starClay->rate = 20;

	PointCloud* pointCloud = player->AddComponent<PointCloud>();
	pointCloud->index = 0;
	pointCloud->scale = point3d(0.3f, 0.3f, 0.3f);
	pointCloud->pointSize = 0.01f;
	pointCloud->brightness = 0.2f;
	pointCloud->color = point3d(1, 0.6f, 0.9f);

	Entity* grabHitbox = entityStorage->CreateEntity("GrabHitbox", player);
	transform = grabHitbox->AddComponent<Transform>();
	transform->position = point3d(0, 0, 4);
	sphereCollider = grabHitbox->AddComponent<SphereCollider>();
	sphereCollider->radius = 4;
	sphereCollider->isTouchable = false;

	entityStorage->SaveEntityToFile(player, "Player");*/

	Entity* player = entityStorage->LoadEntityFromFile("Player");

	return player;
}


void LevelManagerClass::CreateUI()
{
	Entity* entity;
	Transform2D* transform2D;
	Rect* rect;
	Button* button;
	TextLabel* textLabel;

	Entity* uiFolder = entityStorage->CreateEntity("UI");



	entity = entityStorage->CreateEntity("CenterPoint", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->ratio = ScreenAspectRatio::YY;
	transform2D->scale = point3d(0.005f, 0.005f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(1.0f, 0.0f, 0.0f);
	rect->cornerRadius = 1.0f;



	entity = entityStorage->CreateEntity("HealthHolder", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XY;
	transform2D->position = point3d(-0.9f, -0.6f, 0.0f);
	transform2D->scale = point3d(0.18f, 0.04f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.5f, 0.5f, 0.5f);
	rect->opacity = 0.5f;

	entity = entityStorage->CreateEntity("HealthBar", entity);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->parentAnchor = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XY;
	rect = entity->AddComponent<Rect>();

	entity = entityStorage->CreateEntity("StaminaHolder", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XY;
	transform2D->position = point3d(-0.9f, -0.7f, 0.0f);
	transform2D->scale = point3d(0.18f, 0.02f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.5f, 0.5f, 0.5f);
	rect->opacity = 0.5f;

	entity = entityStorage->CreateEntity("StaminaBar", entity);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->parentAnchor = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XY;
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.8f, 0.8f, 1);

	entity = entityStorage->CreateEntity("HealthLabel", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->ratio = ScreenAspectRatio::XY;
	transform2D->position = point3d(-0.9f, -0.64f, 0.0f);
	textLabel = entity->AddComponent<TextLabel>();
	textLabel->textW = L"ЗДОРОВЬЕ";
	textLabel->fontFamilyW = L"Impact";
	textLabel->fontFilePathW = L"..\\dx11minimal\\Resourses\\Fonts\\Impact.ttf";
	textLabel->fontWeight = 900;
	textLabel->fontSizePx = 44;
	textLabel->fontScale = 0.40f;
	textLabel->letterSpacingPx = 1.0f;

	entity = entityStorage->CreateEntity("StaminaLabel", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->ratio = ScreenAspectRatio::XY;
	transform2D->position = point3d(-0.9f, -0.74f, 0.0f);
	textLabel = entity->AddComponent<TextLabel>();
	textLabel->textW = L"ВЫНОСЛИВОСТЬ";
	textLabel->fontFamilyW = L"Impact";
	textLabel->fontFilePathW = L"..\\dx11minimal\\Resourses\\Fonts\\Impact.ttf";
	textLabel->fontWeight = 900;
	textLabel->fontSizePx = 38;
	textLabel->fontScale = 0.34f;
	textLabel->letterSpacingPx = 1.0f;

	entity = entityStorage->CreateEntity("ElementLabel", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->ratio = ScreenAspectRatio::XY;
	transform2D->position = point3d(0.8f, -0.64f, 0.0f);
	textLabel = entity->AddComponent<TextLabel>();
	textLabel->textW = L"НЕТ";
	textLabel->fontFamilyW = L"Impact";
	textLabel->fontFilePathW = L"..\\dx11minimal\\Resourses\\Fonts\\Impact.ttf";
	textLabel->fontWeight = 900;
	textLabel->fontSizePx = 44;
	textLabel->fontScale = 0.40f;
	textLabel->letterSpacingPx = 1.0f;


	// UI Prototypes
	// Selected weapon - circle
	entity = entityStorage->CreateEntity("Rect", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XX;
	transform2D->position = point3d(-0.9f, -0.85f, 0.0f);
	transform2D->scale = point3d(0.05f, 0.05f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.75f, 0.0f, 0.0f);
	//rect->opacity = 0.5f;
	rect->cornerRadius = 1.f;

	// Charged attack - circle
	entity = entityStorage->CreateEntity("Rect", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XX;
	transform2D->position = point3d(-0.75f, -0.85f, 0.0f);
	transform2D->scale = point3d(0.05f, 0.05f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.75f, 0.0f, 0.0f);
	rect->cornerRadius = 1.f;

	// Battle end timer - icon
	entity = entityStorage->CreateEntity("Rect", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XX;
	transform2D->position = point3d(-0.9f, 0.85f, 0.0f);
	transform2D->scale = point3d(0.05f, 0.05f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.75f, 0.0f, 0.0f);

	// Battle end timer - text
	entity = entityStorage->CreateEntity("Rect", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(-1, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XX;
	transform2D->position = point3d(-0.75f, 0.85f, 0.0f);
	transform2D->scale = point3d(0.05f, 0.05f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.75f, 0.0f, 0.0f);

	// Enemy name - text
	entity = entityStorage->CreateEntity("Rect", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(0, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XX;
	transform2D->position = point3d(0, 0.85f, 0.0f);
	transform2D->scale = point3d(0.2f, 0.05f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.75f, 0.0f, 0.0f);

	// Enemy HP bar - rectangle
	entity = entityStorage->CreateEntity("BossHealth", uiFolder);
	transform2D = entity->AddComponent<Transform2D>();
	transform2D->anchorPoint = point3d(0, 0, 0);
	transform2D->ratio = ScreenAspectRatio::XY;
	transform2D->position = point3d(0, 0.7f, 0.0f);
	transform2D->scale = point3d(0.5f, 0.025f, 0.0f);
	rect = entity->AddComponent<Rect>();
	rect->color = point3d(0.75f, 0.0f, 0.0f);

	m_Transform2DDebugUI.Create(entityStorage, uiFolder);
}




void LevelManagerClass::CreateSpaceBackground(Entity* folder, int quality)
{
	int outerSpace_cnt = 6853 / quality;

	Entity* entity = entityStorage->CreateEntity("OuterSpace", folder);

	Nebula* nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 25;
	nebula->count = outerSpace_cnt;
	nebula->mode = pMode::point;
	nebula->isOnBackground = true;
}


void LevelManagerClass::CreateAries(Entity* folder)
{
	Entity* entity;
	Constellation* constellation;
	Transform* transform;
	PhysicBody* physicBody;

	SphereCollider* sphereCollider;

	PointCloud* pointCloud;
	DelayedDestroy* delayedDestroy;
	Health* health;
	CameraTarget* cameraTarget;

	Entity* aries = entityStorage->CreateEntity("Aries", folder);
	transform = aries->AddComponent<Transform>();
	transform->scale = point3d(4, 4, 4);
	transform->position = point3d(0.0f, 20.0f, 50.0f);
	//pointCloud = aries->AddComponent<PointCloud>();
	//pointCloud->index = 2;
	//pointCloud->pointSize = 1.0f;
	//pointCloud->brightness = 0.4f;
	////pointCloud->color = point3d(1, 0.2, 0.25);
	//pointCloud->instances = 1;
	//pointCloud->frustumRadius = 8;
	//pointCloud->compress = RenderCompress::x2;
	/*health = aries->AddComponent<Health>();
	health->hp = 1000;
	health->maxHp = 1000;
	cameraTarget = aries->AddComponent<CameraTarget>();*/

	/*constellation = aries->AddComponent<Constellation>();
	constellation->starSize = 0.04f;
	constellation->stars = {
		point3d(0, 2.5f, 3.0f),
		point3d(0, 1.7f, -1.4f),
		point3d(0, 0.4f, -3.9f),
		point3d(0, -0.9f, -6.0f),

		point3d(1.8f, -2.0f, 0.4f),
		point3d(-1.8f, -2.0f, 0.4f),
		point3d(1.9f, -4.2f, 0.6f),
		point3d(-1.9f, -4.2f, 0.6f),
		point3d(1.9f, -6.6f, 1.1f),
		point3d(-1.9f, -6.6f, 1.1f),

		point3d(1.8f, -2.0f, -6.0f),
		point3d(-1.8f, -2.0f, -6.0f),
		point3d(1.6f, -3.5f, -6.8f),
		point3d(-1.6f, -3.5f, -6.8f),
		point3d(1.6f, -6.6f, -6.6f),
		point3d(-1.6f, -6.6f, -6.6f),
	};
	constellation->links = {
		{0,1},
		{1,2},
		{2,3},

		{1,4},
		{1,5},
		{4,6},
		{5,7},
		{6,8},
		{7,9},

		{3,10},
		{3,11},
		{10,12},
		{11,13},
		{12,14},
		{13,15},
	};*/

	//entity = entityStorage->CreateEntity("Armor", aries);
	//transform = entity->AddComponent<Transform>();
	//pointCloud = entity->AddComponent<PointCloud>();
	//pointCloud->index = 3;
	//pointCloud->pointSize = 0.75f;
	//pointCloud->brightness = 0.2f;
	//pointCloud->color = point3d(1, 0.9f, 0.2f);
	//pointCloud->frustumRadius = 8;
	////pointCloud->compress = RenderCompress::x2;

	////Body

	//entity = entityStorage->CreateEntity("Collider", aries);
	//transform = entity->AddComponent<Transform>();
	//transform->position = point3d(0, 0.08, 0.1);
	//sphereCollider = entity->AddComponent<SphereCollider>();
	//sphereCollider->softness = 0.7;
	//sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	//sphereCollider->radius = 3;

	//entity = entityStorage->CreateEntity("Collider", aries);
	//transform = entity->AddComponent<Transform>();
	//transform->position = point3d(0, -0.05, -1.5);
	//sphereCollider = entity->AddComponent<SphereCollider>();
	//sphereCollider->softness = 0.7;
	//sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	//sphereCollider->radius = 2.2;

	//entity = entityStorage->CreateEntity("Collider", aries);
	//transform = entity->AddComponent<Transform>();
	//transform->position = point3d(0, -0.1, -0.7);
	//sphereCollider = entity->AddComponent<SphereCollider>();
	//sphereCollider->softness = 0.7;
	//sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	//sphereCollider->radius = 2.7;

	////Head

	//entity = entityStorage->CreateEntity("Collider", aries);
	//transform = entity->AddComponent<Transform>();
	//transform->position = point3d(0, 0.9, 0.8);
	//sphereCollider = entity->AddComponent<SphereCollider>();
	//sphereCollider->softness = 0.7;
	//sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	//sphereCollider->radius = 2;

	//entity = entityStorage->CreateEntity("Collider", aries);
	//transform = entity->AddComponent<Transform>();
	//transform->position = point3d(0, 0.7, 1.4);
	//sphereCollider = entity->AddComponent<SphereCollider>();
	//sphereCollider->softness = 0.7;
	//sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	//sphereCollider->radius = 1.1;

	////Horns

	//entity = entityStorage->CreateEntity("Collider", aries);
	//transform = entity->AddComponent<Transform>();
	//transform->position = point3d(0.5, 1.2, 0.5);
	//sphereCollider = entity->AddComponent<SphereCollider>();
	//sphereCollider->softness = 0.7;
	//sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	//sphereCollider->radius = 1.6;

	//entity = entityStorage->CreateEntity("Collider", aries);
	//transform = entity->AddComponent<Transform>();
	//transform->position = point3d(-0.5, 1.2, 0.5);
	//sphereCollider = entity->AddComponent<SphereCollider>();
	//sphereCollider->softness = 0.7;
	//sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	//sphereCollider->radius = 1.6;
}

void LevelManagerClass::CreateZenithLocation(Entity* folder, int quality)
{
	Entity* entity;
	Nebula* nebula;
	Transform* transform;
	SphereCollider* sphereCollider;
	MultiDamager* multiDamager;

	int pillars_cnt = 3725470 / 2 / quality;
	int galaxy_cnt = 182361 / quality;

	Entity* location = entityStorage->CreateEntity("Zenith location", folder);

	transform = location->AddComponent<Transform>();
	transform->position = point3d(0, 0, 200);

	// Pillars hand | point
	entity = entityStorage->CreateEntity("PHP", folder);
	transform = entity->AddComponent<Transform>();

	nebula = location->AddComponent<Nebula>();
	nebula->vShader = 24;
	nebula->count = pillars_cnt;
	nebula->mode = pMode::point;
	nebula->scale = 5;
	nebula->frustumRadius = 40;

	// Inside nebula | point
	entity = entityStorage->CreateEntity("INP", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(150, 0, 0);

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 23;
	nebula->count = pillars_cnt;
	nebula->mode = pMode::point;
	nebula->color = point3d(0.3, 0.5, 1.0);
	nebula->scale = 10;
	nebula->frustumRadius = 40;

	// Pillars hand | glow
	entity = entityStorage->CreateEntity("PHG", location);
	transform = entity->AddComponent<Transform>();

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 24;
	nebula->count = pillars_cnt;
	nebula->skipper = 1394 / 2;
	nebula->mode = pMode::glow;
	nebula->scale = 5;
	nebula->frustumRadius = 40;

	// Inside nebula | glow
	entity = entityStorage->CreateEntity("ING", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(150, 0, 0);

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 23;
	nebula->count = pillars_cnt;
	nebula->skipper = 1394;
	nebula->mode = pMode::glow;
	nebula->color = point3d(0.3, 0.5, 1.0);
	nebula->scale = 10;
	nebula->frustumRadius = 40;

	// 2 Nebula
	entity = entityStorage->CreateEntity("INP1", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(0, 0, 0);

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 23;
	nebula->count = pillars_cnt;
	nebula->mode = pMode::point;
	nebula->color = point3d(0.8, 0.4, 0.2);
	nebula->scale = 10;
	nebula->frustumRadius = 40;

	entity = entityStorage->CreateEntity("ING1", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(0, 0, 0);

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 23;
	nebula->count = pillars_cnt;
	nebula->skipper = 1394;
	nebula->mode = pMode::glow;
	nebula->color = point3d(0.8, 0.4, 0.2);
	nebula->scale = 10;
	nebula->frustumRadius = 40;
}

void LevelManagerClass::CreateNebula(Entity* folder, int quality) {

	Entity* entity;
	Nebula* nebula;
	Transform* transform;
	SphereCollider* sphereCollider;
	MultiDamager* multiDamager;

	int pillars_cnt = 3725470 / 2 / quality;
	int galaxy_cnt = 182361 / quality;

	Entity* location = entityStorage->CreateEntity("Zenith location", folder);

	transform = location->AddComponent<Transform>();
	transform->position = point3d(0, 50, 0);

	// first nebula Heal
	entity = entityStorage->CreateEntity("INP", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(-150, 50, 0);

	sphereCollider = entity->AddComponent<SphereCollider>();
	sphereCollider->radius = 30.f;
	sphereCollider->isTouchable = false;

	multiDamager = entity->AddComponent<MultiDamager>();
	multiDamager->damage = -5;
	multiDamager->repeats = -1;
	multiDamager->interval = 1000;

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 26;
	nebula->count = pillars_cnt;
	nebula->mode = pMode::point;
	nebula->color = point3d(1, 0.3, 0.5);  // Розоватый
	nebula->color = point3d(0.2, 0.8, 0.3); // Зеленый
	nebula->color = point3d(0.3, 0.5, 1.0); // Голубой
	nebula->scale = 1;
	nebula->frustumRadius = 40;

	// first nebula Heal Glow
	entity = entityStorage->CreateEntity("ING", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(-150, 50, 0);


	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 26;
	nebula->count = pillars_cnt;
	nebula->skipper = 1394;
	nebula->mode = pMode::glow;
	nebula->color = point3d(1, 0.3, 0.5);  // Розоватый
	nebula->color = point3d(0.2, 0.8, 0.3); // Зеленый
	nebula->color = point3d(0.3, 0.5, 1.0); // Голубой
	nebula->scale = 1;
	nebula->frustumRadius = 40;


	// First Nebula DMG

	entity = entityStorage->CreateEntity("INP1", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(-450, 50, 0);

	sphereCollider = entity->AddComponent<SphereCollider>();
	sphereCollider->radius = 30.f;
	sphereCollider->isTouchable = false;

	multiDamager = entity->AddComponent<MultiDamager>();
	multiDamager->damage = 5;
	multiDamager->repeats = -1;
	multiDamager->interval = 1000;

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 26;
	nebula->count = pillars_cnt;
	nebula->mode = pMode::point;
	nebula->color = point3d(0.8, 0.4, 0.2);
	nebula->scale = 1;
	nebula->frustumRadius = 40;
	nebula->isInteractive = true;

	sphereCollider = entity->AddComponent<SphereCollider>();
	sphereCollider->radius = 40;
	sphereCollider->isTouchable = false;

	// First Nebula DMG Glow
	entity = entityStorage->CreateEntity("ING1", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(-450, 50, 0);

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 26;
	nebula->count = pillars_cnt;
	nebula->skipper = 1394;
	nebula->mode = pMode::glow;
	nebula->color = point3d(0.8, 0.4, 0.2);
	nebula->scale = 1;
	nebula->frustumRadius = 40;


	// Test Nebula

	entity = entityStorage->CreateEntity("INP1", location);
	transform = entity->AddComponent<Transform>();
	transform->position = point3d(0, 50, 0);

	sphereCollider = entity->AddComponent<SphereCollider>();
	sphereCollider->radius = 30.f;
	sphereCollider->isTouchable = false;

	multiDamager = entity->AddComponent<MultiDamager>();
	multiDamager->damage = 5;
	multiDamager->repeats = -1;
	multiDamager->interval = 1000;

	nebula = entity->AddComponent<Nebula>();
	nebula->vShader = 29;
	nebula->count = pillars_cnt;
	nebula->mode = pMode::point;
	nebula->color = point3d(0.6, 0.3, 0.1);

	nebula->scale = 1;
	nebula->frustumRadius = 40;



	// СОЗДАНИЕ ПОРТАЛА
	//Entity* portal = entityStorage->CreateEntity("CosmicPortal", folder);
	//Transform* portalTransform = portal->AddComponent<Transform>();
	//portalTransform->position = point3d(0, 0, 0);  // Позиция портала

	//// Добавляем Nebula компонент с шейдером портала
	//Nebula* portalNebula = portal->AddComponent<Nebula>();
	//portalNebula->vShader = 32;      // Ваш вершинный шейдер портала
	////portalNebula->pShader = 23;      // Пиксельный шейдер (базовый)
	//portalNebula->count = 50000;     // Количество частиц
	//portalNebula->mode = pMode::point; // Режим отображения (точки)
	//portalNebula->scale = 2.0f;      // Масштаб портала
	//portalNebula->frustumRadius = 30.0f; // Радиус отсечения

	//// Добавляем свечение портала
	//Entity* portalGlow = entityStorage->CreateEntity("PortalGlow", portal);
	//Nebula* glowNebula = portalGlow->AddComponent<Nebula>();
	//glowNebula->vShader = 32;
	////glowNebula->pShader = 24;        // Шейдер для свечения
	//glowNebula->count = 50000;
	//glowNebula->skipper = 1394;      // Пропуск частиц для эффекта свечения
	//glowNebula->mode = pMode::glow;  // Режим свечения
	//glowNebula->scale = 2.2f;        // Чуть больше основного портала
	//glowNebula->frustumRadius = 35.0f;

	//// Добавляем коллайдер для взаимодействия
	//SphereCollider* portalCollider = portal->AddComponent<SphereCollider>();
	//portalCollider->radius = 25.0f;
	//portalCollider->isTouchable = true;
	//portalCollider->collisionGroup = CollisionFilter::Group::Player;

	//// Делаем портал видимым для камеры
	//CameraTarget* cameraTarget = portal->AddComponent<CameraTarget>();
	//cameraTarget->active = true;

}



void LevelManagerClass::CreateStarQuestLoc(Entity* folder, int quality)
{
	Entity* location = entityStorage->CreateEntity("StarQuestLocation", folder);
	Transform* transform = location->AddComponent<Transform>();
	transform->position = point3d(0, 0, 150);

	
	Entity* m_CentralStar = entityStorage->CreateEntity("CentralStar", location);
	transform = m_CentralStar->AddComponent<Transform>();
	transform->position = point3d(0, 0, 0);

	QuestGiver* questGiver = m_CentralStar->AddComponent<QuestGiver>();
	questGiver->radius = 40;
	questGiver->questType = "QuestStarCollection";

	Star* star = m_CentralStar->AddComponent<Star>();
	star->radius = 20.0f;
	star->crownRadius = 25.0f;
	star->color1 = point3d(0.99f, 1.0f, 0.51f);
	star->color2 = point3d(0.75f, 0.2f, 0.37f);
	star->crownColor = point3d(0.87f, 0.25f, 0.15f);

	SphereCollider* sphereCollider = m_CentralStar->AddComponent<SphereCollider>();
	sphereCollider->radius = 20.0f;
	sphereCollider->collisionGroup = CollisionFilter::Group::Enemy;
	sphereCollider->isTouchable = true;

	Health* health = m_CentralStar->AddComponent<Health>();
	health->hp = 100;
	health->maxHp = 100;
	health->fraction = Fraction::Player;

	m_CentralStar->AddComponent<Grabbable>();


	Entity* collectionTrigger = entityStorage->CreateEntity("CollectionTrigger", m_CentralStar);
	Transform* triggerTransform = collectionTrigger->AddComponent<Transform>();
	triggerTransform->position = point3d(0, 0, 0);

	SphereCollider* triggerCollider = collectionTrigger->AddComponent<SphereCollider>();
	triggerCollider->radius = 40;
	triggerCollider->collisionGroup = CollisionFilter::Group::Projectile;
	triggerCollider->isTouchable = false;
}

void LevelManagerClass::ShowGameOverMessage(const wchar_t* message, const point3d& color)
{
	// Создаём UI контейнер для сообщения
	Entity* gameOverContainer = entityStorage->CreateEntity("GameOverContainer", nullptr);
	Transform2D* containerTransform = gameOverContainer->AddComponent<Transform2D>();
	containerTransform->anchorPoint = point3d(0, 0, 0);
	containerTransform->ratio = ScreenAspectRatio::XY;
	containerTransform->position = point3d(0, 0, 0);
	containerTransform->scale = point3d(1, 1, 0);

	// Фон
	Entity* bg = entityStorage->CreateEntity("GameOverBG", gameOverContainer);
	Transform2D* bgTransform = bg->AddComponent<Transform2D>();
	bgTransform->anchorPoint = point3d(0, 0, 0);
	bgTransform->ratio = ScreenAspectRatio::XY;
	bgTransform->position = point3d(-0.5f, -0.2f, 0);
	bgTransform->scale = point3d(1, 0.4f, 0);
	Rect* bgRect = bg->AddComponent<Rect>();
	bgRect->color = point3d(0, 0, 0);
	bgRect->opacity = 0.8f;

	// Текст сообщения
	Entity* text = entityStorage->CreateEntity("GameOverText", gameOverContainer);
	Transform2D* textTransform = text->AddComponent<Transform2D>();
	textTransform->anchorPoint = point3d(0, 0, 0);
	textTransform->ratio = ScreenAspectRatio::XY;
	textTransform->position = point3d(-0.25f, -0.1f, 0);
	TextLabel* textLabel = text->AddComponent<TextLabel>();
	textLabel->textW = message;
	textLabel->fontFamilyW = L"Impact";
	textLabel->fontFilePathW = L"..\\dx11minimal\\Resourses\\Fonts\\Impact.ttf";
	textLabel->fontWeight = 900;
	textLabel->fontSizePx = 80;
	textLabel->fontScale = 1.5f;
	textLabel->color = color;
	textLabel->opacity = 1.0f;

	// Автоудаление через 3 секунды
	DelayedDestroy* delayed = gameOverContainer->AddComponent<DelayedDestroy>();
	delayed->lifeTime = 3000;
}