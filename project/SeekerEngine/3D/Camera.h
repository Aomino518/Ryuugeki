#pragma once
#include "CreateResorceUtils.h"
#include <numbers>
#include <nlohmann/json.hpp>

class Camera
{
public:
	Camera();
	void Update();

	// Getter
	const Transform& GetTransform() const { return transform_; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetScale() const { return transform_.scale; }
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	Matrix4x4 GetBillboardMatrix();
	const Transform& GetTransform() { return transform_; }
	const Vector3& GetTergetPos() { return targetPos_; }
	const Vector3& GetTergetRot() { return targetRot_; }
	const Vector3& GetStartPos() { return startPos_; }
	const Vector3& GetStartRot() { return startRot_; }
	const float& GetSmoothSpeedPos() const { return smoothSpeedPos_; }
	const float& GetSmoothSpeedRot() const { return smoothSpeedRot_; }
	const bool GetIsLerp() const { return isLerp_; }
	const float& GetDuration() { return duration_; }
	const bool GetIsLerpEnd() const { return isLerpEnd_; }

	// Setter
	void SetTranslate(const Vector3& translate) { this->transform_.translate = translate; }
	void SetRotate(const Vector3& rotate) { this->transform_.rotate = rotate; }
	void SetScale(const Vector3& scale) { this->transform_.scale = scale; }
	void SetFovY(float fovY) { this->fovY_ = fovY; }
	void SetAspectRatio(float aspectRatio) { this->aspectRatio_ = aspectRatio; }
	void SetNearClip(float nearClip) { this->nearClip_ = nearClip; }
	void SetFarClip(float farClip) { this->farClip_ = farClip; }
	void SetTransform(const Transform& transform) { this->transform_ = transform; }
	void SetTergetPos(const Vector3& tergetPos) { this->targetPos_ = tergetPos; }
	void SetTergetRot(const Vector3& tergetRot) { this->targetRot_ = tergetRot; }
	void SetStartPos(const Vector3& startPos) { this->startPos_ = startPos; }
	void SetStartRot(const Vector3& startRot) { this->startRot_ = startRot; }
	void SetSmoothSpeedPos(const float& speed) { this->smoothSpeedPos_ = speed; }
	void SetSmoothSpeedRot(const float& speed) { this->smoothSpeedRot_ = speed; }
	void SetIsLerp(bool enabled) { this->isLerp_ = enabled; }
	void SetDuration(float duration) { this->duration_ = duration; }

	// json保存と読み込み
	nlohmann::json SaveToJson() const;
	void LoadFromJson(const nlohmann::json& j);
	nlohmann::json SaveCamera();
	void LoadCamera(const nlohmann::json& j);

	void UpdateLerpCamera();
	void StartLerp();
private:

	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	float fovY_;
	float aspectRatio_;
	float nearClip_;
	float farClip_;
	
	bool isLerp_ = false;
	bool isLerpEnd_ = false;
	float smoothSpeedPos_ = 1.0f;
	float smoothSpeedRot_ = 1.0f;
	Vector3 startPos_ = { 0.0f, 0.0f, 0.0f };
	Vector3 startRot_ = { 0.0f, 0.0f, 0.0f };
	Vector3 targetPos_ = {0.0f, 0.0f, 0.0f};
	Vector3 targetRot_ = {0.0f, 0.0f, 0.0f};
	float elapsed_ = 0.0f;
	float duration_ = 3.0f;

};

