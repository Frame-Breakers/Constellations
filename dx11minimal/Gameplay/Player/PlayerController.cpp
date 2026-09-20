#include "PlayerController.h"

#include "../../Engine/UI/Text/TextLabel.h"

using namespace std;


PlayerController::PlayerController()
{
	playerEntity = 0;

	playerTransform = 0;
	playerPhysicBody = 0;
	playerPointCloud = 0;

	camera = 0;
	mouse = 0;
	window = 0;
}

PlayerController::PlayerController(const PlayerController& other)
{
}


PlayerController::~PlayerController()
{
}


void PlayerController::Initialize(Entity* Player)
{
	playerEntity = Player;

	playerTransform = Player->GetComponent<Transform>();
	playerPhysicBody = Player->GetComponent<PhysicBody>();
	playerPointCloud = Player->GetComponent<PointCloud>();
	playerHealth = Player->GetComponent<Health>();

	World* world = Singleton::GetInstance<World>();

	ui = world->entityStorage->GetEntityByName("UI");
	healthBar = ui->GetChildByName("HealthBar", true);
	staminaBar = ui->GetChildByName("StaminaBar", true);
	bossHealthBar = ui->GetChildByName("BossHealth", true);
	elementLabel = ui->GetChildByName("ElementLabel", true);

	camera = world->m_Camera;
	mouse = Singleton::GetInstance<MouseClass>();
	window = Singleton::GetInstance<WindowClass>();
	entityStorage = Singleton::GetInstance<EntityStorage>();
	collisionManager = Singleton::GetInstance<CollisionManagerClass>();

	abilities = Singleton::GetInstance<PlayerAbilities>();
	comboManager = Singleton::GetInstance<ComboManager>();

	currentMaxSpeed = PLAYER_MOVE_SPEED;
	isRunning = false;

	cameraTarget = nullptr;
	lockMovementOnTarget = false;

	movementType = MovementType::Classic;

	SetCursorPos(window->width / 2, window->height / 2);
}


void PlayerController::Shutdown()
{
	if (abilities) {
		abilities->Shutdown();
		delete abilities;
		abilities = 0;
	}

	if (playerEntity)
		playerEntity = 0;

	if (playerTransform)
		playerTransform = 0;

	if (playerPhysicBody)
		playerPhysicBody = 0;

	if (playerPointCloud)
		playerPointCloud = 0;

	if (playerHealth)
		playerHealth = 0;

	if (ui)
		ui = 0;

	if (healthBar)
		healthBar = 0;

	if (staminaBar)
		staminaBar = 0;

	if (camera)
		camera = 0;

	if (mouse)
		mouse = 0;

	if (window)
		window = 0;
}


void PlayerController::ProcessInput()
{
	if (playerEntity == nullptr || !playerEntity->IsActive())
		return;

	ProcessMovement();

		// Обработка кнопки F для щита
	static bool fKeyPressed = false;
	if (input::IsKeyDown('F') && !fKeyPressed) {
		fKeyPressed = true;
		abilities->ShieldStart(); // Активируем щит при нажатии F
		comboManager->ClearInputBuffer();
	}
	else if (!input::IsKeyDown('F') && fKeyPressed) {
		fKeyPressed = false;
		abilities->ShieldEnd(); // Деактивируем щит при отпускании F
	}
	
	if (input::IsKeyDown('C')) {
		Dash();
	}

	if (input::IsKeyPressed('1') && abilities->weapon != PlayerWeapons::Fists) {
		abilities->weapon = PlayerWeapons::Fists;
		comboManager->SaveInput(ComboInputType::TakeFists);
	}
	if (input::IsKeyPressed('2') && abilities->weapon != PlayerWeapons::Sword) {
		abilities->weapon = PlayerWeapons::Sword;
		comboManager->SaveInput(ComboInputType::TakeSword);
	}
	if (input::IsKeyPressed('3') && abilities->weapon != PlayerWeapons::Bow) {
		abilities->weapon = PlayerWeapons::Bow;
		comboManager->SaveInput(ComboInputType::TakeBow);
	}

	if (input::IsKeyPressed('T')) {
		abilities->Timestop();
	}

	if (input::IsKeyPressed('V')) {
		abilities->ParticleVacuumStart();
	}
	if (input::IsKeyReleased('V')) {
		abilities->ParticleVacuumEnd();
	}

	/*static bool gKeyPressed = false;
	if (input::IsKeyDown('G') && !gKeyPressed) {
		gKeyPressed = true;
		if (abilities) {
			abilities->BlowGasStart(); 
		}
	}
	else if (!input::IsKeyDown('G') && gKeyPressed) {
		gKeyPressed = false;
		if (abilities) {
			abilities->BlowGasEnd();
		}
	}*/

	if (input::IsKeyPressed('G')) {
		abilities->Grab();
	}

	if (input::IsKeyDown('K'))
	{
		abilities->StartRadar();
	}

	if (input::IsKeyPressed('E')) {
		movementType = MovementType::Stopped;
		abilities->Grap();
	}

	if (input::IsKeyPressed(VK_MBUTTON)) {
		LockOnTarget();
	}

	if (input::IsKeyPressed('P')) {
		int i = (int)abilities->element;
		i = i < 4 ? i + 1 : 0;
		abilities->element = (Elements)i;
	}

	if (input::IsKeyPressed('O')) {
		int i = (int)abilities->element;
		i = i > 0 ? i - 1 : 4;
		abilities->element = (Elements)i;
	}

	abilities->Execution();
}
		


