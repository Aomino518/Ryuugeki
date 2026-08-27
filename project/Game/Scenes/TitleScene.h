#pragma once
#include "SceneIncludes.h"
#include "BaseScene.h"
#include "Particle2DEmitter.h"
#include "Fade.h"

class TitleScene : public BaseScene
{
public:
	void Init() override;

	void Update() override;

	void Draw() override;

	void Shutdown() override;

	const char* GetSceneName() const override { return "TITLE"; }

private:
	// シーンフェーズ
	enum struct ScenePhase {
		FADEIN,
		MAIN,
		FADEOUT
	};

	// カメラフェーズ
	enum struct CameraPhase {
		BACK,
		LEFTSIDE,
		FRONT,
		RIGHTSIDE
	};

	// メンバ関数
	void UpdateImGui();
	void UpdateCamera();
	void LoadSound();
	void LoadCamera();
	void LoadTexture();
	void LoadSprite();
	void LoadModel();

	// メンバ変数
	float camSpeed_ = 0.1f;
	int frameCount_ = 0;

	// テクスチャ
	uint32_t texTitleLogo_;
	uint32_t texPressSpace_;
	uint32_t texMaou_;

	// スプライト
	std::unique_ptr<Sprite> sprTitleLogo_;
	std::unique_ptr<Sprite> sprUiPressSpace_;
	std::unique_ptr<Sprite> sprUiMaou_;

	// モデル
	std::unique_ptr<Entity3D> modelPlayer_;
	std::unique_ptr<Entity3D> modelSkydome_;

	Vector3 camPos_;
	Vector3 camRot_;

	// シーンフェーズ
	ScenePhase phase_ = ScenePhase::FADEIN;
	// カメラフェーズ
	CameraPhase camPhase_ = CameraPhase::BACK;

	// フェード
	Fade fade_;
};