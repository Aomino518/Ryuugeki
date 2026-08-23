#include "GameTime.h"
#include <algorithm>

Time::TimePoint Time::previousTime_{};

float Time::deltaTime_ = 0.0f;
float Time::unscaledDeltaTime_ = 0.0f;
float Time::elapsedTime_ = 0.0f;
float Time::timeScale_ = 1.0f;
float Time::fps_ = 0.0f;

void Time::Init()
{
	previousTime_ = Clock::now();

	deltaTime_ = 0.0f;
	unscaledDeltaTime_ = 0.0f;
	elapsedTime_ = 0.0f;
	timeScale_ = 1.0f;
	fps_ = 0.0f;
}

void Time::Update()
{
	const TimePoint currentTime = Clock::now();

	const std::chrono::duration<float> elapsed = currentTime - previousTime_;

	previousTime_ = currentTime;
	unscaledDeltaTime_ = elapsed.count();

	// ブレイクポイントやウィンドウ移動後に極端な時間がゲームに反映されるのを防ぐ
	constexpr float maxDeltaTime = 0.1f;
	unscaledDeltaTime_ = std::clamp(unscaledDeltaTime_, 0.0f, maxDeltaTime);

	deltaTime_ = unscaledDeltaTime_ * timeScale_;
	elapsedTime_ += unscaledDeltaTime_;

	if (unscaledDeltaTime_ > 0.0f) {
		fps_ = 1.0f / unscaledDeltaTime_;
	}
}

float Time::GetDeltaTime()
{
	return deltaTime_;
}

float Time::GetUnscaledDeltaTime()
{
	return unscaledDeltaTime_;
}

float Time::ElapsedTime()
{
	return elapsedTime_;
}

float Time::GetTimeScale()
{
	return timeScale_;
}

void Time::SetTimeScale(float timeScale)
{
	timeScale_ = timeScale;
}
