#include "AssetManager.h"
#include <cassert>

#include "../Core/DebugUtil.h"

namespace Engine {

void AssetManager::Initialize(AssetFactory* factory) {
    factory_ = factory;

    // === 読み込むモデルをここに登録する ===
    RegisterModel("Resource", "fence.obj");
}

void AssetManager::RegisterModel(const std::string& directoryPath, const std::string& filename) {
    std::string path = directoryPath + "/" + filename;
    modelData_[path] = factory_->CreateModelData(directoryPath, filename);
}

std::unique_ptr<Model> AssetManager::CreateModel(const std::string& path) {
    auto it = modelData_.find(path);
    if (it == modelData_.end()) {
        // 未登録でも止めず、球で代用して原因をログに残す（Releaseビルドでも安全に動く）
        DebugUtil::Log("[AssetManager] 登録されていないモデルです。球で代用します: " + path + "\n");
        return factory_->CreateModelInstance(factory_->CreateSphereModelData());
    }

    return factory_->CreateModelInstance(it->second);
}

void AssetManager::Clear() {
    modelData_.clear();
}

} // namespace Engine
