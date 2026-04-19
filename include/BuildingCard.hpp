//
// Created by m0938 on 2026/4/14.
//

#ifndef STACKLANDS_BUILDINGCARD_HPP
#define STACKLANDS_BUILDINGCARD_HPP

#pragma once
#include <string>
#include "Card.hpp"

class BuildingCard : public Card {
public:
    BuildingCard(float x, float y, const std::string& name, int sellValue,
                 const std::string& iconPath, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::BUILDING, scale)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Building.png");
        SetIconImage(iconPath);

        m_PriceText = InitLabelText(std::to_string(sellValue), Util::Color(100, 111, 128));

        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return true;
    }

    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_PriceText) RebuildLabelText(m_PriceText, std::to_string(m_SellValue), Util::Color(100, 111, 128));
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

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText) m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_PriceText;
};

#endif //STACKLANDS_BUILDINGCARD_HPP
