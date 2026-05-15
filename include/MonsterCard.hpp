//
// Created by m0938 on 2026/5/15.
//

#ifndef STACKLANDS_MONSTERCARD_HPP
#define STACKLANDS_MONSTERCARD_HPP

#pragma once
#include <array>
#include <string>
#include <vector>
#include "Card.hpp"
#include "CardData.hpp"

class MonsterCard : public Card {
public:
    MonsterCard(float x, float y, const std::string& name, int sellValue,
                const std::string& iconPath, float scale,
                int health, int attack, int defense,
                float attackSpeed, float hitChance,
                const std::vector<std::pair<std::string, int>>& dropCards = {})
        : Card(x, y, name, sellValue, CardType::MONSTER, scale),
          m_BaseHealth(health), m_BaseAttack(attack), m_BaseDefense(defense),
          m_BaseAttackSpeed(attackSpeed), m_BaseHitChance(hitChance),
          m_Health(health), m_Attack(attack), m_Defense(defense),
          m_AttackSpeed(attackSpeed), m_HitChance(hitChance),
          m_CurrentHealth(health),
          m_DropCards(dropCards),
          m_TargetX(x), m_TargetY(y)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Monster.png");
        SetIconImage(iconPath);
        m_HealthText = InitLabelText(std::to_string(m_CurrentHealth), Util::Color(255, 80, 80));
        UpdateVisualPositions();
    }

    void Update() override;

    // 不可被玩家拖動
    bool CanDrag() const override { return false; }

    // 不會被疊到其他卡上
    bool CanStackOnto() override { return false; }

    // 只接受角色卡、動物卡疊上來（戰鬥觸發）
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        CardType t = cardAbove->GetType();
        return t == CardType::CHARACTER || t == CardType::ANIMAL;
    }

    // ── 裝備系統（保留以供未來擴展）────────────────────────────────
    void StoreEquipment(EquipSlot slot, const std::string& name,
                        int bonusAtk, int bonusHp, int bonusDef,
                        float bonusAtkSpd, float bonusHitChance) {
        auto& e      = m_Equips[static_cast<int>(slot)];
        e.name           = name;
        e.bonusAtk       = bonusAtk;
        e.bonusHp        = bonusHp;
        e.bonusDef       = bonusDef;
        e.bonusAtkSpd    = bonusAtkSpd;
        e.bonusHitChance = bonusHitChance;
        RecalculateStats();
    }
    void ClearEquipment(EquipSlot slot) {
        m_Equips[static_cast<int>(slot)] = {};
        RecalculateStats();
    }
    void RecalculateStats() {
        int   totAtk = m_BaseAttack;
        int   totHp  = m_BaseHealth;
        int   totDef = m_BaseDefense;
        float totSpd = m_BaseAttackSpeed;
        float totHit = m_BaseHitChance;
        for (const auto& e : m_Equips) {
            totAtk += e.bonusAtk;
            totHp  += e.bonusHp;
            totDef += e.bonusDef;
            totSpd += e.bonusAtkSpd;
            totHit += e.bonusHitChance;
        }
        m_Attack      = totAtk;
        m_Health      = totHp;
        m_Defense     = totDef;
        m_AttackSpeed = totSpd;
        m_HitChance   = totHit;
        UpdateVisualPositions();
    }

    // ── 戰鬥介面 ────────────────────────────────────────────────────
    void TakeDamage(int dmg) override {
        m_CurrentHealth -= dmg;
        if (m_CurrentHealth < 0) m_CurrentHealth = 0;
        if (m_HealthText)
            RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), Util::Color(255, 80, 80));
    }
    bool  IsDead()          const override { return m_CurrentHealth <= 0; }
    int   GetAttack()       const override { return m_Attack; }
    int   GetDefense()      const override { return m_Defense; }
    float GetAttackSpeed()  const override { return m_AttackSpeed; }
    float GetHitChance()    const override { return m_HitChance; }
    int   GetHealth()       const override { return m_CurrentHealth; }
    int   GetMaxHealth()    const          { return m_Health; }

    // 戰鬥狀態：停止移動
    void SetInCombat(bool inCombat) { m_InCombat = inCombat; }
    bool IsInCombat() const { return m_InCombat; }

    // 掉落
    std::string RollDrop() const;

    // ── 視覺 ────────────────────────────────────────────────────────
    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_HealthText)
            RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), Util::Color(255, 80, 80));
    }

    void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_HealthText) {
            m_HealthText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::HEALTH_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
            RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), Util::Color(255, 80, 80));
        }
    }

    void StopDragging() override {
        Card::StopDragging();
        if (m_HealthText) m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_HealthText) objs.push_back(m_HealthText);
        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_HealthText;

    int   m_BaseHealth, m_BaseAttack, m_BaseDefense;
    float m_BaseAttackSpeed, m_BaseHitChance;

    int   m_Health, m_Attack, m_Defense;
    float m_AttackSpeed, m_HitChance;
    int   m_CurrentHealth;

    std::array<EquipSlotData, 4> m_Equips{};

    // 掉落表 {名稱, 權重}
    std::vector<std::pair<std::string, int>> m_DropCards;

    // 隨機移動
    float m_MoveTimer    = 0.0f;
    float m_MoveCooldown = 0.0f;
    float m_TargetX      = 0.0f;
    float m_TargetY      = 0.0f;
    bool  m_IsMoving     = false;
    bool  m_InCombat     = false;
};

#endif // STACKLANDS_MONSTERCARD_HPP
