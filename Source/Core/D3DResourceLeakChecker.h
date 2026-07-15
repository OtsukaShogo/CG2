#pragma once

namespace Engine {

/// <summary>
/// スコープを抜ける際にDirectX12リソースのリーク（未解放）をレポートするクラス
/// </summary>
class D3DResourceLeakChecker {
public:
    /// <summary>
    /// 破棄時にD3D12リソースのリークをデバッグ出力にレポートする
    /// </summary>
    ~D3DResourceLeakChecker();
};

} // namespace Engine
