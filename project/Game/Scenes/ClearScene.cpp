#include "ClearScene.h"
#include "SeekerEngine.h"
#include "SceneIncludes.h"
#include <numbers>
#include <algorithm>

void ClearScene::Init()
{
	Logger::Write("現在シーンClearScene");
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("se_game_clear", "se_game_clear.mp3");
	soundMgr->Load("se_selected", "se_selected.mp3");

	//===========================
	// カメラマネージャー
	//===========================
	auto camMgr = CameraManager::GetInstance();
	camMgr->SetActiveCameraByName("MainCamera");
	camMgr->GetCamera("MainCamera")->SetIsLerp(false);
	auto entityCommon = Entity3DCommon::GetInstance();
	entityCommon->SetCameraManager(camMgr);
	entityCommon->SetDebugCamera(camMgr->GetDebugCamera());
	entityCommon->SetDefaultCamera(camMgr->GetActiveCamera());

	//===========================
	// テクスチャ
	//===========================
	auto texMgr = TextureManager::GetInstance();
	texUiGameClear_ = texMgr->Load("resources/sprites/ui_game_clear.png");
	texUiBackTitle_ = texMgr->Load("resources/sprites/ui_space_backtitle.png");

	//===========================
	// スプライト
	//===========================
	sprGameClear_ = std::make_unique<Sprite>();
	sprGameClear_->Init();
	sprGameClear_->Create(texUiGameClear_, { 620.0f, 215.0f }, Color::WHITE);
	sprGameClear_->SetAnchorPoint({ 0.5f, 0.5f });
	Editor::GetInstance()->RegisterSprite("sprGameClear", sprGameClear_.get());

	sprUiSpaceBackTitle_ = std::make_unique<Sprite>();
	sprUiSpaceBackTitle_->Init();
	sprUiSpaceBackTitle_->Create(texUiBackTitle_, { 370.0f, 510.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiSpaceBackTitle", sprUiSpaceBackTitle_.get());

	//===========================
	// モデル
	//===========================
	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("skydome");
	modelSkydome_->SetIsLighting(false);
	modelSkydome_->SetPicked(false);
	Editor::GetInstance()->RegisterModel("skydome", modelSkydome_.get());

	modelPlayer_ = std::make_unique<Entity3D>();
	modelPlayer_->Init();
	modelPlayer_->SetModel("AirShip");
	Editor::GetInstance()->RegisterModel("AirShip", modelPlayer_.get());

	//===========================
	// パーティクル
	//===========================
	auto particleMgr = Particle2DManager::GetInstance();
	auto emitterMgr = Emitter2DManager::GetInstance();

	uint32_t texture = TextureManager::GetInstance()->Load("resources/sprites/circle.png");

	// 同じ名前でグループとエミッターを作る
	particleMgr->CreateParticleGroup("ClearSpark", texture);
	particleMgr->SetBlendMode("ClearSpark", kBlendModeAdd);

	// 設定
	ParticleConfig config{};
	emitterMgr->CreateEmitter("ClearSpark", config, 30, 0.1f);
	Editor::GetInstance()->RegisterParticle2D("ClearSpark");

	//===========================
	// クラス
	//===========================
	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
	ImGuiManager::GetInstance()->LoadScenesJson();
	basePosition_ = modelPlayer_->GetTranslate();
	clearSprBaseSize_ = sprGameClear_->GetSize();
	clearSprTime_ = 0.0f;
	if (sceneManager_->IsPlaying()) {
		sprGameClear_->SetSize({ 0.0f, 0.0f });
	}

	std::string error;
	if (!camMgr->LoadCameraFile(
		"resources/json/cameras/clearScene_Enter.json", error)) {
		Logger::Write(Logger::LogLevel::Warning, "カメラ設定の読み込みに失敗しました: " + error);
	}
	camMgr->SetActiveCameraByName("MainCamera");
	camMgr->GetCamera("MainCamera")->StartLerp();
}

void ClearScene::Update()
{
	auto camMgr = CameraManager::GetInstance();
	auto soundMgr = SoundManager::GetInstance();
	/*-- 更新処理 --*/
	switch (phase_) {
	case ScenePhase::FADEIN:
		if (fade_.IsFinished()) {
			phase_ = ScenePhase::MAIN;
		}
		break;
	case ScenePhase::MAIN:
		if (!playClearSe_ && camMgr->GetCamera("MainCamera")->GetIsLerpEnd()) {
			soundMgr->PlaySE("se_game_clear");
			playClearSe_ = true;
		}

		if (Input::GetInstance()->IsPress(DIK_SPACE) || Input::GetInstance()->IsXbBtnPress(XINPUT_GAMEPAD_A)) {
			soundMgr->PlaySE("se_selected");
			fade_.Start(Fade::Status::FadeOut, 1.0f);
			phase_ = ScenePhase::FADEOUT;
		}

		break;
	case ScenePhase::FADEOUT:
		if (fade_.IsFinished()) {
			soundMgr->StopSE();
			SceneManager::GetInstance()->ChangeScene("TITLE");
		}
		break;
	}

	// カメラの更新処理
	camMgr->Update(SceneManager::GetInstance()->IsPlaying());

	// モデルの更新処理
	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelSkydome_->SetCamera(camera);
		modelPlayer_->SetCamera(camera);
	}

	modelSkydome_->Update();
	ModelAnimation();

	// スプライトの更新処理
	if (camMgr->GetCamera("MainCamera")->GetIsLerp()) {
		// カメラ移動中は見えないサイズにしておく
		sprGameClear_->SetSize({ 0.0f, 0.0f });
		sprGameClear_->Update();
	}

	SpriteAnimation();
	sprUiSpaceBackTitle_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
	sprUiSpaceBackTitle_->Update();
	UpdateParticle();
	fade_.Update();
}

void ClearScene::EditorUpdate()
{
	auto* camMgr = CameraManager::GetInstance();

	if (auto* camera = camMgr->GetActiveCamera()) {
		camera->Update();

		modelSkydome_->SetCamera(camera);
		modelPlayer_->SetCamera(camera);
	}

	modelSkydome_->Update();
	modelPlayer_->Update();

	sprGameClear_->Update();
	sprUiSpaceBackTitle_->Update();
}

void ClearScene::Draw()
{
	/*-- 描画処理 --*/
	// Model
	modelSkydome_->Draw();
	modelPlayer_->Draw();

	// Sprite
	sprGameClear_->Draw();
	if (!sceneManager_->IsPlaying() || CameraManager::GetInstance()->GetCamera("MainCamera")->GetIsLerpEnd()) {
		sprUiSpaceBackTitle_->Draw();
	}
	Particle2DManager::GetInstance()->Draw();

	fade_.Draw();
}

void ClearScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->StopBGM();
	soundMgr->StopSE();
	soundMgr->Unload("se_selected");
	soundMgr->Unload("se_game_clear");
	Emitter2DManager::GetInstance()->RemoveEmitter("ClearSpark");
	Particle2DManager::GetInstance()->RemoveParticleGroup("ClearSpark");
    Editor::GetInstance()->Clear();
}

