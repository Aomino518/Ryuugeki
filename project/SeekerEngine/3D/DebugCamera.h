#pragma once
#include "MathFunc.h"
#include "Application.h"
#include <windows.h>

class DebugCamera
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();
  
	const Matrix4x4& GetViewMatrix() const{ return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const{ return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	const Vector3& GetTranslate() const { return translate_; }
	const Matrix4x4& GetMatRot() const { return matRot_; }
	const bool IsRightDrag() const { return isRightDrag_; }
	Matrix4x4 GetBillboardMatrix();

	void SetTranslate(Vector3& translate) { translate_ = translate; }
	void SetMatRot(Matrix4x4& matRot) { matRot_ = matRot; }
	void SetIsRightDrag(bool enable) { isRightDrag_ = enable; }

private:
	// 累積回転行列
	Matrix4x4 matRot_;
	// ローカル座標
	Vector3 translate_ = { 0.0f, 0.0f, -50.0f };
	// ビュー行列
	Matrix4x4 viewMatrix_;
	// 射影行列
	Matrix4x4 projectionMatrix_;

	Matrix4x4 viewProjectionMatrix_;

	float moveSpeed_ = 0.5f;
	float rotateSpeed_ = 0.005f;

	POINT preMousePos_ = {};
	bool isRightDrag_ = false;

};

