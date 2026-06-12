#pragma once
#include <wrl/client.h>
#include <xaudio2.h>
#include <string>

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};

class AudioManager {
public:
	static AudioManager* GetInstance();

	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	// XAudio2エンジン初期化
	void Initialize();

	// 終了処理
	void Finalize();

	// 音声データ読み込み
	SoundData LoadWave(const std::string& filePath);

	// 音声データ解放
	void Unload(SoundData* soundData);

	// 音声再生
	void Play(const SoundData& soundData);

private:
	AudioManager() = default;
	~AudioManager() = default;

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;
};
