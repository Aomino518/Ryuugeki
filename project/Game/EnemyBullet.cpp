#include "EnemyBullet.h"
#include "Player.h"
#include "MathFunc.h"

void EnemyBullet::Init(const Vector3& position)
{
	model_ = std::make_unique<Entity3D>();
	model_->Init();
	model_->SetModel("bullet");
	model_->SetMaterial(Color::BLACK);
	model_->SetTranslate(position);
	sphere_ = { position, Vector3{ 0.6f, 0.6f, 0.6f } };
	isShot_ = false;
}

void EnemyBullet::Update()
{
	auto camMgr = CameraManager::GetInstance();

	if (debugHitTimer_ > 0) {
		debugHitTimer_--;

		if (debugHitTimer_ <= 0) {
			debugIsHit_ = false;
		}
	}

	if (isShot_) {
		Vector3 pos = model_->GetTranslate();
		pos.z -= speed_;
		sphere_.center = pos;
		model_->SetTranslate(pos);

		lifeTimer_ += Time::GetDeltaTime();

		if (lifeTimer_ >= lifeTime_) {
			isShot_ = false;
		}
	}

	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		model_->SetCamera(camera);
	}

	model_->Update();
}

void EnemyBullet::Draw()
{
	if (isShot_) {
		model_->Draw();
	}
}

void EnemyBullet::DebuDraw()
{
	if (!debugIsHit_ && isShot_) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::GREEN, DebugDrawMode::Wireframe);
	} else  if (debugHitTimer_ > 0) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::RED, DebugDrawMode::Wireframe);
	}
}

void EnemyBullet::SetIsDebugHit()
{
	debugIsHit_ = true;
	debugHitTimer_ = 10;
}

void EnemyBullet::Fire(const Vector3& position)
{
	model_->SetTranslate(position);
	sphere_.center = position;
	lifeTimer_ = 0.0f;
	isShot_ = true;
}

bool EnemyBullet::BulletIsCollision(const Player& player) const
{
	if (!isShot_) {
		return false;
	}

	return IsCollision(sphere_, player.GetSphere());
}
