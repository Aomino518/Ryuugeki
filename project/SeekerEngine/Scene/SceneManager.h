#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include <optional>

enum class RunMode {
	Edit, // 編集中
	Play // ゲーム実行中
};

class SceneManager
{
public:
	static SceneManager* GetInstance();

	void SetNextScene(std::unique_ptr<BaseScene> nextScene) { nextScene_ = std::move(nextScene); }

	void Update();

	void Draw();

	void Shutdown();

	void ChangeScene(const std::string& sceneName);

	// setter
	void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory) { 
		this->sceneFactory_ = std::move(sceneFactory);
	}

	const char* GetCurrentSceneName() const;
	BaseScene* GetCurrentScene() const { return scene_.get(); }

	bool GetIsEndRequest() { return scene_->GetIsEndRequest(); }

	void DrawImGui();

	// 編集モード切り替え用
	// 現在モードの取得
	RunMode GetRunMode() const { return runMode_; }
	// ゲームが実行中か確認
	bool IsPlaying() const { return runMode_ == RunMode::Play; }
	// モード変更
	void SetRunMode(RunMode mode) { requestedRunMode_ = mode; }

private:
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	std::unique_ptr<BaseScene> scene_;
	std::unique_ptr<BaseScene> nextScene_;
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;

	// 現在のモード
	RunMode runMode_ = RunMode::Play;
	// 次に切り替えたいモード
	RunMode requestedRunMode_ = RunMode::Play;
};

