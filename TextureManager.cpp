#include "TextureManager.h"
#include"ConvertString.h"

TextureManager::TextureManager() {}

TextureManager::~TextureManager() {}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミップマップ付きのデータを返す
	return mipImages;
}

/*void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages){
    // Meta情報を取得
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    // 全MipMapについて
    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
        // MipMapLevelを指定して各Imageを取得
        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
        // Textureに転送
        HRESULT hr = texture->WriteToSubresource(
            UINT(mipLevel),
            nullptr,                // 全領域へコピー
            img->pixels,            // 元データアドレス
            UINT(img->rowPitch),    // 1ラインサイズ
            UINT(img->slicePitch)   // 1枚サイズ
        );
        assert(SUCCEEDED(hr));
    }
}*/