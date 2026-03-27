//
// Created by m0938 on 2026/3/27.
//

#ifndef STACKLANDS_EQUIPMENTCARD_HPP
#define STACKLANDS_EQUIPMENTCARD_HPP

#pragma once
#include "Card.hpp"
#include "Util/Text.hpp"

class EquipmentCard : public Card {
public:
    // 建構子：接收 bonusAtk 和 bonusHp
    EquipmentCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, int bonusAtk, int bonusHp, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::EQUIPMENT, scale), m_BonusAttack(bonusAtk), m_BonusHealth(bonusHp) {

        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Resource.png"); // 暫用資源底圖
        SetIconImage(iconPath);

        // --- 顯示裝備加成數值 (例如: ATK +2  HP +1) ---
        m_BonusText = std::make_shared<Util::GameObject>();
        int fontSize = static_cast<int>(350 * m_Scale);
        if (fontSize < 16) fontSize = 16;

        std::string buffStr = "";
        if (m_BonusAttack > 0) buffStr += "ATK +" + std::to_string(m_BonusAttack) + "  ";
        if (m_BonusHealth > 0) buffStr += "HP +" + std::to_string(m_BonusHealth);

        // 用亮綠色顯示，代表這是 Buff 裝備！
        m_BonusText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize, buffStr, Util::Color(50, 205, 50)));
        m_BonusText->m_Transform.scale = {m_Scale, m_Scale};

        UpdateVisualPositions();
    }

    int GetBonusAttack() const { return m_BonusAttack; }
    int GetBonusHealth() const { return m_BonusHealth; }

    // 裝備卡本身不允許其他東西疊在它上面 (它是最上層)
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return false;
    }

    // --- 覆寫排版與圖層邏輯 ---
    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_BonusText) {
            float textOffsetY = m_Height * -0.3f; // 顯示在下方
            m_BonusText->m_Transform.translation = glm::vec2(m_X, m_Y + textOffsetY);
            m_BonusText->SetZIndex(m_Background->GetZIndex() + 2);
        }
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_BonusText) objs.push_back(m_BonusText);
        return objs;
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_BonusText) m_BonusText->SetZIndex(42);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_BonusText) m_BonusText->SetZIndex(m_Background->GetZIndex() + 2);
    }

protected:
    int m_BonusAttack;
    int m_BonusHealth;
    std::shared_ptr<Util::GameObject> m_BonusText;
};

#endif //STACKLANDS_EQUIPMENTCARD_HPP