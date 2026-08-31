#pragma once
#include "SceneIncludes.h"
#include "BaseScene.h"
#include "Fade.h"
#include "Player.h"
#include "EnemyManager.h"
#include "Reticle.h"

class PlayScene : public BaseScene
{
public:
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	void Shutdown() override;

	const char* GetSceneName() const override { return "GAMEPLAY"; }

private:
	// シーンフェーズ
	enum struct ScenePhase {
		FADEIN,
		MAIN,
		FADEOUT
	};

	// メンバ関数
	void UpdatePlay(); // プレイ中の更新処理
	void LoadSound();
	void LoadCamera();
	void LoadTexture();
	void LoadSprite();
	void LoadModel();
	void InitClass();

	// メンバ変数
	bool isClear = false;
	bool isGameOver = false;

	// テクスチャ
	uint32_t texReticle_;
	uint32_t texUiPlayOperate_;
	uint32_t texUiController_;

	// スプライト
	std::unique_ptr<Sprite> sprUiPlayOperate_;
	std::unique_ptr<Sprite> sprUiConroller_;

	// モデル
	std::unique_ptr<Entity3D> modelSkydome_;
	std::unique_ptr<Entity3D> modelTerrain_;

	// シーンフェーズ
	ScenePhase phase_ = ScenePhase::FADEIN;

	// クラス
	Fade fade_;
	std::shared_ptr<Player> player_;
	std::unique_ptr<EnemyManager> enemyMgr_;
	Reticle reticle_;

	bool isController_ = false;
	bool isMovePlayer_ = true;
	float playerMoveTimer_ = 0.0f;
	float playerMoveDuration_ = 120.0f;
	float startPos = -10.0f;
	float endPos = 500.0f;

#ifdef _DEBUG
	// ゲーム一時停止
	bool isGameStop_ = false;
#endif
};