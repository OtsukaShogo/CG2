#include "AssetManager.h"
#include <cassert>

namespace Engine {

void AssetManager::Initialize(AssetFactory* factory) {
    factory_ = factory;

    // === 読み込むモデルをここに登録する ===
    RegisterModel("resources", "fence.obj");
}

void AssetManager::RegisterModel(const std::string& directoryPath, const std::string& filename) {
    std::string path = directoryPath + "/" + filename;
    modelData_[path] = factory_->CreateModelData(directoryPath, filename);
}

std::unique_ptr<Model> AssetManager::CreateModel(const std::string& path) {
    auto it = modelData_.find(path);
    assert(it != modelData_.end() && "指定されたパスのモデルはAssetManagerに登録されていません");

    return factory_->CreateModelInstance(it->second);
}

void AssetManager::Clear() {
    modelData_.clear();
}

} // namespace Engine
