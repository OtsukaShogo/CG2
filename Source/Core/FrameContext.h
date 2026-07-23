#pragma once
#include <d3d12.h>

namespace Engine {

/// <summary>
/// 「今フレームで使うべきコマンドリスト」を保持するだけの共有オブジェクト。
/// DirectXCommonがBeginFrame()のたびに中身を更新し、Model/Spriteなど描画側はこれを
/// 生成時に1度だけ受け取って保持しておくことで、Draw()の引数からcommandListを省略できる。
/// 将来コマンドリストをフレームごとに複数使い回す設計に変えても、
/// DirectXCommon側の更新だけで済み、参照側の変更は不要になる。
/// </summary>
struct FrameContext {
    ID3D12GraphicsCommandList* commandList = nullptr;
};

} // namespace Engine
