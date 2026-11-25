#include "JetEngineParticle.h"
#include "CameraManager.h"
#include "EmitterManager.h"
#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include "TextureManager.h"
#include "Editor.h"

void JetEngineParticle::Init(const std::string& name)
{
	name_ = name;
	ParticleManager* particleMgr = ParticleManager::GetInstance();
	EmitterManager* emitterMgr = EmitterManager::GetInstance();

	texId_ = TextureManager::GetInstance()->Load("resources/sprites/circle.png");
	particleMgr->CreateParticleGroup(name_, texId_);

	// パーティクル設定
	ParticleConfig config;
	emitterMgr->CreateEmitter(name_, config, 30);
	particleMgr->SetUseBillboard(name_, true);
	particleMgr->SetBlendMode(name_, BlendMode::kBlendModeAdd);
	Editor::GetInstance()->RegisterParticle(name_);
}

void JetEngineParticle::Update(Vector3& position)
{
	Vector3 pos = position + offset_;
	EmitterManager::GetInstance()->GetEmitter(name_)->SetTranslate(pos);
}

void JetEngineParticle::Draw()
{
	ParticleManager::GetInstance()->Draw();
}

void JetEngineParticle::Unload()
{
	EmitterManager::GetInstance()->RemoveEmitter(name_);
	ParticleManager::GetInstance()->RemoveParticleGroup(name_);
}
