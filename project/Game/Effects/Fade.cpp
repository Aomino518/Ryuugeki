#include "Fade.h"
#include <algorithm>
#include "Application.h"
#include "Color.h"
#include "TextureManager.h"
#include "GameTime.h"

void Fade::Init() {
	// テクスチャ
	tex_ = TextureManager::GetInstance()->Load("resources/sprites/texWhite.png");

	// スプライト
	for (int y = 0; y < kRows; ++y) {
		for (int x = 0; x < kColumns; ++x) {
			auto& sprite = squares_[y * kColumns + x];
			sprite = std::make_unique<Sprite>();
			sprite->Create(
				tex_,
				{ static_cast<float>(x * kCell),  static_cast<float>(y * kCell) },
				Color::BLACK,
				{ kCell, kCell });

			// 四角の中心を基準に拡大
			sprite->SetAnchorPoint({ 0.5f, 0.5f });
			sprite->SetBlendMode(kBlendModeNormal);
		}
	}

	Applay();
}

void Fade::Update() {
	if (status_ == Status::None) {
		return;
	}

	elapsed_ += Time::GetDeltaTime();
	if (elapsed_ >= duration_) {
		elapsed_ = duration_;
	}

	Applay();
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}

	for (auto& sprite : squares_) {
		// サイズ0の四角は描画しない
		if (sprite->GetSize().x > 0.0f) {
			sprite->Draw();
		}
	}
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	elapsed_ = 0.0f;
	Applay();
}

void Fade::Stop() {
	status_ = Status::None;
}

bool Fade::IsFinished() const {
	return status_ == Status::None || elapsed_ >= duration_;
}

void Fade::Applay()
{
	float t = elapsed_ / duration_;

	// FadeInの場合は逆再生
	if (status_ == Status::FadeIn) {
		t = 1.0f - t;
	}

	for (int y = 0; y < kRows; ++y) {
		for (int x = 0; x < kColumns; x++) {
			const float order = float(x + y) / float(kColumns + kRows - 2);
			const float delay = order * 0.6f;
			float p = std::clamp((t - delay) / 0.4f, 0.0f, 1.0f);

			p = p * p * (3.0f - 2.0f * p);

			const float size = (kCell + 1.0f) * p;
			auto& sprite = squares_[y * kColumns + x];
			sprite->SetSize({ size, size });
			sprite->Update();
		}
	}
}
