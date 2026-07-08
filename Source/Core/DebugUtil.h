#pragma once
#include <Windows.h>
#include<string>
#include<fstream>

class DebugUtil {
public:

	//デバッグ出力ウィンドウに文字列を出力する関数
	static void Log(const std::string& message);

	static void CreateLogFile();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

private:
	static std::ofstream logStream_;
};