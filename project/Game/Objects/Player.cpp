#include "Player.h"
#include "Vector3.h"
#include "SceneIncludes.h"

void Player::Init(const Vector3& position) {
	modelPlayer_ = std::make_unique<Entity3D>();
	modelPlayer_->Init();
	modelPlayer_->SetModel("player");
	Editor::GetInstance()->RegisterModel("player", modelPlayer_.get());
	rot_ = modelPlayer_->GetRotate();

	bullets_.resize(bulletMax_);
	for (int i = 0; i < bulletMax_; i++) {
		bullets_[i] = std::make_unique<Bullet>();
		bullets_[i]->Init(position);
	}

	modelPlayer_->SetTranslate(position);
	sphere_ = { Vector3{position.x, position.y, position.z - 0.5f}, Vector3{3.0f, 2.0f, 3.0f} };
}

void Player::Update() {
	auto camMgr = CameraManager::GetInstance();

	if (isAlive_) {
		Move();
		Vector3 pos = modelPlayer_->GetTranslate();
		sphere_.center = pos;
	}

	for(auto& bullet : bullets_) {
		bullet->Update();
	}

	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelPlayer_->SetCamera(camera);
	}

	modelPlayer_->Update();
}

void Player::Draw() {

	for (auto& bullet : bullets_) {
		if (bullet->GetIsShot()) {
			bullet->Draw();
		}
	}

	if (isAlive_) {
		modelPlayer_->Draw();
	}

}

void Player::DebugDraw()
{
	DebugDraw::DrawSphere(sphere_.center, sphere_.radius, Color::GREEN, DebugDrawMode::Wireframe);

	for (int i = 0; i < bulletMax_; i++) {
		bullets_[i]->DebugDraw();
	}
}

void Player::Move()
{
	auto camMgr = CameraManager::GetInstance();
	if (camMgr->GetIsDebug()) {
		return;
	}

	auto input = Input::GetInstance();

	inputDir = { 0.0f, 0.0f };
	rot_ = { 0.0f, 0.0f, 0.0f };

	// 移動入力
	if (input->IsPress(DIK_W)) {
		inputDir.y += 1.0f;
		isController_ = false;
	}

	if (input->IsPress(DIK_S)) {
		inputDir.y -= 1.0f;
		isController_ = false;
	}

	if (input->IsPress(DIK_A)) {
		inputDir.x -= 1.0f;
		isController_ = false;
	}

	if (input->IsPress(DIK_D)) {
		inputDir.x += 1.0f;
		isController_ = false;
	}

	//=========================================
	// コントローラーの左スティック
	//=========================================
	Vector2 leftStick = input->GetXbLeftStickVector();
	bool isLeftStickInput = leftStick.x != 0.0f || leftStick.y != 0.0f;

	if (isLeftStickInput) {
		isController_ = true;
	}

	inputDir.x += leftStick.x;
	inputDir.y += leftStick.y;

	if (inputDir.x > 0.01f) {
		rot_.z = -0.5f;
	} else if (inputDir.x < -0.01f) {
		rot_.z = 0.5f;
	}

	modelPlayer_->SetRotate(rot_);

	float length = sqrtf(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
	// 入力方向正規化
	if (length > 0.0f) {
		if (length > 1.0f) {
			inputDir.x /= length;
			inputDir.y /= length;
		}

		velocity_.x += inputDir.x * acceleration;
		velocity_.y += inputDir.y * acceleration;
	}

	velocity_.x *= (1.0f - deceleration);
	velocity_.y *= (1.0f - deceleration);

	float speed = sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
	if (speed > maxSpeed) {
		velocity_.x = (velocity_.x / speed) * maxSpeed;
		velocity_.y = (velocity_.y / speed) * maxSpeed;
	}

	Vector3 pos = modelPlayer_->GetTranslate();
	pos.x += velocity_.x;
	pos.y += velocity_.y;
	// 移動範囲制限
	pos.x = std::clamp(pos.x, -33.0f, 33.0f);
	pos.y = std::clamp(pos.y, -20.0f, 20.0f);
	modelPlayer_->SetTranslate(pos);

	if (bulletCooldown_ > 0) {
		bulletCooldown_--;
	}

	for (auto& bullet : bullets_) {
		if (!bullet->GetIsShot()) {
			if (Input::GetInstance()->IsPress(DIK_SPACE)) {
				isController_ = false;
			} else if (input->GetXbRightTrigger() > 0.1f) {
				isController_ = true;
			}

			if (Input::GetInstance()->IsPress(DIK_SPACE) || input->GetXbRightTrigger() > 0.1f) {

				if (bulletCooldown_ <= 0) {
					if (!bullet->GetIsShot()) {
						bullet->Fire(pos, aimTarget_);
						bulletCooldown_ = 10;
						break;
					}
				}

			} else {
				bulletCooldown_ = 0;
			}
		}
	}
}
