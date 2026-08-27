#pragma once
#include "SceneIncludes.h"

class Boss
{
public:
	void Init(const Vector3& position, const Vector3& playerPosition);

	void Update(const Vector3& playerPosition);

	void Draw();

	void DrawDebug();

	const Vector3& GetPosition() const
	{
		return position_;
	}

private:
	Vector3 position_{};

	// 左右移動の基準位置
	float centerOffsetX_ = 0.0f;

	// プレイヤーとのZ方向の距離
	float distanceFromPlayer_ = 80.0f;

	// プレイヤーとの高さの差
	float heightOffset_ = 0.0f;

	// 左右へ動く幅
	float moveAmplitude_ = 20.0f;

	// 左右移動の速さ
	float moveSpeed_ = 1.5f;

	// sinへ渡す時間
	float moveTime_ = 0.0f;

	bool isAlive_ = false;
};

