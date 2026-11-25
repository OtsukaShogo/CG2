#include "Game.h"

#include "Game/GameAudio.h"
#include "Scenes/DemoScene.h"
#include "Scenes/GameOverScene.h"
#include "Scenes/GameScene.h"
#include "Scenes/TitleScene.h"

using namespace Engine;

void Game::OnInitialize() {
	// 効果音と BGM を、あらかじめ読み込む（シーンをまたいで使うので、ここで1度だけ）
	Shooter::GameAudio::Get().Initialize(GetAudio());

	// シーン登録。流れは、Title →（決定）→ Game →（倒れる）→ GameOver →（決定）→ Game / （戻る）→ Title
	SceneManager* sceneMgr = GetSceneManager();
	sceneMgr->Register<TitleScene>("Title");
	sceneMgr->Register<GameScene>("Game");
	sceneMgr->Register<GameOverScene>("GameOver");
	sceneMgr->Register<DemoScene>("Demo"); // エンジンの動作確認用（炎のパーティクル）

	// 最初のシーン
	sceneMgr->ChangeScene("Title");
}

void Game::OnUpdate() {
	Shooter::GameAudio::Get().Update(GetDeltaTime());
}

void Game::OnFinalize() {
	// オーディオの終了より前に、BGM を止めて、読み込んだ音を手放す
	Shooter::GameAudio::Get().Shutdown();
}
