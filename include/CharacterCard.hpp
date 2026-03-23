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

        // HP visaulize
        m_HealthBg = std::make_shared<Util::GameObject>();
        m_HealthBg->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/card/character/HeartBg.png"));
        m_HealthBg->m_Transform.scale = card_scale * glm::vec2{0.4f, 0.4f};
        m_HealthBg->SetZIndex(11);

        // 4. 建立血量文字
        m_HealthText = std::make_shared<Util::GameObject>();
        int fontSize = static_cast<int>(500 * m_Scale);
        if (fontSize < 22) fontSize = 22;

        m_HealthText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(health), Util::Color(255, 255, 255)));
        m_HealthText->m_Transform.scale = card_scale;
        // 血量文字蓋在血量底圖上
        m_HealthText->SetZIndex(m_HealthBg->GetZIndex() + 1); // Z=16

        UpdateVisualPositions();
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
            m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", static_cast<int>(500 * m_Scale), m_Name, Util::Color(0, 0, 0)));
        }
    }

    // 月底被扣除飽食度的邏輯
    void OnMonthEnd() override {
        // ... 實作村民需要吃食物的邏輯 ...
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_HealthBg) {
            //卡牌中心 + 卡牌寬度*0.8/2, 卡牌中心 - 卡牌高度*0.8/2
            float hpX = m_X + (m_Width / 2.0f) * 0.6f;
            float hpY = m_Y - (m_Height / 2.0f) * 0.65f;
            m_HealthBg->m_Transform.translation = glm::vec2(hpX, hpY);
            m_HealthBg->SetZIndex(m_Background->GetZIndex() + 1);
        }

        // 3. 計算並更新血量文字位置 (蓋在血量底圖中心點)
        if (m_HealthText && m_HealthBg) {
            // 文字不需要像卡牌名稱文字那樣靠左，文字置中在底圖上即可
            m_HealthText->m_Transform.translation = glm::vec2(m_HealthBg->m_Transform.translation.x, m_HealthBg->m_Transform.translation.y);
            m_HealthText->SetZIndex(m_HealthBg->GetZIndex() + 1);
        }
    }

    virtual void StopDragging() override{
        Card::StopDragging();
        m_HealthBg->SetZIndex(m_Background->GetZIndex() + 1);
        m_HealthText->SetZIndex(m_HealthBg->GetZIndex() + 1);
    };

    // 【修正 2】將血量相關的 GameObject 加入清單，否則 Renderer 畫不出來！
    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        // 1. 先取得基礎卡牌的清單 (Background、Icon、Name Text)
        std::vector<std::shared_ptr<Util::GameObject>> objs = Card::GetGameObjects();

        // 2. 加入血量圖層
        if (m_HealthBg) objs.push_back(m_HealthBg);
        if (m_HealthText) objs.push_back(m_HealthText);

        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_HealthBg;
    std::shared_ptr<Util::GameObject> m_HealthText;
    int health, attack;

};

#endif //STACKLANDS_CHARACTERCARD_HPP