#pragma once
#include "SceneIncludes.h"

class Enemy;
class Reticle
{
public:
	void Init();

	void Update(const Vector3& playerPosition, std::vector<std::unique_ptr<Enemy>>& enemies);

	void Draw();

	Vector3 GetAimTarget() const { return aimTarget_; }

	Vector2 GetScreenPosition() const { return screenPosition_; }

private:
	void UpdateInput();
	void UpdateScreenPosition(const Vector3& playerPos, const Camera& camera);
	Vector3 CulculateAimTarget(const Vector3& playerPos, const Camera& camera, std::vector<std::unique_ptr<Enemy>>& enemies);

	std::unique_ptr<Sprite> sprite_;

	Vector2 screenPosition_{};
	Vector3 aimTarget_{};

	Vector2 aimOffset_{};
	Vector2 moveAimOffset_{};

	static constexpr float deadZone_ = 0.15f;
	static constexpr float aimSpeed_ = 12.0f;
	static constexpr float moveAimDistance_ = 20.0f;
	static constexpr float aimReturnSpeed_ = 0.12f;
	static constexpr float aimDistance_ = 50.0f;
	static constexpr float hitRadius_ = 45.0f;
	static constexpr float screenMargin_ = 32.0f;
};