void PlayerController::ProcessCamera()
{
	if (mouse->state == MouseState::Locked)
		return;

	Transform wt = GetWorldTransform(playerEntity);
	camera->position = camera->position.lerp(wt.position - wt.GetLookVector() * camera->distance + wt.GetUpVector() * 2, 0.4f);

	XMMATRIX matrixRotation;
	CheckTargetValid();
	if (cameraTarget == nullptr) {
		matrixRotation = wt.mRotation;
	}
	else {
		point3d direction = GetWorldTransform(cameraTarget).position - camera->position;
		matrixRotation = GetMatrixFromDirection(direction, wt.GetUpVector());
	}

	camera->SetMatrixRotation(LerpMatrix(camera->GetMatrixRotation(), matrixRotation, 0.15f));
}


void PlayerController::ProcessMouse()
{
	float x = mouse->absolutePos.x - window->width / 2;
	float y = mouse->absolutePos.y - window->height / 2;
	point3d mousePos = point3d(x / window->width / window->aspect, y / window->height, 0);

	switch (mouse->state)
	{
	case MouseState::Centered:
	{
		if (playerEntity != nullptr && playerEntity->IsActive()) {
			float length = mousePos.magnitude();

			if (length > CURSOR_IGNORE_ZONE) {
				if (length > MAX_CURSOR_DEVIATION) {
					mousePos = mousePos.normalized() * MAX_CURSOR_DEVIATION;

					mouse->absolutePos = point3d(mousePos.x * window->width * window->aspect + window->width / 2, mousePos.y * window->height + window->height / 2, 0);
					mouse->pos = point3d(mouse->absolutePos.x / window->width * 2 - 1, -(mouse->absolutePos.y / window->height * 2 - 1), 0.0f);

					SetCursorPos(mouse->absolutePos.x, mouse->absolutePos.y);
				}

				CheckTargetValid();
				if (!lockMovementOnTarget || cameraTarget == nullptr) {
					float k = (length - CURSOR_IGNORE_ZONE) / MAX_CURSOR_DEVIATION;
					mousePos *= MOUSE_SENSIVITY * k;

					XMMATRIX additionalRotation = XMMatrixRotationRollPitchYaw(XMConvertToRadians(mousePos.y), XMConvertToRadians(mousePos.x), 0);

					playerPhysicBody->mAngVelocity = playerPhysicBody->mAngVelocity * additionalRotation;
				}
			}

			// Обработка атак мышью - проверяем что щит не активен
			if (!abilities->IsShieldActive()) {

				/*if (mouse->IsLButtonDown()) {
					abilities->Charging();
				}
				else if (mouse->IsLButtonUnclicked()) {
					abilities->Attack(*playerTransform, mouse->GetMouseDirection());
				}*/

				/*if (mouse->IsRButtonClicked()) {
					abilities->BlockStart();
				}
				else if (mouse->IsRButtonUnclicked()) {
					abilities->BlockEnd();
				}*/

				if (mouse->IsLButtonClicked()) {
					comboManager->StartHeldInput(ComboInputType::Light);
				}

				if (mouse->IsRButtonClicked()) {
					comboManager->StartHeldInput(ComboInputType::Heavy);
				}

				if (mouse->IsLButtonReleased()) {
					comboManager->SaveInput(ComboInputType::Light);
				}

				if (mouse->IsRButtonReleased()) {
					comboManager->SaveInput(ComboInputType::Heavy);
				}
			}
		}
		break;
	}
	case MouseState::Free:
	{
		if (mouse->IsLButtonClicked() && mouse->visible)
		{
			for (int i = 0; i < 20; i++)
			{
				MouseParticle particle = MouseParticle();
				particle.pos = mouse->pos;
				particle.angle = (float)getRandom(0, 100) / 100.0f * PI * 2.0f;
				particle.lifetime = getRandom(500, 1500);
				particle.startTime = timer::currentTime;

				mouse->particles.push_back(particle);
			}
		}
		break;
	}
	case MouseState::Locked:
		if (playerEntity != nullptr && playerEntity->IsActive()) {
			point3d mPos = mouse->pos;
			mPos.y *= -1;

			mouse->absolutePos = point3d(window->width / 2, window->height / 2, 0);
			mouse->pos = point3d();
			SetCursorPos(mouse->absolutePos.x, mouse->absolutePos.y);

			CheckTargetValid();
			if (!lockMovementOnTarget || cameraTarget == nullptr) {
				mPos *= MOUSE_SENSIVITY * 10;

				Transform wt = GetWorldTransform(playerEntity);

				XMMATRIX additionalRotation = XMMatrixRotationRollPitchYaw(XMConvertToRadians(mPos.y), XMConvertToRadians(mPos.x), 0);
				point3d upVector = wt.GetUpVector();


				XMMATRIX cameraMatrix = camera->GetMatrixRotation();
				cameraMatrix = additionalRotation * cameraMatrix;
				cameraMatrix = TransformMatrixToUpVector(cameraMatrix, upVector);

				point3d cameraUpVector = point3d(cameraMatrix.r[1].m128_f32[0], cameraMatrix.r[1].m128_f32[1], cameraMatrix.r[1].m128_f32[2]).normalized();
				additionalRotation = XMMatrixRotationRollPitchYaw(0, XMConvertToRadians(mPos.x), 0);

				if (abs(GetSignedAngleBetweenVectors(cameraUpVector, upVector, true)) >= 85) {
					cameraMatrix = camera->GetMatrixRotation();
					cameraMatrix = additionalRotation * cameraMatrix;
					cameraMatrix = TransformMatrixToUpVector(cameraMatrix, upVector);
				}

				point3d cameraLookVector = point3d(cameraMatrix.r[2].m128_f32[0], cameraMatrix.r[2].m128_f32[1], cameraMatrix.r[2].m128_f32[2]).normalized();

				camera->SetMatrixRotation(cameraMatrix);
				camera->position = wt.position - cameraLookVector * camera->distance + cameraUpVector * 2;
			}

			// Обработка атак мышью - проверяем что щит не активен
			if (!abilities->IsShieldActive()) {

				/*if (mouse->IsLButtonDown()) {
					abilities->Charging();
				}
				else if (mouse->IsLButtonUnclicked()) {
					abilities->Attack(*playerTransform, mouse->GetMouseDirection());
				}*/

				/*if (mouse->IsRButtonClicked()) {
					abilities->BlockStart();
				}
				else if (mouse->IsRButtonUnclicked()) {
					abilities->BlockEnd();
				}*/

				if (mouse->IsLButtonClicked()) {
					comboManager->StartHeldInput(ComboInputType::Light);
					Entity* starProjectileEntity = entityStorage->CreateEntity("Star");
					SphereCollider* sphereCollider = starProjectileEntity->AddComponent<SphereCollider>();
					sphereCollider->softness = 0.7;
					sphereCollider->isTouchable = false;
					sphereCollider->radius = 3.0f;
					//sphereCollider->collisionGroup = CollisionFilter::Group::Player;
					Transform* transform = starProjectileEntity->AddComponent<Transform>();
					transform->position = GetWorldTransform(playerEntity).position;
					Star* star = starProjectileEntity->AddComponent<Star>();
					star->color1 = point3d(1.0f, 0, 0);
					star->color2 = point3d(0, 1.0f, 0);
					star->crownColor = point3d(0, 0, 1.0f);
					star->radius = 3.0f;
					PhysicBody* physicBody = starProjectileEntity->AddComponent<PhysicBody>();
					physicBody->velocity = GetLookVectorFromMatrix(camera->GetMatrixRotation())*100;
					physicBody->airFriction = 0;
					SingleDamager* singleDamager = starProjectileEntity->AddComponent<SingleDamager>();
					singleDamager->target = Fraction::Enemy;
					singleDamager->damage = 50;
					singleDamager->destroyable = true;
					DelayedDestroy* starDelayedDestroy = starProjectileEntity->AddComponent<DelayedDestroy>();
					starDelayedDestroy->lifeTime = 5000;
					ParticleEmitter* particleEmitter = starProjectileEntity->AddComponent<ParticleEmitter>();
					particleEmitter->lifetime = 1000;
					particleEmitter->size = { 20.0f, 0.5f };
					particleEmitter->color = point3d(1.0f, 0.5f, 0.0f);
					particleEmitter->opacity = { 0.8f, 0.0f };
					particleEmitter->rate = 70;
				}

				if (mouse->IsRButtonClicked()) {
					comboManager->StartHeldInput(ComboInputType::Heavy);
				}

				if (mouse->IsLButtonReleased()) {
					comboManager->SaveInput(ComboInputType::Light);
				}

				if (mouse->IsRButtonReleased()) {
					comboManager->SaveInput(ComboInputType::Heavy);
				}
			}
		}
		break;
	}

	comboManager->Update();
}


