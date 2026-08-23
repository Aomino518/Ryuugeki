#pragma once
#include "SeekerEngine.h"
#include "Vector3.h"
#include "SceneIncludes.h"

class Enemy;
class Bullet
{
public:
	// 初期化
	void Init(const Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	void DebugDraw();

	void SetDebugHit();

	// Getter
	Vector3 GetPosition() const { return model_->GetTranslate(); }
	bool GetIsShot() const { return isShot_; }
	Sphere GetSphere() const { return sphere_; }

	// Setter
	void SetPosition(const Vector3& pos) { model_->SetTranslate(pos); }
	void SetIsShot(bool isShot) { isShot_ = isShot; }

	bool BulletIsCollision(const Enemy& enemy) const;
	void Fire(const Vector3& startPosition, const Vector3& targetPosition);

private:
	// モデル
	std::unique_ptr<Entity3D> model_;

	// メンバ変数
	bool isShot_ = false;
	float speed_ = 5.0f;
	Sphere sphere_;
	float maxDistance_ = 100.0f;
	// 発射方向
	Vector3 direction_{};
	// 発射した地点
	Vector3 startPosition_{};

	bool debugIsHit_ = false;
	int debugHitTimer_ = 0;
};

