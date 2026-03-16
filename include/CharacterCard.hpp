//
// Created by m0938 on 2026/3/13.
//

#ifndef STACKLANDS_CHARACTERCARD_HPP
#define STACKLANDS_CHARACTERCARD_HPP

#pragma once
#include "Card.hpp"

class CharacterCard : public Card {
public:
    // 【修改】在最後面加上 scale 參數，並給予預設值 1.0f
    CharacterCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, float scale = 1.0f)
        // 【修改】把 scale 參數接力傳給基礎的 Card 建構子
        : Card(x, y, name, sellValue, CardType::CHARACTER, scale) {

        // 1. 切換成人物專屬的底圖
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card.jpg");

        // 2. 設定這張人物卡專屬的圖示
        SetIconImage(iconPath);

        // 3. (選用) 由於我們已經在 Card.cpp 裡面用比例來自動排版了，
        // 這裡就不需要再寫死 m_Icon 的大小和位置了，交給底層的 UpdateVisualPositions 處理即可！
    }

    // ==========================================
    // 未來可以擴充的人物專屬功能
    // ==========================================

    // 例如：被物品轉換職業 (村民 + 矛 = 民兵)
    void ChangeProfession(const std::string& newName, const std::string& newIconPath) {
        m_Name = newName;
        SetIconImage(newIconPath);
        // 若有需要，也可以一併更新文字
        if (m_NameText) {
            m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", 24, m_Name, Util::Color(0, 0, 0)));
        }
    }

    // 月底被扣除飽食度的邏輯
    void OnMonthEnd() override {
        // ... 實作村民需要吃食物的邏輯 ...
    }
};

#endif //STACKLANDS_CHARACTERCARD_HPP