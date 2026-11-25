#include "GameAudio.h"

using namespace Engine;

namespace Shooter {

GameAudio& GameAudio::Get() {
	static GameAudio instance;
	return instance;
}

Sound GameAudio::GetSound(const std::string& path) {
	auto it = sounds_.find(path);
	if (it != sounds_.end()) {
		return it->second;
	}
	Sound sound = audio_ ? audio_->Load(path) : nullptr;
	sounds_[path] = sound; // 失敗（nullptr）も覚えておく（毎回、読み込みを試さないように）
	return sound;
}

void GameAudio::Initialize(AudioManager* audio) {
	audio_ = audio;
	sounds_.clear();
	throttle_.Reset();
	bgmVoice_ = kInvalidVoiceId;
	bgmPath_.clear();

	// 表にあるすべてのファイルを読み込む
	for (int i = static_cast<int>(SeId::kShot); i < static_cast<int>(SeId::kCount); ++i) {
		(void)GetSound(GetSePath(static_cast<SeId>(i)));
	}
	for (int i = 0; i < static_cast<int>(BgmId::kCount); ++i) {
		(void)GetSound(GetBgmPath(static_cast<BgmId>(i)));
	}
}

void GameAudio::Shutdown() {
	StopBgm();
	sounds_.clear();
	audio_ = nullptr;
}

void GameAudio::Update(float deltaTime) {
	throttle_.Update(deltaTime);
}

void GameAudio::PlaySe(SeId id) {
	if (!audio_ || !throttle_.TryPlay(id)) {
		return;
	}
	const Sound sound = GetSound(GetSePath(id));
	if (!sound) {
		return;
	}
	PlayParams params;
	params.volume = GetSeVolume(id);
	(void)audio_->Play(sound, params);
}

void GameAudio::PlayBgm(BgmId id) {
	if (!audio_) {
		return;
	}
	const std::string path = GetBgmPath(id);
	const bool playing = audio_->IsPlaying(bgmVoice_);

	if (DecideBgmTransition(bgmPath_, playing, path) == BgmTransition::kKeep) {
		audio_->SetVolume(bgmVoice_, GetBgmVolume(id)); // 曲は続けたまま、音量だけ、新しい指定に合わせる
		return;
	}

	StopBgm();
	const Sound sound = GetSound(path);
	if (!sound) {
		return;
	}
	PlayParams params;
	params.volume = GetBgmVolume(id);
	params.loop = true;
	bgmVoice_ = audio_->Play(sound, params);
	bgmPath_ = path;
}

void GameAudio::StopBgm() {
	if (audio_ && bgmVoice_ != kInvalidVoiceId) {
		audio_->Stop(bgmVoice_);
	}
	bgmVoice_ = kInvalidVoiceId;
	bgmPath_.clear();
}

} // namespace Shooter
