//
// Created by m0938 on 2026/3/26.
//

#ifndef STACKLANDS_COINCARD_HPP
#define STACKLANDS_COINCARD_HPP

#pragma once
#include "Card.hpp"
#include "Util/Text.hpp"

class CoinCard : public Card {
public:
    // 金幣建構子：我們把預設價值設為 1 (m_SellValue = 1)
    CoinCard(float x, float y, float scale = 1.0f)
        : Card(x, y, "Coin", 1, CardType::COIN, scale) {

        // 請準備一張金幣的圖片 (例如 Coin.png) 放在 Image/card 裡面
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Coin.png");
        if (m_Icon) m_Icon->SetVisible(false);

        UpdateVisualPositions();
    }

    // only coin
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        if (cardAbove->GetType() == CardType::COIN) {
            return true;
        }
        return false;
    }

    int GetTotalStackValue() {
        int total = m_SellValue; // 先算自己的價值 (1)

        auto current = m_CardAbove;
        while (current != nullptr) {
            total += current->GetSellValue();
            current = current->GetCardAbove();
        }

        current = m_CardBelow;
        while (current != nullptr) {
            total += current->GetSellValue();
            current = current->GetCardBelow();
        }
        return total;
    }

};

#endif //STACKLANDS_COINCARD_HPP