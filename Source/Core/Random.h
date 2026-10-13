#pragma once
#include <random>
#include "Vector3.h"

namespace Engine {

/// <summary>
/// 乱数生成を行うクラス
/// </summary>
class Random {
public:
	/// <summary>
	/// シングルトンインスタンスを取得する
	/// </summary>
	/// <returns>Randomのインスタンス</returns>
	[[nodiscard]] static Random* GetInstance();

	/// <summary>
	/// 指定範囲のfloat乱数を取得する
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>min以上max以下のfloat乱数</returns>
	[[nodiscard]] float GetFloat(float min, float max);

	/// <summary>
	/// 各成分が指定範囲のVector3乱数を取得する
	/// </summary>
	/// <param name="min">各成分の最小値</param>
	/// <param name="max">各成分の最大値</param>
	/// <returns>各成分がmin以上max以下のVector3乱数</returns>
	[[nodiscard]] Vector3 GetVector3(const Vector3& min, const Vector3& max);

	/// <summary>
	/// 内部で使用している乱数エンジンを取得する（std::uniform_real_distribution等と組み合わせて使う場合）
	/// </summary>
	/// <returns>乱数エンジン</returns>
	[[nodiscard]] std::mt19937& GetEngine() { return engine_; }

private:
	Random();
	~Random() = default;

	Random(const Random&) = delete;
	Random& operator=(const Random&) = delete;

private:
	std::mt19937 engine_;
};

} // namespace Engine
