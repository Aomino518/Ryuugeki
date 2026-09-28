#define NOMINMAX
#include "Player.h"
#include "Vector3.h"
#include "SceneIncludes.h"
#include "Utilities/InputUtility.h"

/// <summary>
/// 初期化処理関数
/// </summary>
/// <param name="position">初期位置</param>
void Player::Init(const Vector3& position) {
	modelPlayer_ = std::make_unique<Entity3D>();
	modelPlayer_->Init();
	modelPlayer_->SetModel("player");
	Editor::GetInstance()->RegisterModel("player", modelPlayer_.get());
	rot_ = modelPlayer_->GetRotate();

	// 武器の初期化
	weapon_.Init(position);
	dethParticle_.Init();

	modelPlayer_->SetTranslate(position);
	scrollPosition_ = position;
	sphere_ = { Vector3{position.x, position.y, position.z - 0.5f}, Vector3{3.0f, 2.0f, 3.0f} };
	hpBar_.Init(Vector2{ 25.0f, 670.0f }, Vector2{300.0f, 30.0f}, Color::GREEN);
}

void Player::SetScrollPosition(const Vector3& position)
{
	Vector3 pos = modelPlayer_->GetTranslate();
	pos.x += position.x - scrollPosition_.x;
	pos.y += position.y - scrollPosition_.y;
	pos.z += position.z - scrollPosition_.z;
	scrollPosition_ = position;
	modelPlayer_->SetTranslate(pos);
}

void Player::Update() {
	if (debugHitTimer_ > 0) {
		debugHitTimer_--;

		if (debugHitTimer_ <= 0) {
			debugIsHit_ = false;
		}
	}

	if (isAlive_) {
		UpdateMovement();
		UpdateShooting();
		Vector3 pos = modelPlayer_->GetTranslate();
		sphere_.center = pos;
	} else {
		dethParticle_.Update();
	}

	UpdateCamera();
	weapon_.Update();
	UpdateHitEffect();
	modelPlayer_->Update();
	hpBar_.SetVisible(true);
	if (isAlive_) {
		hpBar_.Update(hp_, kMaxHp);
	}
}

void Player::Draw() {
	weapon_.Draw();
	dethParticle_.Draw();

	if (isAlive_) {
		modelPlayer_->Draw();
	}
	hpBar_.Draw();
}

void Player::DebugDraw()
{
#ifdef _DEBUG
	if (!debugIsHit_) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::GREEN, DebugDrawMode::Wireframe);
	} else  if (!isAlive_ && debugHitTimer_ > 0) {
		DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::RED, DebugDrawMode::Wireframe);
	}

	weapon_.DebugDraw();
#endif
}

void Player::DrawImGui()
{
#ifdef _DEBUG
	ImGui::Begin("Player Status");
	ImGui::Text("HP : %d", hp_);
	ImGui::Text("isAlive : %s", isAlive_ ? "true" : "false");
	ImGui::Text("invincibleTimer : %0.2f", invincibleTimer_);
	ImGui::End();
#endif
}

void Player::SetIsDebugHit()
{
	debugIsHit_ = true;
	debugHitTimer_ = 10;
}

void Player::Damage()
{
	if (!isAlive_ || invincibleTimer_ > 0.0f) {
		return;
	}

	hitEffectTimer_ = kHitEffectDuration;
	invincibleTimer_ = invincibleTime_;

	hp_--;

	if (hp_ <= 0) {
		hp_ = 0;
		isAlive_ = false;
		dethParticle_.SpawnHitEffect(modelPlayer_->GetTranslate());
	}
}

/// <summary>
/// 移動入力値を返す関数
/// </summary>
/// <returns>入力値</returns>
Vector2 Player::GetMovementInput()
{
	auto input = Input::GetInstance();
	Vector2 direction{};

	// 移動入力
	if (input->IsPress(DIK_W)) {
		direction.y += 1.0f;
		isController_ = false;
	}

	if (input->IsPress(DIK_S)) {
		direction.y -= 1.0f;
		isController_ = false;
	}

	if (input->IsPress(DIK_A)) {
		direction.x -= 1.0f;
		isController_ = false;
	}

	if (input->IsPress(DIK_D)) {
		direction.x += 1.0f;
		isController_ = false;
	}

	//=========================================
	// コントローラーの左スティック
	//=========================================
	Vector2 leftStick = input->GetXbLeftStickVector();
	leftStick = InputUtility::ApplyDeadZone(leftStick, deadZone_);

	if (leftStick.x != 0.0f || leftStick.y != 0.0f) {
		direction.x += leftStick.x;
		direction.y += leftStick.y;
		isController_ = true;
	}

	return direction;
}

