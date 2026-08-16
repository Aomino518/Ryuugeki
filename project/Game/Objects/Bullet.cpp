#include "Bullet.h"
#include "Enemy.h"
#include "SceneIncludes.h"

void Bullet::Init(const Vector3& position) {
	model_ = std::make_unique<Entity3D>();
	model_->Init();
	model_->SetModel("bullet");
	model_->SetTranslate(position);
	sphere_ = { position, Vector3{ 1.0f, 1.0f, 1.0f } };
}

void Bullet::Update() {
	auto camMgr = CameraManager::GetInstance();

	if (isShot_) {
		Vector3 pos = model_->GetTranslate();
		pos.x += direction_.x * speed_;
		pos.y += direction_.y * speed_;
		pos.z += direction_.z * speed_;

		sphere_.center = pos;
		model_->SetTranslate(pos);
		
		Vector3 movedVector{
			pos.x - startPosition_.x,
			pos.y - startPosition_.y,
			pos.z - startPosition_.z
		};

		float movedDistance = sqrtf(movedVector.x * movedVector.x + movedVector.y * movedVector.y + movedVector.z * movedVector.z);

		if (movedDistance >= maxDistance_) {
			isShot_ = false;
		}
	}

	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		model_->SetCamera(camera);
	}

	model_->Update();
}

void Bullet::Draw() {
	if (isShot_) {
		model_->Draw();
	}
}

void Bullet::DebugDraw()
{
	if (isShot_) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::GREEN, DebugDrawMode::Wireframe);
	}
}

bool Bullet::BulletIsCollision(const Enemy& enemy) const
{
	return IsCollision(sphere_, enemy.GetSphere());
}

void Bullet::Fire(const Vector3& startPosition, const Vector3& targetPosition)
{
	Vector3 toTarget{
		targetPosition.x - startPosition.x,
		targetPosition.y - startPosition.y,
		targetPosition.z - startPosition.z
	};

	float length = sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);

	if (length <= 0.0001f) {
		return;
	}

	// 発射方向を長さ1.0に正規化
	direction_.x = toTarget.x / length;
	direction_.y = toTarget.y / length;
	direction_.z = toTarget.z / length;

	startPosition_ = startPosition;

	model_->SetTranslate(startPosition);
	sphere_.center = startPosition;

	isShot_ = true;
}
