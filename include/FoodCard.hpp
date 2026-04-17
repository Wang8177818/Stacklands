//
// Created by m0938 on 2026/4/15.
//

#ifndef STACKLANDS_FOODCARD_HPP
#define STACKLANDS_FOODCARD_HPP

#pragma once
#include <string>
#include <algorithm>
#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Image.hpp"

class FoodCard : public Card {
public:
    FoodCard(float x, float y, const std::string& name, int sellValue,
             const std::string& iconPath, int nutritionValue = 1, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::FOOD, scale),
          m_NutritionValue(nutritionValue)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Food.png");
        SetIconImage(iconPath);

        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));

        m_PriceText = std::make_shared<Util::GameObject>();
        m_PriceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(sellValue), Util::Color(100, 111, 128)));
        m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);

        m_NutritionText = std::make_shared<Util::GameObject>();
        m_NutritionText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(nutritionValue), Util::Color(255, 160, 60)));
        m_NutritionText->m_Transform.scale = {m_Scale, m_Scale};
        m_NutritionText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return true;
    }

    int GetNutritionValue() const { return m_NutritionValue; }

    void SetScale(float scale) override {
        Card::SetScale(scale);
        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        if (m_PriceText) {
            m_PriceText->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjh.ttc", fontSize,
                std::to_string(m_SellValue), Util::Color(100, 111, 128)));
            m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        }
        if (m_NutritionText) {
            m_NutritionText->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjh.ttc", fontSize,
                std::to_string(m_NutritionValue), Util::Color(255, 160, 60)));
            m_NutritionText->m_Transform.scale = {m_Scale, m_Scale};
        }
    }

    void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            // 左下角：與其他卡片售價位置一致
            m_PriceText->m_Transform.translation =
                glm::vec2(m_X + m_Width * -0.32f, m_Y + m_Height * -0.3f);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
        if (m_NutritionText) {
            // 右下角：與售價對稱
            m_NutritionText->m_Transform.translation =
                glm::vec2(m_X + m_Width * 0.32f, m_Y + m_Height * -0.3f);
            m_NutritionText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText)     m_PriceText->SetZIndex(42);
        if (m_NutritionText) m_NutritionText->SetZIndex(42);
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
