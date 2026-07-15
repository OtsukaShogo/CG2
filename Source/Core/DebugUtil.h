#pragma once
#include <Windows.h>
#include<string>
#include<fstream>

namespace Engine {

/// <summary>
/// デバッグ出力・ログファイル出力・クラッシュダンプ出力をまとめて扱うユーティリティクラス
/// </summary>
class DebugUtil {
public:

	/// <summary>
	/// デバッグ出力ウィンドウとログファイルに文字列を出力する
	/// </summary>
	/// <param name="message">出力する文字列</param>
	static void Log(const std::string& message);

	/// <summary>
	/// 実行時刻を名前に含むログファイルを作成し、以後のログ出力先とする
	/// </summary>
	static void CreateLogFile();

	/// <summary>
	/// 未処理例外発生時にクラッシュダンプファイルを出力する
	/// </summary>
	/// <param name="exception">発生した例外の情報</param>
	/// <returns>例外処理の続行方法を示す値</returns>
	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

private:
	static std::ofstream logStream_;
};

} // namespace Engine
