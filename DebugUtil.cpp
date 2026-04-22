#include<Windows.h>
#include<fstream>
#include<chrono>
#include<format>
#include<filesystem>
#include<strsafe.h>

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

LONG WINAPI DebugUtil::ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリ以下に出力
	SYSTEMTIME time;

	GetLocalTime(&time);

	wchar_t filePath[MAX_PATH] = { 0 };

	CreateDirectory(L"./Dumps", nullptr);

	StringCchPrintfW(
		filePath, 
		MAX_PATH, 
		L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
		time.wYear, 
		time.wMonth, 
		time.wDay, 
		time.wHour, 
		time.wMinute
	);

	HANDLE dumpFileHandle = CreateFile(
		filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		0,
		CREATE_ALWAYS,
		0,
		0
	);

	// processId（このexeのID）とスレッド（例外）の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();

	DWORD threadId = GetCurrentThreadId();

	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };

	minidumpInformation.ThreadId = threadId;

	minidumpInformation.ExceptionPointers = exception;

	minidumpInformation.ClientPointers = TRUE;

	// Dumpを出力。MiniDumpNormalは最も限の情報を出力するフラグ
	MiniDumpWriteDump(
		GetCurrentProcess(),
		processId,
		dumpFileHandle,
		MiniDumpNormal,
		&minidumpInformation,
		nullptr,
		nullptr
	);

	return EXCEPTION_EXECUTE_HANDLER;
}