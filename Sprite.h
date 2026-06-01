#pragma once
#include <d3d12.h>
#include <wrl/client.h> 
#include "Vector4.h"
#include "TransformMatrix.h"
#include "WorldTransform.h"
#include "Mesh.h"
#include "Material.h"

class Sprite {
public:
	Sprite();
	~Sprite();

	static Sprite* Create(float width, float height);

	void Draw(const Matrix4x4& viewProjection);

public:

	Vector4& GetColor() { return material_.GetColor(); }
	Vector3& GetTranslate() { return worldTransform_.translate; }

	void SetColor(const Vector4& color) { material_.SetColor(color); }
	void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { material_.SetTextureHandle(handle); }

private:

	void CreateWvpBuffer();

private:

	Mesh     mesh_;
	Material material_;
	WorldTransform worldTransform_ = { { 1.0f, 1.0f, 1.0f } ,{ 0.0f, 0.0f, 0.0f } ,{ 0.0f, 0.0f, 0.0f } };

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	TransformationMatrix* wvpData_ = nullptr;
};