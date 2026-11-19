#include "GamepadInput.h"
#include "Input.h"
#include "Game/Utilities/InputUtility.h"

Vector2 GamepadInput::GetMoveDirection()
{
    const Vector2 direction = Input::GetInstance()->GetXbLeftStickVector();
    return InputUtility::ApplyDeadZone(direction, deadZone_);
}
