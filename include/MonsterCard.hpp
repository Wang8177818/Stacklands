#pragma once
#ifndef STACKLANDS_MONSTERCARD_HPP
#define STACKLANDS_MONSTERCARD_HPP

#include "WanderingCard.hpp"

class MonsterCard : public WanderingCard {
public:
    MonsterCard(float x, float y, const std::string& name, int sellValue,
                const std::string& iconPath, float scale,
                int health, int attack, int defense,
                float attackSpeed, float hitChance,
                const std::vector<std::pair<std::string, int>>& dropCards = {});

    void Update() override;

    bool CanDrag()     const override { return false; }
    bool CanStackOnto()      override { return false; }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        CardType t = cardAbove->GetType();
        return t == CardType::CHARACTER || t == CardType::ANIMAL;
    }

protected:
    // 紅色血量文字
    Util::Color HealthTextColor() const override { return Util::Color(255, 80, 80); }
};

#endif // STACKLANDS_MONSTERCARD_HPP
