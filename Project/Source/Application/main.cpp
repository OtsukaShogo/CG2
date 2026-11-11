#include <Windows.h>

#include "../Engine/Core/DebugUtil.h"
#include "../Engine/Core/D3DResourceLeakChecker.h"
#include "Game.h"

using namespace Engine;

/// <summary>
/// アプリケーションのエントリーポイント。初期化・メインループ・終了処理は Framework::Run() が行う
/// </summary>
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// リソースリークチェッカー（最後に破棄されるよう、Game より先に生成する）
	D3DResourceLeakChecker leakCheck;

	SetUnhandledExceptionFilter(DebugUtil::ExportDump);
	DebugUtil::CreateLogFile();

	Game game;
	game.Run();

	return 0;
}
