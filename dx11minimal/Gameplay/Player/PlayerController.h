////////////////////////////////////////////////////////////////////////////////
// Filename: PlayerController.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _PLAYERCONTROLLER_H_
#define _PLAYERCONTROLLER_H_


//////////////
// INCLUDES //
//////////////
#include "../../Engine/Lib/isingleton.h"
#include "../../Engine/Lib/input.h"

#include "../../Engine/Camera/cameraclass.h"
#include "../../Engine/Mouse/mouseclass.h"
#include "../../Engine/Window/windowclass.h"

#include "../../Engine/Lib/timer.h"
#include "../../Engine/ECS_Base/world.h"
#include "../../Engine/ECS_Base/entity.h"

#include "../../Engine/BasicComponents/Transform.h"
#include "../../Engine/Physic/Movement/PhysicBody.h"
#include "../../Engine/Render/PointCloud.h"
#include "../../Engine/Compute/Combat/Health.h"
#include "CameraTarget.h"

#include "PlayerAbilities.h"
#include "MovementTypes.h"
#include "Combo/comboManager.h"

#include "../../GlobalConfigs.h"


/////////////
// GLOBALS //
/////////////
const float CURSOR_IGNORE_ZONE = 0.05f;
const float MAX_CURSOR_DEVIATION = 0.45f;
const float CURSOR_ZONE_DELTA = MAX_CURSOR_DEVIATION - CURSOR_IGNORE_ZONE;

#define ROLL_SPEED 1.0f
#define PLAYER_MOVE_SPEED 100.0f

#define DASH_SPEED 200.0f
#define DASH_AIR_FRICTION 0.35f
#define DASH_COST 50.0f
#define DASH_CD 1000


////////////////////////////////////////////////////////////////////////////////
// Class name: PlayerController
////////////////////////////////////////////////////////////////////////////////

class PlayerController
{
public:
	PlayerAbilities* abilities;

public:
	PlayerController();
	PlayerController(const PlayerController&);
	~PlayerController();

	void Initialize(Entity*);
	void Shutdown();

	void ProcessInput();
	void ProcessCamera();
	void ProcessMouse();
	void ProcessUI();

	void ProcessMovement();

private:
	ComboManager* comboManager;
	MovementType movementType;

	EntityStorage* entityStorage;
	CollisionManagerClass* collisionManager;

	Entity* playerEntity;
	Entity* ui;
	Entity* starProjectileEntity = nullptr;
	SphereCollider* sphereColliderProjectile = nullptr;
	Transform* transformProjectile = nullptr;
	PhysicBody* physicBodyProjectile = nullptr;
	SingleDamager* singleDamagerProjectile = nullptr;
	DelayedDestroy* starDelayedDestroyProjectile = nullptr;
	ParticleEmitter* particleEmitterProjectile = nullptr;
	double currentTime;
	Star* starProjectile = nullptr;
	Transform* playerTransform;
	PhysicBody* playerPhysicBody;
	PointCloud* playerPointCloud;
	Health* playerHealth;

	Entity* healthBar;
	Entity* staminaBar;
	Entity* bossHealthBar;
	Entity* elementLabel;

	CameraClass* camera;
	MouseClass* mouse;
	WindowClass* window;

	bool movementLocked = false;

	float currentMaxSpeed;
	bool isRunning;

	DWORD lastDashTime = 0;

	Entity* cameraTarget;
	bool lockMovementOnTarget;

private:
	void Dash();
	void LockOnTarget();
	void CheckTargetValid();
};

#endif