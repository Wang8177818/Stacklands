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

        int fontSize = static_cast<int>(500 * m_Scale);
        if (fontSize < 22) fontSize = 22;

        m_PriceText = std::make_shared<Util::GameObject>();

        m_PriceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(sellValue), Util::Color(100, 111, 128)));
        m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return true;
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            float priceOffsetX = m_Width * -0.35f;
            float priceOffsetY = m_Height * -0.3f;
            m_PriceText->m_Transform.translation = glm::vec2(m_X + priceOffsetX ,m_Y + priceOffsetY);

            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        std::vector<std::shared_ptr<Util::GameObject>> objs = Card::GetGameObjects();
        // 把價格加進去清單
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(42);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText) m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

protected:
    std::shared_ptr<Util::GameObject> m_PriceText;
};

#endif //STACKLANDS_RESOURCECARD_HPP