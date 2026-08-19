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
	modelMgr->LoadModel("terrain.obj");

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

	modelTerrain_ = std::make_unique<Entity3D>();
	modelTerrain_->Init();
	modelTerrain_->SetModel("terrain");
	modelTerrain_->SetTranslate({ 0.0f, -10.0f, 0.0f });
	modelTerrain_->SetScale({ 5.0f, 5.0f, 50.0f });
	Editor::GetInstance()->RegisterModel("terrain", modelTerrain_.get());
	
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

	/*if (isGameStop_) {
		enemyMgr_->SetIsMoveStop(true);
	} else {
		enemyMgr_->SetIsMoveStop(false);
	}*/
#endif
	UpdateReticle();
	Vector3 aimTarget = CalculateAimTarget();
	player_->SetAimTarget(aimTarget);
	player_->Update();

	isController_ = player_->GetIsConroller();
	modelSkydome_->Update();
	//enemyMgr_->Update();

	modelTerrain_->SetCamera(camMgr->GetActiveCamera());
	Vector3 terrainPos = modelTerrain_->GetTranslate();
	terrainPos.z -= 0.1f;
	modelTerrain_->SetTranslate(terrainPos);
	modelTerrain_->Update();

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

	Vector2 rightStick = Input::GetInstance()->GetXbRightStickVector();

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

	ImGui::Text(
		"RightStick : %.2f, %.2f",
		rightStick.x,
		rightStick.y
	);

	ImGui::End();

    ImGuiManager::GetInstance()->EndFrame();
}

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
	// Model
	player_->Draw();
	//enemyMgr_->Draw();
	modelSkydome_->Draw();
	modelTerrain_->Draw();

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

	Input* input = Input::GetInstance();

	Vector2 leftStick = input->GetXbLeftStickVector();
	Vector2 rightStick = input->GetXbRightStickVector();

	constexpr float deadZone = 0.15f;

	if (fabsf(leftStick.x) < deadZone) {
		leftStick.x = 0.0f;
	}

	if (fabsf(leftStick.y) < deadZone) {
		leftStick.y = 0.0f;
	}

	if (fabsf(rightStick.x) < deadZone) {
		rightStick.x = 0.0f;
	}

	if (fabsf(rightStick.y) < deadZone) {
		rightStick.y = 0.0f;
	}

	//=========================================
	// 右スティックによる自由照準
	//=========================================
	aimOffset_.x += rightStick.x * aimSpeed_;

	// 上入力でレティクルを上へ動かす
	aimOffset_.y -= rightStick.y * aimSpeed_;

	// 右スティックを離したら徐々に中央へ戻す
	if (rightStick.x == 0.0f) {
		aimOffset_.x += (0.0f - aimOffset_.x) * aimReturnSpeed_;
	}

	if (rightStick.y == 0.0f) {
		aimOffset_.y += (0.0f - aimOffset_.y) * aimReturnSpeed_;
	}

	//=========================================
	// 左スティック方向へ照準を先行させる
	//=========================================
	Vector2 targetMoveOffset{
		leftStick.x * moveAimDistance_,
		-leftStick.y * moveAimDistance_
	};

	// 急に動かず、少し遅れて追従
	moveAimOffset_.x += (targetMoveOffset.x - moveAimOffset_.x) * 0.15f;

	moveAimOffset_.y += (targetMoveOffset.y - moveAimOffset_.y) * 0.15f;

	//=========================================
	// レティクルの移動範囲
	//=========================================
	aimOffset_.x = std::clamp(aimOffset_.x, -reticleLimitX_, reticleLimitX_);

	aimOffset_.y = std::clamp(aimOffset_.y, -reticleLimitY_, reticleLimitY_);

	Vector2 screenCenter{
		static_cast<float>(Graphics::GetWidth()) * 0.5f,
		static_cast<float>(Graphics::GetHeight()) * 0.5f
	};

	reticleScreenPosition_ = {
		screenCenter.x + aimOffset_.x + moveAimOffset_.x,
		screenCenter.y + aimOffset_.y + moveAimOffset_.y
	};

	// 画面外へ出ないようにする
	constexpr float margin = 32.0f;

	reticleScreenPosition_.x = std::clamp(
		reticleScreenPosition_.x,
		margin,
		static_cast<float>(Graphics::GetWidth()) - margin
	);

	reticleScreenPosition_.y = std::clamp(
		reticleScreenPosition_.y,
		margin,
		static_cast<float>(Graphics::GetHeight()) - margin
	);

	sprReticle_->SetPosition(reticleScreenPosition_);
}

