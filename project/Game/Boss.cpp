#include "Boss.h"

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
}

void Boss::Update(const Vector3& playerPosition)
{
	if (isAlive_) {
		UpdateMovement(playerPosition);
		UpdateAttack();
		auto camMgr = CameraManager::GetInstance();
		if (!camMgr->GetIsDebug()) {
			Camera* camera = camMgr->GetActiveCamera();
			model_->SetCamera(camera);
		}

		model_->Update();
	}

	UpdateBullets();
}

void Boss::Draw()
{
	if (isAlive_) {
		model_->Draw();
	}

	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
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

	if (hp_ < 0) {
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
	ImGui::Text("isAlive : %s", isAlive_ ? "true" : "false");
	ImGui::End();
#endif
}

void Boss::UpdateMovement(const Vector3& playerPosition)
{
	moveTime_ += Time::GetDeltaTime();
	Vector3 translate = model_->GetTranslate();
	// -1.0～1.0の間を往復
	const float moveOffsetX = std::sin(moveTime_ * moveSpeed_) * moveAmplitude_;
	// プレイヤーを基準に左右移動
	translate.x = playerPosition.x + centerOffsetX_ + moveOffsetX;
	// プレイヤーとの高さを維持
	translate.y = playerPosition.y + heightOffset_;
	// プレイヤーとの前後距離を維持
	translate.z = playerPosition.z + distanceFromPlayer_;
	// 当たり判定をボスの現在位置へ追従
	position_ = translate;
	sphere_.center = translate;
	model_->SetTranslate(translate);
}

void Boss::UpdateAttack()
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
		isBurstAttacking_ = true;
		burstShotCount_ = 0;

		burstTimer_ = burstInterval_;
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
