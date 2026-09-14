#include "Boss.h"
#include <algorithm>
#include "MathFunc.h"
#include <cmath>

void Boss::Init(const Vector3& position, const Vector3& playerPosition)
{
	position_ = position;
	model_ = std::make_unique<Entity3D>();
	model_->Init();
	model_->SetModel("boss");
	Editor::GetInstance()->RegisterModel("boss", model_.get());
	model_->SetTranslate(position);
	sphere_.center = position;
	sphere_.radius = { 5.0f, 5.0f, 5.0f };
	moveTime_ = 0.0f;
	isAlive_ = true;

	Vector3 translate = model_->GetTranslate();
	// プレイヤーから見た初期位置を保存
	centerOffsetX_ = translate.x - playerPosition.x;
	heightOffset_ = translate.y - playerPosition.y;
	distanceFromPlayer_ = translate.z - playerPosition.z;

	hp_ = kMaxHp;
	isSecondPhase_ = false;
	shootingDirection_ = RushDirection::LeftToRight;
	shootingMoveTime_ = 0.0f;
	rushDirection_ = RushDirection::LeftToRight;
	rushTime_ = 0.0f;
	isRushing_ = false;
	nextAttackIsRush_ = false;
	attackTimer_ = 0.0f;
	burstTimer_ = 0.0f;
	isBurstAttacking_ = false;
	burstShotCount_ = 0;
	phase_ = BossPhase::Phase1;
	dethParticle_.Init();
	const uint32_t texWhite = TextureManager::GetInstance()->Load("resources/sprites/texWhite.png");
	rushWarning_ = std::make_unique<Sprite>();
	rushWarning_->Init();
	rushWarning_->Create(texWhite, { 0.0f, 0.0f }, { 1.0f, 0.2f, 0.1f, 1.0f });
	rushWarning_->SetAnchorPoint({ 0.0f, 0.5f });
}

void Boss::Update(const Vector3& playerPosition)
{
	if (isAlive_) {
		if (phase_ == BossPhase::Phase1 && hp_ <= kMaxHp / 2) {
			phase_ = BossPhase::Phase2;
			isSecondPhase_ = true;
			color_ = { 1.0f, 0.3f, 0.3f, 1.0f };
			// 画面外からの射撃移動を先に行い、その後は突進と交互に使う
			nextAttackIsRush_ = false;
			attackTimer_ = 0.0f;
			isBurstAttacking_ = false;
			burstShotCount_ = 0;
			burstTimer_ = 0.0f;
			shootingMoveTime_ = 0.0f;
			dethParticle_.SpawnHitEffect(position_);
		}

		if (isRushing_) {
			UpdateRush(playerPosition);
		} else {
			UpdateMovement(playerPosition);
			UpdateAttack(playerPosition);
		}

		auto camMgr = CameraManager::GetInstance();
		if (!camMgr->GetIsDebug()) {
			Camera* camera = camMgr->GetActiveCamera();
			model_->SetCamera(camera);
		}

		model_->Update();
	}

	UpdateBullets();
	dethParticle_.Update();
}

void Boss::Draw()
{
	if (isAlive_) {
		model_->SetMaterial(color_);
		model_->Draw();
	}

	for (auto& bullet : bullets_) {
		bullet->Draw();
	}

	dethParticle_.Draw();
}

