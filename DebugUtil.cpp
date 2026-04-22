#include<Windows.h>
#include<fstream>
#include<chrono>
#include<format>
#include<filesystem>

#include "DebugUtil.h"

std::ofstream DebugUtil::logStream_;

void DebugUtil::Log(const std::string& message) {
	logStream_ << message << std::endl;
	OutputDebugStringA(message.c_str());
}

void DebugUtil::CreateLogFile() {
	//ログのディレクトリを用意
	std::filesystem::create_directory("logs");

	// 現在時刻を取得 (UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	// 日本時間 (PCの設定時間) に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	// formatを使って年月日, 時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";

	// ファイルを作って書き込み準備
    logStream_.open(logFilePath);
}