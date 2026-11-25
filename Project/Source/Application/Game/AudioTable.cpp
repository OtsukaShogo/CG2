#include "AudioTable.h"
#include <algorithm>

namespace Shooter {

namespace {

// 仮置き: すべての効果音は shot.mp3、すべての BGM は game.mp3。本物の音ができたら、パスだけを書き換える。
// 音量は、同じ音でも、種類が聞き分けられるよう、少しずつ変えてある
constexpr const char* kPlaceholderSe = "Resource/Audio/SE/shot.mp3";
constexpr const char* kPlaceholderBgm = "Resource/Audio/BGM/game.mp3";

struct SeEntry {
	const char* path;
	float volume;
	float minInterval;
};

constexpr SeEntry kSeTable[static_cast<size_t>(SeId::kCount)] = {
	{ "", 0.0f, 0.0f },                   // kNone
	{ kPlaceholderSe, 0.30f, 0.06f },     // kShot（連射されるので、小さめ・間引く）
	{ kPlaceholderSe, 0.80f, 0.10f },     // kPlayerHit
	{ kPlaceholderSe, 0.40f, 0.05f },     // kEnemyHit
	{ kPlaceholderSe, 0.65f, 0.08f },     // kEnemyKill
	{ kPlaceholderSe, 0.60f, 0.10f },     // kConfirm
};

struct BgmEntry {
	const char* path;
	float volume;
};

constexpr BgmEntry kBgmTable[static_cast<size_t>(BgmId::kCount)] = {
	{ kPlaceholderBgm, 0.35f },           // kTitle
	{ kPlaceholderBgm, 0.35f },           // kGame
	{ kPlaceholderBgm, 0.35f },           // kResult
};

const SeEntry& SeEntryOf(SeId id) {
	const size_t index = static_cast<size_t>(id);
	return kSeTable[index < static_cast<size_t>(SeId::kCount) ? index : 0];
}

} // namespace

const char* GetSePath(SeId id) { return SeEntryOf(id).path; }
float GetSeVolume(SeId id) { return SeEntryOf(id).volume; }
float GetSeMinInterval(SeId id) { return SeEntryOf(id).minInterval; }

const char* GetBgmPath(BgmId id) {
	const size_t index = static_cast<size_t>(id);
	return index < static_cast<size_t>(BgmId::kCount) ? kBgmTable[index].path : "";
}

float GetBgmVolume(BgmId id) {
	const size_t index = static_cast<size_t>(id);
	return index < static_cast<size_t>(BgmId::kCount) ? kBgmTable[index].volume : 0.0f;
}

void SoundThrottle::Update(float deltaTime) {
	for (float& remaining : cooldown_) {
		remaining = (std::max)(0.0f, remaining - deltaTime);
	}
}

bool SoundThrottle::TryPlay(SeId id) {
	const size_t index = static_cast<size_t>(id);
	if (id == SeId::kNone || index >= cooldown_.size()) {
		return false;
	}
	if (cooldown_[index] > 0.0f) {
		return false;
	}
	cooldown_[index] = GetSeMinInterval(id);
	return true;
}

void SoundThrottle::Reset() {
	cooldown_.fill(0.0f);
}

BgmTransition DecideBgmTransition(const std::string& currentPath, bool currentIsPlaying, const std::string& nextPath) {
	if (currentIsPlaying && !currentPath.empty() && currentPath == nextPath) {
		return BgmTransition::kKeep;
	}
	return BgmTransition::kSwitch;
}

} // namespace Shooter
