#include "Game.h"

#include "Scenes/DemoScene.h"
#include "Scenes/GameOverScene.h"
#include "Scenes/GameScene.h"
#include "Scenes/TitleScene.h"

using namespace Engine;

void Game::OnInitialize() {
	// シーン登録。流れは、Title →（決定）→ Game →（倒れる）→ GameOver →（決定）→ Game / （戻る）→ Title
	SceneManager* sceneMgr = GetSceneManager();
	sceneMgr->Register<TitleScene>("Title");
	sceneMgr->Register<GameScene>("Game");
	sceneMgr->Register<GameOverScene>("GameOver");
	sceneMgr->Register<DemoScene>("Demo"); // エンジンの動作確認用（球で鏡面反射を確認）

	// 最初のシーン
	sceneMgr->ChangeScene("Title");
}
