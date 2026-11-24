#include "TestFramework.h"
#include "Source/Application/Game/GameSession.h"
#include "Source/Application/Game/ScreenFade.h"

using namespace Shooter;

namespace {

// 一定のフレーム間隔で、指定の秒数だけ進める
void Advance(ScreenFade& fade, float seconds, float dt = 1.0f / 60.0f) {
	const int frames = static_cast<int>(seconds / dt + 0.5f);
	for (int i = 0; i < frames; ++i) {
		fade.Update(dt);
	}
}

GameResult Result(int kills, float time) {
	GameResult r;
	r.kills = kills;
	r.survivalTime = time;
	return r;
}

} // namespace

TEST_CASE(ScreenFade_StartsClear) {
	ScreenFade fade;
	CHECK(fade.GetAlpha() == 0.0f);
	CHECK(!fade.IsFading());
	CHECK(!fade.IsFadeOutFinished());
	fade.Update(1.0f); // 何も始めていなければ、何も起きない
	CHECK(fade.GetAlpha() == 0.0f);
}

TEST_CASE(ScreenFade_FadeInGoesFromBlackToClear) {
	ScreenFade fade;
	fade.StartFadeIn(0.5f);
	CHECK(fade.GetAlpha() == 1.0f);   // 真っ暗から
	CHECK(fade.IsFading());
	Advance(fade, 0.25f);
	CHECK_NEAR(fade.GetAlpha(), 0.5, 0.02);
	Advance(fade, 0.3f);
	CHECK(fade.GetAlpha() == 0.0f);   // 明るくなる
	CHECK(!fade.IsFading());
	CHECK(!fade.IsFadeOutFinished());
}

TEST_CASE(ScreenFade_FadeOutFinishesBlack) {
	ScreenFade fade;
	fade.StartFadeOut(0.4f);
	CHECK(fade.IsFading());
	CHECK(!fade.IsFadeOutFinished());
	Advance(fade, 0.2f);
	CHECK_NEAR(fade.GetAlpha(), 0.5, 0.02);
	CHECK(!fade.IsFadeOutFinished());  // 途中は、まだ切り替えてはいけない
	Advance(fade, 0.3f);
	CHECK(fade.GetAlpha() == 1.0f);
	CHECK(fade.IsFadeOutFinished());   // 真っ暗になって、切り替えてよい
	CHECK(!fade.IsFading());
	Advance(fade, 5.0f);
	CHECK(fade.GetAlpha() == 1.0f);    // 終わったあとも、真っ暗のまま
	CHECK(fade.IsFadeOutFinished());
}

TEST_CASE(ScreenFade_FadeOutContinuesFromCurrentAlpha) {
	ScreenFade fade;
	fade.StartFadeIn(1.0f);
	Advance(fade, 0.5f);                 // 半分まで明るくなった
	const float midway = fade.GetAlpha();
	fade.StartFadeOut(1.0f);             // すぐ、フェードアウトに切り替える（フェードインの途中で、遷移を要求）
	CHECK_NEAR(fade.GetAlpha(), midway, 1e-6); // 暗さが、飛ばずに続く
	Advance(fade, 0.5f);
	CHECK_NEAR(fade.GetAlpha(), 1.0, 0.03);
	Advance(fade, 0.1f);
	CHECK(fade.IsFadeOutFinished());
}

TEST_CASE(ScreenFade_ZeroDurationIsInstant) {
	ScreenFade fadeIn;
	fadeIn.StartFadeIn(0.0f);
	CHECK(fadeIn.GetAlpha() == 0.0f);
	CHECK(!fadeIn.IsFading());
	ScreenFade fadeOut;
	fadeOut.StartFadeOut(0.0f);
	CHECK(fadeOut.GetAlpha() == 1.0f);
	CHECK(fadeOut.IsFadeOutFinished());
	ScreenFade negative;
	negative.StartFadeOut(-1.0f);
	CHECK(negative.IsFadeOutFinished());
}

