//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_RESOURCECARD_HPP
#define STACKLANDS_RESOURCECARD_HPP

#pragma once
#include "Card.hpp"

class ResourceCard : public Card {
public:
    ResourceCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::RESOURCE, scale) {

        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Resource.png");
        SetIconImage(iconPath);

        m_PriceText = InitLabelText(std::to_string(sellValue), Util::Color(100, 111, 128));

        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return true;
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            m_PriceText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::PRICE_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText) m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

protected:
    std::shared_ptr<Util::GameObject> m_PriceText;
};

#endif //STACKLANDS_RESOURCECARD_HPP
