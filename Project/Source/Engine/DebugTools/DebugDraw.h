#pragma once
#include <d3d12.h>
#include <cstdint>
#include <vector>
#include "DebugShapes.h"
#include "../Collision/Collision2D.h"
#include "../Math/Matrix4x4.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Vector4.h"

namespace Engine {

class ConstantBufferAllocator;
class PipelineStateManager;

/// <summary>
/// デバッグ表示用の色（RGBA、0〜1）
/// </summary>
namespace DebugColor {
inline constexpr Vector4 kWhite   = { 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Vector4 kRed     = { 1.0f, 0.2f, 0.2f, 1.0f };
inline constexpr Vector4 kGreen   = { 0.2f, 1.0f, 0.2f, 1.0f };
inline constexpr Vector4 kBlue    = { 0.3f, 0.5f, 1.0f, 1.0f };
inline constexpr Vector4 kYellow  = { 1.0f, 1.0f, 0.2f, 1.0f };
inline constexpr Vector4 kCyan    = { 0.2f, 1.0f, 1.0f, 1.0f };
inline constexpr Vector4 kMagenta = { 1.0f, 0.3f, 1.0f, 1.0f };
inline constexpr Vector4 kGray    = { 0.5f, 0.5f, 0.5f, 1.0f };
} // namespace DebugColor

/// <summary>
/// デバッグ用の線を描くクラス（当たり判定の範囲・移動方向・座標軸の確認など）。
/// Update や Draw の中でいつでも呼べ、貯めた線は、シーンの描画後にまとめて描かれる（1フレームで消える）。
/// 線の太さは1ピクセル固定。
/// Debug と Development のビルドでだけ有効で、Release では何もしない（呼び出しの費用も実質ゼロ）
/// </summary>
class DebugDraw {
public:
	// この構成でデバッグ描画が有効か（NDEBUGが定義されるReleaseだけ無効）
#ifdef NDEBUG
	static constexpr bool kEnabled = false;
#else
	static constexpr bool kEnabled = true;
#endif

	// 1フレームに貯められる頂点数の上限（線1本で2頂点）。超えた分は捨てる（ログは最初の1回だけ）
	static constexpr size_t kMaxVertices = 1 << 17;

	/// <summary>
	/// 円・球の既定の分割数
	/// </summary>
	static constexpr uint32_t kDefaultSegments = 32;

	DebugDraw() = default;
	~DebugDraw() = default;

	DebugDraw(const DebugDraw&) = delete;
	DebugDraw& operator=(const DebugDraw&) = delete;

	/// <summary>
	/// 描画に使うパイプライン管理を設定する（無効な構成では使わない）
	/// </summary>
	void Initialize(PipelineStateManager* pipelineStateManager);

	/// <summary>
	/// 表示のオン・オフ（オフの間は、貯めた線を描かずに捨てる）
	/// </summary>
	void SetVisible(bool visible) { visible_ = visible; }
	[[nodiscard]] bool IsVisible() const { return visible_; }

	/// <summary>
	/// 以降に追加する線を、深度テストする（モデルの後ろに隠れる）かを切り替える。
	/// 既定は false（常に手前に描く）。フレームの最初に false へ戻る
	/// </summary>
	void SetDepthTest(bool enable) { depthTest_ = enable; }

	/// <summary>
	/// 線を1本追加する
	/// </summary>
	void Line(const Vector3& from, const Vector3& to, const Vector4& color = DebugColor::kWhite);

	/// <summary>
	/// 矢印（始点から終点）を追加する。移動方向などの確認用
	/// </summary>
	void Arrow(const Vector3& from, const Vector3& to, const Vector4& color = DebugColor::kWhite, float headLength = 0.3f);

	/// <summary>
	/// 水平な円（XZ平面。見下ろし型の当たり判定の円）を追加する
	/// </summary>
	void CircleXZ(const Vector3& center, float radius, const Vector4& color = DebugColor::kWhite, uint32_t segments = kDefaultSegments);