void PlayerController::ProcessUI()
{
	Transform2D* healthTransform = healthBar->GetComponent<Transform2D>();
	healthTransform->scale = point3d(playerHealth->GetHealthRatio(), 1, 0);

	Transform2D* staminaTransform = staminaBar->GetComponent<Transform2D>();
	staminaTransform->scale = point3d(abilities->stamina / abilities->maxStamina, 1, 0);

	TextLabel* elementText = elementLabel->GetComponent<TextLabel>();
	switch (abilities->element)
	{
		case Elements::None:
			elementText->textW = L"НЕТ";
			break;
		case Elements::Air:
			elementText->textW = L"ВОЗДУХ";
			break;
		case Elements::Fire:
			elementText->textW = L"ОГОНЬ";
			break;
		case Elements::Water:
			elementText->textW = L"ВОДА";
			break;
		case Elements::Earth:
			elementText->textW = L"ЗЕМЛЯ";
			break;
	}
}


void PlayerController::Dash()
{
	if (!movementLocked && timer::currentTime >= lastDashTime + DASH_CD && abilities->stamina >= DASH_COST) {
		lastDashTime = timer::currentTime;

		point3d velocity;
		if (playerPhysicBody->velocity.magnitude() > 0) {
			velocity = playerPhysicBody->velocity.normalized();
		}
		else {
			velocity = playerTransform->GetLookVector();
		}

		playerPhysicBody->velocity = velocity * DASH_SPEED;
		movementLocked = true;
		playerPhysicBody->airFriction = DASH_AIR_FRICTION;

		abilities->stamina -= DASH_COST;

		comboManager->SaveInput(ComboInputType::Dash);
	}
}


