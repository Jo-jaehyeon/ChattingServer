#pragma once

class Player
{
public:
	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::PlayerType::PLAYER_TYPE_NONE;
	GameSessionRef			OwnerSession;
};

