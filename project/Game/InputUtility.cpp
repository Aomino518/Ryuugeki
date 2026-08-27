#include "InputUtility.h"
#include <cmath>

Vector2 InputUtility::ApplyDeadZone(const Vector2& input, float deadZone)
{
	Vector2 result = input;

	// デッドゾーン処理
	if (std::fabs(result.x) < deadZone) {
		result.x = 0.0f;
	}

	if (std::fabs(result.y) < deadZone) {
		result.y = 0.0f;
	}

	return result;
}
