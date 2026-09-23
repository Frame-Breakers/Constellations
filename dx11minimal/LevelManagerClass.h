////////////////////////////////////////////////////////////////////////////////
// Filename: LevelManagerClass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _LEVELMANAGERCLASS_H_
#define _LEVELMANAGERCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <thread>
#include <vector>

#include "Engine/Lib/interp.h"

#include "Engine/Window/windowclass.h"
#include "Engine/Mouse/mouseclass.h"

#include "ecsHeaders.h"

#ifdef _EDITOR
#include "EditorTools/editorclass.h"
#include "EditorTools/EditorUI.h"
#else
#include "Gameplay/Player/PlayerController.h"
#endif

#include "Gameplay/MazeLinkSystem.h"


/////////////
// GLOBALS //
/////////////
//const bool VSYNC_ENABLED = true;
#define SHOW_COLLIDERS true
#define SHOW_GRAVITY false
#define SHOW_PIVOTS false
#define SHOW_LINK_RADIUS false


////////////////////////////////////////////////////////////////////////////////
// Class name: LevelManagerClass
////////////////////////////////////////////////////////////////////////////////


class LevelManagerClass
{
public:
	WindowClass* window;
	MouseClass* mouse;
	EntityStorage* entityStorage;

public:
	LevelManagerClass();
	LevelManagerClass(const LevelManagerClass&);
	~LevelManagerClass();

	void InitWindow();
	bool Initialize();
	void Shutdown();

	void Frame();

private:
	World* m_World;

#ifdef _EDITOR
	EditorClass* editor;
	EditorUI* editorUI;
#else
	PlayerController* playerController;
#endif

	CollisionManagerClass* collisionManager;
	QuestManager* questManager;

	Entity* m_TestAnimEntity = nullptr;
	Transform2DDebugUI m_Transform2DDebugUI;
	int m_TestAnimCycleIndex = 0;
	bool m_WasToggleAnimationPressed = false;

	void LoadModels();
	void LoadTextures();
	void LoadSounds();

	Entity* CreatePlayer(Entity* = nullptr);

	void CreateUI();
	void InitSystems();

	void CreateSpaceBackground(Entity*, int);
	void CreateAries(Entity*);
	void CreateZenithLocation(Entity*, int);
	void CreateNebula(Entity*, int);
	void CreateStarQuestLoc(Entity*, int);
	void UpdateTestAnimationToggle();
	void ShowGameOverMessage(const wchar_t* message, const point3d& color);

	ID3D11Buffer* m_BoneBuffer = nullptr;
	SkinnedMesh m_FoxMesh;
	Skeleton    m_FoxSkeleton;
	std::vector<AnimationClip> m_FoxAnimations;
	SkinnedMesh m_CesiumMesh;
	Skeleton    m_CesiumSkeleton;
	std::vector<AnimationClip> m_CesiumAnimations;


	SkinnedMesh m_PunchComboNewMesh;
	Skeleton    m_PunchComboNewSkeleton;
	std::vector<AnimationClip> m_PunchComboNewAnimations;

	SkinnedMesh m_TestAnimMesh;
	Skeleton    m_TestAnimSkeleton;
	std::vector<AnimationClip> m_TestAnimAnimations;
private: // AI amogus
	Entity* worldFolder;

	bool m_ShowGameOverMessage = false;
	bool m_ShowVictoryMessage = false;
	float m_MessageTimer = 0.0f;
	
};

#endif
