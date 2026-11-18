#include "DebugDraw.h"
#include "../Core/DebugUtil.h"
#include "../Core/PipelineStateManager.h"
#include "../Graphics/ConstantBufferAllocator.h"
#include <algorithm>
#include <cstring>

namespace Engine {

namespace {

// 0〜1の値を、0〜255の整数にする（範囲外は端に丸める）
uint32_t ToByte(float value) {
	return static_cast<uint32_t>((std::clamp)(value, 0.0f, 1.0f) * 255.0f + 0.5f);
}

// DXGI_FORMAT_R8G8B8A8_UNORM は、最下位バイトが R
uint32_t PackColor(const Vector4& color) {
	return ToByte(color.x) | (ToByte(color.y) << 8) | (ToByte(color.z) << 16) | (ToByte(color.w) << 24);
}

} // namespace

void DebugDraw::Initialize(PipelineStateManager* pipelineStateManager) {
	pipelineStateManager_ = pipelineStateManager;
}

void DebugDraw::Commit(const DebugLineList& lines, const Vector4& color) {
	if (!kEnabled || !visible_) {
		return;
	}
	std::vector<Vertex>& target = depthTest_ ? depthVertices_ : overlayVertices_;
	const uint32_t packed = PackColor(color);

	for (const DebugLine& line : lines) {
		if (GetPendingVertexCount() + 2 > kMaxVertices) {
			if (!overflowLogged_) {
				overflowLogged_ = true;
				DebugUtil::Log("[DebugDraw] 1フレームの線の上限に達しました。超えた分は描きません\n");
			}
			return;
		}
		target.push_back({ line.start.x, line.start.y, line.start.z, packed });
		target.push_back({ line.end.x, line.end.y, line.end.z, packed });
	}
}

void DebugDraw::Line(const Vector3& from, const Vector3& to, const Vector4& color) {
	if (!kEnabled) return;
	scratch_.clear();
	scratch_.push_back({ from, to });
	Commit(scratch_, color);
}

void DebugDraw::Arrow(const Vector3& from, const Vector3& to, const Vector4& color, float headLength) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendArrow(scratch_, from, to, headLength);
	Commit(scratch_, color);
}

void DebugDraw::CircleXZ(const Vector3& center, float radius, const Vector4& color, uint32_t segments) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendCircle(scratch_, center, Vector3{ 1.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 1.0f }, radius, segments);
	Commit(scratch_, color);
}

void DebugDraw::Circle(const Vector3& center, const Vector3& normal, float radius, const Vector4& color, uint32_t segments) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendCircle(scratch_, center, normal, radius, segments);
	Commit(scratch_, color);
}

void DebugDraw::Sphere(const Vector3& center, float radius, const Vector4& color, uint32_t segments) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendSphere(scratch_, center, radius, segments);
	Commit(scratch_, color);
}

void DebugDraw::Box(const Vector3& min, const Vector3& max, const Vector4& color) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendBox(scratch_, min, max);
	Commit(scratch_, color);
}

void DebugDraw::Box(const Matrix4x4& world, const Vector4& color) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendBox(scratch_, world);
	Commit(scratch_, color);
}

void DebugDraw::RectXZ(const Vector3& center, const Vector2& halfSize, const Vector4& color) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendRect(scratch_, center, halfSize);
	Commit(scratch_, color);
}

void DebugDraw::Draw(const Engine::Sphere& sphere, const Vector4& color, uint32_t segments) {
	Sphere(sphere.center, sphere.radius, color, segments);
}

void DebugDraw::Draw(const AABB& aabb, const Vector4& color) {
	Box(aabb.min, aabb.max, color);
}

void DebugDraw::Draw(const OBB& obb, const Vector4& color) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendOBB(scratch_, obb);
	Commit(scratch_, color);
}

void DebugDraw::Draw(const Capsule& capsule, const Vector4& color, uint32_t segments) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendCapsule(scratch_, capsule.segment, capsule.radius, segments);
	Commit(scratch_, color);
}

void DebugDraw::Draw(const Segment3D& segment, const Vector4& color) {
	Line(segment.start, segment.end, color);
}

void DebugDraw::DrawCircle2D(const Circle2D& circle, float y, const Vector4& color, uint32_t segments) {
	CircleXZ(FromXZ(circle.center, y), circle.radius, color, segments);
}

void DebugDraw::DrawAABB2D(const AABB2D& aabb, float y, const Vector4& color) {
	const Vector2 center = (aabb.min + aabb.max) * 0.5f;
	const Vector2 halfSize = (aabb.max - aabb.min) * 0.5f;
	RectXZ(FromXZ(center, y), halfSize, color);
}

void DebugDraw::DrawSegment2D(const Segment2D& segment, float y, const Vector4& color) {
	Line(FromXZ(segment.start, y), FromXZ(segment.end, y), color);
}

void DebugDraw::Axis(const Vector3& origin, float length) {
	if (!kEnabled) return;
	Line(origin, origin + Vector3{ length, 0.0f, 0.0f }, DebugColor::kRed);
	Line(origin, origin + Vector3{ 0.0f, length, 0.0f }, DebugColor::kGreen);
	Line(origin, origin + Vector3{ 0.0f, 0.0f, length }, DebugColor::kBlue);
}

void DebugDraw::Grid(float halfExtent, float step, const Vector4& color, const Vector3& center) {
	if (!kEnabled) return;
	scratch_.clear();
	DebugShapes::AppendGrid(scratch_, center, halfExtent, step);
	Commit(scratch_, color);
}

void DebugDraw::DrawVertices(ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers, const std::vector<Vertex>& vertices) {
	if (vertices.empty()) {
		return;
	}
	const size_t sizeInBytes = vertices.size() * sizeof(Vertex);
	// 頂点データは、定数バッファと同じ、フレームごとの切り出し元へ書く（GPUが前のフレームを読んでいても上書きされない）
	ConstantBufferAllocator::Allocation allocation = constantBuffers.Allocate(sizeInBytes);
	std::memcpy(allocation.cpu, vertices.data(), sizeInBytes);

	D3D12_VERTEX_BUFFER_VIEW view{};
	view.BufferLocation = allocation.gpu;
	view.SizeInBytes = static_cast<UINT>(sizeInBytes);
	view.StrideInBytes = sizeof(Vertex);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList->IASetVertexBuffers(0, 1, &view);
	commandList->DrawInstanced(static_cast<UINT>(vertices.size()), 1, 0, 0);
}

void DebugDraw::Render(ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers, const Matrix4x4& viewProjection) {
	if (!kEnabled) {
		return;
	}

	if (visible_ && pipelineStateManager_ && GetPendingVertexCount() > 0) {
		// ビュープロジェクション行列は、2つの描画で共通。ルートシグネチャを切り替えるたびに設定し直す
		D3D12_GPU_VIRTUAL_ADDRESS viewProjectionAddress = constantBuffers.Upload(viewProjection);
		auto drawGroup = [&](const std::vector<Vertex>& vertices, bool depthTest) {
			if (vertices.empty()) {
				return;
			}
			pipelineStateManager_->SetDebugLinePipeline(commandList, depthTest);
			commandList->SetGraphicsRootConstantBufferView(0, viewProjectionAddress);
			DrawVertices(commandList, constantBuffers, vertices);
		};
		drawGroup(depthVertices_, true);
		drawGroup(overlayVertices_, false); // 常に手前の線は、あとから描いて重なっても隠れないようにする
	}

	overlayVertices_.clear();
	depthVertices_.clear();
	depthTest_ = false;
}

} // namespace Engine
