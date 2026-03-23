//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_RESOURCECARD_HPP
#define STACKLANDS_RESOURCECARD_HPP

#pragma once
#include "Card.hpp"

class ResourceCard : public Card {
public:
    // 建構子：跟一般卡片幾乎一樣，但我們可以在這裡指定專屬的資源卡底圖
    ResourceCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::RESOURCE, scale) {

        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Resource.png");
        SetIconImage(iconPath);

        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return true;
    }

    // 其他功能
};

#endif //STACKLANDS_RESOURCECARD_HPP