#pragma once
#include <wrl/client.h>
#include <xaudio2.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Engine {

/// <summary>
/// 読み込んだ音声データ（波形フォーマットとバッファ）。読み込み後は変更されない
/// </summary>
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex = {};
	// 波形データ本体（std::vectorが解放を管理するため手動delete不要）
	std::vector<BYTE> buffer;
};

/// <summary>
/// 音声データへの共有ハンドル。再生中のボイスもコピーを保持するため、
/// 持ち主（シーンなど）が先に手放しても、再生が終わるまでバッファは解放されない。
/// 読み込みに失敗した場合は nullptr になる
/// </summary>
using Sound = std::shared_ptr<const SoundData>;

/// <summary>
/// 再生中の音声を指す識別子。0 は無効。再生が終わったあとに使っても、何も起きずに安全に無視される
/// </summary>
using VoiceId = uint32_t;
constexpr VoiceId kInvalidVoiceId = 0;

/// <summary>
/// 再生時のオプション
/// </summary>
struct PlayParams {
	float volume = 1.0f; // 音量（1.0で原音、0で無音）
	bool loop = false;   // trueで Stop() されるまでループ再生する
};

/// <summary>
/// XAudio2を使った音声の読み込み・再生・停止を管理するクラス
/// </summary>
class AudioManager {
public:
	/// <summary>
	/// シングルトンインスタンスを取得する
	/// </summary>
	/// <returns>AudioManagerのインスタンス</returns>
	[[nodiscard]] static AudioManager* GetInstance();

	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	/// <summary>
	/// XAudio2エンジンを初期化する
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了処理を行う（再生中の音声はすべて停止する）
	/// </summary>
	void Finalize();

	/// <summary>
	/// 毎フレーム呼び、再生が終わったボイスを破棄する
	/// </summary>
	void Update();

	/// <summary>
	/// wavファイルを読み込み、音声データとして返す。
	/// ファイルが存在しない・対応していない形式の場合はログを出して nullptr を返す。
	/// 解放は、返されたハンドルをすべて手放したときに自動で行われる（再生中のものは再生終了まで残る）
	/// </summary>
	/// <param name="filePath">読み込むwavファイルのパス（UTF-8）</param>
	/// <returns>読み込んだ音声データ</returns>
	[[nodiscard]] Sound LoadWave(const std::string& filePath);

	/// <summary>
	/// 音声データを再生する。sound が nullptr の場合は何もしない
	/// </summary>
	/// <param name="sound">再生する音声データ</param>
	/// <param name="params">音量・ループなどの再生オプション</param>
	/// <returns>再生を操作するための識別子（再生できなかった場合は kInvalidVoiceId）</returns>
	VoiceId Play(const Sound& sound, const PlayParams& params = {});

	/// <summary>
	/// 指定の再生を停止する。すでに終了している・無効な識別子の場合は何もしない
	/// </summary>
	void Stop(VoiceId id);

	/// <summary>
	/// 再生中の音声をすべて停止する（シーン切り替え時などに使う）
	/// </summary>
	void StopAll();

	/// <summary>
	/// 指定の再生の音量を変更する
	/// </summary>
	/// <param name="id">Play() が返した識別子</param>
	/// <param name="volume">音量（1.0で原音、0で無音）</param>
	void SetVolume(VoiceId id, float volume);

	/// <summary>
	/// 指定の音声がまだ再生中かを取得する
	/// </summary>
	[[nodiscard]] bool IsPlaying(VoiceId id) const;

	/// <summary>
	/// 全体の音量を変更する
	/// </summary>
	/// <param name="volume">音量（1.0で原音、0で無音）</param>
	void SetMasterVolume(float volume);

private:
	AudioManager() = default;
	~AudioManager() = default;

	// 再生中のボイス1つ分の情報
	struct ActiveVoice {
		VoiceId id = kInvalidVoiceId;
		IXAudio2SourceVoice* voice = nullptr;
		Sound sound; // 再生中は音声データを生かしておく（バッファをXAudio2が直接読むため）
	};

	/// <summary>
	/// 再生が完了したSourceVoiceを破棄・除去する
	/// </summary>
	void ReapFinishedVoices();

	/// <summary>
	/// 識別子に対応するボイスを探す（見つからなければ nullptr）
	/// </summary>
	[[nodiscard]] const ActiveVoice* Find(VoiceId id) const;

	/// <summary>
	/// ボイスを停止して破棄する
	/// </summary>
	static void DestroyVoice(ActiveVoice& active);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;
	std::vector<ActiveVoice> activeVoices_;
	VoiceId nextVoiceId_ = 1;
};

} // namespace Engine
