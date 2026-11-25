#include "ParticleEffectMath.h"
#include "../Math/MathUtil.h"
#include <algorithm>
#include <cmath>

namespace Engine {

namespace {

Vector4 Lerp4(const Vector4& a, const Vector4& b, float t) {
	return Vector4{ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t };
}

// min と max の間の一様乱数（min > max でも動く）
float RandomRange(std::mt19937& random, float a, float b) {
	std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
	return a + (b - a) * uniform(random);
}

} // namespace

bool StepParticle(EffectParticle& particle, float deltaTime) {
	particle.age += deltaTime;
	if (particle.age >= particle.lifetime) {
		return false;
	}
	particle.velocity += particle.acceleration * deltaTime;
	if (particle.drag > 0.0f) {
		particle.velocity *= std::exp(-particle.drag * deltaTime); // フレームレートに依存しない減速
	}
	particle.position += particle.velocity * deltaTime;
	return true;
}

float ParticleProgress(const EffectParticle& particle) {
	if (particle.lifetime <= 0.0f) {
		return 1.0f;
	}
	return (std::clamp)(particle.age / particle.lifetime, 0.0f, 1.0f);
}

Vector4 ParticleColor(const EffectParticle& particle) {
	return Lerp4(particle.startColor, particle.endColor, ParticleProgress(particle));
}

float ParticleSize(const EffectParticle& particle) {
	const float t = ParticleProgress(particle);
	return particle.startSize + (particle.endSize - particle.startSize) * t;
}

Vector3 RandomDirectionInCone(std::mt19937& random, const Vector3& direction, float spreadAngle) {
	// 中心を +Z とみなして、円錐の中で選び、あとで中心の向きへ回す
	const float spread = (std::clamp)(spreadAngle, 0.0f, kPi);
	const float cosMax = std::cos(spread);
	const float cosTheta = RandomRange(random, cosMax, 1.0f); // cos を一様に選ぶと、立体角で一様になる
	const float sinTheta = std::sqrt((std::max)(0.0f, 1.0f - cosTheta * cosTheta));
	const float phi = RandomRange(random, 0.0f, kTwoPi);

	const Vector3 axis = Normalize(direction);
	if (axis == Vector3{ 0.0f, 0.0f, 0.0f }) {
		// 向きの指定がない: 全方向（球面上で一様）
		const float z = RandomRange(random, -1.0f, 1.0f);
		const float r = std::sqrt((std::max)(0.0f, 1.0f - z * z));
		return Vector3{ r * std::cos(phi), r * std::sin(phi), z };
	}

	const Vector3 u = Normalize(Perpendicular(axis));
	const Vector3 v = Cross(axis, u);
	return axis * cosTheta + (u * std::cos(phi) + v * std::sin(phi)) * sinTheta;
}

void GenerateBurst(const EffectBurst& burst, const Vector3& origin, std::mt19937& random, std::vector<EffectParticle>& out) {
	out.reserve(out.size() + burst.count);
	for (uint32_t i = 0; i < burst.count; ++i) {
		EffectParticle p;
		p.position = origin + Vector3{
			RandomRange(random, -burst.positionJitter.x, burst.positionJitter.x),
			RandomRange(random, -burst.positionJitter.y, burst.positionJitter.y),
			RandomRange(random, -burst.positionJitter.z, burst.positionJitter.z),
		};
		p.velocity = RandomDirectionInCone(random, burst.direction, burst.spreadAngle) * RandomRange(random, burst.speedMin, burst.speedMax);
		p.acceleration = burst.acceleration;
		p.drag = burst.drag;
		p.startColor = burst.startColor;
		p.endColor = burst.endColor;
		p.startSize = RandomRange(random, burst.sizeMin, burst.sizeMax);
		p.endSize = p.startSize * burst.endSizeRatio;
		p.lifetime = (std::max)(0.001f, RandomRange(random, burst.lifetimeMin, burst.lifetimeMax));
		p.age = 0.0f;
		out.push_back(p);
	}
}

} // namespace Engine
