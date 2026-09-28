#include "Enemy.h"
#include <numbers>
#include "Vector3.h"

void Enemy::Init(EnemyPattern pattern, const Vector3& position) {
	model_ = std::make_unique<Entity3D>();
	model_->Init();
	model_->SetModel("enemy");
	model_->SetTranslate(position);
	transform_ = model_->GetTransform();
	isAlive_ = true;
	pattern_ = pattern;
	sphere_ = { position, Vector3{ 1.0f, 1.0f, 1.0f } };
	bullet_ = std::make_unique<EnemyBullet>();
	bullet_->Init(position);
	timer_ = shotInterval_;
	enemyState_ = EnemyState::Approach;
	isMoveStop_ = false;
}

void Enemy::Update() {
	UpdateState();
	model_->SetTranslate(transform_.translate);
	sphere_.center = transform_.translate;
	UpdateShooting();

	auto camMgr = CameraManager::GetInstance();
	if (debugHitTimer_ > 0) {
		debugHitTimer_--;

		if (debugHitTimer_ <= 0) {
			debugIsHit_ = false;
		}
	}

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

void Enemy::UpdateMovement()
{
	switch (pattern_) {
		case EnemyPattern::Straight:
			UpdateStraight();
			break;
		case EnemyPattern::SinWave:
			UpdateSinWave();
			break;
		case EnemyPattern::ZigZag:
			UpdateZigZag();
			break;
		case EnemyPattern::SlowFast:
			UpdateSlowFast();
			break;
		case EnemyPattern::KeepStop:
			UpdateKeepStop();
			break;
	}

	sphere_.center = transform_.translate;
	model_->SetTranslate(transform_.translate);
}

//=================================
// 敵の行動更新処理
//=================================
void Enemy::UpdateStraight()
{
	transform_.translate.z -= speed;
}

void Enemy::UpdateSinWave()
{
	theta += std::numbers::pi_v<float> / 60.0f;
	transform_.translate.x += sin(theta) * amplitude;
	transform_.translate.z -= speed;
}

void Enemy::UpdateZigZag()
{
	switchDirTimer--;

	if (switchDirTimer <= 0) {
		dir = -dir;
		switchDirTimer = 30;
	}

	transform_.translate.x += dir * speed;
	transform_.translate.z -= speed;
}

void Enemy::UpdateSlowFast()
{
	// 徐々に加速
	speed += 0.02f;
	transform_.translate.z -= speed;
}

void Enemy::UpdateKeepStop()
{
	transform_.translate.z -= speed;
	distance += speed;
	if (distance >= maxDistance) {
		distance = 0.0f;
		approachBehavior_ = ApproachBehavior::StopAtTarget;
	}
}

//=================================
// 状態の更新処理
//=================================
void Enemy::UpdateState()
{
	switch (enemyState_) {
	case EnemyState::Approach:
		UpdateApproach();
		break;
	case EnemyState::Hover:
		UpdateHover();
		break;
	case EnemyState::Dead:

		break;
	}
}

void Enemy::UpdateApproach()
{
	if (isMoveStop_) {
		return;
	}

	UpdateMovement();

	if (approachBehavior_ == ApproachBehavior::StopAtTarget) {
		EnterHover();
		enemyState_ = EnemyState::Hover;
	}
}

void Enemy::UpdateHover()
{
	hoverTimer_ += Time::GetDeltaTime();

	const float phase =
		2.0f * std::numbers::pi_v<float> *hoverTimer_ / hoverPeriod_;

	transform_.translate = hoverOrigin_;
	transform_.translate.y += std::sin(phase) * hoverAmplitude_;
}

//=================================
// 攻撃更新処理
//=================================
void Enemy::UpdateShooting()
{
	if (isAlive_) {
		timer_--;

		if (timer_ <= 0) {
			bullet_->Fire(model_->GetTranslate());
			timer_ = shotInterval_;
		}
	}

	bullet_->Update();
}

void Enemy::EnterHover()
{
	enemyState_ = EnemyState::Hover;
	hoverOrigin_ = transform_.translate;
	hoverTimer_ = 0.0f;
}
