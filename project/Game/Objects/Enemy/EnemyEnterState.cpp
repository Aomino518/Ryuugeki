#include "EnemyEnterState.h"
#include "Enemy.h"
#include "EnemyActiveState.h"
#include "GameTime.h"
#include <algorithm>
#include <cmath>

void EnemyEnterState::Enter(Enemy& enemy)
{
    target_ = enemy.GetStatePosition();
    start_ = target_ + Vector3{ 0.0f, 50.0f, 0.0f };
    elapsed_ = 0.0f;
    enemy.SetStatePosition(start_);
}

void EnemyEnterState::Update(Enemy& enemy)
{
    elapsed_ += Time::GetDeltaTime();
    const float t = std::clamp(elapsed_ / kDuration, 0.0f, 1.0f);
    const float rate = 1.0f - std::pow(1.0f - t, 3.0f);
    enemy.SetStatePosition(start_ + (target_ - start_) * rate);

    if (t >= 1.0f) {
        enemy.RequestStateChange(std::make_unique<EnemyActiveState>());
    }
}
