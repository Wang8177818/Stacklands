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
    // 建構子新增 EquipSlot 參數
    EquipmentCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, int bonusAtk, int bonusHp, EquipSlot slot, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::EQUIPMENT, scale), m_BonusAttack(bonusAtk), m_BonusHealth(bonusHp), m_Slot(slot) {

        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Resource.png");
        SetIconImage(iconPath);

        // --- 顯示部位與加成數值 (例如: [Hand] ATK +2) ---
        m_BonusText = std::make_shared<Util::GameObject>();
        int fontSize = static_cast<int>(350 * m_Scale);
        if (fontSize < 16) fontSize = 16;

        std::string slotStr = "";
        if (m_Slot == EquipSlot::HEAD) slotStr = "[Head] ";
        else if (m_Slot == EquipSlot::HAND) slotStr = "[Hand] ";
        else if (m_Slot == EquipSlot::BODY) slotStr = "[Body] ";

        std::string buffStr = slotStr;
        if (m_BonusAttack > 0) buffStr += "ATK +" + std::to_string(m_BonusAttack) + " ";
        if (m_BonusHealth > 0) buffStr += "HP +" + std::to_string(m_BonusHealth);

        m_BonusText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize, buffStr, Util::Color(50, 205, 50)));
        m_BonusText->m_Transform.scale = {m_Scale, m_Scale};

        UpdateVisualPositions();
    }

    int GetBonusAttack() const { return m_BonusAttack; }
    int GetBonusHealth() const { return m_BonusHealth; }
    EquipSlot GetEquipSlot() const { return m_Slot; }

    // ==========================================
    // 核心：允許其他裝備疊在我上面 (只要部位沒衝突)
    // ==========================================
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        if (cardAbove->GetType() != CardType::EQUIPMENT) return false;

        // 往下找，直到找到最底層的人物卡
        auto current = GetCardBelow();
        while (current != nullptr && current->GetType() == CardType::EQUIPMENT) {
            current = current->GetCardBelow();
        }

        // 呼叫人物卡的裝備檢查邏輯 (轉交給 CharacterCard 決定)
        if (current != nullptr && current->GetType() == CardType::CHARACTER) {
            return current->OnStacked(cardAbove);
        }
        return false;
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_BonusText) {
            float textOffsetY = m_Height * -0.3f;
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
    EquipSlot m_Slot;
    std::shared_ptr<Util::GameObject> m_BonusText;
};

#endif //STACKLANDS_EQUIPMENTCARD_HPP