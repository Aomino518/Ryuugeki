#pragma once
#include "IInputStrategy.h"
#include <Vector2.h>

class KeybordInput : public IInputStrategy
{
public:
	Vector2 GetMoveDirection() override;
};

