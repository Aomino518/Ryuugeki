#include "Reticle.h"
#include "InputUtility.h"
#include <algorithm>
#include "Enemy.h"

void Reticle::Init()
{
    auto texReticle = TextureManager::GetInstance()->Load("resources/sprites/ui_reticle.png");
    sprite_ = std::make_unique<Sprite>();
    sprite_->Create(texReticle, { 0.0f, 0.0f }, Color::WHITE);
    sprite_->SetAnchorPoint({ 0.5f, 0.5f });
    Editor::GetInstance()->RegisterSprite("sprReticle", sprite_.get());
}

void Reticle::Update(const Vector3& playerPosition, std::vector<std::unique_ptr<Enemy>>& enemies)
{
    CameraManager* cameraManager = CameraManager::GetInstance();

    if (cameraManager->GetIsDebug()) {
        return;
    }

    Camera* camera = cameraManager->GetCamera("MainCamera");

    if (!camera) {
        return;
    }

	UpdateInput();
	UpdateScreenPosition(playerPosition, *camera);
	aimTarget_ = CulculateAimTarget(playerPosition, *camera, enemies);

	sprite_->Update();
}

void Reticle::Draw()
{
    sprite_->Draw();
}

void Reticle::UpdateInput()
{
	if (CameraManager::GetInstance()->GetIsDebug()) {
		return;
	}

	Input* input = Input::GetInstance();

	Vector2 leftStick = input->GetXbLeftStickVector();
	Vector2 rightStick = input->GetXbRightStickVector();
	leftStick = InputUtility::ApplyDeadZone(leftStick, deadZone_);
	rightStick = InputUtility::ApplyDeadZone(rightStick, deadZone_);

	//=========================================
	// 右スティックによる自由照準
	//=========================================
	aimOffset_.x += rightStick.x * aimSpeed_;
	// 上入力でレティクルを上へ動かす
	aimOffset_.y -= rightStick.y * aimSpeed_;

	// 右スティックを離したら徐々に中央へ戻す
	if (rightStick.x == 0.0f) {
		aimOffset_.x += (0.0f - aimOffset_.x) * aimReturnSpeed_;
	}

	if (rightStick.y == 0.0f) {
		aimOffset_.y += (0.0f - aimOffset_.y) * aimReturnSpeed_;
	}

	//=========================================
	// 左スティック方向へ照準を先行させる
	//=========================================
	Vector2 targetMoveOffset{
		leftStick.x * moveAimDistance_,
		-leftStick.y * moveAimDistance_
	};

	constexpr float reticleFollowSpeed = 0.3f;
	moveAimOffset_.x += (targetMoveOffset.x - moveAimOffset_.x) * reticleFollowSpeed;
	moveAimOffset_.y += (targetMoveOffset.y - moveAimOffset_.y) * reticleFollowSpeed;
}

void Reticle::UpdateScreenPosition(const Vector3& playerPos, const Camera& camera)
{
	//=========================================
	// プレイヤー正面を2D座標へ変換
	//=========================================
	Transform cameraTransform = camera.GetTransform();
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
		0.45f,
		static_cast<float>(Graphics::GetWidth()) /
		static_cast<float>(Graphics::GetHeight()),
		0.1f,
		1000.0f
	);

	Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

	Vector3 playerFrontPos{
		playerPos.x,
		playerPos.y,
		playerPos.z + aimDistance_
	};

	Vector3 playerFrontNdc = TransformToVector3(playerPos, viewProjectionMatrix);

	Vector2 playerFrontScreenPos = {
		(playerFrontNdc.x + 1.0f) * 0.5f * static_cast<float>(Graphics::GetWidth()),
		(1.0f - playerFrontNdc.y) * 0.5f * static_cast<float>(Graphics::GetHeight()),
	};

	// プレイヤー正面+左右スティックの照準量
	screenPosition_ = {
		playerFrontScreenPos.x + aimOffset_.x + moveAimOffset_.x,
		playerFrontScreenPos.y + aimOffset_.y + moveAimOffset_.y
	};

	// 画面外へ出ないようにする
	constexpr float margin = 32.0f;

	screenPosition_.x = std::clamp(
		screenPosition_.x,
		margin,
		static_cast<float>(Graphics::GetWidth()) - margin
	);

	screenPosition_.y = std::clamp(
		screenPosition_.y,
		margin,
		static_cast<float>(Graphics::GetHeight()) - margin
	);

	sprite_->SetPosition(screenPosition_);
}

