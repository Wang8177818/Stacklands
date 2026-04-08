//
// Created by m0938 on 2026/3/13.
//

#ifndef STACKLANDS_CHARACTERCARD_HPP
#define STACKLANDS_CHARACTERCARD_HPP

#pragma once
#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Image.hpp"

class CharacterCard : public Card {
public:
    // 建構子：除了基本的 x, y, 名稱, 售價，我們多傳入一個 iconPath 來決定長相
    CharacterCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, float scale, int health = 1, int attack = 1)
        // 把 scale 參數接力傳給基礎的 Card 建構子
        : Card(x, y, name, sellValue, CardType::CHARACTER, scale) {

        glm::vec2 card_scale = {m_Scale, m_Scale};
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Character.png");
        SetIconImage(iconPath);

        m_HealthText = std::make_shared<Util::GameObject>();
        int fontSize = static_cast<int>(1000 * m_Scale);
        if (fontSize < 22) fontSize = 22;

        m_HealthText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(health), Util::Color(255, 255, 255)));
        m_HealthText->m_Transform.scale = card_scale;
        m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    // 例如：被物品轉換職業 (村民 + 矛 = 民兵)
    void ChangeProfession(const std::string& newName, const std::string& newIconPath) {
        m_Name = newName;
        SetIconImage(newIconPath);
        if (m_NameText) {
            m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", static_cast<int>(500 * m_Scale), m_Name, Util::Color(0, 0, 0)));
        }
    }

    // 月底被扣除飽食度的邏輯
    void OnMonthEnd() override {
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_HealthText) {
            float healthOffsetX = m_Width * 0.34f;
            float healthOffsetY = m_Height * -0.3f;
            m_HealthText->m_Transform.translation = glm::vec2(m_X + healthOffsetX ,m_Y + healthOffsetY);

            m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    virtual void StopDragging() override{
        Card::StopDragging();
        m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
    };

    // 【修正 2】將血量相關的 GameObject 加入清單，否則 Renderer 畫不出來！
    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        // 1. 先取得基礎卡牌的清單 (Background、Icon、Name Text)
        std::vector<std::shared_ptr<Util::GameObject>> objs = Card::GetGameObjects();

        if (m_HealthText) objs.push_back(m_HealthText);

        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_HealthText;
    int health, attack;
    float attackSpeed = 1.0f;

};

#endif //STACKLANDS_CHARACTERCARD_HPP