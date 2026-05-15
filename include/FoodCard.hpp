//
// Created by m0938 on 2026/4/15.
//

#ifndef STACKLANDS_FOODCARD_HPP
#define STACKLANDS_FOODCARD_HPP

#pragma once
#include <string>
#include "Card.hpp"

class FoodCard : public Card {
public:
    FoodCard(float x, float y, const std::string& name, int sellValue,
             const std::string& iconPath, int nutritionValue = 1, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::FOOD, scale),
          m_NutritionValue(nutritionValue)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Food.png");
        SetIconImage(iconPath);

        m_PriceText     = InitLabelText(std::to_string(sellValue),      Util::Color(100, 111, 128));
        m_NutritionText = InitLabelText(std::to_string(nutritionValue),  Util::Color(255, 160,  60));

        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return true;
    }

    int GetNutritionValue() const { return m_NutritionValue; }

    // 扣除 nutrition；回傳實際扣掉的量（可能 < amount，因為不會扣到負）。
    // 扣至 0 後卡片應由 CardManager 移除。
    int ConsumeNutrition(int amount) {
        int taken = std::min(amount, m_NutritionValue);
        m_NutritionValue -= taken;
        if (m_NutritionText)
            RebuildLabelText(m_NutritionText, std::to_string(m_NutritionValue),
                             Util::Color(255, 160, 60));
        return taken;
    }

    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_PriceText)     RebuildLabelText(m_PriceText,     std::to_string(m_SellValue),       Util::Color(100, 111, 128));
        if (m_NutritionText) RebuildLabelText(m_NutritionText, std::to_string(m_NutritionValue),  Util::Color(255, 160,  60));
    }

    void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            // 左下角：售價
            m_PriceText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::PRICE_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
        if (m_NutritionText) {
            // 右下角：與售價對稱
            m_NutritionText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::SECONDARY_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_NutritionText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText)     m_PriceText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
        if (m_NutritionText) m_NutritionText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText)     m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        if (m_NutritionText) m_NutritionText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText)     objs.push_back(m_PriceText);
        if (m_NutritionText) objs.push_back(m_NutritionText);
        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_PriceText;
    std::shared_ptr<Util::GameObject> m_NutritionText;
    int m_NutritionValue;
};

#endif //STACKLANDS_FOODCARD_HPP
