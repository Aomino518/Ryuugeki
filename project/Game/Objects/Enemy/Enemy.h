#pragma once
#include <memory>
#include "Entity3D.h"
#include "MathFunc.h"
#include "EnemyBullet.h"
#include "IEnemyState.h"

enum class EnemyPattern {
	Straight,
	KeepStop
};

enum class ApproachBehavior {
	KeepMoving, // 進み続ける
	StopAtTarget // 指定位置で停止
};

class Enemy
{
public:
	void RequestStateChange(std::unique_ptr<IEnemyState> nextState);

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

	void Move(const Vector3& offset);
	float GetSpeed() const { return speed_; }

	// 状態クラスから使う操作。座標はEnemyが保持する。
	Vector3 GetStatePosition() const { return transform_.translate; }
	void SetStatePosition(const Vector3& position) { transform_.translate = position; }
	void FinishExit() { exitFinished_ = true; }
	void UpdateActiveMovement();
	EnemyPattern GetEnemyPattern() const { return pattern_; }

private:
	// メンバ関数
	void UpdateMovement();
	// 行動の更新
	void UpdateStraight();
	void UpdateKeepStop();
	void UpdateState();
	// 状態の更新
	void UpdateApproach();
	void UpdateHover();
	void UpdateShooting();
	void EnterHover();

	void ApplyStateChange();
	
	std::unique_ptr<IEnemyState> state_;
	std::unique_ptr<IEnemyState> nextState_;

	Transform transform_{};
	std::unique_ptr<Entity3D> model_;
	// 敵の弾
	std::unique_ptr<EnemyBullet> bullet_;

	float speed_ = 0.3f;

	bool isAlive_ = false;

	EnemyPattern pattern_;
	Sphere sphere_;

	// 弾の発射周期
	float shotInterval_ = 180.0f;
	float timer_ = shotInterval_;

	bool debugIsHit_ = false;
	int debugHitTimer_ = 0;

	bool isMoveStop_ = false;

	bool isHover_ = false;
	ApproachBehavior approachBehavior_ = ApproachBehavior::KeepMoving;

	Vector3 hoverOrigin_{};
	float hoverTimer_ = 0.0f;
	float hoverAmplitude_ = 0.3f;
	float hoverPeriod_ = 2.0f;
	float distance = 0.0f;
	float maxDistance = 40.0f;

	bool exitFinished_ = false;

};
