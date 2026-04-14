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
                  const std::string& iconPath, int bonusAtk, int bonusHp,
                  EquipSlot slot, float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::EQUIPMENT, scale),
          m_BonusAttack(bonusAtk), m_BonusHealth(bonusHp), m_Slot(slot)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Equipment.png");
        SetIconImage(iconPath);

        m_PriceText = std::make_shared<Util::GameObject>();

        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        m_PriceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(sellValue), Util::Color(102, 102, 102)));
        m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            float priceOffsetX = m_Width * -0.32f;
            float priceOffsetY = m_Height * -0.3f;
            m_PriceText->m_Transform.translation = glm::vec2(m_X + priceOffsetX ,m_Y + priceOffsetY);

            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    int GetBonusAttack() const { return m_BonusAttack; }
    int GetBonusHealth() const { return m_BonusHealth; }
    EquipSlot GetEquipSlot() const { return m_Slot; }

    // 裝備卡已消耗進角色卡插槽，不接受任何卡片堆疊在其上
    bool OnStacked(std::shared_ptr<Card> /*cardAbove*/) override {
        return false;
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(42);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText) m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

protected:
    int m_BonusAttack;
    int m_BonusHealth;
    EquipSlot m_Slot;

    std::shared_ptr<Util::GameObject> m_PriceText;
};

#endif //STACKLANDS_EQUIPMENTCARD_HPP