TEST_CASE(ScreenFade_IsFadingOutOrDonePreventsDoubleRequests) {
	ScreenFade fade;
	CHECK(!fade.IsFadingOutOrDone());
	fade.StartFadeOut(0.3f);
	CHECK(fade.IsFadingOutOrDone());     // 途中
	Advance(fade, 1.0f);
	CHECK(fade.IsFadingOutOrDone());     // 終わった後も
	ScreenFade fadingIn;
	fadingIn.StartFadeIn(1.0f);
	CHECK(!fadingIn.IsFadingOutOrDone()); // フェードインは、含まない
}

TEST_CASE(ScreenFade_BigFrameDoesNotOvershoot) {
	ScreenFade fade;
	fade.StartFadeOut(0.5f);
	fade.Update(10.0f);                  // 長い処理落ちがあっても、範囲を超えない
	CHECK(fade.GetAlpha() == 1.0f);
	ScreenFade in;
	in.StartFadeIn(0.5f);
	in.Update(10.0f);
	CHECK(in.GetAlpha() == 0.0f);
}

TEST_CASE(GameSession_RecordsLastResult) {
	GameSession session;
	CHECK(session.GetLast().kills == 0);
	CHECK(session.Record(Result(12, 34.5f)));   // 最初の記録は、必ずベスト
	CHECK(session.GetLast().kills == 12);
	CHECK_NEAR(session.GetLast().survivalTime, 34.5, 1e-6);
	CHECK(session.IsLastNewBest());
	CHECK(session.GetBest().kills == 12);
}

TEST_CASE(GameSession_BestPrefersKillsThenTime) {
	GameSession session;
	session.Record(Result(10, 50.0f));
	CHECK(!session.Record(Result(8, 90.0f)));    // 撃破数が少ない: 生存時間が長くても、ベストにならない
	CHECK(session.GetBest().kills == 10);
	CHECK(session.GetLast().kills == 8);         // 直近の結果は、更新される
	CHECK(!session.IsLastNewBest());
	CHECK(session.Record(Result(10, 60.0f)));    // 同じ撃破数で、生存時間が長い: ベスト
	CHECK_NEAR(session.GetBest().survivalTime, 60.0, 1e-6);
	CHECK(!session.Record(Result(10, 60.0f)));   // 同じ記録: 更新ではない
	CHECK(session.Record(Result(11, 5.0f)));     // 撃破数が多い: 生存時間が短くても、ベスト
	CHECK(session.GetBest().kills == 11);
}

TEST_CASE(GameSession_BeginPlayKeepsBest) {
	GameSession session;
	session.Record(Result(7, 20.0f));
	session.BeginPlay();                          // 新しいプレイの開始: 直近の結果は消えるが、ベストは残る
	CHECK(session.GetLast().kills == 0);
	CHECK(session.GetLast().survivalTime == 0.0f);
	CHECK(!session.IsLastNewBest());
	CHECK(session.GetBest().kills == 7);
	CHECK(!session.Record(Result(3, 10.0f)));     // ベストが残っているので、低い記録は、更新にならない
}

TEST_CASE(GameSession_ZeroResultIsStillAFirstBest) {
	GameSession session;
	CHECK(session.Record(Result(0, 3.0f)));       // 1体も倒せなくても、最初の記録は、ベスト
	CHECK(!session.Record(Result(0, 2.0f)));
	CHECK(session.Record(Result(0, 4.0f)));
}

TEST_CASE(GameSession_ClearResetsEverything) {
	GameSession session;
	session.Record(Result(9, 40.0f));
	session.Clear();
	CHECK(session.GetBest().kills == 0);
	CHECK(session.GetLast().kills == 0);
	CHECK(session.Record(Result(1, 1.0f)));       // 消したあとの最初の記録は、またベスト
}

TEST_CASE(GameSession_GetReturnsSharedInstance) {
	// アプリ全体で、同じ記録を共有する（ここでは、状態を壊さないよう、読むだけ）
	CHECK(&GameSession::Get() == &GameSession::Get());
}
