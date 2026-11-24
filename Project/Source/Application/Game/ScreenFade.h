#pragma once

namespace Shooter {

/// <summary>
/// 画面の暗転（フェード）の状態。シーンの切り替えを、唐突にしないために使う（描画には依存しない、計算だけの部分）。
/// 暗さ（alpha）は、0=まったく暗くない・1=真っ暗。
/// 使い方: シーンの開始時に StartFadeIn()、シーンを出るときに StartFadeOut() → IsFadeOutFinished() になったら ChangeScene()
/// </summary>
class ScreenFade {
public:
	/// <summary>
	/// 真っ暗な状態から、明るくする。duration が0以下なら、すぐ明るくなる
	/// </summary>
	void StartFadeIn(float duration);

	/// <summary>
	/// 今の暗さから、真っ暗にする（フェードインの途中で呼んでも、今の暗さから続く）。duration が0以下なら、すぐ真っ暗になる
	/// </summary>
	void StartFadeOut(float duration);

	/// <summary>
	/// 経過時間ぶん、進める
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// 今の暗さ（0〜1）
	/// </summary>
	[[nodiscard]] float GetAlpha() const { return alpha_; }

	/// <summary>
	/// フェードの最中か（フェードイン・アウトの途中）
	/// </summary>
	[[nodiscard]] bool IsFading() const { return state_ == State::kFadingIn || state_ == State::kFadingOut; }

	/// <summary>
	/// フェードアウトが終わって、真っ暗になったか（シーンを切り替えてよい状態）
	/// </summary>
	[[nodiscard]] bool IsFadeOutFinished() const { return state_ == State::kFadedOut; }

	/// <summary>
	/// フェードアウトを始めているか（終わった後も含む）。二重に切り替えを要求しないための判定に使う
	/// </summary>
	[[nodiscard]] bool IsFadingOutOrDone() const { return state_ == State::kFadingOut || state_ == State::kFadedOut; }

private:
	enum class State {
		kIdle,      //!< 何もしていない（alpha は 0）
		kFadingIn,
		kFadingOut,
		kFadedOut,  //!< フェードアウトが終わった（alpha は 1）
	};

	State state_ = State::kIdle;
	float alpha_ = 0.0f;
	float duration_ = 0.0f;
};

} // namespace Shooter
