#pragma once
#include "../Engine/Core/Framework.h"

/// <summary>
/// ゲーム本体。Engine::Framework を継承し、シーンの登録と最初のシーンの指定を行う
/// </summary>
class Game : public Engine::Framework {
protected:
	void OnInitialize() override;
};
