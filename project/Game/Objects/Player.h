#pragma once
#include "SeekerEngine.h"
#include "Entity3D.h"
#include "Weapon.h"
#include "DethParticle.h"

class Player
{
public:
	/// <summary>
	/// 初期化処理関数
	/// </summary>
	/// <param name="position">初期位置</param>
	void Init(const Vector3& position);

	void Update();

	void Draw();

	void DebugDraw();

	void SetIsDebugHit();

	// Getter
	Vector3 GetPosition() const { return modelPlayer_->GetTranslate(); }
	Vector3 GetRotate() const { return rot_; }
	bool GetIsAlive() const { return isAlive_; }
	Entity3D* GetModel() const { return modelPlayer_.get(); }
	std::vector<std::unique_ptr<Bullet>>& GetBullets() { return weapon_.GetBullets(); }
	Sphere GetSphere() const { return sphere_; }
	bool GetIsConroller() const { return isController_; }

	// Setter
	void SetPosition(const Vector3& pos) { modelPlayer_->SetTranslate(pos); }
	void SetIsAlive(bool isAlive) { isAlive_ = isAlive; }
	void SetAimTarget(const Vector3& target) { aimTarget_ = target; }

	void Damage();
private:

	// メンバ関数
	/// <summary>
	/// 移動入力値を返す関数
	/// </summary>
	/// <returns>入力値</returns>
	Vector2 GetMovementInput();

	/// <summary>
	/// 移動の更新処理
	/// </summary>
	void UpdateMovement();
	/// <summary>
	/// 回転の更新処理
	/// </summary>
	/// <param name="input">入力値</param>
	void UpdateRotation(const Vector2& input);
	/// <summary>
	/// 速度の更新処理
	/// </summary>
	/// <param name="input">入力値</param>
	void UpdateVelocity(const Vector2& input);
	/// <summary>
	/// 位置の更新処理
	/// </summary>
	void UpdatePosition();
	/// <summary>
	/// 射撃の更新処理
	/// </summary>
	void UpdateShooting();
	/// <summary>
	/// カメラの更新処理
	/// </summary>
	void UpdateCamera();
	
	// モデル
	std::unique_ptr<Entity3D> modelPlayer_;

	// メンバ変数
	// 速度
	Vector2 velocity_ = { 0.0f, 0.0f };
	// 加速度
	float acceleration = 0.05f;
	float deceleration = 0.05f;
	// 限界速度
	float maxSpeed = 0.5f;
	// 生存フラグ
	bool isAlive_ = true;
	// 入力方向
	Vector3 rot_ = { 0.0f, 0.0f, 0.0f };
	// 当たり判定
	Sphere sphere_;

	Weapon weapon_;
	DethParticle dethParticle_;

	Vector3 aimTarget_{};

	bool isController_ = false;

	bool debugIsHit_ = false;
	int debugHitTimer_ = 0;
	static constexpr float deadZone_ = 0.15f;
	static constexpr float maxTiltAngle_ = 0.3f;
	static constexpr float interpolationSpeed_ = 5.0f;
};

