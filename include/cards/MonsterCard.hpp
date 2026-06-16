#pragma once
#ifndef STACKLANDS_MONSTERCARD_HPP
#define STACKLANDS_MONSTERCARD_HPP

#include "cards/WanderingCard.hpp"

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

    // 設定追蹤目標位置（由 CardManager 每幀呼叫）
    void SetChaseTarget(float x, float y) { m_HasTarget = true; m_ChaseX = x; m_ChaseY = y; }
    void ClearChaseTarget()               { m_HasTarget = false; }

protected:
    // 紅色血量文字
    Util::Color HealthTextColor() const override { return Util::Color(255, 80, 80); }

private:
    bool  m_HasTarget = false;
    float m_ChaseX    = 0.0f;
    float m_ChaseY    = 0.0f;
};

#endif // STACKLANDS_MONSTERCARD_HPP
