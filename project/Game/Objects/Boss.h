#pragma once
#include "SceneIncludes.h"
#include "Vector3.h"
#include "EnemyBullet.h"
#include "Effects/DethParticle.h"
#include "UI/HpBar.h"

class Boss
{
public:
	void Init(const Vector3& position, const Vector3& playerPosition);
	void Update(const Vector3& playerPosition);
	void Draw();
	void DrawRushWarning();
	void DrawDebug();
	void Damage();
	void DrawImGui();

	// Getter
	const Vector3& GetPosition() const { return model_->GetTranslate(); }
	const bool GetIsAlive() const { return isAlive_; };
	const Sphere& GetSphere() const { return sphere_; }
	std::vector<std::unique_ptr<EnemyBullet>>& GetBullets() { return bullets_; }

	// Setter
	void SetIsAlive(bool isAlive) { this->isAlive_ = isAlive; }
private:
	enum class RushDirection {
		LeftToRight,
		UpToDown,
		RightToLeft,
		DownToUp
	};

	enum class BossPhase {
		Phase1,
		Transition,
		Phase2
	};

	void UpdateMovement(const Vector3& playerPosition);
	static RushDirection NextDirection(RushDirection direction);
	void UpdateAttack(const Vector3& playerPosition);
	void FireBullet();
	void UpdateBullets();
	void StartRush(const Vector3& playerPosition);
	void UpdateRush(const Vector3& playerPosition);
	void Transition();

	Vector3 position_{};
	Sphere sphere_{};
	std::unique_ptr<Entity3D> model_;
	std::vector<std::unique_ptr<EnemyBullet>> bullets_;

	// 左右移動の基準位置
	float centerOffsetX_ = 0.0f;
	// プレイヤーとのZ方向の距離
	float distanceFromPlayer_ = 80.0f;
	// プレイヤーとの高さの差
	float heightOffset_ = 0.0f;
	// 左右へ動く幅
	float moveAmplitude_ = 20.0f;
	// 左右移動の速さ
	float moveSpeed_ = 1.5f;
	// sinへ渡す時間
	float moveTime_ = 0.0f;

	bool isAlive_ = false;
	static constexpr int kMaxHp = 20;
	int hp_ = kMaxHp;

	float attackTimer_ = 0.0f;
	float burstTimer_ = 0.0f;

	float attackInterval_ = 3.0f;
	float burstInterval_ = 0.15f;
	bool isBurstAttacking_ = false;

	int burstShotCount_ = 0;
	static constexpr uint32_t kBurstShotMax_ = 3;

	RushDirection shootingDirection_ = RushDirection::LeftToRight;
	float shootingMoveTime_ = 0.0f;
	float shootingMoveDuration_ = 6.0f;
	float screenExitMargin_ = 15.0f;

	// 左→右、上→下、右→左、下→上の順で突進
	RushDirection rushDirection_ = RushDirection::LeftToRight;
	float rushTime_ = 0.0f;
	float rushDuration_ = 2.0f;

	// 横切る範囲。画面外に出る値へ調整する
	float rushHalfWidth_ = 70.0f;
	float rushHalfHeight_ = 45.0f;

	bool isRushing_ = false;
	bool nextAttackIsRush_ = false;
	// 突進開始時のプレイヤーの座標
	Vector3 rushTarget_{};
	std::unique_ptr<Sprite> rushWarning_;
	float rushWarningDuration_ = 1.0f;
	float rushWarningThickness_ = 8.0f;

	// ボスの段階
	BossPhase phase_ = BossPhase::Phase1;
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	DethParticle dethParticle_;

	// 段階の移行
	float transitionTime_ = 0.0f;
	float transitionDuration_ = 2.0f;
	Vector3 startPosition_{};
	Vector3 targetPosition_{};

	// HPバー
	HpBar hpBar_;
};