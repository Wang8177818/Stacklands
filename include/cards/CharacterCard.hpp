#pragma once
#ifndef STACKLANDS_CHARACTERCARD_HPP
#define STACKLANDS_CHARACTERCARD_HPP

#include "cards/CombatCard.hpp"

class CharacterCard : public CombatCard {
public:
    CharacterCard(float x, float y, const std::string& name, int sellValue,
                  const std::string& iconPath, float scale,
                  int health, int attack, int defense,
                  float attackSpeed, float hitChance, int foodConsumption = 0)
        : CombatCard(x, y, name, sellValue, CardType::CHARACTER, scale,
                     health, attack, defense, attackSpeed, hitChance),
          m_FoodConsumption(foodConsumption)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Character.png");
        SetIconImage(iconPath);
        m_HealthText = InitLabelText(std::to_string(health), HealthTextColor());
        UpdateVisualPositions();
    }

    void OnMonthEnd() override {}

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::EQUIPMENT ||
               cardAbove->GetType() == CardType::CHARACTER;
    }

    int GetFoodConsumption() const { return m_FoodConsumption; }

    // 向下相容的別名（原本 lowercase b，現在委派到 CombatCard）
    int GetbaseAttack()  const { return m_BaseAttack; }
    int GetbaseHealth()  const { return m_BaseHealth; }
    int GetbaseDefense() const { return m_BaseDefense; }

protected:
    int m_FoodConsumption = 0;
};

#endif // STACKLANDS_CHARACTERCARD_HPP
