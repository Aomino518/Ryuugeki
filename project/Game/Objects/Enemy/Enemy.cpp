#include "Enemy.h"
#include "EnemyEnterState.h"
#include <cmath>
#include "CameraManager.h"
#include "Color.h"
#include "DebugDraw.h"
#include "GameTime.h"
#include <numbers>
#include "Vector3.h"

void Enemy::RequestStateChange(std::unique_ptr<IEnemyState> nextState)
{
	nextState_ = std::move(nextState);
}

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
	// 入場からスタートする
	state_.reset();
	nextState_.reset();
	exitFinished_ = false;
	isHover_ = false;
	distance = 0.0f;
	approachBehavior_ = ApproachBehavior::KeepMoving;
	isMoveStop_ = false;

	RequestStateChange(std::make_unique<EnemyEnterState>());
	ApplyStateChange();
	model_->SetTranslate(transform_.translate);
	sphere_.center = transform_.translate;
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

void Enemy::Move(const Vector3& offset)
{
	transform_.translate += offset;
}

void Enemy::UpdateMovement()
{
	switch (pattern_) {
		case EnemyPattern::Straight:
			UpdateStraight();
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
	transform_.translate.z -= speed_;
}

void Enemy::UpdateKeepStop()
{
	transform_.translate.z -= speed_;
	distance += speed_;
	if (distance >= maxDistance) {
		distance = 0.0f;
		approachBehavior_ = ApproachBehavior::StopAtTarget;
	}
}

void Enemy::ApplyStateChange()
{
	if (!nextState_) {
		return;
	}

	if (state_) {
		state_->Exit(*this);
	}

	state_ = std::move(nextState_);
	state_->Enter(*this);
}

//=================================
// 状態の更新処理
//=================================
void Enemy::UpdateState()
{
	if (!isAlive_ || isMoveStop_) {
		return;
	}

	if (state_) {
		state_->Update(*this);
	}

	ApplyStateChange();
}

void Enemy::UpdateApproach()
{
	if (isMoveStop_) {
		return;
	}

	UpdateMovement();

	if (approachBehavior_ == ApproachBehavior::StopAtTarget) {
		EnterHover();
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
	isHover_ = true;
	hoverOrigin_ = transform_.translate;
	hoverTimer_ = 0.0f;
}

void Enemy::UpdateActiveMovement()
{
	if (isHover_) {
		UpdateHover();
	} else {
		UpdateApproach();
	}
}