	/// <summary>
	/// 任意の向きの円を追加する
	/// </summary>
	/// <param name="normal">円の平面の法線</param>
	void Circle(const Vector3& center, const Vector3& normal, float radius, const Vector4& color = DebugColor::kWhite, uint32_t segments = kDefaultSegments);

	/// <summary>
	/// 球を追加する
	/// </summary>
	void Sphere(const Vector3& center, float radius, const Vector4& color = DebugColor::kWhite, uint32_t segments = kDefaultSegments);

	/// <summary>
	/// 軸に平行な直方体（AABB）を追加する
	/// </summary>
	void Box(const Vector3& min, const Vector3& max, const Vector4& color = DebugColor::kWhite);

	/// <summary>
	/// ワールド行列で変換した直方体（大きさ1の立方体が元。回転した箱もこれで描ける）を追加する
	/// </summary>
	void Box(const Matrix4x4& world, const Vector4& color = DebugColor::kWhite);

	/// <summary>
	/// 水平な長方形（XZ平面。見下ろし型の矩形の当たり判定）を追加する
	/// </summary>
	/// <param name="halfSize">X方向・Z方向の半分の長さ</param>
	void RectXZ(const Vector3& center, const Vector2& halfSize, const Vector4& color = DebugColor::kWhite);

	/// <summary>
	/// 当たり判定の円（Circle2D）を、高さ y の水平な円として追加する
	/// </summary>
	void DrawCircle2D(const Circle2D& circle, float y, const Vector4& color = DebugColor::kWhite, uint32_t segments = kDefaultSegments);

	/// <summary>
	/// 当たり判定の矩形（AABB2D）を、高さ y の水平な長方形として追加する
	/// </summary>
	void DrawAABB2D(const AABB2D& aabb, float y, const Vector4& color = DebugColor::kWhite);

	/// <summary>
	/// 当たり判定の線分（Segment2D）を、高さ y の水平な線として追加する
	/// </summary>
	void DrawSegment2D(const Segment2D& segment, float y, const Vector4& color = DebugColor::kWhite);

	/// <summary>
	/// 座標軸（X=赤・Y=緑・Z=青）を追加する
	/// </summary>
	void Axis(const Vector3& origin, float length = 1.0f);

	/// <summary>
	/// 水平な格子（XZ平面）を追加する
	/// </summary>
	/// <param name="halfExtent">中心から端までの長さ</param>
	/// <param name="step">線の間隔</param>
	void Grid(float halfExtent, float step, const Vector4& color = DebugColor::kGray, const Vector3& center = { 0.0f, 0.0f, 0.0f });

	/// <summary>
	/// 貯めた線を描いて、空にする。シーンの描画後（ImGuiの前）に、フレームワークが毎フレーム呼ぶ
	/// </summary>
	/// <param name="commandList">今フレームのコマンドリスト</param>
	/// <param name="constantBuffers">今フレームの定数バッファの切り出し元（頂点データもここへ書く）</param>
	/// <param name="viewProjection">ビュープロジェクション行列</param>
	void Render(ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers, const Matrix4x4& viewProjection);

	/// <summary>
	/// いま貯まっている頂点数（確認用）
	/// </summary>
	[[nodiscard]] size_t GetPendingVertexCount() const { return overlayVertices_.size() + depthVertices_.size(); }

private:
	// 頂点: 座標とRGBA8の色（16バイト）
	struct Vertex {
		float x, y, z;
		uint32_t color;
	};

	void Commit(const DebugLineList& lines, const Vector4& color);
	void DrawVertices(ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers, const std::vector<Vertex>& vertices);

private:
	PipelineStateManager* pipelineStateManager_ = nullptr;
	bool visible_ = true;
	bool depthTest_ = false;
	bool overflowLogged_ = false;
	std::vector<Vertex> overlayVertices_; // 常に手前に描く線
	std::vector<Vertex> depthVertices_;   // 深度テストする線
	DebugLineList scratch_;               // 図形の生成に使う作業用（確保し直しを避けるため使い回す）
};

} // namespace Engine
