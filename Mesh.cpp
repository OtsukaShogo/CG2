#include "Mesh.h"
#include <numbers>
#include <cstring>
#include "DirectXCommon.h"
#include "D3D12_Util.h"

Mesh::Mesh() {}

Mesh::~Mesh() {}

void Mesh::CreateSphere(uint32_t subdivision) {
	const float kLonEvery = std::numbers::pi_v<float> * 2.0f / float(subdivision);
	const float kLatEvery = std::numbers::pi_v<float> / float(subdivision);

	vertices_.resize(subdivision * subdivision * 4);
	indices_.resize(subdivision * subdivision * 6);

	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			uint32_t vStart = (latIndex * subdivision + lonIndex) * 4;
			uint32_t iStart = (latIndex * subdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;

			float startU = float(lonIndex) / float(subdivision);
			float startV = 1.0f - float(latIndex) / float(subdivision);
			float nextU = float(lonIndex + 1) / float(subdivision);
			float nextV = 1.0f - float(latIndex + 1) / float(subdivision);

			// a
			vertices_[vStart + 0].position.x = std::cos(lat) * std::cos(lon);
			vertices_[vStart + 0].position.y = std::sin(lat);
			vertices_[vStart + 0].position.z = std::cos(lat) * std::sin(lon);
			vertices_[vStart + 0].position.w = 1.0f;
			vertices_[vStart + 0].texcoord = { startU, startV };
			vertices_[vStart + 0].normal = { vertices_[vStart + 0].position.x, vertices_[vStart + 0].position.y, vertices_[vStart + 0].position.z };

			// b
			vertices_[vStart + 1].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertices_[vStart + 1].position.y = std::sin(lat + kLatEvery);
			vertices_[vStart + 1].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertices_[vStart + 1].position.w = 1.0f;
			vertices_[vStart + 1].texcoord = { startU, nextV };
			vertices_[vStart + 1].normal = { vertices_[vStart + 1].position.x, vertices_[vStart + 1].position.y, vertices_[vStart + 1].position.z };

			// c
			vertices_[vStart + 2].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertices_[vStart + 2].position.y = std::sin(lat);
			vertices_[vStart + 2].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertices_[vStart + 2].position.w = 1.0f;
			vertices_[vStart + 2].texcoord = { nextU, startV };
			vertices_[vStart + 2].normal = { vertices_[vStart + 2].position.x, vertices_[vStart + 2].position.y, vertices_[vStart + 2].position.z };

			// d
			vertices_[vStart + 3].position.x = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
			vertices_[vStart + 3].position.y = std::sin(lat + kLatEvery);
			vertices_[vStart + 3].position.z = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
			vertices_[vStart + 3].position.w = 1.0f;
			vertices_[vStart + 3].texcoord = { nextU, nextV };
			vertices_[vStart + 3].normal = { vertices_[vStart + 3].position.x, vertices_[vStart + 3].position.y, vertices_[vStart + 3].position.z };

			// T1: a, b, c
			indices_[iStart + 0] = vStart + 0;
			indices_[iStart + 1] = vStart + 1;
			indices_[iStart + 2] = vStart + 2;

			// T2: d, c, b
			indices_[iStart + 3] = vStart + 3;
			indices_[iStart + 4] = vStart + 2;
			indices_[iStart + 5] = vStart + 1;
		}
	}
}

void Mesh::CreateRect(float width, float height) {
	vertices_.resize(4);

	vertices_[0].position = { 0.0f,  height, 0.0f, 1.0f }; // 左下
	vertices_[0].texcoord = { 0.0f, 1.0f };
	vertices_[0].normal   = { 0.0f, 0.0f, -1.0f };

	vertices_[1].position = { 0.0f,  0.0f,  0.0f, 1.0f }; // 左上
	vertices_[1].texcoord = { 0.0f, 0.0f };
	vertices_[1].normal   = { 0.0f, 0.0f, -1.0f };

	vertices_[2].position = { width, 0.0f,  0.0f, 1.0f }; // 右上
	vertices_[2].texcoord = { 1.0f, 0.0f };
	vertices_[2].normal   = { 0.0f, 0.0f, -1.0f };

	vertices_[3].position = { width, height, 0.0f, 1.0f }; // 右下
	vertices_[3].texcoord = { 1.0f, 1.0f };
	vertices_[3].normal   = { 0.0f, 0.0f, -1.0f };

	indices_ = { 0, 1, 3, 1, 2, 3 };
}

void Mesh::Upload() {
	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	uint32_t vertexCount = GetVertexCount();

	vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * vertexCount);

	VertexData* mapped = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	std::memcpy(mapped, vertices_.data(), sizeof(VertexData) * vertexCount);
	vertexResource_->Unmap(0, nullptr);

	vbv_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vbv_.SizeInBytes = sizeof(VertexData) * vertexCount;
	vbv_.StrideInBytes = sizeof(VertexData);

	if (!indices_.empty()) {
		indexResource_ = CreateBufferResource(device, sizeof(uint32_t) * indices_.size());

		uint32_t* indexMapped = nullptr;
		indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexMapped));
		std::memcpy(indexMapped, indices_.data(), sizeof(uint32_t) * indices_.size());
		indexResource_->Unmap(0, nullptr);

		ibv_.BufferLocation = indexResource_->GetGPUVirtualAddress();
		ibv_.SizeInBytes    = sizeof(uint32_t) * static_cast<UINT>(indices_.size());
		ibv_.Format         = DXGI_FORMAT_R32_UINT;
	}
}

void Mesh::Draw(ID3D12GraphicsCommandList* commandList) {
	commandList->IASetVertexBuffers(0, 1, &vbv_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (!indices_.empty()) {
		commandList->IASetIndexBuffer(&ibv_);
		commandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
	} else {
		commandList->DrawInstanced(GetVertexCount(), 1, 0, 0);
	}
}