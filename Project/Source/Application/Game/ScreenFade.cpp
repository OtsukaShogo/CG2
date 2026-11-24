#include "ScreenFade.h"
#include <algorithm>

namespace Shooter {

void ScreenFade::StartFadeIn(float duration) {
	alpha_ = 1.0f;
	duration_ = duration;
	if (duration <= 0.0f) {
		alpha_ = 0.0f;
		state_ = State::kIdle;
		return;
	}
	state_ = State::kFadingIn;
}

void ScreenFade::StartFadeOut(float duration) {
	duration_ = duration;
	if (duration <= 0.0f) {
		alpha_ = 1.0f;
		state_ = State::kFadedOut;
		return;
	}
	state_ = State::kFadingOut;
}

void ScreenFade::Update(float deltaTime) {
	if (state_ == State::kFadingIn) {
		alpha_ = (std::max)(0.0f, alpha_ - deltaTime / duration_);
		if (alpha_ <= 0.0f) {
			state_ = State::kIdle;
		}
	} else if (state_ == State::kFadingOut) {
		alpha_ = (std::min)(1.0f, alpha_ + deltaTime / duration_);
		if (alpha_ >= 1.0f) {
			state_ = State::kFadedOut;
		}
	}
}

} // namespace Shooter
