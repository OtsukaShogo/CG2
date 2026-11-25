#include "TestFramework.h"
#include "Source/Engine/3D/ParticleEffectMath.h"
#include "Source/Engine/Math/MathUtil.h"

#include <cmath>
#include <random>
#include <vector>

using namespace Engine;

namespace {

Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
Vector4 V4(float x, float y, float z, float w) { return Vector4{ x, y, z, w }; }

}

TEST_CASE(ParticleEffect_StepMovesAndAges) {
	EffectParticle p;
	p.velocity = V3(2, 0, 0);
	p.lifetime = 1.0f;
	CHECK(StepParticle(p, 0.5f));
	CHECK_NEAR(p.position.x, 1.0, 1e-5);
	CHECK_NEAR(p.age, 0.5, 1e-6);
	CHECK(StepParticle(p, 0.25f));
	CHECK_NEAR(p.position.x, 1.5, 1e-5);
}

TEST_CASE(ParticleEffect_StepDiesAtLifetime) {
	EffectParticle p;
	p.lifetime = 0.3f;
	CHECK(StepParticle(p, 0.2f));
	CHECK(!StepParticle(p, 0.2f));  // 寿命を超えたら、消してよい
	EffectParticle exact;
	exact.lifetime = 0.5f;
	CHECK(!StepParticle(exact, 0.5f)); // ちょうどでも、消す
}

TEST_CASE(ParticleEffect_AccelerationAppliesGravity) {
	EffectParticle p;
	p.acceleration = V3(0, -10, 0);
	p.lifetime = 5.0f;
	for (int i = 0; i < 60; ++i) (void)StepParticle(p, 1.0f / 60.0f);
	CHECK_NEAR(p.velocity.y, -10.0, 1e-3); // 1秒で、速度が -10
	CHECK(p.position.y < 0.0f);
}

TEST_CASE(ParticleEffect_DragIsFrameRateIndependent) {
	// 1秒後の速度が、フレームレートによらず、ほぼ同じ（e^-drag 倍）
	for (float dt : { 1.0f / 30.0f, 1.0f / 60.0f, 1.0f / 144.0f }) {
		EffectParticle p;
		p.velocity = V3(10, 0, 0);
		p.drag = 2.0f;
		p.lifetime = 10.0f;
		const int frames = static_cast<int>(1.0f / dt + 0.5f);
		for (int i = 0; i < frames; ++i) (void)StepParticle(p, dt);
		CHECK_NEAR(p.velocity.x, 10.0 * std::exp(-2.0), 0.05);
	}
}

TEST_CASE(ParticleEffect_ColorAndSizeInterpolate) {
	EffectParticle p;
	p.startColor = V4(1, 0, 0, 1);
	p.endColor = V4(0, 0, 1, 0);
	p.startSize = 1.0f;
	p.endSize = 0.0f;
	p.lifetime = 1.0f;
	CHECK(NearlyEqual(ParticleColor(p).x, 1.0f));
	CHECK_NEAR(ParticleSize(p), 1.0, 1e-6);
	p.age = 0.5f;
	CHECK_NEAR(ParticleColor(p).x, 0.5, 1e-6);
	CHECK_NEAR(ParticleColor(p).z, 0.5, 1e-6);
	CHECK_NEAR(ParticleColor(p).w, 0.5, 1e-6); // 透明度も補間する
	CHECK_NEAR(ParticleSize(p), 0.5, 1e-6);
	p.age = 5.0f;                               // 寿命を超えても、終了の値のまま
	CHECK_NEAR(ParticleProgress(p), 1.0, 1e-9);
	CHECK_NEAR(ParticleSize(p), 0.0, 1e-6);
	EffectParticle zero;
	zero.lifetime = 0.0f;                       // 寿命0: 0除算にならない
	CHECK_NEAR(ParticleProgress(zero), 1.0, 1e-9);
}

TEST_CASE(ParticleEffect_RandomDirectionIsUnitAndInsideCone) {
	std::mt19937 random(1);
	const Vector3 axis = Normalize(V3(1, 2, -0.5f));
	const float spread = ToRadians(30.0f);
	int outside = 0, notUnit = 0;
	float maxAngle = 0.0f;
	for (int i = 0; i < 3000; ++i) {
		const Vector3 d = RandomDirectionInCone(random, axis, spread);
		notUnit += std::abs(Length(d) - 1.0f) > 1e-4f;
		const float angle = std::acos((std::clamp)(Dot(d, axis), -1.0f, 1.0f));
		maxAngle = (std::max)(maxAngle, angle);
		outside += angle > spread + 1e-3f;
	}
	CHECK(notUnit == 0);
	CHECK(outside == 0);
	CHECK(maxAngle > spread * 0.9f); // 円錐の縁の近くまで、実際に広がる
}

