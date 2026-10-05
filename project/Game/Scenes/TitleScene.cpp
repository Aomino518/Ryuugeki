#include "TitleScene.h"
#include "SceneIncludes.h"

void TitleScene::Init()
{
    Logger::Write("現在シーンTitleScene");
	LoadSound();
	LoadCamera();
	LoadTexture();
	LoadModel();
	LoadSprite();
	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
    ImGuiManager::GetInstance()->LoadScenesJson();
	auto* skyModel = ModelManager::GetInstance()->FindModel("skydome");

	if (skyModel) {
		skyModel->SetIsLighting(false);
	}
}

void TitleScene::Update()
{
	auto camMgr = CameraManager::GetInstance();
	auto soundMgr = SoundManager::GetInstance();
	switch (phase_) {
	case ScenePhase::FADEIN:
		if (fade_.IsFinished()) {
			phase_ = ScenePhase::MAIN;
		}
		break;
	case ScenePhase::MAIN:
		if (Input::GetInstance()->IsPress(DIK_SPACE) || Input::GetInstance()->IsXbBtnPress(XINPUT_GAMEPAD_A)) {
			soundMgr->PlaySE("se_selected");
			fade_.Start(Fade::Status::FadeOut, 1.0f);
			phase_ = ScenePhase::FADEOUT;
		}

		break;
	case ScenePhase::FADEOUT:
		if (fade_.IsFinished()) {
			soundMgr->StopBGM();
			SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
		}
		break;
	}

	// モデルの更新処理
	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelPlayer_->SetCamera(camera);
		modelSkydome_->SetCamera(camera);
	}
	UpdateTerrain();
	modelPlayer_->Update();
	modelSkydome_->Update();

	// カメラの更新処理
	UpdateCamera();
	camMgr->Update();

	// スプライトの更新処理
	sprTitleLogo_->Update();
	sprUiPressSpace_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
	sprUiPressSpace_->Update();
	sprUiMaou_->Update();
	fade_.Update();
	UpdateImGui();
}

void TitleScene::Draw()
{
	// モデルの描画処理
	for (auto& terrain : modelTerrains_) {
		terrain->Draw();
	}
	modelPlayer_->Draw();
	modelSkydome_->Draw();

	// スプライトの描画処理
	sprTitleLogo_->Draw();
	if (phase_ != ScenePhase::FADEOUT) {
		sprUiPressSpace_->Draw();
	}
	sprUiMaou_->Draw();
	fade_.Draw();

	// ImGuiの描画処理
	ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->StopBGM();
	soundMgr->StopSE();
	soundMgr->Unload("bgm_title");
	soundMgr->Unload("se_selected");
    Editor::GetInstance()->Clear();
}

