#include "PlayScene.h"
#include "SceneIncludes.h"
#include "MathFunc.h"

void PlayScene::Init()
{
    Logger::Write("現在シーンPlayScene");
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("bgm_play", "bgm_play.wav");
	soundMgr->PlayBGM("bgm_play");

	//===========================
	// カメラマネージャー
	//===========================
	auto camMgr = CameraManager::GetInstance();
	auto entityCommon = Entity3DCommon::GetInstance();
	entityCommon->SetCameraManager(camMgr);
	entityCommon->SetDebugCamera(camMgr->GetDebugCamera());
	entityCommon->SetDefaultCamera(camMgr->GetActiveCamera());

	// カメラの初期位置設定
	auto camera = camMgr->GetCamera("MainCamera");
	Vector3 camPos = { 0.0f, 0.0f, -105.0f };
	Vector3 camRot = { 0.0f, 0.0f, 0.0f };
	camera->SetTranslate(camPos);
	camera->SetRotate(camRot);

	//===========================
	// テクスチャ
	//===========================
	auto texMgr = TextureManager::GetInstance();
	texReticle_ = texMgr->Load("resources/sprites/ui_reticle.png");
	texUiPlayOperate_ = texMgr->Load("resources/sprites/ui_play_operate.png");
	auto texUiController = texMgr->Load("resources/sprites/controllerPlayScene.png");

	//===========================
	// モデルロード
	//===========================
	auto modelMgr = ModelManager::GetInstance();
	modelMgr->LoadModel("bullet.obj");
	modelMgr->LoadModel("enemy.obj");

	//===========================
	// スプライト
	//===========================
	sprReticle_ = std::make_unique<Sprite>();
	sprReticle_->Init();
	sprReticle_->Create(texReticle_, { 0.0f, 0.0f }, Color::WHITE);
	sprReticle_->SetAnchorPoint({ 0.5f, 0.5f });
	Editor::GetInstance()->RegisterSprite("sprReticle", sprReticle_.get());

	sprUiPlayOperate_ = std::make_unique<Sprite>();
	sprUiPlayOperate_->Init();
	sprUiPlayOperate_->Create(texUiPlayOperate_, { 845.0f, 668.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiPlayOperate", sprUiPlayOperate_.get());

	sprUiConroller_ = std::make_unique<Sprite>();
	sprUiConroller_->Init();
	sprUiConroller_->Create(texUiController, { 845.0f, 668.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiConroller", sprUiConroller_.get());

	//===========================
	// モデル
	//===========================
	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("starSkyDome");
	Editor::GetInstance()->RegisterModel("starSkyDome", modelSkydome_.get());
	
	//===========================
	// クラス
	//===========================
	player_ = std::make_shared<Player>();
	player_->Init({ 0.0f, 0.0f, -10.0f });

	enemyMgr_ = std::make_unique<EnemyManager>();
	enemyMgr_->Init(player_);

	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
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
	UpdateReticle();
	Vector3 aimTarget = CalculateAimTarget();
	player_->SetAimTarget(aimTarget);

	player_->Update();
	isController_ = player_->GetIsConroller();
	modelSkydome_->Update();
	enemyMgr_->Update();

	// スプライトの更新処理
	sprReticle_->Update();
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

	//================================
	// 照準のデバッグ表示
	//================================
	Vector3 playerPosition =
		player_->GetModel()->GetTranslate();

	// すでに上で計算したaimTargetを使う
	Vector3 difference{
		aimTarget.x - playerPosition.x,
		aimTarget.y - playerPosition.y,
		aimTarget.z - playerPosition.z
	};

	float targetDistance = sqrtf(
		difference.x * difference.x +
		difference.y * difference.y +
		difference.z * difference.z
	);

	ImGui::Begin("Aim Debug");

	ImGui::Text(
		"Player Position : %.2f, %.2f, %.2f",
		playerPosition.x,
		playerPosition.y,
		playerPosition.z
	);

	ImGui::Text(
		"Target Position : %.2f, %.2f, %.2f",
		aimTarget.x,
		aimTarget.y,
		aimTarget.z
	);

	ImGui::Text(
		"Target Distance : %.2f",
		targetDistance
	);

	ImGui::End();

    ImGuiManager::GetInstance()->EndFrame();
}

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
	// Model
	player_->Draw();
	enemyMgr_->Draw();
	modelSkydome_->Draw();

	player_->DebugDraw();
	enemyMgr_->DebugDraw();
	DebugDraw::Draw();

	// Sprite
	sprReticle_->Draw();

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

void PlayScene::UpdateReticle()
{
	if (CameraManager::GetInstance()->GetIsDebug()) {
		return;
	}

	Camera* camera = CameraManager::GetInstance()->GetCamera("MainCamera");

	if (!camera) {
		return;
	}

	Transform playerTransform = player_->GetModel()->GetTransform();
	Transform cameraTransform = CameraManager::GetInstance()->GetActiveCamera()->GetTransform();

	// 前方向取得
	Vector3 forward = GetForwardFromTransform(playerTransform.rotate);
	float length = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);

	if (length > 0.0001f) {
		forward.x /= length;
		forward.y /= length;
		forward.z /= length;
	}

	// 3Dレティクル位置
	reticleWorldPosition_ = playerTransform.translate + forward * 50.0f;

	// View / Projection
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);

	Matrix4x4 projectionMatrix =
		MakePerspectiveFovMatrix(
			0.45f,
			float(Graphics::GetWidth()) / float(Graphics::GetHeight()),
			0.1f,
			100.0f);

	// 3Dから2D
	Vector3 clip = TransformToVector3(reticleWorldPosition_, viewMatrix * projectionMatrix);

	reticleScreenPosition_.x = (clip.x + 1.0f) * 0.5f * Graphics::GetWidth();
	reticleScreenPosition_.y = (1.0f - clip.y) * 0.5f * Graphics::GetHeight();

	// UI反映
	sprReticle_->SetPosition(reticleScreenPosition_);
}

Vector3 PlayScene::CalculateAimTarget()
{
	Camera* camera = CameraManager::GetInstance()->GetCamera("MainCamera");

	if (!camera) {
		return reticleWorldPosition_;
	}

	Transform cameraTransform = camera->GetTransform();

	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);

	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
		0.45f,
		static_cast<float>(Graphics::GetWidth()) /
		static_cast<float>(Graphics::GetHeight()),
		0.1f,
		100.0f
	);

	Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

	Vector3 playerPosition = player_->GetModel()->GetTranslate();

	Vector3 aimTarget = reticleWorldPosition_;
	float nearestScreenDistance = kReticleRadius;

	for (const auto& enemy : enemyMgr_->GetEnemies()) {
		if (!enemy->GetIsAlive()) {
			continue;
		}

		Vector3 enemyPosition = enemy->GetPosition();

		Vector3 toEnemy{
			enemyPosition.x - playerPosition.x,
			enemyPosition.y - playerPosition.y,
			enemyPosition.z - playerPosition.z
		};

		float distance = sqrtf(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y + toEnemy.z * toEnemy.z);

		// 自機から50.0fより遠い敵は狙わない
		if (distance > kAimDistance) {
			continue;
		}

		Vector3 enemyNdc = TransformToVector3(enemyPosition, viewProjectionMatrix);

		Vector2 enemyScreenPosition{};

		enemyScreenPosition.x = (enemyNdc.x + 1.0f) * 0.5f * Graphics::GetWidth();

		enemyScreenPosition.y = (1.0f - enemyNdc.y) * 0.5f * Graphics::GetHeight();

		float dx = enemyScreenPosition.x - reticleScreenPosition_.x;

		float dy = enemyScreenPosition.y - reticleScreenPosition_.y;

		float screenDistance = sqrtf(dx * dx + dy * dy);

		if (screenDistance <= nearestScreenDistance) {
			nearestScreenDistance = screenDistance;

			// レティクル内にいる敵の中心を照準地点にする
			aimTarget = enemyPosition;
		}
	}

	return aimTarget;
}

void PlayScene::UpdateAimCamera()
{
	auto camMgr = CameraManager::GetInstance();

	if (camMgr->GetIsDebug()) {
		return;
	}

	Camera* camera = camMgr->GetCamera("MainCamera");

	if (!camera) {
		return;
	}

	Input* input = Input::GetInstance();

	Vector2 rightStick = input->GetXbRightStickVector();

	// スティックのデットゾーン
	if (fabsf(rightStick.x) < rightStickDeadZone_) {
		rightStick.x = 0.0f;
	}

	if (fabsf(rightStick.y) < rightStickDeadZone_) {
		rightStick.y = 0.0f;
	}

	cameraYaw_ += rightStick.x * cameraSensitivity_;
	cameraPitch_ += rightStick.y * cameraSensitivity_;

	// 真上、真下過ぎない制限
	cameraPitch_ = std::clamp(cameraPitch_, -1.2f, 1.2f);

	Vector3 cameraRotate = {
		cameraPitch_,
		cameraYaw_,
		0.0f
	};
	
	camera->SetRotate(cameraRotate);
}