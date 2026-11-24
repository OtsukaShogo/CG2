#include "GameInput.h"

using namespace Engine;

namespace Shooter {

bool IsConfirmPressed(Input* input) {
	bool pressed = input->GetKeyboard().TriggerKey(DIK_RETURN)
		|| input->GetKeyboard().TriggerKey(DIK_NUMPADENTER)
		|| input->GetKeyboard().TriggerKey(DIK_SPACE)
		|| input->GetMouse().TriggerButton(MouseButton::kLeft);

	const int padIndex = input->GetFirstConnectedGamepadIndex();
	if (padIndex >= 0) {
		const Gamepad& pad = input->GetGamepad(padIndex);
		pressed = pressed || pad.TriggerButton(GamepadButton::kStart) || pad.TriggerButton(GamepadButton::kA);
	}
	return pressed;
}

bool IsBackPressed(Input* input) {
	bool pressed = input->GetKeyboard().TriggerKey(DIK_ESCAPE) || input->GetKeyboard().TriggerKey(DIK_BACK);

	const int padIndex = input->GetFirstConnectedGamepadIndex();
	if (padIndex >= 0) {
		pressed = pressed || input->GetGamepad(padIndex).TriggerButton(GamepadButton::kB);
	}
	return pressed;
}

std::unique_ptr<Sprite> CreateColorBox(AssetFactory* assetFactory, float x, float y, float width, float height, const Vector4& color) {
	std::unique_ptr<Sprite> sprite = assetFactory->CreateSprite(width, height);
	sprite->SetTextureHandle(assetFactory->GetWhiteTexture().gpuHandle);
	sprite->SetColor(color);
	sprite->GetTranslate() = Vector3{ x, y, 0.0f };
	return sprite;
}

} // namespace Shooter
