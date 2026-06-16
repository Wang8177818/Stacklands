#pragma once
#ifndef STACKLANDS_ANIMALCARD_HPP
#define STACKLANDS_ANIMALCARD_HPP

#include "cards/WanderingCard.hpp"
#include "data/ISpawnListener.hpp"

class AnimalCard : public WanderingCard {
public:
    AnimalCard(float x, float y, const std::string& name, const std::string& iconPath,
               float scale,
               int health, int attack, int defense,
               float attackSpeed, float hitChance,
               const std::vector<std::pair<std::string, int>>& dropCards,
               const std::string& abilityName, float abilityCooldown,
               ISpawnListener* listener);

    void Update() override;

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::CHARACTER;
    }
    bool CanStackOnto() override { return false; }

protected:
    // 黑色血量文字（動物卡背景較亮）
    Util::Color HealthTextColor() const override { return Util::Color(0, 0, 0); }

private:
    void ProcessSpecialAbility();

    float       m_AbilityTimer    = 0.0f;
    float       m_AbilityCooldown = 0.0f;
    std::string m_AbilityName;
    ISpawnListener* m_Listener = nullptr;
};

#endif // STACKLANDS_ANIMALCARD_HPP
