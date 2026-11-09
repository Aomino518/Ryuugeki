#include "Material.h"
#include "Logger.h"
#include "Graphics.h"

void Material::Init()
{
	cmdList_ = Graphics::GetInstance()->GetCmdList();
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(MaterialConstants));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->uvTransform = MakeIdentity4x4();
	materialData_->enableLighting = true;
	materialData_->shininess = 32.0f;
	materialData_->specularStrength = 0.05f;
	Logger::Write("MaterialResourceの作成完了");
}

void Material::Draw(bool globalLightingEnabled)
{
	materialData_->enableLighting = (globalLightingEnabled && isLighting_) ? 1u : 0u;
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定する。
	cmdList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
}
