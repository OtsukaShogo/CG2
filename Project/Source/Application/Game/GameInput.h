#pragma once
#include <memory>
#include "../../Engine/2D/Sprite.h"
#include "../../Engine/Graphics/AssetFactory.h"
#include "../../Engine/Input/Input.h"
#include "../../Engine/Math/Vector4.h"

namespace Shooter {

/// <summary>
/// 「決定」の操作が、このフレームで押されたか（Enter・Space・マウス左・ゲームパッドの Start / A）。
/// タイトルやゲームオーバーでの、先へ進む操作に使う。押した瞬間だけ true（押し続けても、1回だけ）
/// </summary>
[[nodiscard]] bool IsConfirmPressed(Engine::Input* input);

/// <summary>
/// 「戻る」の操作が、このフレームで押されたか（Esc・Backspace・ゲームパッドの B）
/// </summary>
[[nodiscard]] bool IsBackPressed(Engine::Input* input);

/// <summary>
/// 画面のピクセル位置・大きさを指定して、単色の板（スプライト）を作る。
/// 文字が使えない間の、仮の表示（ボタンやアイコンの代わり）に使う
/// </summary>
[[nodiscard]] std::unique_ptr<Engine::Sprite> CreateColorBox(Engine::AssetFactory* assetFactory, float x, float y, float width, float height, const Engine::Vector4& color);

} // namespace Shooter