Vector3 Reticle::CulculateAimTarget(const Vector3& playerPos, const Camera& camera, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	Transform cameraTransform = camera.GetTransform();

	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		cameraTransform.scale,
		cameraTransform.rotate,
		cameraTransform.translate
	);

	Matrix4x4 viewMatrix = Inverse(cameraMatrix);

	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
		0.45f,
		static_cast<float>(Graphics::GetWidth()) /
		static_cast<float>(Graphics::GetHeight()),
		0.1f,
		1000.0f
	);

	Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

	Matrix4x4 inverseViewProjection = Inverse(viewProjectionMatrix);

	float screenWidth = static_cast<float>(Graphics::GetWidth());

	float screenHeight = static_cast<float>(Graphics::GetHeight());

	// スクリーン座標をNDCへ変換
	float ndcX = (screenPosition_.x / screenWidth) * 2.0f - 1.0f;

	float ndcY = 1.0f - (screenPosition_.y / screenHeight) * 2.0f;

	Vector3 nearPosition = TransformToVector3(
		{ ndcX, ndcY, 0.0f },
		inverseViewProjection
	);

	Vector3 farPosition = TransformToVector3(
		{ ndcX, ndcY, 1.0f },
		inverseViewProjection
	);

	Vector3 rayDirection{
		farPosition.x - nearPosition.x,
		farPosition.y - nearPosition.y,
		farPosition.z - nearPosition.z
	};

	float rayLength = sqrtf(
		rayDirection.x * rayDirection.x +
		rayDirection.y * rayDirection.y +
		rayDirection.z * rayDirection.z
	);

	if (rayLength > 0.0001f) {
		rayDirection.x /= rayLength;
		rayDirection.y /= rayLength;
		rayDirection.z /= rayLength;
	}

	// カメラではなく、自機から一定距離先を照準地点にする
	aimTarget_ = {
		playerPos.x + rayDirection.x * aimDistance_,
		playerPos.y + rayDirection.y * aimDistance_,
		playerPos.z + rayDirection.z * aimDistance_,
	};

	//=========================================
	// レティクル内の敵を探す
	//=========================================
	float nearestScreenDistance = hitRadius_;

	for (const auto& enemy : enemies) {
		if (!enemy->GetIsAlive()) {
			continue;
		}

		Vector3 enemyPosition = enemy->GetPosition();

		Vector3 playerToEnemy{
			enemyPosition.x - playerPos.x,
			enemyPosition.y - playerPos.y,
			enemyPosition.z - playerPos.z
		};

		float enemyDistance = sqrtf(
			playerToEnemy.x * playerToEnemy.x +
			playerToEnemy.y * playerToEnemy.y +
			playerToEnemy.z * playerToEnemy.z
		);

		// 自機から50.0f以内の敵だけを補正対象にする
		if (enemyDistance > aimDistance_) {
			continue;
		}

		// 敵のワールド座標をNDCへ変換
		Vector3 enemyNdc = TransformToVector3(
			enemyPosition,
			viewProjectionMatrix
		);

		Vector2 enemyScreenPosition{
			(enemyNdc.x + 1.0f) * 0.5f * screenWidth,
			(1.0f - enemyNdc.y) * 0.5f * screenHeight
		};

		float dx = enemyScreenPosition.x - screenPosition_.x;

		float dy = enemyScreenPosition.y - screenPosition_.y;

		float screenDistance = sqrtf(dx * dx + dy * dy);

		// レティクル内で最も中心に近い敵を選ぶ
		if (screenDistance <= nearestScreenDistance) {
			nearestScreenDistance = screenDistance;
			aimTarget_ = enemyPosition;
		}
	}

	return aimTarget_;
}
