#pragma once

class Enemy;
class IEnemyState
{
public:
	virtual ~IEnemyState() = default;

	virtual void Enter(Enemy&) {}
	virtual void Update(Enemy& enemy) = 0;
	virtual void Exit(Enemy&) {}
};
