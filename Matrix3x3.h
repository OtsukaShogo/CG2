#pragma once

// HLSL cbuffer では float3x3 の各行が float4（16byte）にパディングされる
// C++ 側も float[3][4] にして cbuffer レイアウトを合わせる
struct Matrix3x3 {
	float m[3][4] = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f }
	};
};