#pragma once
#include "IPlayerState.h"

class PlayerAliveState : public IPlayerState
{
public:
	void Update(Player& player) override;
	void OnDamage(Player& player) override;
};

