#pragma once
#include <memory>
#include "ScreenFade.h"
#include "../../Engine/2D/Sprite.h"
#include "../../Engine/Graphics/AssetFactory.h"
#include "../../Engine/Math/Matrix4x4.h"

namespace Shooter {

/// <summary>
/// 画面全体を覆う黒い板で、暗転（フェード）を表示する。シーンごとに1つ持ち、最後に描く。
/// フェードの状態（ScreenFade）と、表示（スプライト）を、まとめて扱う
/// </summary>
class FadeOverlay {
public:
	/// <summary>
	/// 画面全体を覆う黒い板を作り、真っ暗な状態から、明るくし始める（シーンの開始時に1度だけ呼ぶ）
	/// </summary>
	void Initialize(Engine::AssetFactory* assetFactory);

	/// <summary>
	/// 暗くする（シーンを出るとき）
	/// </summary>
	void StartFadeOut();

	void Update(float deltaTime);

	/// <summary>
	/// 黒い板を描く（スプライト用のパイプラインがセット済みであること）。まったく暗くないときは、何も描かない
	/// </summary>
	void Draw(const Engine::Matrix4x4& spriteViewProjection);

	[[nodiscard]] bool IsFadeOutFinished() const { return fade_.IsFadeOutFinished(); }
	[[nodiscard]] bool IsFadingOutOrDone() const { return fade_.IsFadingOutOrDone(); }

private:
	ScreenFade fade_;
	std::unique_ptr<Engine::Sprite> sprite_;
};

} // namespace Shooter