void Boss::DrawRushWarning()
{
	if (!isAlive_ || !isRushing_ || rushTime_ >= 0.0f || !rushWarning_) {
		return;
	}
	const Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	Vector3 start = rushTarget_;
	Vector3 end = rushTarget_;
	start.z = end.z = position_.z;
	const bool horizontal = rushDirection_ == RushDirection::LeftToRight ||
		rushDirection_ == RushDirection::RightToLeft;
	if (horizontal) {
		start.x -= rushHalfWidth_;
		end.x += rushHalfWidth_;
	} else {
		start.y -= rushHalfHeight_;
		end.y += rushHalfHeight_;
	}
	// カメラの背後にある線は投影しない
	if (TransformToVector3(start, camera->GetViewMatrix()).z <= 0.0f ||
		TransformToVector3(end, camera->GetViewMatrix()).z <= 0.0f) {
		return;
	}
	const auto& vp = camera->GetViewProjectionMatrix();
	const Vector3 a = TransformToVector3(start, vp);
	const Vector3 b = TransformToVector3(end, vp);
	auto app = Application::GetInstance();
	const float width = static_cast<float>(app->GetWidth());
	const float height = static_cast<float>(app->GetHeight());
	const Vector2 screenStart{ (a.x + 1.0f) * width * 0.5f, (1.0f - a.y) * height * 0.5f };
	const float dx = (b.x - a.x) * width * 0.5f;
	const float dy = (a.y - b.y) * height * 0.5f;
	rushWarning_->SetPosition(screenStart);
	rushWarning_->SetSize({ std::sqrt(dx * dx + dy * dy), rushWarningThickness_ });
	rushWarning_->SetRotation(std::atan2(dy, dx));
	const float pulse = 0.5f + 0.5f * std::sin(rushTime_ * 25.0f);
	rushWarning_->SetColor({ 1.0f, 0.15f + 0.5f * pulse, 0.1f, 1.0f });
	rushWarning_->Update();
	rushWarning_->Draw();
}

void Boss::DrawDebug()
{
#ifdef _DEBUG
	if (isAlive_) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::GREEN, DebugDrawMode::Wireframe);
	}

	for (auto& bullet : bullets_) {
		bullet->DebuDraw();
	}
#endif
}

void Boss::Damage()
{
	if (!isAlive_) {
		return;
	}

	hp_--;

	if (hp_ <= 0) {
		hp_ = 0;
		isAlive_ = false;
	}
}

void Boss::DrawImGui()
{
#ifdef _DEBUG
	ImGui::Begin("Boss Status");
	ImGui::Text("position : %0.2f, %0.2f, %0.2f", position_.x, position_.y, position_.z);
	ImGui::Text("sphere_.center : %0.2f, %0.2f, %0.2f", sphere_.center.x, sphere_.center.y, sphere_.center.z);
	ImGui::Text("HP : %d", hp_);
	ImGui::Text("Phase : %s", isSecondPhase_ ? "Second" : "First");
	ImGui::Text("Rushing : %s", isRushing_ ? "true" : "false");
	ImGui::Text("isAlive : %s", isAlive_ ? "true" : "false");
	ImGui::End();
#endif
}

void Boss::UpdateMovement(const Vector3& playerPosition)
{

	moveTime_ += Time::GetDeltaTime();

	Vector3 translate = playerPosition;
	translate.x += centerOffsetX_
		+ std::sin(moveTime_ * moveSpeed_) * moveAmplitude_;
	translate.y += heightOffset_;
	translate.z += distanceFromPlayer_;

	if (isSecondPhase_) {
		shootingMoveTime_ += Time::GetDeltaTime();
		// 横切った後は画面外で次の突進を待つ
		const float t = std::min(shootingMoveTime_ / shootingMoveDuration_, 1.0f);
		const float offset = t * 2.0f - 1.0f;
		const Camera* camera = CameraManager::GetInstance()->GetCamera("MainCamera");
		const Vector3& cameraPos = camera->GetTranslate();
		const auto& projection = camera->GetProjectionMatrix();
		// 現在の奥行きにおける画面端に、ボスが隠れる余白を加える
		const float depth = translate.z - cameraPos.z;
		const float halfWidth = depth / projection.m[0][0] + screenExitMargin_;
		const float halfHeight = depth / projection.m[1][1] + screenExitMargin_;
		translate.x = cameraPos.x;
		translate.y = cameraPos.y;
		switch (shootingDirection_) {
		case RushDirection::LeftToRight: translate.x += offset * halfWidth; break;
		case RushDirection::UpToDown: translate.y -= offset * halfHeight; break;
		case RushDirection::RightToLeft: translate.x -= offset * halfWidth; break;
		case RushDirection::DownToUp: translate.y += offset * halfHeight; break;
		}
	}

	position_ = translate;
	sphere_.center = translate;
	model_->SetTranslate(translate);
}

