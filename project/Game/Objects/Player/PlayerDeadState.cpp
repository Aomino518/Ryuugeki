#include "PlayerDeadState.h"
#include "Player.h"

void PlayerDeadState::Enter(Player& player)
{
	player.BeginDeth();
}

void PlayerDeadState::Update(Player& player)
{
	player.UpdateDeth();
}
