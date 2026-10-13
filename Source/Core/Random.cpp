#include "Random.h"

namespace Engine {

Random* Random::GetInstance() {
	static Random instance;
	return &instance;
}

// 実行の度に異なる結果になるよう、random_deviceで種を取得して初期化する
Random::Random() {
	std::random_device seedGenerator;
	engine_.seed(seedGenerator());
}

float Random::GetFloat(float min, float max) {
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(engine_);
}

Vector3 Random::GetVector3(const Vector3& min, const Vector3& max) {
	return {
		GetFloat(min.x, max.x),
		GetFloat(min.y, max.y),
		GetFloat(min.z, max.z),
	};
}

} // namespace Engine
