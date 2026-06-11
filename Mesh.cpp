#include "Mesh.h"
#include <numbers>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cassert>
#include "DirectXCommon.h"
#include "D3D12Util.h"
#include"Material.h"

Mesh::Mesh() {}

Mesh::~Mesh() {}

const std::string& Mesh::GetTextureFilePath() const {
	return material_->GetTextureFilePath();
}

void Mesh::CreateSphere(uint32_t subdivision) {
	const float kLonEvery = std::numbers::pi_v<float> * 2.0f / float(subdivision);
	const float kLatEvery = std::numbers::pi_v<float> / float(subdivision);

	vertices_.clear();
	indices_.clear();
	vertices_.reserve(subdivision * subdivision * 4);
	indices_.reserve(subdivision * subdivision * 6);

	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			float startU = float(lonIndex) / float(subdivision);
			float startV = 1.0f - float(latIndex) / float(subdivision);
			float nextU = float(lonIndex + 1) / float(subdivision);
			float nextV = 1.0f - float(latIndex + 1) / float(subdivision);

			uint32_t vStart = static_cast<uint32_t>(vertices_.size());

			// a
			VertexData a{};
			a.position = { std::cos(lat) * std::cos(lon), std::sin(lat), std::cos(lat) * std::sin(lon), 1.0f };
			a.texcoord = { startU, startV };
			a.normal = { a.position.x, a.position.y, a.position.z };

			// b
			VertexData b{};
			b.position = { std::cos(lat + kLatEvery) * std::cos(lon), std::sin(lat + kLatEvery), std::cos(lat + kLatEvery) * std::sin(lon), 1.0f };
			b.texcoord = { startU, nextV };
			b.normal = { b.position.x, b.position.y, b.position.z };

			// c
			VertexData c{};
			c.position = { std::cos(lat) * std::cos(lon + kLonEvery), std::sin(lat), std::cos(lat) * std::sin(lon + kLonEvery), 1.0f };
			c.texcoord = { nextU, startV };
			c.normal = { c.position.x, c.position.y, c.position.z };

			// d
			VertexData d{};
			d.position = { std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery), std::sin(lat + kLatEvery), std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery), 1.0f };
			d.texcoord = { nextU, nextV };
			d.normal = { d.position.x, d.position.y, d.position.z };

			vertices_.push_back(a);
			vertices_.push_back(b);
			vertices_.push_back(c);
			vertices_.push_back(d);

			// T1: a, b, c
			indices_.push_back(vStart + 0);
			indices_.push_back(vStart + 1);
			indices_.push_back(vStart + 2);

			// T2: d, c, b
			indices_.push_back(vStart + 3);
			indices_.push_back(vStart + 2);
			indices_.push_back(vStart + 1);
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

void Mesh::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	//1.中で必要となる変数の宣言
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals;   // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line;               // ファイルから読んだ1行を格納する

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	//3.実際にファイルを読み、vertices_に代入する
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);

		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);

		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);

		} else if (identifier == "f") {
			// 面は三角形限定。面を構成する頂点を逆順に登録することで、回り順を反転させる
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				// 頂点の要素へのインデックスは「位置/UV/法線」の形式で書かれているので分解する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = static_cast<uint32_t>(std::stoi(index));
				}

				// 要素のインデックスから、実際の要素の値を取得して頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal   = normals[elementIndices[2] - 1];
				triangle[faceVertex] = VertexData{ position, texcoord, normal };
			}

			// 頂点を逆順で登録する
			vertices_.push_back(triangle[2]);
			vertices_.push_back(triangle[1]);
			vertices_.push_back(triangle[0]);

		} else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// Materialを生成し、mtlファイルを読み込ませる（基本的にobjファイルと同一階層にmtlは存在する）
			material_ = std::make_unique<Material>();
			material_->LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
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