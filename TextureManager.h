#pragma once
#include"externals/DirectXTex/DirectXTex.h"
#include<string>
#include<d3d12.h>

class TextureManager
{
public:

	TextureManager();
	~TextureManager();

	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	static void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
};

