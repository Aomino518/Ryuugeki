#include "Weapon.h"

/// <summary>
/// 初期化処理関数
/// </summary>
/// <param name="position">初期位置</param>
void Weapon::Init(const Vector3& position)
{
	bullets_.reserve(maxBullets_);

	for (int i = 0; i < maxBullets_; ++i) {
		auto bullet = std::make_unique<Bullet>();
		bullet->Init(position);
		bullets_.push_back(std::move(bullet));
	}
}

void Weapon::Update()
{
	if (coolDown_ > 0) {
		--coolDown_;
	}

	for (auto& bullet : bullets_) {
		bullet->Update();
	}
}

void Weapon::Draw()
{
	for (auto& bullet : bullets_) {
		if (bullet->GetIsShot()) {
			bullet->Draw();
		}
	}
}

void Weapon::DebugDraw()
{
	for (auto& bullet : bullets_) {
		bullet->DebugDraw();
	}
}

/// <summary>
/// 発射処理関数
/// </summary>
/// <param name="position">初期位置</param>
/// <param name="target">目標位置</param>
void Weapon::Fire(const Vector3& position, const Vector3& target)
{
	if (coolDown_ > 0) {
		return;
	}

	for (auto& bullet : bullets_) {
		if (!bullet->GetIsShot()) {
			bullet->Fire(position, target);
			coolDown_ = fireInterval_;
			return;
		}
	}
}

/// <summary>
/// 弾を取得する関数
/// </summary>
/// <returns>bullets_</returns>
std::vector<std::unique_ptr<Bullet>>& Weapon::GetBullets()
{
	return this->bullets_;
}
