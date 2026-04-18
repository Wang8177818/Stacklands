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

struct EquipSlotData {
    std::string name;
    int   bonusAtk      = 0;
    int   bonusHp       = 0;
    int   bonusDef      = 0;
    float bonusAtkSpd   = 0.0f;
    float bonusHitChance = 0.0f;
};

class CharacterCard : public Card {
public:
    CharacterCard(float x, float y, const std::string& name, int sellValue, const std::string& iconPath, float scale,
                  int health, int attack, int defense, float attackSpeed, float hitChance, int food)
        : Card(x, y, name, sellValue, CardType::CHARACTER, scale),
          baseHealth(health), baseAttack(attack), baseDefense(defense),
          baseAttackSpeed(attackSpeed), baseHitChance(hitChance), food(food),
          health(health), attack(attack), defense(defense),
          attackSpeed(attackSpeed), hitChance(hitChance)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Character.png");
        SetIconImage(iconPath);

        m_HealthText = std::make_shared<Util::GameObject>();
        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        m_HealthText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(health), Util::Color(255, 255, 255)));
        m_HealthText->m_Transform.scale = {m_Scale, m_Scale};
        m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    // 儲存裝備到對應插槽
    void StoreEquipment(EquipSlot slot, const std::string& name,
                        int bonusAtk, int bonusHp, int bonusDef,
                        float bonusAtkSpd, float bonusHitChance) {
        auto& data        = m_Equips[static_cast<int>(slot)];
        data.name         = name;
        data.bonusAtk     = bonusAtk;
        data.bonusHp      = bonusHp;
        data.bonusDef     = bonusDef;
        data.bonusAtkSpd  = bonusAtkSpd;
        data.bonusHitChance = bonusHitChance;
        RecalculateStats();
    }

    void ClearEquipment(EquipSlot slot) {
        m_Equips[static_cast<int>(slot)] = {};
        RecalculateStats();
    }

    const std::string& GetEquipName(EquipSlot slot) const {
        return m_Equips[static_cast<int>(slot)].name;
    }

    const EquipSlotData& GetEquipSlotData(EquipSlot slot) const {
        return m_Equips[static_cast<int>(slot)];
    }

    const std::array<EquipSlotData, 4>& GetAllEquipData() const {
        return m_Equips;
    }

    void SetAllEquipData(const std::array<EquipSlotData, 4>& data) {
        m_Equips = data;
        RecalculateStats();
    }

    void RecalculateStats() {
        int   totalAtk    = baseAttack;
        int   totalHp     = baseHealth;
        int   totalDef    = baseDefense;
        float totalAtkSpd = baseAttackSpeed;
        float totalHit    = baseHitChance;
        for (const auto& e : m_Equips) {
            totalAtk    += e.bonusAtk;
            totalHp     += e.bonusHp;
            totalDef    += e.bonusDef;
            totalAtkSpd += e.bonusAtkSpd;
            totalHit    += e.bonusHitChance;
        }
        attack      = totalAtk;
        health      = totalHp;
        defense     = totalDef;
        attackSpeed = totalAtkSpd;
        hitChance   = totalHit;
        UpdateVisualPositions();
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
    int   baseHealth;
    int   baseAttack;
    int   baseDefense;
    float baseAttackSpeed;
    float baseHitChance;
    int   food;

    int   health;
    int   attack;
    int   defense;
    float attackSpeed;
    float hitChance;

    std::array<EquipSlotData, 4> m_Equips;
};

#endif //STACKLANDS_CHARACTERCARD_HPP