void Boss::UpdateAttack(const Vector3& playerPosition)
{
	const float deltaTime = Time::GetDeltaTime();

	// 連射中
	if (isBurstAttacking_) {
		burstTimer_ += deltaTime;

		if (burstTimer_ >= burstInterval_) {
			burstTimer_ -= burstInterval_;

			FireBullet();
			++burstShotCount_;

			if (burstShotCount_ >= kBurstShotMax_) {
				isBurstAttacking_ = false;
				burstShotCount_ = 0;
				attackTimer_ = 0.0f;
			}
		}

		return;
	}

	// 次の攻撃まで待機
	attackTimer_ += deltaTime;

	if (attackTimer_ >= attackInterval_) {
		attackTimer_ = 0.0f;

		if (isSecondPhase_ && nextAttackIsRush_) {
			nextAttackIsRush_ = false;
			StartRush(playerPosition);
		} else {
			isBurstAttacking_ = true;
			burstShotCount_ = 0;
			burstTimer_ = burstInterval_;

			if (isSecondPhase_) {
				nextAttackIsRush_ = true;
			}
		}
	}
}

void Boss::FireBullet()
{
	auto bullet = std::make_unique<EnemyBullet>();

	bullet->Init(position_);
	bullet->Fire(position_);

	bullets_.push_back(std::move(bullet));
}

void Boss::UpdateBullets()
{
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		(*it)->Update();

		if (!(*it)->GetIsShot()) {
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}
}

void Boss::StartRush(const Vector3& playerPosition)
{
	isRushing_ = true;
	// 負の時間の間は進路を予告し、0になってから突進する
	rushTime_ = -rushWarningDuration_;
	rushTarget_ = playerPosition;

	Vector3 translate = rushTarget_;

	switch (rushDirection_) {
	case RushDirection::LeftToRight:
		translate.x -= rushHalfWidth_;
		break;
	case RushDirection::UpToDown:
		translate.y += rushHalfHeight_;
		break;
	case RushDirection::RightToLeft:
		translate.x += rushHalfWidth_;
		break;
	case RushDirection::DownToUp:
		translate.y -= rushHalfHeight_;
		break;
	}

	position_ = translate;
	sphere_.center = translate;
	model_->SetTranslate(translate);
}

void Boss::UpdateRush(const Vector3& playerPosition)
{
	rushTime_ += Time::GetDeltaTime();
	if (rushTime_ < 0.0f) {
		position_.z = playerPosition.z;
		sphere_.center = position_;
		model_->SetTranslate(position_);
		return;
	}

	float t = rushTime_ / rushDuration_;
	if (t > 1.0f) {
		t = 1.0f;
	}

	const float offset = t * 2.0f - 1.0f;

	Vector3 translate = rushTarget_;
	translate.z = playerPosition.z;

	switch (rushDirection_) {
	case RushDirection::LeftToRight:
		translate.x += offset * rushHalfWidth_;
		break;
	case RushDirection::UpToDown:
		translate.y -= offset * rushHalfHeight_;
		break;
	case RushDirection::RightToLeft:
		translate.x -= offset * rushHalfWidth_;
		break;
	case RushDirection::DownToUp:
		translate.y += offset * rushHalfHeight_;
		break;
	}

	position_ = translate;
	sphere_.center = translate;
	model_->SetTranslate(translate);

	if (t >= 1.0f) {
		isRushing_ = false;
		shootingMoveTime_ = 0.0f;
		shootingDirection_ = NextDirection(shootingDirection_);
		attackTimer_ = 0.0f;
		rushTime_ = 0.0f;
		rushDirection_ = NextDirection(rushDirection_);
	}
}

Boss::RushDirection Boss::NextDirection(RushDirection direction)
{
	switch (direction) {
	case RushDirection::LeftToRight: return RushDirection::UpToDown;
	case RushDirection::UpToDown: return RushDirection::RightToLeft;
	case RushDirection::RightToLeft: return RushDirection::DownToUp;
	case RushDirection::DownToUp: return RushDirection::LeftToRight;
	}
	return RushDirection::LeftToRight;
}
