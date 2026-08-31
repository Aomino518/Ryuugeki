#pragma once
#include "SceneIncludes.h"
#include "Vector3.h"
#include "EnemyBullet.h"

class Boss
{
public:
	void Init(const Vector3& position, const Vector3& playerPosition);
	void Update(const Vector3& playerPosition);
	void Draw();
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
	void UpdateMovement(const Vector3& playerPosition);
	void UpdateAttack();
	void FireBullet();
	void UpdateBullets();

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
	int hp_ = 20;

	float attackTimer_ = 0.0f;
	float burstTimer_ = 0.0f;

	float attackInterval_ = 3.0f;
	float burstInterval_ = 0.15f;
	bool isBurstAttacking_ = false;

	int burstShotCount_ = 0;
	static constexpr uint32_t kBurstShotMax_ = 3;
};

