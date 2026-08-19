#pragma once
#include "SceneIncludes.h"
#include "BaseScene.h"
#include "Fade.h"
#include "Player.h"
#include "EnemyManager.h"

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
	void UpdateReticle(); // エイムの更新処理
	Vector3 CalculateAimTarget();

	// メンバ変数
	bool isClear = false;
	bool isGameOver = false;

	// テクスチャ
	uint32_t texReticle_;
	uint32_t texUiPlayOperate_;

	// スプライト
	std::unique_ptr<Sprite> sprReticle_;
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

	// レティクルの位置
	Vector3 reticleWorldPosition_{};
	Vector2 reticleScreenPosition_{};

	static constexpr float kAimDistance = 50.0f;
	static constexpr float kReticleRadius = 32.0f;

	bool isController_ = false;

	float cameraSensitivity_ = 0.035f;
	float rightStickDeadZone_ = 0.15f;

	// 右スティックで動かす照準位置
	Vector2 aimOffset_ = { 0.0f, 0.0f };

	// 左スティックによる先行量
	Vector2 moveAimOffset_ = { 0.0f, 0.0f };

	// レティクル操作速度
	float aimSpeed_ = 12.0f;

	// 自機移動によるレティクル先行距離
	float moveAimDistance_ = 100.0f;

	// レティクルが中央へ戻る強さ
	float aimReturnSpeed_ = 0.12f;

	// レティクルが動ける画面範囲
	float reticleLimitX_ = 400.0f;
	float reticleLimitY_ = 250.0f;

	// 自機から照準地点までの距離
	float aimDistance_ = 50.0f;

	// 敵を狙っていると判定するピクセル半径
	float reticleHitRadius_ = 45.0f;

#ifdef _DEBUG
	// ゲーム一時停止
	bool isGameStop_ = false;
#endif
};