void ClearScene::DrawImGui()
{
#ifdef USE_IMGUI
	
#endif
}

void ClearScene::ModelAnimation()
{
	floatTime_ += Time::GetDeltaTime();
	Vector3 position = basePosition_;
	position.y += amplitude_ * std::sin(floatTime_ * 2.0f * std::numbers::pi_v<float> / period_);
	modelPlayer_->SetTranslate(position);
	modelPlayer_->Update();
}

void ClearScene::SpriteAnimation()
{
	auto* camera = CameraManager::GetInstance()->GetCamera("MainCamera");
	if (camera && camera->GetIsLerpEnd()) {
		clearSprTime_ = std::min(clearSprTime_ + Time::GetDeltaTime(), clearSprDuration_);
		float progress = LerpRateConvert(clearSprTime_, clearSprDuration_);
		scale_ = EaseOutElastic(progress);
		sprGameClear_->SetSize(Vector2{ clearSprBaseSize_.x * scale_, clearSprBaseSize_.y * scale_ });
	}
	sprGameClear_->Update();
}

void ClearScene::UpdateParticle()
{
	auto* camMgr = CameraManager::GetInstance();
	auto* camera = camMgr->GetCamera("MainCamera");
	if (!clearParticleEmitted_ && camera && camera->GetIsLerpEnd()) {
		auto* emitter = Emitter2DManager::GetInstance()->GetEmitter("ClearSpark");
		// ゲームクリアの文字から発生
		const Vector2 position = sprGameClear_->GetPosition();

		Transform transform = emitter->GetTransform();
		transform.translate = { position.x, position.y, 0.0f };
		emitter->SetTransform(transform);
		emitter->EmitOnce();
		clearParticleEmitted_ = true;
	}
	Emitter2DManager::GetInstance()->Update();
	Particle2DManager::GetInstance()->Update();
}
