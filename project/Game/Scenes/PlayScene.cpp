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
	auto camMgr = CameraManager::GetInstance();
	std::string error;
	camMgr->CreateCamera("Camera1");
	camMgr->SetActiveCameraByName("Camera1");
	if (!camMgr->LoadCameraFile(
		"resources/json/cameras/startPlayScene.json", error)) {
		Logger::Write(Logger::LogLevel::Warning, "カメラ設定の読み込みに失敗しました: " + error);
	}
	// フェード中から演出の開始位置を表示し、移動はフェード終了後に始める。
	auto* startCamera = camMgr->GetCamera("Camera1");
	startCamera->SetTranslate(startCamera->GetStartPos());
	startCamera->SetRotate(startCamera->GetStartRot());
	startCamera->SetIsLerp(false);
	startCamera->Update();
	Entity3DCommon::GetInstance()->SetDefaultCamera(startCamera);

	// SkyDomeだけライティングオフ
	auto* skyModel = ModelManager::GetInstance()->FindModel("skydome");

	if (skyModel) {
		skyModel->SetIsLighting(false);
	}

	phase_ = ScenePhase::FADEIN;
	fade_.Start(Fade::Status::FadeIn, 1.0f);

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
			camMgr->GetCamera("Camera1")->StartLerp();
			phase_ = ScenePhase::CAMMOTION;
		}
		break;
	case ScenePhase::CAMMOTION:
		if (camMgr->GetCamera("Camera1")->GetIsLerpEnd()) {
			camMgr->SetActiveCameraByName("MainCamera");
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

	player_->Update();
	UpdatePlayerCamera();

	isController_ = player_->GetIsConroller();

	// スカイドームをカメラを中心に追従
	auto skyDomePos = camMgr->GetIsDebug() ? camMgr->GetDebugCamera()->GetTranslate() : camMgr->GetActiveCamera()->GetTranslate();
	modelSkydome_->SetTranslate(skyDomePos);
	enemyMgr_->Update();

	// モデルの更新処理
	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelSkydome_->SetCamera(camera);
	}
	modelSkydome_->Update();

	UpdateTerrain();

	// スプライトの更新処理
	sprUiPlayOperate_->Update();
	sprUiConroller_->Update();
	fade_.Update();
}

void PlayScene::Draw()
{
	/*-- 描画処理 --*/
	// Model
	modelSkydome_->Draw();
	modelTerrains_->Draw();
	player_->Draw();
	enemyMgr_->Draw();

	player_->DebugDraw();
	enemyMgr_->DebugDraw();
	DebugDraw::Draw();

	// Sprite
	enemyMgr_->DrawRushWarning();

	if (phase_ == ScenePhase::MAIN) {
		reticle_.Draw();
	}

	if (!isController_) {
		sprUiPlayOperate_->Draw();
	} else {
		sprUiConroller_->Draw();
	}

	fade_.Draw();
}

void PlayScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	auto* sound = SoundManager::GetInstance();
	sound->StopBGM();
	sound->StopSE();
	soundMgr->Unload("bgm_play");
	CameraManager::GetInstance()->RemoveCamera("Camera1");
	Editor::GetInstance()->Clear();
}

void PlayScene::DrawImGui()
{
#ifdef USE_IMGUI
	reticle_.DrawImGui(player_->GetPosition());
	enemyMgr_->DrawImGui();
	player_->DrawImGui();
#endif
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

void PlayScene::UpdatePlayerCamera()
{
	auto camMgr = CameraManager::GetInstance();

	// カメラスクロール
	const Vector3 playerPos = player_->GetPosition();
	const float playerX = playerPos.x;
	const float offsetX = playerX - cameraX_;

	if (offsetX > cameraDeadZone_) {
		cameraX_ = playerX - cameraDeadZone_;
	} else if (offsetX < -cameraDeadZone_) {
		cameraX_ = playerX + cameraDeadZone_;
	}

	const float playerY = playerPos.y;
	const float offsetY = playerY - cameraY_;

	if (offsetY > cameraDeadZone_) {
		cameraY_ = playerY - cameraDeadZone_;
	} else if (offsetY < -cameraDeadZone_) {
		cameraY_ = playerY + cameraDeadZone_;
	}

	const Vector3 scrollPos = player_->GetScrollPosition();
	cameraX_ = std::clamp(cameraX_, scrollPos.x - cameraLimit_, scrollPos.x + cameraLimit_);
	cameraY_ = std::clamp(cameraY_, scrollPos.y - cameraLimit_, scrollPos.y + cameraLimit_);

	// 上下左右は既存のスクロール範囲を保ち、奥行きはプレイヤーからの距離を保つ。
	Camera* mainCamera = camMgr->GetCamera("MainCamera");
	Vector3 camPos = playerPos;
	camPos.x = cameraX_;
	camPos.y = cameraY_;
	camPos.z -= cameraDistance_;
	mainCamera->SetTranslate(camPos);
	camMgr->Update();
}

void PlayScene::LoadSound()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("bgm_play", "bgm_play.wav");
	SoundManager::GetInstance()->PlayBGM("bgm_play");
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
	modelMgr->LoadModel("greenTerrain.obj");
	modelMgr->LoadModel("boss.obj");

	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("skydome");
	modelSkydome_->SetPicked(false);
	Editor::GetInstance()->RegisterModel("skydome", modelSkydome_.get());

	modelTerrains_ = std::make_unique<Entity3D>();
	modelTerrains_->Init();
	modelTerrains_->SetModel("greenTerrain");
	modelTerrains_->SetTranslate(Vector3{ 0.0f, -20.0f, 900.0f });
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

void PlayScene::UpdateTerrain()
{
	auto* cameraManager = CameraManager::GetInstance();
	auto* camera = cameraManager->GetActiveCamera();
	
	if (camera) {
		modelTerrains_->SetCamera(camera);
	}
	modelTerrains_->Update();
}
