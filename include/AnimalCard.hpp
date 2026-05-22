//
// Created by m0938 on 2026/5/8.
//

#ifndef STACKLANDS_ANIMALCARD_HPP
#define STACKLANDS_ANIMALCARD_HPP

#pragma once
#include "Card.hpp"
#include <functional>
#include <string>
#include <vector>
#include "EventManager.hpp"

class AnimalCard : public Card {
public:
    // spawnCallback(cardName, x, y) — 由 CardManager 注入，用於生成卡牌
    AnimalCard(float x, float y,
               const std::string& name,
               const std::string& iconPath,
               float scale,
               int health, int attack, int defense,
               float attackSpeed, float hitChance,
               const std::vector<std::pair<std::string, int>>& dropCards,
               const std::string& abilityName,
               float abilityCooldown,
               std::function<void(const std::string&, float, float)> spawnCallback);

    void Update() override;

    // 接受村民疊上來（觸發戰鬥）
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::CHARACTER;
    }

    // 不可被疊到其他卡上
    bool CanStackOnto() override { return false; }

    // 戰鬥狀態：停止移動
    void SetInCombat(bool inCombat) { m_InCombat = inCombat; }
    bool IsInCombat() const { return m_InCombat; }

    void TakeDamage(int dmg);
    bool  IsDead()         const { return m_Health <= 0; }
    int   GetHealth()      const { return m_Health; }
    int   GetMaxHealth()   const { return m_MaxHealth; }
    int   GetAttack()      const { return m_Attack; }
    int   GetDefense()     const { return m_Defense; }
    float GetAttackSpeed() const { return m_AttackSpeed; }
    float GetHitChance()   const { return m_HitChance; }

    // 以權重隨機抽一個掉落物名稱；無掉落則回傳空字串
    std::string RollDrop() const;

    void StartDragging(glm::vec2 mousePos) override;
    void UpdateVisualPositions() override;
    void SetScale(float scale) override;

    // 視角移動/縮放時同步移動目標，避免漂移
    void MoveBy(glm::vec2 delta) override {
        Card::MoveBy(delta);
        m_TargetX += delta.x;
        m_TargetY += delta.y;
    }
    void ScaleAroundPivot(float ratio, glm::vec2 pivot) override {
        Card::ScaleAroundPivot(ratio, pivot);
        m_TargetX = pivot.x + (m_TargetX - pivot.x) * ratio;
        m_TargetY = pivot.y + (m_TargetY - pivot.y) * ratio;
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_HealthText) objs.push_back(m_HealthText);
        return objs;
    }

private:
    void ProcessSpecialAbility();
    void UpdateHealthText();

    int m_Health;
    int m_Attack;

    int   m_MaxHealth    = 0;
    int   m_Defense      = 0;
    float m_AttackSpeed  = 3.0f;
    float m_HitChance    = 0.6f;

    // 隨機移動
    float m_MoveTimer    = 0.0f;
    float m_MoveCooldown = 0.0f;
    float m_TargetX      = 0.0f;
    float m_TargetY      = 0.0f;
    bool  m_IsMoving     = false;
    bool  m_InCombat     = false;

    // 特殊能力
    float m_AbilityTimer    = 0.0f;
    float m_AbilityCooldown = 0.0f;
    std::string m_AbilityName;

    // 掉落表 {名稱, 權重}
    std::vector<std::pair<std::string, int>> m_DropCards;

    // CardManager 注入的生成 callback
    std::function<void(const std::string&, float, float)> m_SpawnCallback;

    std::shared_ptr<Util::GameObject> m_HealthText;
};

#endif // STACKLANDS_ANIMALCARD_HPP