Vector3 PlayScene::CalculateAimTarget()
{
	Camera* camera = CameraManager::GetInstance()->GetCamera("MainCamera");

	if (!camera) {
		return reticleWorldPosition_;
	}

	Transform cameraTransform = camera->GetTransform();

	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		cameraTransform.scale,
		cameraTransform.rotate,
		cameraTransform.translate
	);

	Matrix4x4 viewMatrix = Inverse(cameraMatrix);

	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
		0.45f,
		static_cast<float>(Graphics::GetWidth()) /
		static_cast<float>(Graphics::GetHeight()),
		0.1f,
		1000.0f
	);

	Matrix4x4 viewProjectionMatrix =
		viewMatrix * projectionMatrix;

	Matrix4x4 inverseViewProjection =
		Inverse(viewProjectionMatrix);

	float screenWidth = static_cast<float>(Graphics::GetWidth());

	float screenHeight = static_cast<float>(Graphics::GetHeight());

	// スクリーン座標をNDCへ変換
	float ndcX = (reticleScreenPosition_.x / screenWidth) * 2.0f - 1.0f;

	float ndcY = 1.0f - (reticleScreenPosition_.y / screenHeight) * 2.0f;

	Vector3 nearPosition = TransformToVector3(
		{ ndcX, ndcY, 0.0f },
		inverseViewProjection
	);

	Vector3 farPosition = TransformToVector3(
		{ ndcX, ndcY, 1.0f },
		inverseViewProjection
	);

	Vector3 rayDirection{
		farPosition.x - nearPosition.x,
		farPosition.y - nearPosition.y,
		farPosition.z - nearPosition.z
	};

	float rayLength = sqrtf(
		rayDirection.x * rayDirection.x +
		rayDirection.y * rayDirection.y +
		rayDirection.z * rayDirection.z
	);

	if (rayLength > 0.0001f) {
		rayDirection.x /= rayLength;
		rayDirection.y /= rayLength;
		rayDirection.z /= rayLength;
	}

	Vector3 playerPosition =
		player_->GetModel()->GetTranslate();

	// カメラではなく、自機から一定距離先を照準地点にする
	Vector3 aimTarget{
		playerPosition.x + rayDirection.x * aimDistance_,
		playerPosition.y + rayDirection.y * aimDistance_,
		playerPosition.z + rayDirection.z * aimDistance_
	};

	reticleWorldPosition_ = aimTarget;

	//=========================================
	// レティクル内の敵を探す
	//=========================================
	float nearestScreenDistance = reticleHitRadius_;

	for (const auto& enemy : enemyMgr_->GetEnemies()) {
		if (!enemy->GetIsAlive()) {
			continue;
		}

		Vector3 enemyPosition = enemy->GetPosition();

		Vector3 playerToEnemy{
			enemyPosition.x - playerPosition.x,
			enemyPosition.y - playerPosition.y,
			enemyPosition.z - playerPosition.z
		};

		float enemyDistance = sqrtf(
			playerToEnemy.x * playerToEnemy.x +
			playerToEnemy.y * playerToEnemy.y +
			playerToEnemy.z * playerToEnemy.z
		);

		// 自機から50.0f以内の敵だけを補正対象にする
		if (enemyDistance > aimDistance_) {
			continue;
		}

		// 敵のワールド座標をNDCへ変換
		Vector3 enemyNdc = TransformToVector3(
			enemyPosition,
			viewProjectionMatrix
		);

		Vector2 enemyScreenPosition{
			(enemyNdc.x + 1.0f) * 0.5f * screenWidth,
			(1.0f - enemyNdc.y) * 0.5f * screenHeight
		};

		float dx = enemyScreenPosition.x - reticleScreenPosition_.x;

		float dy = enemyScreenPosition.y - reticleScreenPosition_.y;

		float screenDistance = sqrtf(dx * dx + dy * dy);

		// レティクル内で最も中心に近い敵を選ぶ
		if (screenDistance <= nearestScreenDistance) {
			nearestScreenDistance = screenDistance;
			aimTarget = enemyPosition;
		}
	}

	return aimTarget;
}