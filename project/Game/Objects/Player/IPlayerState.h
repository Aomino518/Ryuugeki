#pragma once

class Player;
class IPlayerState
{
public:
	virtual ~IPlayerState() = default;

	virtual void Enter(Player&) {}
	virtual void Update(Player& player) = 0;
	virtual void Exit(Player&) {}
	virtual void OnDamage(Player&) {}
};

