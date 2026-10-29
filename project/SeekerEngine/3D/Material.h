#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "CreateResorceUtils.h"

class Material
{
public:
	void Init();
	void Draw(bool globalLightingEnabled);

	Vector4& GetMaterial() const { return materialData_->color; }
	bool GetIsLighting() const { return isLighting_; }
	float GetSpecularStrength() const { return materialData_->specularStrength; }

	void SetMaterial(const Vector4& material) { this->materialData_->color = material; }
	void SetIsLighting(const bool isLighting) { isLighting_ = isLighting; }
	void SetSpecularStrength(float strength) { this->materialData_->specularStrength = std::clamp(strength, 0.0f, 1.0f); }

private:
	// Preserve the local setting when global lighting is disabled.
	bool isLighting_ = true;
	// マテリアルデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	MaterialConstants* materialData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};

