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

class AnimalCard : public Card {
public:
    // spawnCallback(cardName, x, y) — 由 CardManager 注入，用於生成卡牌
    AnimalCard(float x, float y,
               const std::string& name,
               const std::string& iconPath,
               float scale,
               int health, int attack,
               const std::vector<std::pair<std::string, int>>& dropCards,
               const std::string& abilityName,
               float abilityCooldown,
               std::function<void(const std::string&, float, float)> spawnCallback);

    void Update() override;

    // 不接受任何卡疊在上面
    bool OnStacked(std::shared_ptr<Card> /*cardAbove*/) override { return false; }

    // 不可被疊到其他卡上
    bool CanStackOnto() override { return false; }

    void TakeDamage(int dmg);
    bool IsDead() const { return m_Health <= 0; }
    int  GetHealth() const { return m_Health; }

    // 以權重隨機抽一個掉落物名稱；無掉落則回傳空字串
    std::string RollDrop() const;

    void UpdateVisualPositions() override;
    void SetScale(float scale) override;

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

    // 隨機移動
    float m_MoveTimer    = 0.0f;
    float m_MoveCooldown = 0.0f; // 隨機決定，3~6 秒

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
