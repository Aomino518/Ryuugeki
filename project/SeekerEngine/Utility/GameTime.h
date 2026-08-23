#pragma once
#include <chrono>

class Time
{
public:
	// エンジン起動時に呼び出し
	static void Init();

	// 毎フレームの先頭呼び出し
	static void Update();

	/// <summary>
	/// TimeScale適用後のデルタタイムを返す関数
	/// </summary>
	/// <returns>デルタタイム</returns>
	static float GetDeltaTime();

	/// <summary>
	/// TimeScale適用しないデルタタイムを返す関数
	/// </summary>
	/// <returns>デルタタイム</returns>
	static float GetUnscaledDeltaTime();

	/// <summary>
	/// エンジン起動後の経過時間
	/// </summary>
	/// <returns>経過時間</returns>
	static float ElapsedTime();

	/// <summary>
	/// FPS
	/// </summary>
	/// <returns>フレームレート</returns>
	static float GetFPS();

	// ゲーム全体の時間倍率
	static float GetTimeScale();
	static void SetTimeScale(float timeScale);

private:
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;

	static TimePoint previousTime_;

	static float deltaTime_;
	static float unscaledDeltaTime_;
	static float elapsedTime_;
	static float timeScale_;
	static float fps_;
};

