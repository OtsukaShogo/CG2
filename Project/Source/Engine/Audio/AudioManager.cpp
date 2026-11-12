#include "AudioManager.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "../Core/ConvertString.h"
#include "../Core/DebugUtil.h"

#pragma comment(lib,"xaudio2.lib")

namespace Engine {

namespace {

	/// <summary>
	/// wavファイルのチャンクヘッダ（ID・サイズ）
	/// </summary>
	struct ChunkHeader {
		char id[4];    // チャンク毎のID
		uint32_t size; // チャンクサイズ
	};

	bool IsChunk(const ChunkHeader& header, const char* id) {
		return std::memcmp(header.id, id, 4) == 0;
	}

	/// <summary>
	/// wavファイルを読み込む。RIFF/WAVEを確認し、"fmt "と"data"が見つかるまでチャンクを読み飛ばしながら進める
	/// （JUNK・LISTなどの余分なチャンクが挟まっていてもよい）。
	/// 失敗した場合は false を返し、理由を errorMessage に入れる
	/// </summary>
	bool ReadWaveFile(const std::string& filePath, SoundData& out, std::string& errorMessage) {
		std::ifstream file(std::filesystem::path(ConvertString(filePath)), std::ios_base::binary);
		if (!file.is_open()) {
			errorMessage = "ファイルを開けません";
			return false;
		}

		// RIFFヘッダー（"RIFF" + 全体サイズ + "WAVE"）
		ChunkHeader riff{};
		char waveType[4]{};
		file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
		file.read(waveType, sizeof(waveType));
		if (!file || !IsChunk(riff, "RIFF") || std::memcmp(waveType, "WAVE", 4) != 0) {
			errorMessage = "wavファイルではありません";
			return false;
		}

		bool hasFormat = false;
		ChunkHeader header{};
		while (file.read(reinterpret_cast<char*>(&header), sizeof(header))) {
			if (IsChunk(header, "fmt ")) {
				if (header.size < 16) {
					errorMessage = "fmtチャンクが不正です";
					return false;
				}
				// 拡張部分があっても、WAVEFORMATEXに収まる分だけ読んで残りは読み飛ばす
				uint32_t readSize = (std::min)(header.size, static_cast<uint32_t>(sizeof(WAVEFORMATEX)));
				out.wfex = {};
				file.read(reinterpret_cast<char*>(&out.wfex), readSize);
				file.seekg(static_cast<std::streamoff>(header.size - readSize + (header.size & 1)), std::ios_base::cur);
				if (!file) {
					errorMessage = "fmtチャンクを読み込めません";
					return false;
				}
				if (out.wfex.wFormatTag != WAVE_FORMAT_PCM && out.wfex.wFormatTag != WAVE_FORMAT_IEEE_FLOAT) {
					errorMessage = "非対応のwav形式です（PCM/Floatのみ対応）";
					return false;
				}
				hasFormat = true;
			} else if (IsChunk(header, "data")) {
				if (!hasFormat) {
					errorMessage = "fmtチャンクがdataより前にありません";
					return false;
				}
				out.buffer.resize(header.size);
				file.read(reinterpret_cast<char*>(out.buffer.data()), header.size);
				if (!file) {
					errorMessage = "波形データを読み込めません";
					return false;
				}
				return true;
			} else {
				// 不要なチャンクは読み飛ばす（チャンクは偶数バイト境界に揃えられる）
				file.seekg(static_cast<std::streamoff>(header.size + (header.size & 1)), std::ios_base::cur);
			}
		}

		errorMessage = "dataチャンクが見つかりません";
		return false;
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

	nextVoiceId_ = 1;
}

void AudioManager::Finalize() {
	// 再生中・再生済みのSourceVoiceを全て破棄してからエンジンを解放する
	StopAll();

	xAudio2_.Reset();
	masterVoice_ = nullptr;
}

void AudioManager::Update() {
	ReapFinishedVoices();
}

Sound AudioManager::LoadWave(const std::string& filePath) {
	auto soundData = std::make_shared<SoundData>();

	std::string errorMessage;
	if (!ReadWaveFile(filePath, *soundData, errorMessage)) {
		DebugUtil::Log("[AudioManager] 音声の読み込みに失敗しました (" + errorMessage + "): " + filePath + "\n");
		return nullptr;
	}

	return soundData;
}

void AudioManager::DestroyVoice(ActiveVoice& active) {
	// DestroyVoice はXAudio2の処理スレッドがボイスを使い終えるまで待つので、その後であればバッファを手放してよい
	active.voice->Stop();
	active.voice->DestroyVoice();
	active.voice = nullptr;
	active.sound.reset();
}

void AudioManager::ReapFinishedVoices() {
	for (auto it = activeVoices_.begin(); it != activeVoices_.end();) {
		XAUDIO2_VOICE_STATE state{};
		it->voice->GetState(&state);

		// 再生キューが空になっていれば再生完了とみなして破棄する（ループ再生中は空にならない）
		if (state.BuffersQueued == 0) {
			DestroyVoice(*it);
			it = activeVoices_.erase(it);
		} else {
			++it;
		}
	}
}

const AudioManager::ActiveVoice* AudioManager::Find(VoiceId id) const {
	if (id == kInvalidVoiceId) {
		return nullptr;
	}
	auto it = std::find_if(activeVoices_.begin(), activeVoices_.end(),
		[id](const ActiveVoice& v) { return v.id == id; });
	return it == activeVoices_.end() ? nullptr : &*it;
}

VoiceId AudioManager::Play(const Sound& sound, const PlayParams& params) {
	if (!sound || sound->buffer.empty() || !xAudio2_) {
		return kInvalidVoiceId;
	}

	// 再生が終わった過去のSourceVoiceを破棄してからリソースを積み増す
	ReapFinishedVoices();

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT result = xAudio2_->CreateSourceVoice(&sourceVoice, &sound->wfex);
	if (FAILED(result)) {
		DebugUtil::Log("[AudioManager] SourceVoiceの生成に失敗しました\n");
		return kInvalidVoiceId;
	}

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = sound->buffer.data();
	buf.AudioBytes = static_cast<UINT32>(sound->buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = params.loop ? XAUDIO2_LOOP_INFINITE : 0;

	result = sourceVoice->SubmitSourceBuffer(&buf);
	if (FAILED(result)) {
		sourceVoice->DestroyVoice();
		DebugUtil::Log("[AudioManager] 音声バッファの登録に失敗しました\n");
		return kInvalidVoiceId;
	}

	sourceVoice->SetVolume(params.volume);
	sourceVoice->Start();

	// 再生完了後に破棄できるよう保持しておく。音声データもここで共有して、再生中は解放されないようにする
	ActiveVoice active;
	active.id = nextVoiceId_++;
	active.voice = sourceVoice;
	active.sound = sound;
	activeVoices_.push_back(std::move(active));
	return activeVoices_.back().id;
}

void AudioManager::Stop(VoiceId id) {
	auto it = std::find_if(activeVoices_.begin(), activeVoices_.end(),
		[id](const ActiveVoice& v) { return v.id == id; });
	if (id == kInvalidVoiceId || it == activeVoices_.end()) {
		return;
	}
	DestroyVoice(*it);
	activeVoices_.erase(it);
}

void AudioManager::StopAll() {
	for (ActiveVoice& active : activeVoices_) {
		DestroyVoice(active);
	}
	activeVoices_.clear();
}

void AudioManager::SetVolume(VoiceId id, float volume) {
	if (const ActiveVoice* active = Find(id)) {
		active->voice->SetVolume(volume);
	}
}

bool AudioManager::IsPlaying(VoiceId id) const {
	const ActiveVoice* active = Find(id);
	if (!active) {
		return false;
	}
	XAUDIO2_VOICE_STATE state{};
	active->voice->GetState(&state);
	return state.BuffersQueued > 0;
}

void AudioManager::SetMasterVolume(float volume) {
	if (masterVoice_) {
		masterVoice_->SetVolume(volume);
	}
}

} // namespace Engine
