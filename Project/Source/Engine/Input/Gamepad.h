#pragma once
#include <Windows.h>
#include <Xinput.h>
#include <cstdint>

#include "../Math/Vector2.h"

namespace Engine {

/// <summary>
/// ゲームパッドのボタン（XInputのボタンビットに対応）
/// </summary>
enum class GamepadButton : WORD {
	kDpadUp = XINPUT_GAMEPAD_DPAD_UP,
	kDpadDown = XINPUT_GAMEPAD_DPAD_DOWN,
	kDpadLeft = XINPUT_GAMEPAD_DPAD_LEFT,
	kDpadRight = XINPUT_GAMEPAD_DPAD_RIGHT,
	kStart = XINPUT_GAMEPAD_START,
	kBack = XINPUT_GAMEPAD_BACK,
	kLeftThumb = XINPUT_GAMEPAD_LEFT_THUMB,   //!< 左スティック押し込み
	kRightThumb = XINPUT_GAMEPAD_RIGHT_THUMB, //!< 右スティック押し込み
	kLeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
	kRightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	kA = XINPUT_GAMEPAD_A,
	kB = XINPUT_GAMEPAD_B,
	kX = XINPUT_GAMEPAD_X,
	kY = XINPUT_GAMEPAD_Y,
};

/// <summary>
/// XInput対応ゲームパッド1台分の状態を管理するクラス。
/// スティックはデッドゾーン処理済みの値（-1〜1、上・右が+）で取得できる
/// </summary>
class Gamepad {
public:
	/// <summary>
	/// 状態を最新のものに更新する（Inputが毎フレーム呼ぶ）。接続されていない場合は一定間隔でのみ接続を確認する
	/// </summary>
	/// <param name="index">パッド番号（0〜3）</param>
	void Update(DWORD index);

	/// <summary>
	/// 接続されているかを取得する
	/// </summary>
	/// <returns>接続されている場合はtrue</returns>
	[[nodiscard]] bool IsConnected() const { return connected_; }

	/// <summary>
	/// ボタンが押されているかを取得する
	/// </summary>
	[[nodiscard]] bool PushButton(GamepadButton button) const;

	/// <summary>
	/// ボタンが今フレームで押された瞬間かを取得する
	/// </summary>
	[[nodiscard]] bool TriggerButton(GamepadButton button) const;

	/// <summary>
	/// ボタンが今フレームで離された瞬間かを取得する
	/// </summary>
	[[nodiscard]] bool ReleaseButton(GamepadButton button) const;

	/// <summary>
	/// 左スティックの傾きを取得する（デッドゾーン処理済み、各成分-1〜1、右と上が+、長さは最大1）
	/// </summary>
	[[nodiscard]] Vector2 GetLeftStick() const { return leftStick_; }

	/// <summary>
	/// 右スティックの傾きを取得する（デッドゾーン処理済み、各成分-1〜1、右と上が+、長さは最大1）
	/// </summary>
	[[nodiscard]] Vector2 GetRightStick() const { return rightStick_; }

	/// <summary>
	/// 左トリガーの押し込み量を取得する（0〜1）
	/// </summary>
	[[nodiscard]] float GetLeftTrigger() const { return leftTrigger_; }

	/// <summary>
	/// 右トリガーの押し込み量を取得する（0〜1）
	/// </summary>
	[[nodiscard]] float GetRightTrigger() const { return rightTrigger_; }

	/// <summary>
	/// トリガーをボタンのように判定する（押し込み量が閾値を超えているか）
	/// </summary>
	[[nodiscard]] bool PushLeftTrigger() const { return leftTrigger_ > kTriggerThreshold; }
	[[nodiscard]] bool PushRightTrigger() const { return rightTrigger_ > kTriggerThreshold; }

	/// <summary>
	/// 今フレームでトリガーが閾値を超えた瞬間かを取得する
	/// </summary>
	[[nodiscard]] bool TriggerLeftTrigger() const { return leftTrigger_ > kTriggerThreshold && leftTriggerPrev_ <= kTriggerThreshold; }
	[[nodiscard]] bool TriggerRightTrigger() const { return rightTrigger_ > kTriggerThreshold && rightTriggerPrev_ <= kTriggerThreshold; }

	/// <summary>
	/// 振動を設定する（次に変更するまで続く。0で停止）
	/// </summary>
	/// <param name="low">低周波モーター（左・重い振動）の強さ 0〜1</param>
	/// <param name="high">高周波モーター（右・細かい振動）の強さ 0〜1</param>
	void SetVibration(float low, float high);

private:
	// トリガーをボタン扱いする閾値
	static constexpr float kTriggerThreshold = 0.12f;
	// 未接続パッドの接続確認間隔（フレーム数）。XInputGetStateは未接続時に重いので毎フレームは呼ばない
	static constexpr int kReconnectInterval = 120;

	/// <summary>
	/// スティックの生値にデッドゾーンを適用し、-1〜1へ正規化する（円形デッドゾーン）
	/// </summary>
	[[nodiscard]] static Vector2 ApplyStickDeadZone(SHORT x, SHORT y, SHORT deadZone);

	bool connected_ = false;
	DWORD index_ = 0;
	int reconnectCounter_ = 0;

	WORD buttons_ = 0;
	WORD buttonsPrev_ = 0;
	Vector2 leftStick_ = {};
	Vector2 rightStick_ = {};
	float leftTrigger_ = 0.0f;
	float rightTrigger_ = 0.0f;
	float leftTriggerPrev_ = 0.0f;
	float rightTriggerPrev_ = 0.0f;
};

} // namespace Engine
