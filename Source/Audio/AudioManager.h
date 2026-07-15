#pragma once
#include <wrl/client.h>
#include <xaudio2.h>
#include <string>
#include <vector>

namespace Engine {

/// <summary>
/// 読み込んだ音声データ（波形フォーマットとバッファ）
/// </summary>
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// 波形データ本体（std::vectorが解放を管理するため手動delete不要）
	std::vector<BYTE> buffer;
};

/// <summary>
/// XAudio2を使った音声の読み込み・再生・解放を管理するクラス
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
	/// 終了処理を行う
	/// </summary>
	void Finalize();

	/// <summary>
	/// wavファイルを読み込み、音声データとして返す
	/// </summary>
	/// <param name="filePath">読み込むwavファイルのパス</param>
	/// <returns>読み込んだ音声データ</returns>
	[[nodiscard]] SoundData LoadWave(const std::string& filePath);

	/// <summary>
	/// 読み込んだ音声データを解放する
	/// </summary>
	/// <param name="soundData">解放する音声データ</param>
	void Unload(SoundData* soundData);

	/// <summary>
	/// 音声データを再生する
	/// </summary>
	/// <param name="soundData">再生する音声データ</param>
	void Play(const SoundData& soundData);

private:
	AudioManager() = default;
	~AudioManager() = default;

	/// <summary>
	/// 再生が完了したSourceVoiceをactiveVoices_から破棄・除去する
	/// </summary>
	void ReapFinishedVoices();

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;
	// Play()で生成中のSourceVoice一覧（再生完了後にDestroyVoiceするため保持）
	std::vector<IXAudio2SourceVoice*> activeVoices_;
};

} // namespace Engine
