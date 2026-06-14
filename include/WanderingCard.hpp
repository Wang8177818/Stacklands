#pragma once
#ifndef STACKLANDS_WANDERINGCARD_HPP
#define STACKLANDS_WANDERINGCARD_HPP

#include "CombatCard.hpp"
#include <random>
#include <string>
#include <vector>
#include <utility>

// 可自訂的漫遊參數（避免建構子中呼叫虛函數）
struct WanderParams {
    float speed       = 4.0f;
    float cooldownMin = 3000.0f;
    float cooldownMax = 6000.0f;
    float offsetMin   = -90.0f;
    float offsetMax   =  90.0f;
};

// 帶隨機漫遊行為 + 掉落表的戰鬥卡基底
// AnimalCard 和 MonsterCard 均繼承此類
class WanderingCard : public CombatCard {
public:
    WanderingCard(float x, float y, const std::string& name, int sellValue,
                  CardType type, float scale,
                  int health, int attack, int defense,
                  float attackSpeed, float hitChance,
                  const std::vector<std::pair<std::string, int>>& dropCards = {},
                  WanderParams wander = {});

    // 拖曳時同步目標座標，防止放開後漂移
    void StartDragging(glm::vec2 mousePos)              override;
    // 視角平移/縮放時同步目標
    void MoveBy(glm::vec2 delta)                        override;
    void ScaleAroundPivot(float ratio, glm::vec2 pivot) override;

    void SetInCombat(bool v) { m_InCombat = v; }
    bool IsInCombat()  const { return m_InCombat; }

    // 依權重隨機抽掉落物，無則回傳空字串
    std::string RollDrop() const;

protected:
    // 子類別 Update() 呼叫此方法驅動漫遊邏輯
    void UpdateWander(float dtMs);

    WanderParams m_WanderParams;

    float m_TargetX      = 0.0f;
    float m_TargetY      = 0.0f;
    float m_MoveTimer    = 0.0f;
    float m_MoveCooldown = 0.0f;
    bool  m_IsMoving     = false;
    bool  m_InCombat     = false;

    std::vector<std::pair<std::string, int>> m_DropCards;

protected:
    static std::mt19937 s_Rng;
};

#endif // STACKLANDS_WANDERINGCARD_HPP
