#pragma once
#include <DirectXCollision.h>
#include <cmath>
#include <cstring>
#include "Matrix4x4.h"

namespace Picking {

inline DirectX::XMMATRIX LoadMatrix(const Matrix4x4& matrix)
{
	// 自作のMatrix4x4行列をXMFLOAT4X4に変換する
	DirectX::XMFLOAT4X4 values;
#ifdef _DEBUG
	static_assert(sizeof(values) == sizeof(matrix), "Picking::LoadMatrix : 型のサイズが不一致");
#endif
	// 内容をvaluesにコピー
	std::memcpy(&values, &matrix, sizeof(values));
	return DirectX::XMLoadFloat4x4(&values);
}

// クリック判定用の半直線を保持する構造体
struct LocalRay {
	DirectX::XMVECTOR origin;
	DirectX::XMVECTOR direction;
	float length;
};

// モデルのローカル座標でRayを作る関数
inline bool MakeLocalRay(float u, float v, DirectX::FXMMATRIX wvp, LocalRay& ray)
{
	using namespace DirectX;
	// 画面内の位置か確認
	if (!(u >= 0.0f && u < 1.0f && v >= 0.0f && v < 1.0f)) {
		return false;
	}

	// 座標変換を逆向きに行う行列を作る
	XMVECTOR determinant;
	const XMMATRIX inverse = XMMatrixInverse(&determinant, wvp);
	const float det = XMVectorGetX(determinant);
	if (!std::isfinite(det) || det == 0.0f) {
		return false;
	}

	// 画面の位置を変換に使う座標に直す
	const float x = u * 2.0f - 1.0f;
	const float y = 1.0f - v * 2.0f;
	ray.origin = XMVector3TransformCoord(XMVectorSet(x, y, 0.0f, 1.0f), inverse);
	// 手前と奥の二点をローカル座標に戻す
	const XMVECTOR farPoint = XMVector3TransformCoord(XMVectorSet(x, y, 1.0f, 1.0f), inverse);
	// 二点の差から、向きと長さを求める
	const XMVECTOR delta = XMVectorSubtract(farPoint, ray.origin);
	ray.length = XMVectorGetX(XMVector3Length(delta));
	// 有効なRayか確認、無限やNaNなどになっていないか
	if (!std::isfinite(ray.length) || ray.length <= 0.0f ||
		XMVector3IsNaN(ray.origin) || XMVector3IsInfinite(ray.origin)) {
		return false;
	}
	// 正規化
	ray.direction = XMVectorScale(delta, 1.0f / ray.length);
	return true;
}

inline float HitDepth(const LocalRay& ray, float distance, DirectX::FXMMATRIX wvp)
{
	using namespace DirectX;
	// 当たった深度を計算
	const XMVECTOR hit = XMVectorAdd(ray.origin, XMVectorScale(ray.direction, distance));
	return XMVectorGetZ(XMVector3TransformCoord(hit, wvp));
}

template<class Vertices, class Indices>
bool IntersectMesh(const LocalRay& ray, const DirectX::BoundingBox& bounds,
	const Vertices& vertices, const Indices& indices, float& distance)
{
	using namespace DirectX;
	float boundsDistance = 0.0f;
	// model頂点がないかrayが囲む領域にboundsがない
	if (vertices.empty() || !bounds.Intersects(ray.origin, ray.direction, boundsDistance)) {
		return false;
	}

	// 有効な交点は見つかったか
	bool found = false;
	// 距離はrayの長さに
	distance = ray.length;

	// 頂点座標を取り出すラムダ式関数
	const auto position = [&vertices](size_t index) {
		const auto& p = vertices[index].position;
		return XMVectorSet(p.x, p.y, p.z, 1.0f);
	};

	// 三角形ごとに交差を調べる
	for (size_t i = 0; i + 2 < indices.size(); i += 3) {
		float hitDistance = 0.0f;

		if (TriangleTests::Intersects(ray.origin, ray.direction,
			position(indices[i]), position(indices[i + 1]), position(indices[i + 2]), hitDistance) &&
			hitDistance <= distance) {
			distance = hitDistance;
			found = true;
		}
	}
	return found;
}
}
