#include "PlayerAliveState.h"
#include "PlayerDeadState.h"
#include "Player.h"

void PlayerAliveState::Update(Player& player)
{
	player.UpdateAlive();
}

void PlayerAliveState::OnDamage(Player& player)
{
    if (!player.ApplyDamage()) {
        return;
    }

    if (player.GetHp() <= 0) {
        player.RequestStateChange(std::make_unique<PlayerDeadState>());
    }
}
