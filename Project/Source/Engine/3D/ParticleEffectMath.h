#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include "../Math/Vector3.h"
#include "../Math/Vector4.h"

namespace Engine {

/// <summary>
/// 演出用の粒1つ分の状態（火花・煙・軌跡など）。D3D12に依存しない（計算だけの）データ。
/// 色と大きさは、寿命の進み具合に応じて、開始の値から終了の値へ、直線的に変化する
/// </summary>
struct EffectParticle {
	Vector3 position = { 0.0f, 0.0f, 0.0f };
	Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	Vector3 acceleration = { 0.0f, 0.0f, 0.0f }; //!< 毎秒の速度の変化（重力など）
	float drag = 0.0f;                           //!< 空気抵抗（毎秒、速度が e^-drag 倍になる。0なら減速しない）
	Vector4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 endColor = { 1.0f, 1.0f, 1.0f, 0.0f };
	float startSize = 0.2f;                      //!< 画面上の大きさ（ワールドの長さ）
	float endSize = 0.0f;
	float lifetime = 0.5f;                       //!< 寿命（秒）
	float age = 0.0f;                            //!< 生まれてからの時間（秒）
};

/// <summary>
/// 粒を一度にまとめて出す（爆発・火花・被弾）ときの設定
/// </summary>
struct EffectBurst {
	uint32_t count = 10;
	float speedMin = 2.0f;
	float speedMax = 6.0f;
	float lifetimeMin = 0.3f;
	float lifetimeMax = 0.6f;
	float sizeMin = 0.1f;
	float sizeMax = 0.2f;
	float endSizeRatio = 0.0f;                    //!< 終了時の大きさ（開始時に対する割合）
	Vector4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 endColor = { 1.0f, 1.0f, 1.0f, 0.0f };
	Vector3 direction = { 0.0f, 1.0f, 0.0f };     //!< 飛び散る中心の向き（長さ1でなくてよい）
	float spreadAngle = 3.14159265f;              //!< 中心の向きからの広がり（ラジアン。π で全方向、0 で一直線）
	Vector3 positionJitter = { 0.0f, 0.0f, 0.0f }; //!< 出現位置のばらつき（各軸の最大のずれ）
	Vector3 acceleration = { 0.0f, 0.0f, 0.0f };  //!< 全員にかかる加速度（重力など）
	float drag = 0.0f;
};

/// <summary>
/// 粒を、経過時間ぶん進める（位置・速度・年齢）。寿命を超えたら false（消してよい）を返す
/// </summary>
[[nodiscard]] bool StepParticle(EffectParticle& particle, float deltaTime);

/// <summary>
/// 粒の寿命の進み具合（0〜1）
/// </summary>
[[nodiscard]] float ParticleProgress(const EffectParticle& particle);

/// <summary>
/// いまの色（開始と終了の間を、進み具合で補間する）
/// </summary>
[[nodiscard]] Vector4 ParticleColor(const EffectParticle& particle);

/// <summary>
/// いまの大きさ（開始と終了の間を、進み具合で補間する）
/// </summary>
[[nodiscard]] float ParticleSize(const EffectParticle& particle);

/// <summary>
/// 中心の向き direction から spreadAngle 以内の、ランダムな向き（長さ1）を選ぶ（立体角で一様）。
/// direction がゼロベクトルなら、全方向から選ぶ
/// </summary>
[[nodiscard]] Vector3 RandomDirectionInCone(std::mt19937& random, const Vector3& direction, float spreadAngle);

/// <summary>
/// 設定に従って、粒を burst.count 個作り、out の末尾へ追加する。origin が出現位置の基準になる
/// </summary>
void GenerateBurst(const EffectBurst& burst, const Vector3& origin, std::mt19937& random, std::vector<EffectParticle>& out);

} // namespace Engine
