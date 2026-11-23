#pragma once
#include "BaseScene.h"
#include <cstdint>
#include <memory>
#include "Entity3D.h"
#include "Sprite.h"
#include "Effects/Fade.h"

class ClearScene : public BaseScene
{
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	void EditorUpdate() override;

	// 描画
	void Draw() override;

	// 終了
	void Shutdown() override;

	const char* GetSceneName() const override { return "CLEAR"; }

	void DrawImGui() override;

private:
	void ModelAnimation();
	void SpriteAnimation();
	void UpdateParticle();

	// シーンフェーズ
	enum struct ScenePhase {
		FADEIN,
		MAIN,
		FADEOUT
	};

	// テクスチャ
	uint32_t texUiGameClear_;
	uint32_t texUiBackTitle_;

	// スプライト
	std::unique_ptr<Sprite> sprGameClear_;
	std::unique_ptr<Sprite> sprUiSpaceBackTitle_;

	// モデル
	std::unique_ptr<Entity3D> modelSkydome_;
	std::unique_ptr<Entity3D> modelPlayer_;

	// シーンフェーズ
	ScenePhase phase_ = ScenePhase::FADEIN;

	// クラス
	Fade fade_;

	// アニメーション変数
	float floatTime_ = 0.0f;
	Vector3 basePosition_;
	const float amplitude_ = 0.3f;
	const float period_ = 2.0f;

	float scale_;
	float clearSprTime_ = 0.0f;
	const float clearSprDuration_ = 1.0f;
	Vector2 clearSprBaseSize_{};
	bool clearParticleEmitted_ = false;
	bool playClearSe_ = false;
};

