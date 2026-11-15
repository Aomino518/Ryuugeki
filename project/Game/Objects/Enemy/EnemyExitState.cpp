#include "EnemyExitState.h"
#include "Enemy.h"
#include "GameTime.h"
#include <algorithm>

void EnemyExitState::Enter(Enemy& enemy)
{
    start_ = enemy.GetStatePosition();
    target_ = start_ + Vector3{ 0.0f, 20.0f, 0.0f };
    elapsed_ = 0.0f;
}

void EnemyExitState::Update(Enemy& enemy)
{
    elapsed_ += Time::GetDeltaTime();
    const float t = std::clamp(elapsed_ / kDuration, 0.0f, 1.0f);
    const float rate = t * t * t;
    enemy.SetStatePosition(start_ + (target_ - start_) * rate);

    if (t >= 1.0f) {
        enemy.FinishExit();
    }
}
