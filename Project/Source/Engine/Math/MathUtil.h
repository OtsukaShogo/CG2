#pragma once
#include <algorithm>
#include <cmath>
#include <numbers>

namespace Engine {

/// <summary>
/// 円周率
/// </summary>
inline constexpr float kPi = std::numbers::pi_v<float>;

/// <summary>
/// 2π（1周分のラジアン）
/// </summary>
inline constexpr float kTwoPi = std::numbers::pi_v<float> * 2.0f;

/// <summary>
/// 浮動小数点の比較・ゼロ判定に使う既定の許容誤差
/// </summary>
inline constexpr float kEpsilon = 1e-5f;

/// <summary>
/// 度をラジアンに変換する
/// </summary>
[[nodiscard]] constexpr float ToRadians(float degrees) {
	return degrees * (std::numbers::pi_v<float> / 180.0f);
}

/// <summary>
/// ラジアンを度に変換する
/// </summary>
[[nodiscard]] constexpr float ToDegrees(float radians) {
	return radians * (180.0f / std::numbers::pi_v<float>);
}

/// <summary>
/// 2つの値が許容誤差の範囲内で等しいかを判定する
/// </summary>
/// <param name="a">値1</param>
/// <param name="b">値2</param>
/// <param name="epsilon">許容誤差</param>
[[nodiscard]] inline bool NearlyEqual(float a, float b, float epsilon = kEpsilon) {
	return std::abs(a - b) <= epsilon;
}

/// <summary>
/// 線形補間する（t=0でa、t=1でb。tは範囲外でも外挿される）
/// </summary>
[[nodiscard]] constexpr float Lerp(float a, float b, float t) {
	return a + (b - a) * t;
}

/// <summary>
/// 値を0〜1の範囲に収める
/// </summary>
[[nodiscard]] constexpr float Saturate(float value) {
	return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

/// <summary>
/// 値を [min, max] の範囲に収める
/// </summary>
[[nodiscard]] constexpr float Clamp(float value, float min, float max) {
	return value < min ? min : (value > max ? max : value);
}

/// <summary>
/// currentから targetへ、最大 maxDelta だけ近づける（行き過ぎない）
/// </summary>
/// <param name="current">現在の値</param>
/// <param name="target">目標値</param>
/// <param name="maxDelta">1回で動かせる最大量（0以上）</param>
[[nodiscard]] inline float MoveTowards(float current, float target, float maxDelta) {
	if (std::abs(target - current) <= maxDelta) {
		return target;
	}
	return current + (target > current ? maxDelta : -maxDelta);
}

/// <summary>
/// 角度（ラジアン）を (-π, π] の範囲に収める
/// </summary>
[[nodiscard]] inline float WrapAngle(float radians) {
	float wrapped = std::fmod(radians, kTwoPi);
	if (wrapped > kPi) {
		wrapped -= kTwoPi;
	} else if (wrapped <= -kPi) {
		wrapped += kTwoPi;
	}
	return wrapped;
}

/// <summary>
/// 2つの角度（ラジアン）の差を、最短経路での符号付きの値（-π〜π）で返す。
/// 例：350度から10度への差は、-340度ではなく +20度になる
/// </summary>
[[nodiscard]] inline float DeltaAngle(float from, float to) {
	return WrapAngle(to - from);
}

/// <summary>
/// 角度（ラジアン）を最短経路で補間する（0度と359度の間を、遠回りせずに補間する）
/// </summary>
[[nodiscard]] inline float LerpAngle(float from, float to, float t) {
	return from + DeltaAngle(from, to) * t;
}

} // namespace Engine
