#include "Mesh.h"
#include <numbers>
#include <cstring>
#include "DirectXCommon.h"
#include "D3D12_Util.h"

Mesh::Mesh() {}

Mesh::~Mesh() {}

void Mesh::CreateSphere(uint32_t subdivision) {
	const float kLonEvery = std::numbers::pi_v<float> *2.0f / float(subdivision);
	const float kLatEvery = std::numbers::pi_v<float> / float(subdivision);

	vertices_.resize(subdivision * subdivision * 6);

	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			uint32_t start = (latIndex * subdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;

			float startU = float(lonIndex) / float(subdivision);
			float startV = 1.0f - float(latIndex) / float(subdivision);
			float nextU = float(lonIndex + 1) / float(subdivision);
			float nextV = 1.0f - float(latIndex + 1) / float(subdivision);

			// a
			vertices_[start + 0].position.x = std::cos(lat) * std::cos(lon);
			vertices_[start + 0].position.y = std::sin(lat);
			vertices_[start + 0].position.z = std::cos(lat) * std::sin(lon);
			vertices_[start + 0].position.w = 1.0f;
			vertices_[start + 0].texcoord = { startU, startV };

			// b
			vertices_[start + 1].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertices_[start + 1].position.y = std::sin(lat + kLatEvery);
			vertices_[start + 1].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertices_[start + 1].position.w = 1.0f;
			vertices_[start + 1].texcoord = { startU, nextV };

			// c
			vertices_[start + 2].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertices_[start + 2].position.y = std::sin(lat);
			vertices_[start + 2].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertices_[start + 2].position.w = 1.0f;
			vertices_[start + 2].texcoord = { nextU, startV };

			// d
			vertices_[start + 3].position.x = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
			vertices_[start + 3].position.y = std::sin(lat + kLatEvery);
			vertices_[start + 3].position.z = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
			vertices_[start + 3].position.w = 1.0f;
			vertices_[start + 3].texcoord = { nextU, nextV };

			// c
			vertices_[start + 4] = vertices_[start + 2];

			// b
			vertices_[start + 5] = vertices_[start + 1];
		}
	}
}

void Mesh::CreateRect(float width, float height) {
	vertices_.resize(6);

	vertices_[0].position = { 0.0f,  height, 0.0f, 1.0f }; // 左下
	vertices_[0].texcoord = { 0.0f, 1.0f };

	vertices_[1].position = { 0.0f,  0.0f,  0.0f, 1.0f }; // 左上
	vertices_[1].texcoord = { 0.0f, 0.0f };

	vertices_[2].position = { width, height, 0.0f, 1.0f }; // 右下
	vertices_[2].texcoord = { 1.0f, 1.0f };

	vertices_[3].position = { 0.0f,  0.0f,  0.0f, 1.0f }; // 左上
	vertices_[3].texcoord = { 0.0f, 0.0f };

	vertices_[4].position = { width, 0.0f,  0.0f, 1.0f }; // 右上
	vertices_[4].texcoord = { 1.0f, 0.0f };

	vertices_[5].position = { width, height, 0.0f, 1.0f }; // 右下
	vertices_[5].texcoord = { 1.0f, 1.0f };
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
}

void Mesh::Draw(ID3D12GraphicsCommandList* commandList) {
	commandList->IASetVertexBuffers(0, 1, &vbv_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(GetVertexCount(), 1, 0, 0);
}