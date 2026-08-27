#include "Boss.h"

void Boss::Init(const Vector3& position, const Vector3& playerPosition)
{
	position_ = position;

	moveTime_ = 0.0f;

	isAlive_ = true;

	// プレイヤーから見た初期位置を保存
	centerOffsetX_ = position_.x - playerPosition.x;

	heightOffset_ = position_.y - playerPosition.y;

	distanceFromPlayer_ = position_.z - playerPosition.z;
}

void Boss::Update(const Vector3& playerPosition)
{
	if (isAlive_) {
		moveTime_ += Time::GetDeltaTime();

		// -1.0～1.0の間を往復
		const float moveOffsetX = std::sin(moveTime_ * moveSpeed_) * moveAmplitude_;

		// プレイヤーを基準に左右移動
		position_.x = playerPosition.x + centerOffsetX_ + moveOffsetX;

		// プレイヤーとの高さを維持
		position_.y = playerPosition.y + heightOffset_;

		// プレイヤーとの前後距離を維持
		position_.z = playerPosition.z + distanceFromPlayer_;
	}
}

void Boss::Draw()
{

}

void Boss::DrawDebug()
{
	if (isAlive_) {
		DebugDraw::DrawSphere(position_, Vector3{ 5.0f, 5.0f, 5.0f }, Color::RED, DebugDrawMode::Solid);
	}
}
