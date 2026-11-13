#pragma once
#include <d3d12.h>
#include <cstdint>

namespace Engine { class ConstantBufferAllocator; }

namespace Engine {

/// <summary>
/// CPUが先行して準備できるフレーム数（GPUが処理中のフレームと、CPUが作成中のフレームの合計）。
/// CPUが毎フレーム書き換えるリソースは、この数だけ用意して frameIndex で切り替える
/// </summary>
constexpr uint32_t kFrameCount = 2;

/// <summary>
/// 「今フレームで使うべきコマンドリスト」などを保持するだけの共有オブジェクト。
/// DirectXCommonがフレームの開始時に中身を更新し、Model/Spriteなど描画側はこれを
/// 生成時に1度だけ受け取って保持しておくことで、Draw()の引数からcommandListを省略できる。
/// </summary>
struct FrameContext {
    ConstantBufferAllocator* constantBuffers = nullptr; // 描画ごとの定数バッファの切り出し元
    ID3D12GraphicsCommandList* commandList = nullptr;
    uint32_t frameIndex = 0; // 今のフレームが使うスロット（0〜kFrameCount-1）。CPUが毎フレーム書くリソースの選択に使う
};

} // namespace Engine
