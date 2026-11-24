#pragma once

namespace Shooter {

/// <summary>
/// 1回のプレイの結果
/// </summary>
struct GameResult {
	int kills = 0;              //!< 倒した敵の数
	float survivalTime = 0.0f;  //!< 生き延びた時間（秒）
};

/// <summary>
/// シーンをまたいで残る、ゲーム全体の記録（直近の結果と、ベスト）。
/// シーンの作り直しでは消えないので、ゲームシーンで結果を書き込み、ゲームオーバーのシーンで読む。
/// ファイルへは保存しない（アプリを終了すると消える）
/// </summary>
class GameSession {
public:
	/// <summary>
	/// アプリ全体で共有する記録を取得する
	/// </summary>
	[[nodiscard]] static GameSession& Get();

	/// <summary>
	/// 新しいプレイを始める。直近の結果を消す（ベストは残す）
	/// </summary>
	void BeginPlay();

	/// <summary>
	/// プレイが終わったときの結果を記録する。ベストを更新したら true
	/// （ベストは、撃破数が多いものが優先。同じなら、生存時間が長いもの）
	/// </summary>
	bool Record(const GameResult& result);

	/// <summary>
	/// 直近の結果
	/// </summary>
	[[nodiscard]] const GameResult& GetLast() const { return last_; }

	/// <summary>
	/// ベストの結果（まだ記録がなければ、すべて0）
	/// </summary>
	[[nodiscard]] const GameResult& GetBest() const { return best_; }

	/// <summary>
	/// 直近の結果が、ベストを更新したものか
	/// </summary>
	[[nodiscard]] bool IsLastNewBest() const { return lastWasNewBest_; }

	/// <summary>
	/// 記録を、すべて消す
	/// </summary>
	void Clear();

private:
	GameResult last_;
	GameResult best_;
	bool hasBest_ = false;
	bool lastWasNewBest_ = false;
};

} // namespace Shooter
