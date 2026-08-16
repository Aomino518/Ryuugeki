#pragma once
#include "SeekerEngine.h"
#include "Vector3.h"
#include "SceneIncludes.h"

class Player;
class EnemyBullet
{
public:
	void Init(const Vector3& position);

	void Update();

	void Draw();

	void DebuDraw();

	void Fire(const Vector3& position);

	bool GetIsShot() const { return isShot_; }

	void SetIsShot(bool flag) { isShot_ = flag; }

	Sphere GetSphere() const { return sphere_; }

	bool BulletIsCollision(const Player& player) const;

private:
	// モデル
	std::unique_ptr<Entity3D> model_;

	// メンバ変数
	bool isShot_ = false;
	float speed_ = 0.5f;
	Sphere sphere_;
};

