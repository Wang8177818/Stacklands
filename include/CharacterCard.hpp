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
#include "EquipmentCard.hpp"

class CharacterCard : public Card {
public:
    CharacterCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, float scale, int health = 1, int attack = 1)
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
        Card::UpdateVisualPositions(); // 排底圖

        // === 【轉職邏輯】檢查手部裝備 ===
        std::string displayProfession = m_Name;
        auto current = m_CardAbove;
        while (current != nullptr) {
            if (current->GetType() == CardType::EQUIPMENT) {
                auto equip = std::static_pointer_cast<EquipmentCard>(current);
                if (equip->GetEquipSlot() == EquipSlot::HAND) {
                    // 根據拿什麼武器決定職業名稱！
                    if (equip->GetName() == "Sword") displayProfession = "Swordsman";
                    else if (equip->GetName() == "Spear") displayProfession = "Militia";
                    else if (equip->GetName() == "Magic Staff") displayProfession = "Mage";
                    else displayProfession = m_Name + " (" + equip->GetName() + ")"; // 預設組合名
                }
            }
            current = current->GetCardAbove();
        }

        // 強制更新卡片名稱 UI
        if (m_NameText) {
            int fontSize = static_cast<int>(500 * m_Scale);
            if (fontSize < 22) fontSize = 22;
            m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", fontSize, displayProfession, Util::Color(0, 0, 0)));

            // 重新計算文字置中 (從 Card.cpp 抄過來的)
            float textOffsetY = m_Height * 0.35;
            float textWidth = m_NameText->GetScaledSize().x;
            float padding = m_Width * 0.1f;
            float textX = (m_X - m_Width / 2.0f) + (textWidth / 2.0f) + padding;
            m_NameText->m_Transform.translation = glm::vec2(textX, m_Y + textOffsetY);
            m_NameText->SetZIndex(m_Background->GetZIndex() + 2);
        }

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

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        std::vector<std::shared_ptr<Util::GameObject>> objs = Card::GetGameObjects();
        if (m_HealthText) objs.push_back(m_HealthText);
        return objs;
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        if (cardAbove->GetType() == CardType::EQUIPMENT) {
            auto incomingEquip = std::static_pointer_cast<EquipmentCard>(cardAbove);

            // 往上掃描，看看自己頭上是不是已經有同部位的裝備了
            auto checkNode = m_CardAbove;
            while (checkNode != nullptr) {
                if (checkNode->GetType() == CardType::EQUIPMENT) {
                    auto existingEquip = std::static_pointer_cast<EquipmentCard>(checkNode);
                    if (existingEquip->GetEquipSlot() == incomingEquip->GetEquipSlot()) {
                        return false;
                    }
                }
                checkNode = checkNode->GetCardAbove();
            }
            return true;
        }
        return false;
    }

    int GetTotalAttack() {
        int totalAtk = attack;
        auto current = m_CardAbove;
        while (current != nullptr) {
            if (current->GetType() == CardType::EQUIPMENT) {
                totalAtk += std::static_pointer_cast<EquipmentCard>(current)->GetBonusAttack();
            }
            current = current->GetCardAbove();
        }
        return totalAtk;
    }



protected:
    std::shared_ptr<Util::GameObject> m_HealthText;
    int health, attack;
    float attackSpeed = 1.0f;

};

#endif //STACKLANDS_CHARACTERCARD_HPP