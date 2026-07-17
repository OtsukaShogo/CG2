#include "Mesh.h"
#include <numbers>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cassert>
#include "D3D12Util.h"
#include"Material.h"

namespace Engine {

Mesh::Mesh() {}

Mesh::~Mesh() {}

const std::string& Mesh::GetTextureFilePath() const {
	return material_->GetTextureFilePath();
}

void Mesh::CreateSphere(uint32_t subdivision) {
	// 緯度・経度1区画（クアッド）あたりの頂点数・インデックス数（三角形2枚分）
	constexpr uint32_t kVerticesPerQuad = 4;
	constexpr uint32_t kIndicesPerQuad  = 6;

	const float kLonEvery = std::numbers::pi_v<float> * 2.0f / float(subdivision);
	const float kLatEvery = std::numbers::pi_v<float> / float(subdivision);

	vertices_.clear();
	indices_.clear();
	vertices_.reserve(subdivision * subdivision * kVerticesPerQuad);
	indices_.reserve(subdivision * subdivision * kIndicesPerQuad);

	// 緯度・経度をsubdivision分割し、各区画（クアッド）ごとに4頂点・2三角形を生成してUV球を作る
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
	// 矩形の頂点数（左下・左上・右上・右下の4点）
	constexpr size_t kRectVertexCount = 4;
	vertices_.resize(kRectVertexCount);

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

void Mesh::ParseVertexLine(std::istringstream& s, std::vector<Vector4>& positions) {
	Vector4 position;
	s >> position.x >> position.y >> position.z;
	position.x *= -1.0f;
	position.w = 1.0f;
	positions.push_back(position);
}

void Mesh::ParseTexcoordLine(std::istringstream& s, std::vector<Vector2>& texcoords) {
	Vector2 texcoord;
	s >> texcoord.x >> texcoord.y;
	texcoord.y = 1.0f - texcoord.y;
	texcoords.push_back(texcoord);
}

void Mesh::ParseNormalLine(std::istringstream& s, std::vector<Vector3>& normals) {
	Vector3 normal;
	s >> normal.x >> normal.y >> normal.z;
	normal.x *= -1.0f;
	normals.push_back(normal);
}

void Mesh::ParseFaceLine(
	std::istringstream& s,
	const std::vector<Vector4>& positions,
	const std::vector<Vector2>& texcoords,
	const std::vector<Vector3>& normals) {
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
}

void Mesh::ParseMtllibLine(std::istringstream& s, const std::string& directoryPath) {
	// materialTemplateLibraryファイルの名前を取得する
	std::string materialFilename;
	s >> materialFilename;
	// Materialを生成し、mtlファイルを読み込ませる（基本的にobjファイルと同一階層にmtlは存在する）
	material_ = std::make_unique<Material>();
	material_->LoadMaterialTemplateFile(directoryPath, materialFilename);
}

void Mesh::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals;   // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line;               // ファイルから読んだ1行を格納する

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	// 1行ずつ読み、先頭の識別子に応じて対応するパーサーへ振り分ける
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			ParseVertexLine(s, positions);
		} else if (identifier == "vt") {
			ParseTexcoordLine(s, texcoords);
		} else if (identifier == "vn") {
			ParseNormalLine(s, normals);
		} else if (identifier == "f") {
			ParseFaceLine(s, positions, texcoords, normals);
		} else if (identifier == "mtllib") {
			ParseMtllibLine(s, directoryPath);
		}
	}
}

// CPU上に構築した頂点・インデックスデータをGPUバッファへコピーし、各種ビューを作成する
void Mesh::Upload(ID3D12Device* device) {
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

// インデックスバッファがあればインデックス付き描画、なければ通常描画を行う
void Mesh::Draw(ID3D12GraphicsCommandList* commandList) const {
	commandList->IASetVertexBuffers(0, 1, &vbv_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (!indices_.empty()) {
		commandList->IASetIndexBuffer(&ibv_);
		commandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
	} else {
		commandList->DrawInstanced(GetVertexCount(), 1, 0, 0);
	}
}

} // namespace Engine