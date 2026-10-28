#pragma once
#include "BaseScene.h"
#include <cstdint>
#include <memory>
#include "Entity3D.h"
#include "Sprite.h"
#include "Effects/Fade.h"

class GameOverScene : public BaseScene
{
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// 終了
	void Shutdown() override;

	const char* GetSceneName() const override { return "GAMEOVER"; }

	void DrawImGui() override;

private:
	void Animation();

	// シーンフェーズ
	enum struct ScenePhase {
		FADEIN,
		MAIN,
		FADEOUT
	};

	// テクスチャ
	uint32_t texUiGameOver_;
	uint32_t texUiBackTitle_;

	// スプライト
	std::unique_ptr<Sprite> sprGameOver_;
	std::unique_ptr<Sprite> sprUiSpaceBackTitle_;

	// モデル
	std::unique_ptr<Entity3D> modelSkydome_;
	std::unique_ptr<Entity3D> modelPlayer_;

	// シーンフェーズ
	ScenePhase phase_ = ScenePhase::FADEIN;

	// クラス
	Fade fade_;

	float kAnimeTime = 5.0f;
	float timer_ = 0.0f;
};

