#include "SceneManager.h"

#include <cassert>

namespace Engine {

void Scene::ChangeScene(const std::string& name) {
	assert(sceneManager_);
	sceneManager_->ChangeScene(name);
}

SceneManager::~SceneManager() = default;

void SceneManager::Initialize(Framework* framework) {
	framework_ = framework;
}

void SceneManager::Register(const std::string& name, SceneFactory factory) {
	factories_[name] = std::move(factory);
}

void SceneManager::ChangeScene(const std::string& name) {
	// 未登録のシーン名は即座に気付けるよう、要求時点で検証する
	assert(factories_.contains(name));
	nextSceneName_ = name;
}

void SceneManager::ApplyPendingChange() {
	if (nextSceneName_.empty()) {
		return;
	}

	// Initialize中に再度ChangeScene()された場合に備え、要求を先に取り出しておく
	std::string name = std::move(nextSceneName_);
	nextSceneName_.clear();

	Finalize();

	currentScene_ = factories_.at(name)();
	currentScene_->framework_ = framework_;
	currentScene_->sceneManager_ = this;
	currentSceneName_ = name;
	currentScene_->Initialize();
}

void SceneManager::Update() {
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::Draw(ID3D12GraphicsCommandList* commandList) {
	if (currentScene_) {
		currentScene_->Draw(commandList);
	}
}

void SceneManager::Finalize() {
	if (currentScene_) {
		currentScene_->Finalize();
		currentScene_.reset();
	}
	currentSceneName_.clear();
}

} // namespace Engine
