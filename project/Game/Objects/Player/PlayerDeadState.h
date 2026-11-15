#pragma once
#include "IPlayerState.h"

class PlayerDeadState : public IPlayerState
{
public:
	void Enter(Player& player) override;
	void Update(Player& player) override;
};

