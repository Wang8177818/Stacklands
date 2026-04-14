//
// Created by m0938 on 2026/4/14.
//

#ifndef STACKLANDS_BUILDINGCARD_HPP
#define STACKLANDS_BUILDINGCARD_HPP

#pragma once
#include <string>
#include <algorithm>
#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Image.hpp"

class BuildingCard : public Card {
public:
    BuildingCard(float x, float y, const std::string& name, int sellValue,
                 const std::string& iconPath, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::BUILDING, scale)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Building.png");
        SetIconImage(iconPath);

        m_PriceText = std::make_shared<Util::GameObject>();

        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        m_PriceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(sellValue), Util::Color(100, 111, 128)));
        m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    // ── 堆疊規則 ──────────────────────────────────────────────
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return true;
    }

    // ── 月底結算 ──────────────────────────────────────────────
    void OnMonthEnd() override {}

    // ── 縮放（同步重建 PriceText 字體）────────────────────────
    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_PriceText) {
            int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
            m_PriceText->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjh.ttc", fontSize,
                std::to_string(m_SellValue), Util::Color(100, 111, 128)));
            m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        }
    }

    // ── 視覺更新 ──────────────────────────────────────────────
    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            float priceOffsetX = m_Width * -0.32f;
            float priceOffsetY = m_Height * -0.3f;
            m_PriceText->m_Transform.translation =
                glm::vec2(m_X + priceOffsetX, m_Y + priceOffsetY);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(42);
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