void PlayerController::LockOnTarget()
{
	if (!lockMovementOnTarget) {
		point3d mouseDirection = mouse->GetMouseDirection();
		RayInfo rayInfo = RayInfo(camera->position, mouseDirection * RAY_DISTANCE, CollisionFilter::Group::PlayerRay, false);
		RaycastResult result = collisionManager->Raycast(rayInfo);

		if (result.hit) {
			pair<Entity*, CameraTarget*> targetRes = result.entity->GetAncestorWithComponent<CameraTarget>();
			if (targetRes.first != nullptr) {
				cameraTarget = targetRes.first;
				lockMovementOnTarget = true;
			}
		}
	}
	else {
		lockMovementOnTarget = false;
		cameraTarget = nullptr;
	}
}


void PlayerController::CheckTargetValid()
{
	if (cameraTarget == nullptr)
		return;

	if (!IsEntityValid(cameraTarget)) {
		lockMovementOnTarget = false;
		cameraTarget = nullptr;
	}
}


void PlayerController::ProcessMovement()
{
	switch (movementType)
	{
	case MovementType::Classic:
		isRunning = false;

		// Проверяем Shift и наличие выносливости
		if (input::IsKeyDown(VK_SHIFT) && abilities->stamina > 0) {
			isRunning = true;
			currentMaxSpeed = PLAYER_MOVE_SPEED * 10.0f;

			// Тратим выносливость только если двигаемся
			if (input::IsKeyDown('W') || input::IsKeyDown('S') || input::IsKeyDown('A') || input::IsKeyDown('D')) {
				abilities->stamina = max(0.0f, abilities->stamina - 0.5f);
			}
		}
		else {
			// Если Shift не нажат - возвращаем обычную скорость
			currentMaxSpeed = PLAYER_MOVE_SPEED;
		}

		// Восстанавливаем выносливость, если не бежим
		if (!isRunning && abilities->stamina < abilities->maxStamina) {
			abilities->stamina = min(abilities->maxStamina, abilities->stamina + 0.2f);
		}


		if (movementLocked && playerPhysicBody->velocity.magnitude() < PLAYER_MOVE_SPEED) {
			movementLocked = false;
			playerPhysicBody->airFriction = 1;
		}

		CheckTargetValid();
		if (lockMovementOnTarget && cameraTarget != nullptr) {
			point3d targetPos = GetWorldTransform(cameraTarget).position;
			point3d playerPos = GetWorldTransform(playerEntity).position;

			point3d direction = targetPos - playerPos;
			playerTransform->mRotation = LerpMatrix(playerTransform->mRotation, GetMatrixFromDirection(direction, playerTransform->GetUpVector()), 0.25f);
		}

		if (!movementLocked)
		{
			point3d velocity = point3d();
			point3d upVector = playerTransform->GetUpVector();
			XMMATRIX cameraMatrix = camera->GetMatrixRotation();

			point3d lookVector = point3d(cameraMatrix.r[2].m128_f32[0], cameraMatrix.r[2].m128_f32[1], cameraMatrix.r[2].m128_f32[2]).normalized();
			lookVector = (lookVector - upVector * lookVector.dot(upVector)).normalized();

			point3d rightVecttor = point3d(cameraMatrix.r[0].m128_f32[0], cameraMatrix.r[0].m128_f32[1], cameraMatrix.r[0].m128_f32[2]).normalized();
			rightVecttor = (rightVecttor - upVector * rightVecttor.dot(upVector)).normalized();

			if (input::IsKeyDown('W')) {
				velocity += lookVector;
			}
			if (input::IsKeyDown('S')) {
				velocity += lookVector * -1;
			}
			if (input::IsKeyDown('A')) {
				velocity += rightVecttor * -1;
			}
			if (input::IsKeyDown('D')) {
				velocity += rightVecttor;
			}
			if (input::IsKeyDown(VK_SPACE)) {
				velocity += upVector;
			}
			if (input::IsKeyDown(VK_CONTROL)) {
				velocity += upVector * -1;
			}

			if (velocity.magnitude() > 0) {
				/*point3d newVelocity = playerPhysicBody->velocity + velocity.normalized();
				if (newVelocity.magnitude() > currentMaxSpeed) {
					playerPhysicBody->velocity = newVelocity.normalized() * currentMaxSpeed;
				}
				else {
					playerPhysicBody->velocity = newVelocity;
				}*/

				velocity = velocity.normalized();

				playerPhysicBody->acceleration += velocity * currentMaxSpeed;

				float factor = velocity.dot(upVector);
				if (factor > -1 && factor < 1) {
					playerTransform->mRotation = GetMatrixFromDirection((velocity - upVector * factor).normalized(), upVector);
				}
			}
		}
		break;
	case MovementType::Stopped:
		break;
	}
}