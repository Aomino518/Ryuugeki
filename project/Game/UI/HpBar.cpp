#include "HpBar.h"
#include "Color.h"
#include "TextureManager.h"
#include <algorithm>

void HpBar::Init(const Vector2& position, const Vector2& size, const Vector4& color)
{
	texId_ = TextureManager::GetInstance()->Load("resources/sprites/texWhite.png");
	size_ = size;
	background_ = std::make_unique<Sprite>();
	background_->Init();
	background_->Create(texId_, position, Color::BLACK, size);

	fill_ = std::make_unique<Sprite>();
	fill_->Init();
	fill_->Create(texId_, position, color, size);
}

void HpBar::Update(int currentHp, int maxHp)
{
	ratio_ = maxHp > 0 ? std::clamp(static_cast<float>(currentHp) / static_cast<float>(maxHp), 0.0f, 1.0f) : 0.0f;
	fill_->SetSize({ size_.x * ratio_, size_.y });

	background_->Update();
	fill_->Update();
}

void HpBar::Draw()
{
	if (!isVisible_) {
		return;
	}

	background_->Draw();

	if (ratio_ > 0.0f) {
		fill_->Draw();
	}
}

void HpBar::SetVisible(bool visible)
{
	isVisible_ = visible;
}
