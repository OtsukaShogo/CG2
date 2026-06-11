#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "TransformMatrix.h"
#include "WorldTransform.h"
#include "Mesh.h"
#include "Material.h"

class Model {
public:
	Model();
	~Model();

	static Model* CreateSphere();

	// Objファイルを読み込んでモデルを生成する
	static Model* CreateFromObj(const std::string& directoryPath, const std::string& filename);

	void Update();

	void Draw(const Matrix4x4& viewProjection);

public:

	Vector4& GetColor() { return material_.GetColor(); }
	Vector3& GetTranslate() { return worldTransform_.translate; }
	Vector3& GetScale() { return worldTransform_.scale; }
	Vector3& GetRotate() { return worldTransform_.rotate; }

	void SetColor(const Vector4& color) { material_.SetColor(color); }
	void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { material_.SetTextureHandle(handle); }

private:

	void CreateWvpBuffer();

private:
	Mesh mesh_;
	Material material_;
	WorldTransform worldTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	TransformationMatrix* wvpData_ = nullptr;
};