void TitleScene::UpdateImGui()
{
	ImGuiManager::GetInstance()->BeginFrame();
	ImGuiManager::GetInstance()->DrawMainMenuBar();
	ImGuiManager::GetInstance()->DrawCameraWindow(CameraManager::GetInstance());
	ImGuiManager::GetInstance()->DrawEditor();
	ImGuiManager::GetInstance()->Stats();
	ImGuiManager::GetInstance()->DrawSoundWindow();
	ImGuiManager::GetInstance()->DrawLoggerWindow();
	ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::UpdateCamera()
{
	auto camera = CameraManager::GetInstance()->GetCamera("MainCamera");
	if (CameraManager::GetInstance()->GetIsDebug() || !camera) {
		return;
	}

	const float dt = Time::GetDeltaTime();
	const bool starting = phase_ == ScenePhase::FADEOUT;
	if (!starting) {
		cameraTime_ += dt;
	}

	const Vector3 playerPos = modelPlayer_->GetTranslate();

	const float angle = std::sin(cameraTime_ * 1.0f) * 1.0f;
	const float distance = starting ? 32.0f : 50.0f;

	const Vector3 targetPos = {
	  playerPos.x + std::sin(angle) * distance,
	  playerPos.y + 10.0f,
	  playerPos.z + std::cos(angle) * distance
	};

	const float smoothSpeed = starting ? 3.0f : 0.2f;
	camPos_ = LerpCameraTranslate(camPos_, targetPos, smoothSpeed, dt);
	const Vector3 focus = {
		playerPos.x,
		playerPos.y,
		playerPos.z
	};

	const float dx = focus.x - camPos_.x;
	const float dy = focus.y - camPos_.y;
	const float dz = focus.z - camPos_.z;
	const float horizontal = std::sqrt(dx * dx + dz * dz);

	camRot_ = {
		-std::atan2(dy, horizontal),
		 std::atan2(dx, dz),
		 0.0f
	};

	camera->SetTranslate(camPos_);
	camera->SetRotate(camRot_);

	switch(camPhase_) {
	case CameraPhase::BACK:
		if(cameraTime_ >= 5.0f) {
			camPhase_ = CameraPhase::LEFTSIDE;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	case CameraPhase::LEFTSIDE:
		camPos_.z -= camSpeed_;
		frameCount_ += 1;

		if (frameCount_ >= 300) {
			camPos_ = { 0.0f, 7.3f, 40.0f };
			camRot_ = { 0.14f, 91.1f, 0.0f };
			frameCount_ = 0;
			camPhase_ = CameraPhase::FRONT;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	case CameraPhase::FRONT:
		camPos_.z -= camSpeed_;
		frameCount_ += 1;

		if (frameCount_ >= 300) {
			camPos_ = { 30.0f, 7.3f, 30.0f };
			camRot_ = { 0.14f, -90.0f, 0.0f };
			frameCount_ = 0;
			camPhase_ = CameraPhase::RIGHTSIDE;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	case CameraPhase::RIGHTSIDE:
		camPos_.z -= camSpeed_;
		frameCount_ += 1;

		if (frameCount_ >= 300) {
			camPos_ = { 0.0f, 7.3f, -50.0f };
			camRot_ = { 0.14f, 0.0f, 0.0f };
			frameCount_ = 0;
			camPhase_ = CameraPhase::BACK;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	}
}

void TitleScene::LoadSound()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("bgm_title", "bgm_title.wav");
	soundMgr->Load("se_selected", "se_selected.mp3");
	soundMgr->PlayBGM("bgm_title");
}

void TitleScene::LoadCamera()
{
	auto camMgr = CameraManager::GetInstance();
	auto entityCommon = Entity3DCommon::GetInstance();
	entityCommon->SetCameraManager(camMgr);
	entityCommon->SetDebugCamera(camMgr->GetDebugCamera());
	entityCommon->SetDefaultCamera(camMgr->GetActiveCamera());

	// カメラの初期位置設定
	auto camera = camMgr->GetActiveCamera();
	camPos_ = { 0.0f, 7.3f, -50.0f };
	camRot_ = { 0.14f, 0.0f, 0.0f };
	camera->SetTranslate(camPos_);
	camera->SetRotate(camRot_);
}

void TitleScene::LoadTexture()
{
	auto texMgr = TextureManager::GetInstance();
	texTitleLogo_ = texMgr->Load("resources/sprites/spr_title_logo.png");
	texPressSpace_ = texMgr->Load("resources/sprites/ui_press_space.png");
	texMaou_ = texMgr->Load("resources/sprites/ui_maou.png");
}

void TitleScene::LoadSprite()
{
	sprTitleLogo_ = std::make_unique<Sprite>();
	sprTitleLogo_->Init();
	sprTitleLogo_->Create(texTitleLogo_, { 385.0f, 31.0f }, Color::WHITE, { 500.0f, 290.0f });
	Editor::GetInstance()->RegisterSprite("sprTitleLogo", sprTitleLogo_.get());

	sprUiPressSpace_ = std::make_unique<Sprite>();
	sprUiPressSpace_->Init();
	sprUiPressSpace_->Create(texPressSpace_, { 415.0f, 575.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiPressSpace", sprUiPressSpace_.get());

	sprUiMaou_ = std::make_unique<Sprite>();
	sprUiMaou_->Init();
	sprUiMaou_->Create(texMaou_, { 1070.0f, 665.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiMaou", sprUiMaou_.get());
}

void TitleScene::LoadModel()
{
	auto modelMgr = ModelManager::GetInstance();
	modelMgr->LoadModel("player.obj");
	modelMgr->LoadModel("skydome.obj");
	modelMgr->LoadModel("greenTerrain.obj");

	modelPlayer_ = std::make_unique<Entity3D>();
	modelPlayer_->Init();
	modelPlayer_->SetModel("player");
	Editor::GetInstance()->RegisterModel("player", modelPlayer_.get());

	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("skydome");
	Editor::GetInstance()->RegisterModel("skydome", modelSkydome_.get());

	for (int i = 0; i < kTerrainCount; ++i) {
		auto& terrain = modelTerrains_[i];
		terrain = std::make_unique<Entity3D>();
		terrain->Init();
		terrain->SetModel("greenTerrain");
		terrain->SetScale({
			terrainScale_,
			terrainScale_,
			terrainScale_
			});
		terrain->SetTranslate({
			0.0f,
			-20.0f,
			terrainLength_ * static_cast<float>(i)
			});
	}
}

void TitleScene::UpdateTerrain()
{
	auto* camera = CameraManager::GetInstance()->GetCamera("MainCamera");

	const float halfLength = terrainLength_ * 0.5f;
	const float totalLength = terrainLength_ * kTerrainCount;
	// カメラから少し後ろで折り返す
	const float recycleZ = modelPlayer_->GetTranslate().z - 300.0f;
	const float moveAmount = terrainSpeed_ * Time::GetDeltaTime();

	for (auto& terrain : modelTerrains_) {
		Vector3 pos = terrain->GetTranslate();

		// 地形を手前へ動かす。
		pos.z -= moveAmount;

		// 地形の奥側の端までカメラ後方へ抜けたら、
		// 3枚分先へ戻す。移動の余りも保持する。
		while (pos.z + halfLength < recycleZ) {
			pos.z += totalLength;
		}

		terrain->SetTranslate(pos);
		terrain->SetCamera(camera);
		terrain->Update();
	}
}