TEST_CASE(ParticleEffect_RandomDirectionFullSphereCoversAllSides) {
	std::mt19937 random(2);
	int up = 0, down = 0, left = 0, right = 0;
	for (int i = 0; i < 4000; ++i) {
		const Vector3 d = RandomDirectionInCone(random, V3(0, 1, 0), kPi);
		up += d.y > 0.5f; down += d.y < -0.5f; left += d.x < -0.5f; right += d.x > 0.5f;
	}
	CHECK(up > 400 && down > 400 && left > 400 && right > 400); // 全方向に出る
	// 向きの指定がなければ、全方向
	int upward = 0;
	for (int i = 0; i < 2000; ++i) upward += RandomDirectionInCone(random, V3(0, 0, 0), 1.0f).y > 0.0f;
	CHECK(upward > 800 && upward < 1200);
}

TEST_CASE(ParticleEffect_RandomDirectionZeroSpreadIsStraight) {
	std::mt19937 random(3);
	const Vector3 axis = Normalize(V3(0, 0, 1));
	bool straight = true;
	for (int i = 0; i < 100; ++i) {
		straight = straight && NearlyEqual(RandomDirectionInCone(random, axis, 0.0f), axis, 1e-3f);
	}
	CHECK(straight);
}

TEST_CASE(ParticleEffect_GenerateBurstFollowsSettings) {
	std::mt19937 random(4);
	EffectBurst burst;
	burst.count = 50;
	burst.speedMin = 3.0f;
	burst.speedMax = 5.0f;
	burst.lifetimeMin = 0.2f;
	burst.lifetimeMax = 0.4f;
	burst.sizeMin = 0.1f;
	burst.sizeMax = 0.3f;
	burst.endSizeRatio = 0.5f;
	burst.direction = V3(0, 0, 1);
	burst.spreadAngle = ToRadians(20.0f);
	burst.positionJitter = V3(0.1f, 0.0f, 0.1f);
	burst.acceleration = V3(0, -9, 0);
	burst.drag = 1.5f;

	std::vector<EffectParticle> out;
	GenerateBurst(burst, V3(10, 1, 5), random, out);
	CHECK(out.size() == 50);

	int bad = 0;
	for (const EffectParticle& p : out) {
		const float speed = Length(p.velocity);
		bad += speed < 3.0f - 1e-3f || speed > 5.0f + 1e-3f;
		bad += p.lifetime < 0.2f - 1e-5f || p.lifetime > 0.4f + 1e-5f;
		bad += p.startSize < 0.1f - 1e-5f || p.startSize > 0.3f + 1e-5f;
		bad += std::abs(p.endSize - p.startSize * 0.5f) > 1e-5f;
		bad += std::abs(p.position.x - 10.0f) > 0.1f + 1e-5f || p.position.y != 1.0f || std::abs(p.position.z - 5.0f) > 0.1f + 1e-5f;
		bad += !(p.acceleration == V3(0, -9, 0)) || p.drag != 1.5f || p.age != 0.0f;
		bad += Dot(Normalize(p.velocity), V3(0, 0, 1)) < std::cos(ToRadians(20.0f)) - 1e-3f; // 円錐の内側
	}
	CHECK(bad == 0);

	// 追加は、末尾へ（すでにあるものを壊さない）
	GenerateBurst(burst, V3(0, 0, 0), random, out);
	CHECK(out.size() == 100);
}

TEST_CASE(ParticleEffect_GenerateBurstIsReproducible) {
	EffectBurst burst;
	burst.count = 5;
	std::vector<EffectParticle> a, b;
	std::mt19937 r1(77), r2(77);
	GenerateBurst(burst, V3(0, 0, 0), r1, a);
	GenerateBurst(burst, V3(0, 0, 0), r2, b);
	bool same = a.size() == b.size();
	for (size_t i = 0; same && i < a.size(); ++i) same = a[i].velocity == b[i].velocity && a[i].lifetime == b[i].lifetime;
	CHECK(same); // 同じ種なら、同じ結果
}

TEST_CASE(ParticleEffect_GenerateBurstZeroCountAndReversedRanges) {
	std::mt19937 random(5);
	std::vector<EffectParticle> out;
	EffectBurst none;
	none.count = 0;
	GenerateBurst(none, V3(0, 0, 0), random, out);
	CHECK(out.empty());

	EffectBurst reversed;           // min > max でも、範囲内に収まり、寿命が0にならない
	reversed.count = 20;
	reversed.speedMin = 5.0f;
	reversed.speedMax = 2.0f;
	reversed.lifetimeMin = 0.0f;
	reversed.lifetimeMax = 0.0f;
	GenerateBurst(reversed, V3(0, 0, 0), random, out);
	bool ok = true;
	for (const EffectParticle& p : out) {
		ok = ok && Length(p.velocity) >= 2.0f - 1e-3f && Length(p.velocity) <= 5.0f + 1e-3f && p.lifetime > 0.0f;
	}
	CHECK(ok);
}
