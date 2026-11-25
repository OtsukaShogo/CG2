#pragma once
#include <string>
#include <unordered_map>
#include "AudioTable.h"
#include "../../Engine/Audio/AudioManager.h"

namespace Shooter {

/// <summary>
/// ゲームの効果音と BGM を鳴らす。シーンをまたいで残る（BGM が、シーンの切り替えで途切れないように）。
/// どのファイルを、どの音量で鳴らすかは、AudioTable の表で決まる。
/// 使い方: アプリの開始時に Initialize()、毎フレーム Update()、終了時に Shutdown()。
/// 鳴らすときは、PlaySe(SeId::kShot) や PlayBgm(BgmId::kGame) を呼ぶだけ
/// </summary>
class GameAudio {
public:
	[[nodiscard]] static GameAudio& Get();

	GameAudio(const GameAudio&) = delete;
	GameAudio& operator=(const GameAudio&) = delete;

	/// <summary>
	/// 表にあるすべての音のファイルを、あらかじめ読み込む（鳴らす瞬間に、読み込みで止まらないように）。
	/// 読み込めなかったファイルは、ログを出して、鳴らさないだけで、ゲームは続く
	/// </summary>
	void Initialize(Engine::AudioManager* audio);

	/// <summary>
	/// BGM を止めて、読み込んだ音を、すべて手放す（オーディオの終了より前に呼ぶ）
	/// </summary>
	void Shutdown();

	/// <summary>
	/// 時間を進める（鳴らす間隔の間引きに使う）。毎フレーム呼ぶ
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// 効果音を1回鳴らす。直前に鳴らした同じ音から、最小の間隔が空いていなければ、鳴らさない
	/// </summary>
	void PlaySe(SeId id);

	/// <summary>
	/// BGM を切り替える。すでに同じファイルが再生中なら、そのまま続ける（最初に戻さない）。
	/// 違うファイルなら、いまの曲を止めて、新しい曲を、最初からループ再生する
	/// </summary>
	void PlayBgm(BgmId id);

	/// <summary>
	/// BGM を止める
	/// </summary>
	void StopBgm();

private:
	GameAudio() = default;
	~GameAudio() = default;

	/// <summary>
	/// ファイルを読み込む（同じファイルは、1度だけ読んで、使い回す）
	/// </summary>
	[[nodiscard]] Engine::Sound GetSound(const std::string& path);

private:
	Engine::AudioManager* audio_ = nullptr;
	std::unordered_map<std::string, Engine::Sound> sounds_;
	SoundThrottle throttle_;
	Engine::VoiceId bgmVoice_ = Engine::kInvalidVoiceId;
	std::string bgmPath_;
};

} // namespace Shooter
