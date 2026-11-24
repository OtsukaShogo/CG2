#include "GameSession.h"

namespace Shooter {

GameSession& GameSession::Get() {
	static GameSession instance;
	return instance;
}

void GameSession::BeginPlay() {
	last_ = GameResult{};
	lastWasNewBest_ = false;
}

bool GameSession::Record(const GameResult& result) {
	last_ = result;
	const bool better = !hasBest_
		|| result.kills > best_.kills
		|| (result.kills == best_.kills && result.survivalTime > best_.survivalTime);
	lastWasNewBest_ = better;
	if (better) {
		best_ = result;
		hasBest_ = true;
	}
	return better;
}

void GameSession::Clear() {
	last_ = GameResult{};
	best_ = GameResult{};
	hasBest_ = false;
	lastWasNewBest_ = false;
}

} // namespace Shooter
