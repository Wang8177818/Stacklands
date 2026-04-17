//
// Created by m0938 on 2026/3/13.
//

#ifndef STACKLANDS_CHARACTERCARD_HPP
#define STACKLANDS_CHARACTERCARD_HPP

#pragma once
#include <array>
#include <string>
#include <algorithm>
#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Image.hpp"
#include "EquipmentCard.hpp"

class CharacterCard : public Card {
public:
    CharacterCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, float scale,
                  int health, int attack, int defense, float attackSpeed, float hitChance, int food)
        : Card(x, y, name, sellValue, CardType::CHARACTER, scale),
          health(health), attack(attack)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Character.png");
        SetIconImage(iconPath);

        m_HealthText = std::make_shared<Util::GameObject>();
        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        m_HealthText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(baseHealth), Util::Color(255, 255, 255)));
        m_HealthText->m_Transform.scale = {m_Scale, m_Scale};
        m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    // 裝備槽
    // (int)EquipSlot：[0=NONE, 1=HEAD, 2=HAND, 3=BODY]

    // 儲存裝備名稱到對應插槽
    void StoreEquipment(EquipSlot slot, const std::string& name) {
        m_EquipNames[static_cast<int>(slot)] = name;
    }

    const std::string& GetEquipName(EquipSlot slot) const {
        return m_EquipNames[static_cast<int>(slot)];
    }
    // get All
    const std::array<std::string, 4>& GetEquipNames() const {
        return m_EquipNames;
    }
    // set All
    void SetEquipNames(const std::array<std::string, 4>& names) {
        m_EquipNames = names;
    }

    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_HealthText) {
            int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
            m_HealthText->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjh.ttc", fontSize,
                std::to_string(health), Util::Color(255, 255, 255)));
            m_HealthText->m_Transform.scale = {m_Scale, m_Scale};
        }
    }

    void OnMonthEnd() override {}

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_HealthText) {
            m_HealthText->m_Transform.translation =
                glm::vec2(m_X + m_Width * 0.34f, m_Y + m_Height * -0.3f);
            m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
            // hp Change
            int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
            m_HealthText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(health), Util::Color(255, 255, 255)));
        }
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_HealthText) m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_HealthText) objs.push_back(m_HealthText);
        return objs;
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::EQUIPMENT;
    }

    int GetbaseAttack() const { return baseAttack; }
    int GetAttack() const { return attack; }

    int GetbaseHealth() const { return baseHealth; }
    int GetHealth() const { return health; }

    int GetbaseDefense() const { return baseDefense; }
    int GetDefense() const { return defense; }



protected:
    std::shared_ptr<Util::GameObject> m_HealthText;
    int   baseHealth = 15;
    int   baseAttack = 2;
    int   baseDefense = 1;
    float baseAttackSpeed = 2.9f;
    float baseHitChance = 0.68f;
    int   food = 2;

    int   health = baseHealth;
    int   attack = baseAttack;
    int   defense = baseDefense;
    float attackSpeed = baseAttackSpeed;
    float hitChance = baseHitChance;


    std::array<std::string, 4> m_EquipNames = {"", "", "", ""};
};

#endif //STACKLANDS_CHARACTERCARD_HPP
