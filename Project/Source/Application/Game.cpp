#include "Game.h"

#include "Scenes/DemoScene.h"

using namespace Engine;

void Game::OnInitialize() {
	// シーン登録
	SceneManager* sceneMgr = GetSceneManager();
	sceneMgr->Register<DemoScene>("Demo");

	// 最初のシーン
	sceneMgr->ChangeScene("Demo");
}
