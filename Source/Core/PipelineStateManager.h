#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <unordered_map>

struct IDxcBlob;

namespace Engine {

class ShaderManager;

/// <summary>
/// ブレンドモード（描画時の合成方法）の種類
/// </summary>
enum class BlendMode {
    kNone,             // ブレンドなし
    kNormal,           // 通常のブレンド（アルファブレンド）
    kAdd,              // 加算
    kSubtract,         // 減算
    kMultiply,         // 乗算
    kScreen,           // スクリーン
    kCountOfBlendMode, // ブレンドモードの数
};

/// <summary>
/// BlendModeの表示名一覧（ImGui等での表示用、BlendModeの並び順と対応）
/// </summary>
static constexpr const char* kBlendModeNames[] = {
    "None", "Normal", "Add", "Subtract", "Multiply", "Screen",
};

/// <summary>
/// グラフィックスパイプラインステート(PSO)とルートシグネチャの生成・管理を行うクラス
/// </summary>
class PipelineStateManager {
public:
    // PSO名
    static constexpr const char* kObject3D = "Object3D";
    static constexpr const char* kSprite   = "Sprite";

    /// <summary>
    /// ブレンドモードに応じたブレンドステート設定を作成する
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    /// <returns>対応するブレンドステート設定</returns>
    [[nodiscard]] static D3D12_BLEND_DESC MakeBlendDesc(BlendMode blendMode);

    /// <summary>
    /// 共通ルートシグネチャにおけるルートパラメータのスロット番号
    /// </summary>
    enum class RootParameter : UINT {
        kMaterial         = 0, // PixelShader用CBV (b0)
        kWVP              = 1, // VertexShader用CBV (b0)
        kTexture          = 2, // SRVテーブル (t0)
        kDirectionalLight = 3, // PixelShader用CBV (b1)
    };

    /// <summary>
    /// コンストラクタ
    /// </summary>
    PipelineStateManager();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~PipelineStateManager();

    /// <summary>
    /// 汎用的なグラフィックスパイプラインを作成してキャッシュする
    /// </summary>
    /// <param name="name">PSOをキャッシュする際の名前</param>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="vsBlob">頂点シェーダーのバイナリ</param>
    /// <param name="psBlob">ピクセルシェーダーのバイナリ</param>
    /// <param name="inputLayout">入力レイアウト</param>
    /// <param name="blendDesc">ブレンドステート設定</param>
    /// <param name="rasterizerDesc">ラスタライザステート設定</param>
    /// <param name="depthStencilDesc">深度ステンシルステート設定</param>
    /// <param name="rtvFormat">レンダーターゲットのフォーマット</param>
    /// <param name="dsvFormat">深度ステンシルのフォーマット</param>
    void CreateGraphicsPipeline(
        const std::string& name,
        ID3D12Device* device,
        IDxcBlob* vsBlob,
        IDxcBlob* psBlob,
        const D3D12_INPUT_LAYOUT_DESC& inputLayout,
        const D3D12_BLEND_DESC& blendDesc,
        const D3D12_RASTERIZER_DESC& rasterizerDesc,
        const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
        DXGI_FORMAT rtvFormat,
        DXGI_FORMAT dsvFormat);

    /// <summary>
    /// Object3D用のPSOを、全ブレンドモード分作成してキャッシュする
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="shaderMgr">シェーダーのコンパイルに使用するShaderManager</param>
    void CreateObject3DPipeline(ID3D12Device* device, ShaderManager* shaderMgr);

    /// <summary>
    /// Sprite(2D)用のPSOを、全ブレンドモード分作成してキャッシュする
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="shaderMgr">シェーダーのコンパイルに使用するShaderManager</param>
    void CreateSpritePipeline(ID3D12Device* device, ShaderManager* shaderMgr);

    /// <summary>
    /// 既に作成済みのPSOを名前から取得する
    /// </summary>
    /// <param name="name">取得するPSOの名前</param>
    /// <returns>該当するPSO（存在しない場合はnullptr）</returns>
    [[nodiscard]] ID3D12PipelineState* GetPipelineState(const std::string& name) const;

    /// <summary>
    /// 共通のルートシグネチャを取得する
    /// </summary>
    /// <returns>共通ルートシグネチャ</returns>
    [[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

    /// <summary>
    /// 共通のルートシグネチャを初期化する（最初に一度だけ呼ぶ）
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void InitializeRootSignature(ID3D12Device* device);

    /// <summary>
    /// 指定した名前・ブレンドモードのPSOとルートシグネチャをコマンドリストにセットする
    /// </summary>
    /// <param name="commandList">セット対象のコマンドリスト</param>
    /// <param name="name">セットするPSOの名前</param>
    /// <param name="blendMode">セットするブレンドモード</param>
    void SetPipeline(ID3D12GraphicsCommandList* commandList, const std::string& name, BlendMode blendMode);

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> pipelineStates_;
};

} // namespace Engine
