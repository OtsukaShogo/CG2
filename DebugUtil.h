#pragma once
#include<string>
#include<fstream>

class DebugUtil {
public:

	//デバッグ出力ウィンドウに文字列を出力する関数
	static void Log(const std::string& message);

	static void CreateLogFile();

private:
	static std::ofstream logStream_;
};