/// <summary>
/// 移動の更新処理
/// </summary>
void Player::UpdateMovement()
{
	if (CameraManager::GetInstance()->GetIsDebug()) {
		return;
	}

	const Vector2 inputDirection = GetMovementInput();

	UpdateRotation(inputDirection);
	UpdateVelocity(inputDirection);
	UpdatePosition();
}

/// <summary>
/// 回転の更新処理
/// </summary>
/// <param name="input">入力値</param>
void Player::UpdateRotation(const Vector2& input)
{
	// スティックを倒した量から目標角度を決める
	float targetRotX = input.y * maxTiltAngle_;
	float targetRotY = input.x * maxTiltAngle_;

	// 目標角度までの時間を0.0 ~ 1.0にする
	float t = interpolationSpeed_ * Time::GetDeltaTime();
	t = std::clamp(t, 0.0f, 1.0f);

	// 傾きを補間
	rot_.x = Lerp(rot_.x, -targetRotX, t);
	rot_.y = Lerp(rot_.y, targetRotY, t);

	modelPlayer_->SetRotate(rot_);
}

/// <summary>
/// 速度の更新処理
/// </summary>
/// <param name="input">入力値</param>
void Player::UpdateVelocity(const Vector2& input)
{
	Vector2 direction = input;
	float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
	// 入力方向正規化
	if (length > 0.0f) {
		// 斜め入力で直線移動よりも速くならないように正規化
		if (length > 1.0f) {
			direction.x /= length;
			direction.y /= length;
		}

		velocity_.x += direction.x * acceleration;
		velocity_.y += direction.y * acceleration;
	}

	velocity_.x *= (1.0f - deceleration);
	velocity_.y *= (1.0f - deceleration);

	float speed = sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
	// 速度を正規化
	if (speed > maxSpeed) {
		velocity_.x = (velocity_.x / speed) * maxSpeed;
		velocity_.y = (velocity_.y / speed) * maxSpeed;
	}
}

/// <summary>
/// 位置の更新処理
/// </summary>
void Player::UpdatePosition()
{
	Vector3 pos = modelPlayer_->GetTranslate();
	pos.x += velocity_.x;
	pos.y += velocity_.y;
	// 移動範囲制限
	pos.x = std::clamp(pos.x, scrollPosition_.x - 18.0f, scrollPosition_.x + 18.0f);
	pos.y = std::clamp(pos.y, scrollPosition_.y - 10.0f, scrollPosition_.y + 12.0f);
	modelPlayer_->SetTranslate(pos);
}

/// <summary>
/// 射撃の更新処理
/// </summary>
void Player::UpdateShooting()
{
	if (Input::GetInstance()->IsPress(DIK_SPACE)) {
		isController_ = false;
	} else if (Input::GetInstance()->GetXbRightTrigger() > 0.1f) {
		isController_ = true;
	}

	Vector3 pos = modelPlayer_->GetTranslate();

	if (Input::GetInstance()->IsPress(DIK_SPACE) || Input::GetInstance()->GetXbRightTrigger() > 0.1f) {
		weapon_.Fire(pos, aimTarget_);
	}
}

/// <summary>
/// カメラの更新処理
/// </summary>
void Player::UpdateCamera()
{
	auto camMgr = CameraManager::GetInstance();
	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();
		modelPlayer_->SetCamera(camera);
	}
}

void Player::UpdateHitEffect()
{
	Vector3 drawRotation = rot_;
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (invincibleTimer_ > 0.0f) {
		invincibleTimer_ -= Time::GetDeltaTime();
	} else if (invincibleTimer_ < 0.0f) {
		invincibleTimer_ = 0.0f;
	}

	if (hitEffectTimer_ > 0.0f) {
		const float elapsed = kHitEffectDuration - hitEffectTimer_;
		const float strength = hitEffectTimer_ / kHitEffectDuration;

		float kPi = std::numbers::pi_v<float>;
		drawRotation.z += std::sin(elapsed * 2.0f * kPi * 20.0f) * kHitShakeAngle * strength;

		color = {
			1.0f,
			1.0f - 0.8f * strength,
			1.0f - 0.8f * strength,
			1.0f
		};

		hitEffectTimer_ = std::max(0.0f, hitEffectTimer_ - Time::GetDeltaTime());

		modelPlayer_->SetRotate(drawRotation);
		modelPlayer_->SetMaterial(color);
	}
}
