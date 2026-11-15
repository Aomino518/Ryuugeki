#include "EnemyActiveState.h"
#include "Enemy.h"
#include "EnemyExitState.h"
#include "GameTime.h"

void EnemyActiveState::Enter(Enemy&)
{
    elapsed_ = 0.0f;
}

void EnemyActiveState::Update(Enemy& enemy)
{
    elapsed_ += Time::GetDeltaTime();
    if (elapsed_ >= kDuration) {
        enemy.RequestStateChange(std::make_unique<EnemyExitState>());
        return;
    }

    enemy.UpdateActiveMovement();
}
