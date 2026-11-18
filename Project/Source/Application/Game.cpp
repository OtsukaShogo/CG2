#include "Game.h"

#include "Scenes/DemoScene.h"
#include "Scenes/GameScene.h"

using namespace Engine;

void Game::OnInitialize() {
	// シーン登録
	SceneManager* sceneMgr = GetSceneManager();
	sceneMgr->Register<GameScene>("Game");
	sceneMgr->Register<DemoScene>("Demo"); // エンジンの動作確認用（炎のパーティクル）

	// 最初のシーン
	sceneMgr->ChangeScene("Game");
}
