#pragma once
#include "IEnemyState.h"
#include "Vector3.h"

// 上から出現して、指定されたスポーン位置まで入場する。
class EnemyEnterState : public IEnemyState
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy) override;

private:
    Vector3 start_{};
    Vector3 target_{};
    float elapsed_ = 0.0f;
    static constexpr float kDuration = 1.0f;
};
