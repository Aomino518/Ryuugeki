#pragma once
#include <cstdint>
#include "Vector3.h"
#include <string>

class JetEngineParticle
{
public:
	void Init(const std::string& name);

	void Update(Vector3& position);

	void Draw();

	void Unload();

	void SetOffset(Vector3& offset) { offset_ = offset; }
private:
	uint32_t texId_;
	std::string name_;
	Vector3 offset_ = { 0.0f, 0.0f, 0.0f };
};