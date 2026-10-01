#include "DethParticle.h"

void DethParticle::Init()
{
	ParticleManager* particleMgr = ParticleManager::GetInstance();
	EmitterManager* emitterMgr = EmitterManager::GetInstance();

	texId_ = TextureManager::GetInstance()->Load("resources/sprites/circle.png");

	particleMgr->CreateParticleGroup(kHitEffectName_, texId_);

	// パーティクル設定
	ParticleConfig config;
    config.shape = SpawnShape::Sphere;
    config.sphereRadius = 0.5f;

    // 飛ぶ方向と速さ
    config.minVelocity = {
        -5.0f,
        -5.0f,
        -5.0f
    };

    config.maxVelocity = {
         5.0f,
         5.0f,
         5.0f
    };

    // 最初の回転
    config.minRotate = {
        0.0f,
        0.0f,
        0.0f
    };

    config.maxRotate = {
        0.0f,
        0.0f,
        6.28f
    };

    // 回転速度
    config.minRotateVelocity = {
        0.0f,
        0.0f,
        -3.0f
    };

    config.maxRotateVelocity = {
        0.0f,
        0.0f,
         3.0f
    };

    // 発生直後の色
    config.startColorMin = {
        1.0f,
        0.3f,
        0.0f,
        1.0f
    };

    config.startColorMax = {
        1.0f,
        1.0f,
        0.2f,
        1.0f
    };

    // 消える直前の色
    config.endColorMin = {
        0.3f,
        0.0f,
        0.0f,
        0.0f
    };

    config.endColorMax = {
        1.0f,
        0.0f,
        0.0f,
        0.0f
    };

    // 最初の大きさ
    config.startScaleMin = {
        0.2f,
        0.2f,
        0.2f
    };

    config.startScaleMax = {
        0.5f,
        0.5f,
        0.5f
    };

    // 終了時の大きさ
    config.endScaleMin = {
        0.0f,
        0.0f,
        0.0f
    };

    config.endScaleMax = {
        0.1f,
        0.1f,
        0.1f
    };

    // 生存時間
    config.minLifeTime = 0.3f;
    config.maxLifeTime = 0.8f;

	emitterMgr->CreateEmitter(kHitEffectName_, config, 30);
	particleMgr->SetUseBillboard(kHitEffectName_, true);
    particleMgr->SetBlendMode(kHitEffectName_, BlendMode::kBlendModeAdd);
}

void DethParticle::Update()
{
	CameraManager* cameraMgr = CameraManager::GetInstance();
	ParticleManager* particleMgr = ParticleManager::GetInstance();
	EmitterManager* emitterMgr = EmitterManager::GetInstance();

	emitterMgr->Update();
	particleMgr->Update(cameraMgr);
}

void DethParticle::Draw()
{
	ParticleManager* particleMgr = ParticleManager::GetInstance();
	particleMgr->Draw();
}

void DethParticle::SpawnHitEffect(const Vector3& position)
{
    ParticleEmitter* emitter = EmitterManager::GetInstance()->GetEmitter("HitEffect");

    Transform transform{};
    transform.translate = position;
    transform.rotate = { 0.0f, 0.0f, 0.0f };
    transform.scale = { 1.0f, 1.0f, 1.0f };
    emitter->SetTransform(transform);

    emitter->EmitOnce();
}
