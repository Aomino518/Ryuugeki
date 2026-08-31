#include "PlayScene.h"
#include "SceneIncludes.h"
#include "MathFunc.h"

void PlayScene::Init()
{
    Logger::Write("現在シーンPlayScene");
	LoadSound();
	LoadCamera();
	LoadTexture();
	LoadSprite();
	LoadModel();
	InitClass();
    ImGuiManager::GetInstance()->LoadScenesJson();
}

void PlayScene::Update()
{
	auto camMgr = CameraManager::GetInstance();
    /*-- 更新処理 --*/
	auto soundMgr = SoundManager::GetInstance();
	/*-- 更新処理 --*/
	switch (phase_) {
	case ScenePhase::FADEIN:
		if (fade_.IsFinished()) {
			phase_ = ScenePhase::MAIN;
		}
		break;
	case ScenePhase::MAIN:
		UpdatePlay();
		break;
	case ScenePhase::FADEOUT:
		if (fade_.IsFinished()) {
			if (isClear) {
				soundMgr->StopBGM();
				SceneManager::GetInstance()->ChangeScene("CLEAR");
			} else if (isGameOver) {
				soundMgr->StopBGM();
				SceneManager::GetInstance()->ChangeScene("GAMEOVER");
			}
		}
		break;
	}

	// カメラの更新処理
	Camera* mainCamera = camMgr->GetCamera("MainCamera");
	Vector3 camPos = { 0.0f, 0.0f, player_->GetPosition().z - 50.0f};
	mainCamera->SetTranslate(camPos);
	camMgr->Update();

	// モデルの更新処理
	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelSkydome_->SetCamera(camera);
	}

#ifdef _DEBUG
	if (Input::GetInstance()->IsPress(DIK_1)) {
		isGameStop_ = !isGameStop_;
	}

	if (isGameStop_) {
		enemyMgr_->SetIsMoveStop(true);
	} else {
		enemyMgr_->SetIsMoveStop(false);
	}
#endif
	reticle_.Update(player_->GetPosition(), enemyMgr_->GetEnemies());
	player_->SetAimTarget(reticle_.GetAimTarget());

	if (isMovePlayer_) {
		playerMoveTimer_ += Time::GetDeltaTime();
		// 0.0fから1.0fの補間率に変換
		float t = LerpRateConvert(playerMoveTimer_, playerMoveDuration_);
		Vector3 playerPos = player_->GetPosition();
		playerPos.z = Lerp(startPos, endPos, t);
		player_->SetPosition(playerPos);

		if (t >= 1.0f) {
			isMovePlayer_ = false;
		}
	}
	player_->Update();

	isController_ = player_->GetIsConroller();
	modelSkydome_->Update();
	enemyMgr_->Update();

	modelTerrain_->SetCamera(camMgr->GetActiveCamera());
	modelTerrain_->Update();

	// スプライトの更新処理
	sprUiPlayOperate_->Update();
	sprUiConroller_->Update();
	fade_.Update();

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->DrawCameraWindow(camMgr);
    ImGuiManager::GetInstance()->DrawEditor();
    ImGuiManager::GetInstance()->Stats();
	ImGuiManager::GetInstance()->DrawSoundWindow();
    ImGuiManager::GetInstance()->DrawLoggerWindow();
	reticle_.DrawImGui(player_->GetPosition());
	enemyMgr_->DrawImGui();

    ImGuiManager::GetInstance()->EndFrame();
}

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
	// Model
	modelSkydome_->Draw();
	modelTerrain_->Draw();
	player_->Draw();
	enemyMgr_->Draw();

	player_->DebugDraw();
	enemyMgr_->DebugDraw();
	DebugDraw::Draw();

	// Sprite
	reticle_.Draw();

	if (!isController_) {
		sprUiPlayOperate_->Draw();
	} else {
		sprUiConroller_->Draw();
	}

	fade_.Draw();

    ImGuiManager::GetInstance()->Draw();
}

void PlayScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Unload("bgm_play");
    Editor::GetInstance()->Clear();
}

void PlayScene::UpdatePlay()
{
	if (enemyMgr_->GetIsFinished()) {
		isClear = true;
		fade_.Start(Fade::Status::FadeOut, 1.0f);
		phase_ = ScenePhase::FADEOUT;
	}

	if (!player_->GetIsAlive()) {
		isGameOver = true;
		fade_.Start(Fade::Status::FadeOut, 1.0f);
		phase_ = ScenePhase::FADEOUT;
	}
}

void PlayScene::LoadSound()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("bgm_play", "bgm_play.wav");
	soundMgr->PlayBGM("bgm_play");
}

void PlayScene::LoadCamera()
{
	auto camMgr = CameraManager::GetInstance();
	auto entityCommon = Entity3DCommon::GetInstance();
	entityCommon->SetCameraManager(camMgr);
	entityCommon->SetDebugCamera(camMgr->GetDebugCamera());
	entityCommon->SetDefaultCamera(camMgr->GetActiveCamera());

	// カメラの初期位置設定
	auto camera = camMgr->GetCamera("MainCamera");
	Vector3 camPos = { 0.0f, 0.0f, -60.0f };
	Vector3 camRot = { 0.0f, 0.0f, 0.0f };
	camera->SetTranslate(camPos);
	camera->SetRotate(camRot);
}

void PlayScene::LoadTexture()
{
	auto texMgr = TextureManager::GetInstance();
	texReticle_ = texMgr->Load("resources/sprites/ui_reticle.png");
	texUiPlayOperate_ = texMgr->Load("resources/sprites/ui_play_operate.png");
	texUiController_ = texMgr->Load("resources/sprites/controllerPlayScene.png");
}

void PlayScene::LoadSprite()
{
	sprUiPlayOperate_ = std::make_unique<Sprite>();
	sprUiPlayOperate_->Init();
	sprUiPlayOperate_->Create(texUiPlayOperate_, { 845.0f, 668.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiPlayOperate", sprUiPlayOperate_.get());

	sprUiConroller_ = std::make_unique<Sprite>();
	sprUiConroller_->Init();
	sprUiConroller_->Create(texUiController_, { 845.0f, 668.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiConroller", sprUiConroller_.get());
}

void PlayScene::LoadModel()
{
	auto modelMgr = ModelManager::GetInstance();
	modelMgr->LoadModel("bullet.obj");
	modelMgr->LoadModel("enemy.obj");
	modelMgr->LoadModel("terrain.obj");
	modelMgr->LoadModel("boss.obj");

	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("starSkyDome");
	Editor::GetInstance()->RegisterModel("starSkyDome", modelSkydome_.get());

	modelTerrain_ = std::make_unique<Entity3D>();
	modelTerrain_->Init();
	modelTerrain_->SetModel("terrain");
	modelTerrain_->SetTranslate({ 0.0f, -10.0f, 0.0f });
	modelTerrain_->SetScale({ 5.0f, 5.0f, 50.0f });
	Editor::GetInstance()->RegisterModel("terrain", modelTerrain_.get());
}

void PlayScene::InitClass()
{
	player_ = std::make_shared<Player>();
	player_->Init(Vector3{ 0.0f, 0.0f, -10.0f });

	enemyMgr_ = std::make_unique<EnemyManager>();
	enemyMgr_->Init(player_);

	reticle_.Init();

	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
}
