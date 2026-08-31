#include "Enemy.h"
#include <numbers>
#include "Vector3.h"

void Enemy::Init(EnemyPattern pattern, const Vector3& position) {
	model_ = std::make_unique<Entity3D>();
	model_->Init();
	model_->SetModel("enemy");
	model_->SetTranslate(position);
	isAlive_ = true;
	pattern_ = pattern;
	sphere_ = { position, Vector3{ 1.0f, 1.0f, 1.0f } };
	bullet_ = std::make_unique<EnemyBullet>();
	bullet_->Init(position);
	timer_ = shotInterval_;
}

void Enemy::Update() {
	auto camMgr = CameraManager::GetInstance();
	if (debugHitTimer_ > 0) {
		debugHitTimer_--;

		if (debugHitTimer_ <= 0) {
			debugIsHit_ = false;
		}
	}

	if (!isMoveStop_) {
		transform_ = model_->GetTransform();
		if (isAlive_) {
			switch (pattern_) {
			case EnemyPattern::Straight:
				transform_.translate.z -= speed;

				break;
			case EnemyPattern::SinWave:
				theta += std::numbers::pi_v<float> / 60.0f;
				transform_.translate.x += sin(theta) * amplitude;
				transform_.translate.z -= speed;

				break;
			case EnemyPattern::ZigZag:
				switchDirTimer--;

				if (switchDirTimer <= 0) {
					dir = -dir;
					switchDirTimer = 30;
				}

				transform_.translate.x += dir * speed;
				transform_.translate.z -= speed;

				break;
			case EnemyPattern::SlowFast:
				// 徐々に加速
				speed += 0.02f;
				transform_.translate.z -= speed;

				break;
			}
		}

		sphere_.center = transform_.translate;
		model_->SetTranslate(transform_.translate);
	}

	//================================
	// 3秒間隔の弾発射
	//================================
	if (isAlive_) {
		timer_--;

		if (timer_ <= 0) {
			bullet_->Fire(model_->GetTranslate());
			timer_ = shotInterval_;
		}
	}

	bullet_->Update();

	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		model_->SetCamera(camera);
	}
	model_->Update();
}

void Enemy::Draw() {
	bullet_->Draw();
	if (isAlive_) {
		model_->Draw();
	}
}

void Enemy::DebugDraw()
{
#ifdef _DEBUG
	if (!debugIsHit_) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::GREEN, DebugDrawMode::Wireframe);
	} else  if (!isAlive_ && debugHitTimer_ > 0) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::RED, DebugDrawMode::Wireframe);
	}
	bullet_->DebuDraw();
#endif
}

void Enemy::SetIsDebugHit()
{
	debugIsHit_ = true;
	debugHitTimer_ = 10;
}