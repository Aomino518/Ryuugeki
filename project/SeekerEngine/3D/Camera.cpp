#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Camera.h"
#include "Application.h"
#include "Graphics.h"
#include "JsonTransform.h"
#include "GameTime.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

Camera::Camera()
	: transform_({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }),
	fovY_(0.45f),
	aspectRatio_(float(Application::GetInstance()->GetWidth()) / float(Application::GetInstance()->GetHeight())),
	nearClip_(0.1f),
	farClip_(100.0f),
	worldMatrix_(MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate)),
	viewMatrix_(Inverse(worldMatrix_)),
	projectionMatrix_(MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_)),
	viewProjectionMatrix_(Multiply(viewMatrix_, projectionMatrix_))
{}

void Camera::Update()
{
	aspectRatio_ = float(Graphics::GetInstance()->GetWidth()) / float(Graphics::GetHeight());
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

Matrix4x4 Camera::GetBillboardMatrix()
{
	Matrix4x4 invView = Inverse(viewMatrix_);

	// 平行移動は不要なので0.0fに
	invView.m[3][0] = 0.0f;
	invView.m[3][1] = 0.0f;
	invView.m[3][2] = 0.0f;

	return invView;
}

nlohmann::json Camera::SaveToJson() const
{
	return nlohmann::json {
		{"transform", TransformToJson(transform_) },
	};
}

void Camera::LoadFromJson(const nlohmann::json& j)
{
	if (j.contains("transform")) {
		TransformFromJson(j.at("transform"), transform_);
	}
}

nlohmann::json Camera::SaveCamera()
{
	return nlohmann::json{
		{"transform", TransformToJson(transform_) },
		{"startPos", ToJson(startPos_)},
		{"startRot", ToJson(startRot_)},
		{"targetPos", ToJson(targetPos_)},
		{"targetRot", ToJson(targetRot_)},
		{"smoothSpeedPos", ToJson(smoothSpeedPos_)},
		{"smoothSpeedRot", ToJson(smoothSpeedRot_)},
		{"duration", ToJson(duration_)}
	};
}

void Camera::LoadCamera(const nlohmann::json& j)
{
	// トランフォーム
	if (j.contains("transform")) {
		TransformFromJson(j.at("transform"), transform_);
	}

	// 座標補間の始点
	if (j.contains("startPos")) {
		FromJson(j.at("startPos"), startPos_);
	}

	// 角度補完の始点
	if (j.contains("startRot")) {
		FromJson(j.at("startRot"), startRot_);
	}
	// 座標補間の終点
	if (j.contains("targetPos")) {
		FromJson(j.at("targetPos"), targetPos_);
	}

	// 角度補完の終点
	if (j.contains("targetRot")) {
		FromJson(j.at("targetRot"), targetRot_);
	}

	// 座標補間の速度
	if (j.contains("smoothSpeedPos")) {
		FromJson(j.at("smoothSpeedPos"), smoothSpeedPos_);
	}

	// 角度補完の速度
	if (j.contains("smoothSpeedRot")) {
		FromJson(j.at("smoothSpeedRot"), smoothSpeedRot_);
	}

	// 補間秒数
	if (j.contains("duration")) {
		FromJson(j.at("duration"), duration_);
	}
}

void Camera::UpdateLerpCamera()
{
	if (isLerp_) {
		elapsed_ += Time::GetDeltaTime();

		const float t = std::clamp(elapsed_ / duration_, 0.0f, 1.0f);
		const float eased = t * t * (3.0f - 2.0f * t);

		transform_.translate = Lerp(startPos_, targetPos_, eased);
		transform_.rotate = Lerp(startRot_, targetRot_, eased);

		if (t >= 1.0f) {
			transform_.translate = targetPos_;
			transform_.rotate = targetRot_;
			isLerp_ = false;
			isLerpEnd_ = true;
		}
	}
}

void Camera::StartLerp()
{
	transform_.translate = startPos_;
	transform_.rotate = startRot_;

	elapsed_ = 0.0f;
	duration_ = std::max(duration_, 0.01f);
	isLerp_ = true;
	isLerpEnd_ = false;
}
