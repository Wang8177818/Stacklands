//
// Created by m0938 on 2026/4/10.
//

#ifndef STACKLANDS_SELLSLOT_HPP
#define STACKLANDS_SELLSLOT_HPP

#pragma once

#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "Card.hpp"
#include "CardManager.hpp"

class SellSlot : public Card{
public:
    SellSlot(float x, float y,
             const std::string& name = "Sell",
             int sellValue = 0,
             float scale = 0.05f) : Card(x, y, name, sellValue, CardType::CHARACTER, scale){

        m_Background = std::make_shared<Util::GameObject>(
        std::make_unique<Util::Image>(RESOURCE_DIR"/Image/card/Sellslot.png"), 1);
        m_Background->m_Transform.translation = { m_X, m_Y };
        m_Background->SetVisible(true);

        int fontSize = static_cast<int>(2000 * m_Scale);
        if (fontSize < 22) fontSize = 22;

        m_NameText->SetZIndex(m_Background->GetZIndex() + 1);
        m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", fontSize, "Sell", Util::Color(255, 255, 255)));
        // m_NameText->m_Transform.translation = m_Background->m_Transform.translation;

        glm::vec2 card_scale = {m_Scale, m_Scale};
        m_Background->m_Transform.scale = card_scale * 2.0f;
        m_NameText->m_Transform.scale = card_scale;

        UpdateVisualPositions();
    }

    void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        float textOffsetY = m_Height * -0.30;
        m_NameText->m_Transform.translation = glm::vec2(m_X, m_Y + textOffsetY);
    };

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override
    {
        return Card::GetGameObjects();
    }

    bool IsMouseHovering(glm::vec2 /*mousePos*/) override { return false; }

    void StartDragging(glm::vec2 mousePos) override{};
    void StopDragging() override{};

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        if (cardAbove->GetType() == CardType::CHARACTER || cardAbove->GetType() == CardType::COIN || cardAbove->GetType() == CardType::PACK) return false;
        return true;
    }

    // 計算堆疊在上方所有卡片的總售價
    int GetTotalPrice() {
        int value = 0;
        std::shared_ptr<Card> temp = GetCardAbove();
        while (temp != nullptr) {
            value += temp->GetSellValue();
            temp = temp->GetCardAbove();
        }
        return value;
    }

    // 取出並斷開所有堆疊在上方的卡片，回傳待刪除清單
    std::vector<std::shared_ptr<Card>> PopAllCards() {
        std::vector<std::shared_ptr<Card>> cards;
        std::shared_ptr<Card> temp = GetCardAbove();
        while (temp != nullptr) {
            cards.push_back(temp);
            temp = temp->GetCardAbove();
        }
        if (GetCardAbove() != nullptr) {
            GetCardAbove()->SetCardBelow(nullptr);
        }
        SetCardAbove(nullptr);
        return cards;
    }

protected:
    int m_price = 0;
};

#endif // STACKLANDS_SELLSLOT_HPP