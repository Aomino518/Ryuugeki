#pragma once
#include "SceneIncludes.h"
#include "Vector3.h"

class DethParticle
{
public:
	void Init();

	void Update();

	void Draw();

	void SpawnHitEffect(const Vector3& position);

private:
	uint32_t texId_;
	const char* kHitEffectName_ = "HitEffect";
};

