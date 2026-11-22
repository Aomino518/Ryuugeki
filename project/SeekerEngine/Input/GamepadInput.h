#pragma once
#include "IInputStrategy.h"
#include "Vector2.h"

class GamepadInput : public IInputStrategy
{
public:
	Vector2 GetMoveDirection() override;

private:
	static constexpr float deadZone_ = 0.15f;
};

