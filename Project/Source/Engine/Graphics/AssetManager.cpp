#include "AssetManager.h"
#include <cassert>

#include "../Core/DebugUtil.h"

namespace Engine {

void AssetManager::Initialize(AssetFactory* factory) {
    factory_ = factory;
}

void AssetManager::RegisterModel(const std::string& directoryPath, const std::string& filename) {
    std::string path = directoryPath + "/" + filename;
    if (modelData_.contains(path)) {
        return; // 登録済み
    }
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
