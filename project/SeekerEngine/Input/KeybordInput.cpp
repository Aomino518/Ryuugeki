#include "KeybordInput.h"
#include "Input.h"

Vector2 KeybordInput::GetMoveDirection()
{
	Vector2 direction{};

	if (Input::GetInstance()->IsPress(DIK_W)) {
		direction.y += 1.0f;
	}

	if (Input::GetInstance()->IsPress(DIK_S)) {
		direction.y -= 1.0f;
	}

	if (Input::GetInstance()->IsPress(DIK_A)) {
		direction.x -= 1.0f;
	}

	if (Input::GetInstance()->IsPress(DIK_D)) {
		direction.x += 1.0f;
	}

	return direction;
}
