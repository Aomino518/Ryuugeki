#pragma once
#include "Bullet.h"

class Weapon
{
public:
	/// <summary>
	/// 初期化処理関数
	/// </summary>
	/// <param name="position">初期位置</param>
	void Init(const Vector3& position);

	void Update();

	void Draw();

	void DebugDraw();

	/// <summary>
	/// 発射処理関数
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="target">目標位置</param>
	void Fire(const Vector3& position, const Vector3& target);

	/// <summary>
	/// 弾を取得する関数
	/// </summary>
	/// <returns>bullets_</returns>
	std::vector<std::unique_ptr<Bullet>>& GetBullets();

private:
	// メンバ変数
	std::vector<std::unique_ptr<Bullet>> bullets_;

	int coolDown_ = 0;

	static constexpr int maxBullets_ = 10;
	static constexpr int fireInterval_ = 10;
};

