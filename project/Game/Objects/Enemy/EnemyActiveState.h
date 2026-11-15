#pragma once
#include "IEnemyState.h"

// 入場と退場をつなぐ活動状態。直進・停止の計算はEnemyへ委譲する。
class EnemyActiveState : public IEnemyState
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy) override;

private:
    float elapsed_ = 0.0f;
    static constexpr float kDuration = 3.0f;
};
