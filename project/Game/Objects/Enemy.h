#pragma once
#include <memory>
#include "Entity3D.h"
#include "MathFunc.h"
#include "EnemyBullet.h"

enum class EnemyPattern {
	Straight,
	SinWave,
	ZigZag,
	SlowFast,
	KeepStop
};

enum class EnemyState {
	Approach, // 接近
	Enter,
	Hover, // 停止・浮遊
	Exit,
	Dead
};

enum class ApproachBehavior {
	KeepMoving, // 進み続ける
	StopAtTarget // 指定位置で停止
};

class Enemy
{
public:
	void Init(EnemyPattern pattern, const Vector3& position);

	void Update();

	void Draw();

	void DebugDraw();

	void SetIsDebugHit();

	Vector3 GetPosition() const { return model_->GetTranslate(); }
	bool GetIsAlive() const { return isAlive_; }
	Sphere GetSphere() const { return sphere_; }
	std::unique_ptr<EnemyBullet>& GetBullet() { return bullet_; }
	
	bool GetIsMoveStop() const { return isMoveStop_; }
	void SetIsMoveStop(bool flag) { isMoveStop_ = flag; }

	bool GetExitFinished() const { return exitFinished_; }

private:
	// メンバ関数
	void UpdateMovement();
	// 行動の更新
	void UpdateStraight();
	void UpdateSinWave();
	void UpdateZigZag();
	void UpdateSlowFast();
	void UpdateKeepStop();
	void UpdateState();
	// 状態の更新
	void UpdateApproach();
	void UpdateHover();
	void UpdateShooting();
	void EnterHover();
	// スポーン時と退場の動きの更新
	void UpdateEnter();
	void UpdateExit();
	void BeginExit();

	Transform transform_{};
	std::unique_ptr<Entity3D> model_;
	// 敵の弾
	std::unique_ptr<EnemyBullet> bullet_;

	float speed = 0.3f;
	float amplitude = 0.5f;
	float theta = 0.0f;
	bool isAlive_ = false;
	int switchDirTimer = 30;
	float dir = 1.0f;
	EnemyPattern pattern_;
	Sphere sphere_;

	// 弾の発射周期
	float shotInterval_ = 180.0f;
	float timer_ = shotInterval_;

	bool debugIsHit_ = false;
	int debugHitTimer_ = 0;

	bool isMoveStop_ = false;

	EnemyState enemyState_ = EnemyState::Approach;
	ApproachBehavior approachBehavior_ = ApproachBehavior::KeepMoving;

	Vector3 hoverOrigin_{};
	float hoverTimer_ = 0.0f;
	float hoverAmplitude_ = 0.3f;
	float hoverPeriod_ = 2.0f;
	float distance = 0.0f;
	float maxDistance = 40.0f;

	// 入場開始時に保存する値 
	Vector3 enterStart_;
	Vector3 enterTarget_;
	float enterElapsed_ = 0.0f;
	float enterDuration_ = 1.0f;

	float activeElapsed_ = 0.0f;
	float activeDuration_ = 3.0f;
	Vector3 exitStart_{};
	Vector3 exitTarget_{};
	float exitElapsed_ = 0.0f;
	float exitDuration_ = 1.0f;
	bool exitFinished_ = false;

};
