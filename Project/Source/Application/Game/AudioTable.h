#pragma once
#include <array>
#include <string>

namespace Shooter {

/// <summary>
/// 効果音の種類
/// </summary>
enum class SeId {
	kNone,       //!< 鳴らさない
	kShot,       //!< 弾を撃つ
	kPlayerHit,  //!< 自機の被弾
	kEnemyHit,   //!< 敵の被弾
	kEnemyKill,  //!< 敵の撃破
	kConfirm,    //!< 決定
	kCount,
};

/// <summary>
/// BGM の種類（画面ごとに切り替える）
/// </summary>
enum class BgmId {
	kTitle,
	kGame,
	kResult,
	kCount,
};

// 音のファイル・音量・鳴らす間隔の表。差し替えるときは、AudioTable.cpp の表だけを書き換える（今は、仮のファイルで、すべて同じ）。

/// <summary>
/// 効果音のファイルのパス（kNone・kCount は空）
/// </summary>
[[nodiscard]] const char* GetSePath(SeId id);

/// <summary>
/// 効果音の音量（0〜1）
/// </summary>
[[nodiscard]] float GetSeVolume(SeId id);

/// <summary>
/// 同じ効果音を、続けて鳴らす最小の間隔（秒）。連射の弾などで、同じ音が、重なりすぎないようにする
/// </summary>
[[nodiscard]] float GetSeMinInterval(SeId id);

/// <summary>
/// BGM のファイルのパス（kCount は空）
/// </summary>
[[nodiscard]] const char* GetBgmPath(BgmId id);

/// <summary>
/// BGM の音量（0〜1）
/// </summary>
[[nodiscard]] float GetBgmVolume(BgmId id);

/// <summary>
/// 効果音を鳴らすタイミングの間引き。種類ごとに、最小の間隔を空けないと、鳴らせない
/// </summary>
class SoundThrottle {
public:
	/// <summary>
	/// 時間を進める（毎フレーム呼ぶ）
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// いま鳴らしてよいか。鳴らしてよい場合は、true を返し、次に鳴らせるまでの待ち時間を、始める
	/// </summary>
	[[nodiscard]] bool TryPlay(SeId id);

	/// <summary>
	/// すべての待ち時間を、0 に戻す
	/// </summary>
	void Reset();

private:
	std::array<float, static_cast<size_t>(SeId::kCount)> cooldown_ = {};
};

/// <summary>
/// BGM を切り替えるときの、判断
/// </summary>
enum class BgmTransition {
	kKeep,   //!< いまの曲を、そのまま続ける（同じ曲なので、最初に戻さない）
	kSwitch, //!< いまの曲を止めて、新しい曲を、最初から始める
};

/// <summary>
/// BGM を切り替える指示があったとき、そのまま続けるか、切り替えるかを決める。
/// すでに同じファイルが再生中なら、続ける（画面が切り替わっても、同じ曲なら、途切れない）
/// </summary>
/// <param name="currentPath">いま再生している曲のファイル（再生していなければ空）</param>
/// <param name="currentIsPlaying">いまの曲が、実際に再生中か</param>
/// <param name="nextPath">新しく指示された曲のファイル</param>
[[nodiscard]] BgmTransition DecideBgmTransition(const std::string& currentPath, bool currentIsPlaying, const std::string& nextPath);

} // namespace Shooter
