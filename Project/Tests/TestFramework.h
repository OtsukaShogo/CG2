#pragma once
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

// 外部ライブラリを使わない、最小の単体テスト用フレームワーク。
//
// 使い方:
//   TEST_CASE(テスト名) {
//       CHECK(条件);
//       CHECK_NEAR(値, 期待値, 許容誤差);
//   }
// 1つのCHECKが失敗しても、そのテストは最後まで実行され、失敗したCHECKがすべて表示される。
// 実行ファイルは、失敗が1つでもあれば0以外の終了コードを返す（CIで失敗を検出するため）。
// 引数にテスト名の一部を渡すと、その名前を含むテストだけを実行する。

namespace TestFramework {

struct TestCase {
	std::string name;
	std::function<void()> body;
};

inline std::vector<TestCase>& Registry() {
	static std::vector<TestCase> registry;
	return registry;
}

struct State {
	int checks = 0;
	int failedChecks = 0;
	bool currentFailed = false;
};

inline State& GetState() {
	static State state;
	return state;
}

struct Registrar {
	Registrar(const char* name, void (*body)()) { Registry().push_back({ name, body }); }
};

inline void ReportFailure(const char* file, int line, const std::string& message) {
	State& state = GetState();
	++state.failedChecks;
	state.currentFailed = true;
	std::printf("    FAILED %s:%d  %s\n", file, line, message.c_str());
}

inline void Check(bool condition, const char* expression, const char* file, int line) {
	++GetState().checks;
	if (!condition) {
		ReportFailure(file, line, expression);
	}
}

inline void CheckNear(double actual, double expected, double tolerance, const char* expression, const char* file, int line) {
	++GetState().checks;
	// NaN は比較がすべて false になるので、「差が許容誤差以内」という形で判定して NaN も失敗にする
	if (!(std::abs(actual - expected) <= tolerance)) {
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "%s  (actual=%g, expected=%g, tolerance=%g)", expression, actual, expected, tolerance);
		ReportFailure(file, line, buffer);
	}
}

inline int RunAll(int argc, char** argv) {
	const std::string filter = argc > 1 ? argv[1] : "";
	State& state = GetState();
	int ran = 0;
	int failedTests = 0;

	for (const TestCase& test : Registry()) {
		if (!filter.empty() && test.name.find(filter) == std::string::npos) {
			continue;
		}
		++ran;
		state.currentFailed = false;
		std::printf("[ RUN  ] %s\n", test.name.c_str());
		test.body();
		if (state.currentFailed) {
			++failedTests;
			std::printf("[ FAIL ] %s\n", test.name.c_str());
		} else {
			std::printf("[  OK  ] %s\n", test.name.c_str());
		}
	}

	std::printf("\n%d tests, %d failed, %d checks, %d failed checks\n", ran, failedTests, state.checks, state.failedChecks);
	// 1つも実行されなかった場合は、フィルタの指定ミスなどなので失敗にする
	return (failedTests == 0 && ran > 0) ? 0 : 1;
}

} // namespace TestFramework

#define TEST_CONCAT_INNER(a, b) a##b
#define TEST_CONCAT(a, b) TEST_CONCAT_INNER(a, b)

#define TEST_CASE(name)                                                                             \
	static void TEST_CONCAT(TestBody_, name)();                                                     \
	static TestFramework::Registrar TEST_CONCAT(TestRegistrar_, name)(#name, &TEST_CONCAT(TestBody_, name)); \
	static void TEST_CONCAT(TestBody_, name)()

#define CHECK(condition) TestFramework::Check((condition), #condition, __FILE__, __LINE__)
#define CHECK_NEAR(actual, expected, tolerance) \
	TestFramework::CheckNear(static_cast<double>(actual), static_cast<double>(expected), static_cast<double>(tolerance), #actual " ~= " #expected, __FILE__, __LINE__)
