#include "AudioManager.h"

#include <cassert>
#include <fstream>

#pragma comment(lib,"xaudio2.lib")

namespace Engine {

namespace {

	/// <summary>
	/// wavファイルのチャンクヘッダ（ID・サイズ）
	/// </summary>
	struct ChunkHeader {
		char id[4]; // チャンク毎のID
		int32_t size;  // チャンクサイズ
	};

	/// <summary>
	/// wavファイルのRIFFヘッダチャンク
	/// </summary>
	struct RiffHeader {
		ChunkHeader chunk;  // "RIFF"
		char type[4]; // "WAVE"
	};

	/// <summary>
	/// wavファイルのフォーマットチャンク（波形フォーマット情報）
	/// </summary>
	struct FormatChunk {
		ChunkHeader chunk; // "fmt "
		WAVEFORMATEX fmt; // 波形フォーマット
	};

	// wavファイルの各チャンクを識別するID文字列
	constexpr char kChunkIdRiff[] = "RIFF";
	constexpr char kChunkIdWave[] = "WAVE";
	constexpr char kChunkIdFmt[]  = "fmt ";
	constexpr char kChunkIdData[] = "data";
	constexpr char kChunkIdJunk[] = "JUNK";

	/// <summary>
	/// RIFFヘッダーを読み込み、"RIFF"/"WAVE"であることを確認する
	/// </summary>
	RiffHeader ReadRiffHeader(std::ifstream& file) {
		RiffHeader riff;
		file.read((char*)&riff, sizeof(riff));

		// ファイルがRIFFかチェック
		assert(strncmp(riff.chunk.id, kChunkIdRiff, sizeof(kChunkIdRiff) - 1) == 0);

		// タイプがWAVEかチェック
		assert(strncmp(riff.type, kChunkIdWave, sizeof(kChunkIdWave) - 1) == 0);

		return riff;
	}

	/// <summary>
	/// Formatチャンクを読み込み、"fmt "であることを確認する
	/// </summary>
	FormatChunk ReadFormatChunk(std::ifstream& file) {
		FormatChunk format = {};

		// チャンクヘッダーの確認
		file.read((char*)&format, sizeof(ChunkHeader));
		assert(strncmp(format.chunk.id, kChunkIdFmt, sizeof(kChunkIdFmt) - 1) == 0);

		// チャンク本体の読み込み
		assert(format.chunk.size <= sizeof(format.fmt));
		file.read((char*)&format.fmt, format.chunk.size);

		return format;
	}

	/// <summary>
	/// Dataチャンクのヘッダーを読み込む（JUNKチャンクが挟まっている場合は読み飛ばす）
	/// </summary>
	ChunkHeader ReadDataChunkHeader(std::ifstream& file) {
		ChunkHeader data;
		file.read((char*)&data, sizeof(data));

		// JUNKチャンクを検出した場合
		if (strncmp(data.id, kChunkIdJunk, sizeof(kChunkIdJunk) - 1) == 0) {
			// 読み取り位置をJUNKチャンクの終わりまで進める
			file.seekg(data.size, std::ios_base::cur);

			// 再読み込み
			file.read((char*)&data, sizeof(data));
		}

		assert(strncmp(data.id, kChunkIdData, sizeof(kChunkIdData) - 1) == 0);

		return data;
	}

	/// <summary>
	/// Dataチャンクのデータ部（波形データ本体）を読み込む
	/// </summary>
	std::vector<BYTE> ReadWaveBuffer(std::ifstream& file, int32_t size) {
		std::vector<BYTE> buffer(size);
		file.read(reinterpret_cast<char*>(buffer.data()), size);
		return buffer;
	}

} // namespace

AudioManager* AudioManager::GetInstance() {
	static AudioManager instance;
	return &instance;
}

void AudioManager::Initialize() {

	HRESULT result;

	// XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスを生成
	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result));
}

void AudioManager::Finalize() {
	// 再生中・再生済みのSourceVoiceを全て破棄してからエンジンを解放する
	for (IXAudio2SourceVoice* voice : activeVoices_) {
		voice->Stop();
		voice->DestroyVoice();
	}
	activeVoices_.clear();

	xAudio2_.Reset();
	masterVoice_ = nullptr;
}

SoundData AudioManager::LoadWave(const std::string& filePath) {

	// .wavファイルをバイナリモードで開く
	std::ifstream file(filePath, std::ios_base::binary);
	assert(file.is_open());

	// RIFF → Format → Data の順にチャンクを読み進める
	ReadRiffHeader(file);
	FormatChunk format = ReadFormatChunk(file);
	ChunkHeader data = ReadDataChunkHeader(file);
	std::vector<BYTE> buffer = ReadWaveBuffer(file, data.size);

	file.close();

	// returnする為の音声データ
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.buffer = std::move(buffer);

	return soundData;
}

void AudioManager::Unload(SoundData* soundData) {
	// バッファのメモリを解放
	soundData->buffer.clear();
	soundData->buffer.shrink_to_fit();
	soundData->wfex = {};
}

void AudioManager::ReapFinishedVoices() {
	for (auto it = activeVoices_.begin(); it != activeVoices_.end();) {
		XAUDIO2_VOICE_STATE state{};
		(*it)->GetState(&state);

		// 再生キューが空になっていれば再生完了とみなして破棄する
		if (state.BuffersQueued == 0) {
			(*it)->DestroyVoice();
			it = activeVoices_.erase(it);
		} else {
			++it;
		}
	}
}

void AudioManager::Play(const SoundData& soundData) {

	HRESULT result;

	// 再生が終わった過去のSourceVoiceを破棄してからリソースを積み増す
	ReapFinishedVoices();

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

	// 再生完了後に破棄できるよう保持しておく
	activeVoices_.push_back(pSourceVoice);
}

} // namespace Engine
