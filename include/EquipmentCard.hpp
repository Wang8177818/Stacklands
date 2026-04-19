//
// Created by m0938 on 2026/3/27.
//

#ifndef STACKLANDS_EQUIPMENTCARD_HPP
#define STACKLANDS_EQUIPMENTCARD_HPP

#pragma once
#include "Card.hpp"

class EquipmentCard : public Card {
public:
    EquipmentCard(float x, float y, const std::string& name, int sellValue,
                  const std::string& iconPath, int bonusAtk, int bonusHp, int bonusDef,
                  float bonusAtkSpd, float bonusHitChance,
                  EquipSlot slot, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::EQUIPMENT, scale),
          m_BonusAttack(bonusAtk), m_BonusHealth(bonusHp), m_BonusDefense(bonusDef),
          m_BonusAttackSpeed(bonusAtkSpd), m_BonusHitChance(bonusHitChance), m_Slot(slot)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Equipment.png");
        SetIconImage(iconPath);

        m_PriceText = InitLabelText(std::to_string(sellValue), Util::Color(102, 102, 102));

        UpdateVisualPositions();
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            m_PriceText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::PRICE_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    int   GetBonusAttack()       const { return m_BonusAttack; }
    int   GetBonusHealth()       const { return m_BonusHealth; }
    int   GetBonusDefense()      const { return m_BonusDefense; }
    float GetBonusAttackSpeed()  const { return m_BonusAttackSpeed; }
    float GetBonusHitChance()    const { return m_BonusHitChance; }
    EquipSlot GetEquipSlot()     const { return m_Slot; }

    bool OnStacked(std::shared_ptr<Card> /*cardAbove*/) override {
        return false;
    }

    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_PriceText) RebuildLabelText(m_PriceText, std::to_string(m_SellValue), Util::Color(100, 111, 128));
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText) m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

protected:
    int   m_BonusAttack;
    int   m_BonusHealth;
    int   m_BonusDefense;
    float m_BonusAttackSpeed;
    float m_BonusHitChance;
    EquipSlot m_Slot;

    std::shared_ptr<Util::GameObject> m_PriceText;
};

#endif //STACKLANDS_EQUIPMENTCARD_HPP
