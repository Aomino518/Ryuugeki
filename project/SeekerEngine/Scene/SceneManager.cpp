#include "SceneManager.h"
#include "Logger.h"
#include <cassert>
#include "Editor.h"
#include "CameraManager.h"
#include "Graphics.h"
#include "SoundManager.h"

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
}

void SceneManager::Update() {
	// 保存済みならStopで元の編集シーンへ
	if (runMode_ == RunMode::Play && requestedRunMode_ == RunMode::Edit && editSnapshot_.has_value()) {
		if (!RestoreEditScene()) {
			requestedRunMode_ = runMode_;
		}

		return;
	}

	if (nextScene_) {
		// 旧シーン
		if (scene_) {
			scene_->Shutdown();
			scene_.reset();
		}

		// シーン切り替え
		scene_ = std::move(nextScene_);

		scene_->SetSceneManager(this);

		scene_->Init();

		if (!IsPlaying()) {
			scene_->OnEnterEdit();
		}
	}

	// 予約されたモードが現在と違う場合に切り替え
	if (scene_ && runMode_ != requestedRunMode_) {
		if (runMode_ == RunMode::Edit && requestedRunMode_ == RunMode::Play) {
			editSnapshot_ = Editor::GetInstance()->CaptureSceneJson();
			editSceneName_ = scene_->GetSceneName();
			Logger::Write("Captured edit snapshot: " + editSceneName_);
		}

		runMode_ = requestedRunMode_;

		if (IsPlaying()) {
			scene_->OnEnterPlay();
		} else {
			scene_->OnEnterEdit();
		}
	}

	// 実行中シーンを更新する
	if (scene_) {
		if (IsPlaying()) {
			scene_->Update();
		} else {
			scene_->EditorUpdate();
		}
	}
}

void SceneManager::Draw() {
	if (scene_) {
		scene_->Draw();
	}
}

void SceneManager::Shutdown() {
	scene_.reset();
	nextScene_.reset();
	sceneFactory_.reset();
	editSnapshot_.reset();
	editSceneName_.clear();
	Logger::Write("SceneManager Shutdown");
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);

	nextScene_ = sceneFactory_->CreateScene(sceneName);
}

const char* SceneManager::GetCurrentSceneName() const
{
	if (!scene_) {
		return "Unknown";
	}
	return scene_->GetSceneName();
}

void SceneManager::DrawImGui()
{
#ifdef USE_IMGUI
	if (scene_) {
		scene_->DrawImGui();
	}
#endif
}

bool SceneManager::RestoreEditScene()
{
	if (!editSnapshot_ || editSceneName_.empty() || !sceneFactory_) {
		return false;
	}

	auto restoredScene = sceneFactory_->CreateScene(editSceneName_);
	if (!restoredScene) {
		Logger::Write(Logger::LogLevel::Error, "Faild to restore scene: " + editSceneName_);
		return false;
	}

	// 古い描画リソースをGPUが使用し終わるまで待つ
	Graphics::GetInstance()->WaitGPU();
	nextScene_.reset();

	if (scene_) {
		scene_->Shutdown();
		scene_.reset();
	}

	Editor::GetInstance()->Clear();

	runMode_ = RunMode::Edit;
	requestedRunMode_ = RunMode::Edit;

	// 初期化中に参照できる通常をカメラを用意
	auto* camMgr = CameraManager::GetInstance();
	camMgr->CreateCamera("MainCamera");
	camMgr->SetActiveCameraByName("MainCamera");

	scene_ = std::move(restoredScene);
	scene_->SetSceneManager(this);

	// オブジェクトを再生成してEditorに再登録
	scene_->Init();
	Editor::GetInstance()->ApplySceneJson(*editSnapshot_);

	auto* sound = SoundManager::GetInstance();
	sound->StopBGM();
	sound->StopSE();

	// 音を止め、編集カメラへ切り替える
	scene_->OnEnterEdit();

	// 描画前にモデルなどの行列を更新する
	scene_->EditorUpdate();

	Logger::Write("Restored edit scene: " + editSceneName_);
	return true;
}
