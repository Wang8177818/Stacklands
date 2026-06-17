#pragma once
#ifndef STACKLANDS_WANDERINGCARD_HPP
#define STACKLANDS_WANDERINGCARD_HPP

#include "cards/CombatCard.hpp"
#include <random>
#include <string>
#include <vector>
#include <utility>

// 可自訂的漫遊參數（避免建構子中呼叫虛函數）
struct WanderParams {
    float speed       = 4.0f;     // 指數補間速度（每段移動的趨近快慢）
    float cooldownMin = 3000.0f;  // 隨機漫遊：每段之間的停頓下限
    float cooldownMax = 6000.0f;  // 隨機漫遊：每段之間的停頓上限
    float offsetMin   = -90.0f;   // 隨機漫遊：單段位移下限
    float offsetMax   =  90.0f;   // 隨機漫遊：單段位移上限

    float seekStep     = 60.0f;   // 追擊：朝目標單段前進距離（避免連續直線移動）
    float seekPauseMin = 250.0f;  // 追擊：每段之間的停頓下限
    float seekPauseMax = 600.0f;  // 追擊：每段之間的停頓上限
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
    void SetPosition(float x, float y)                  override;
    void ScaleAroundPivot(float ratio, glm::vec2 pivot) override;

    void SetInCombat(bool v) { m_InCombat = v; }
    bool IsInCombat()  const { return m_InCombat; }

    // 主動追擊：設定追擊目標座標（覆寫隨機漫遊）／取消追擊
    void SetSeekTarget(float x, float y) {
        // 由漫遊轉為追擊時，若正在停頓就立即結束，避免最長要等 cooldownMax 才反應
        if (!m_HasSeek && !m_IsMoving) m_MoveTimer = m_MoveCooldown;
        m_HasSeek = true; m_SeekX = x; m_SeekY = y;
    }
    void ClearSeekTarget()               { m_HasSeek = false; }
    bool HasSeekTarget() const           { return m_HasSeek; }

    // 依權重隨機抽掉落物，無則回傳空字串
    std::string RollDrop() const;

protected:
    // 子類別 Update() 呼叫此方法驅動漫遊邏輯
    void UpdateWander(float dtMs);

private:
    // 朝 (m_TargetX, m_TargetY) 趨近一段，回傳是否已抵達
    bool StepTowardTarget(float dtMs);
    // 依當前模式（追擊／漫遊）挑選下一段移動目標
    void PickNextTarget();
    // 依當前模式擲出下一次停頓時間
    void RollPause();

protected:
    WanderParams m_WanderParams;

    float m_TargetX      = 0.0f;
    float m_TargetY      = 0.0f;
    float m_MoveTimer    = 0.0f;
    float m_MoveCooldown = 0.0f;
    bool  m_IsMoving     = false;
    bool  m_InCombat     = false;

    // 主動追擊狀態（由 CardManager 每幀驅動，怪物靠近角色用）
    bool  m_HasSeek      = false;
    float m_SeekX        = 0.0f;
    float m_SeekY        = 0.0f;

    std::vector<std::pair<std::string, int>> m_DropCards;

protected:
    static std::mt19937 s_Rng;
};

#endif // STACKLANDS_WANDERINGCARD_HPP
