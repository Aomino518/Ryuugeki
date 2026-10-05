#pragma once
#include <cstdint>
#include <memory>
#include "Sprite.h"
#include "Vector2.h"
#include "Vector4.h"

class HpBar
{
public:
	void Init(const Vector2& position, const Vector2& size, const Vector4& color);
	void Update(int currentHp, int maxHp);
	void Draw();
	void SetVisible(bool visible);

private:
	uint32_t texId_;
	std::unique_ptr<Sprite> background_;
	std::unique_ptr<Sprite> fill_;
	Vector2 size_{};
	bool isVisible_ = false;
	float ratio_ = 1.0f;
};

