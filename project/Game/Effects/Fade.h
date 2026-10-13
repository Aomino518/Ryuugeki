#pragma once
#include <cstdint>
#include <memory>
#include "Sprite.h"

class Fade {
public:
	// フェードの状態
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};

	void Init();

	void Update();

	void Draw();

	// フェード開始
	void Start(Status status, float duration);

	// フェード停止
	void Stop();

	// フェード終了判定
	bool IsFinished() const;

private:
	void Applay();
	uint32_t tex_;

	// 現在のフェードの状態
	Status status_ = Status::None;

	// フェードの持続時間
	float duration_ = 0.0f;
	// 経過時間カウンター
	float elapsed_ = 0.0f;

	static constexpr int kColumns = 16;
	static constexpr int kRows = 9;
	static constexpr float kCell = 86.0f;

	std::array<std::unique_ptr<Sprite>, kColumns * kRows> squares_;
};

