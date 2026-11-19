#pragma once
#include <Vector2.h>

// 入力の共通インターフェース
class IInputStrategy {
public:
	virtual ~IInputStrategy() = default;

	virtual Vector2 GetMoveDirection() = 0;
};