//
// Created by m0938 on 2026/5/22.
//

#ifndef STACKLANDS_IDEACARD_HPP
#define STACKLANDS_IDEACARD_HPP

#pragma once
#include "Card.hpp"
#include "GameConstants.hpp"

class IdeaCard : public Card {
public:
    IdeaCard(float x, float y, const std::string& name, int sellValue,
             float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::IDEA, scale)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Idea.png");
        m_PriceText = InitLabelText(std::to_string(sellValue), Util::Color(100, 111, 128));
        UpdateVisualPositions();
    }

    void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_PriceText) {
            m_PriceText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::PRICE_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText) m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_PriceText;
};

#endif // STACKLANDS_IDEACARD_HPP
