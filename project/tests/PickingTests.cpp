#include "Picking.h"
#include <array>
#include <cassert>
#include <iostream>
#include <limits>

using namespace DirectX;
struct Vertex { XMFLOAT3 position; };

int main()
{
	const std::array<Vertex, 3> vertices = {{{{-1, -1, 0}}, {{0, 1, 0}}, {{1, -1, 0}}}};
	const std::array<unsigned, 3> indices = {0, 1, 2};
	const BoundingBox bounds({0, 0, 0}, {1, 1, 0});
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(1.0f, 16.0f / 9.0f, 0.1f, 100.0f);
	const XMMATRIX world = XMMatrixScaling(-2, 3, 0.5f) * XMMatrixRotationY(0.4f) * XMMatrixTranslation(0, 0, 5);
	const XMMATRIX wvp = world * projection;
	Picking::LocalRay ray;
	float distance;
	assert(Picking::MakeLocalRay(0.5f, 0.5f, wvp, ray));
	assert(Picking::IntersectMesh(ray, bounds, vertices, indices, distance));
	const float nearDepth = Picking::HitDepth(ray, distance, wvp);
	assert(nearDepth > 0 && nearDepth < 1);
	const XMMATRIX farther = world * XMMatrixTranslation(0, 0, 5) * projection;
	assert(Picking::MakeLocalRay(0.5f, 0.5f, farther, ray));
	assert(Picking::IntersectMesh(ray, bounds, vertices, indices, distance));
	assert(Picking::HitDepth(ray, distance, farther) > nearDepth);
	// A moved/rotated camera and an off-center click must hit the same point.
	const XMMATRIX view = XMMatrixLookAtLH(XMVectorSet(2, 1, -3, 1),
		XMVectorSet(0, 0, 4, 1), XMVectorSet(0, 1, 0, 0));
	const XMMATRIX movedCamera = world * view * projection;
	const XMVECTOR projected = XMVector3TransformCoord(XMVectorZero(), movedCamera);
	assert(Picking::MakeLocalRay((XMVectorGetX(projected) + 1) * 0.5f,
		(1 - XMVectorGetY(projected)) * 0.5f, movedCamera, ray));
	assert(Picking::IntersectMesh(ray, bounds, vertices, indices, distance));
	const XMVECTOR localHit = XMVectorAdd(ray.origin, XMVectorScale(ray.direction, distance));
	assert(XMVectorGetX(XMVector3Length(localHit)) < 0.001f);

	// AABB hit in a triangle's empty corner must not select the mesh.
	ray = {XMVectorSet(0.9f, 0.9f, -1, 1), XMVectorSet(0, 0, 1, 0), 10};
	assert(bounds.Intersects(ray.origin, ray.direction, distance));
	assert(!Picking::IntersectMesh(ray, bounds, vertices, indices, distance));
	ray = {XMVectorSet(0, 0, -1, 1), XMVectorSet(0, 0, 1, 0), 0.5f};
	assert(!Picking::IntersectMesh(ray, bounds, vertices, indices, distance));
	ray.origin = XMVectorSet(0, 0, 1, 1);
	ray.length = 10;
	assert(!Picking::IntersectMesh(ray, bounds, vertices, indices, distance));

	assert(!Picking::MakeLocalRay(-0.1f, 0.5f, wvp, ray));
	assert(!Picking::MakeLocalRay(1, 0.5f, wvp, ray));
	assert(!Picking::MakeLocalRay(0.5f, 1, wvp, ray));
	assert(!Picking::MakeLocalRay(std::numeric_limits<float>::quiet_NaN(), 0.5f, wvp, ray));
	assert(!Picking::MakeLocalRay(0.5f, 0.5f, XMMatrixScaling(0, 1, 1), ray));

	// Screen Y grows downward; orthographic projection and matrix conversion.
	XMFLOAT4X4 stored;
	XMStoreFloat4x4(&stored, XMMatrixOrthographicLH(4, 4, 0.1f, 10));
	Matrix4x4 engineMatrix;
	std::memcpy(&engineMatrix, &stored, sizeof(stored));
	assert(Picking::MakeLocalRay(0.25f, 0.25f, Picking::LoadMatrix(engineMatrix), ray));
	assert(std::abs(XMVectorGetX(ray.origin) + 1) < 0.001f);
	assert(std::abs(XMVectorGetY(ray.origin) - 1) < 0.001f);
	std::cout << "Picking tests passed\n";
}
