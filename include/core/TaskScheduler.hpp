#pragma once
#ifndef STACKLANDS_TASKSCHEDULER_HPP
#define STACKLANDS_TASKSCHEDULER_HPP

#include "cards/Card.hpp"
#include "combat/CombatArena.hpp"
#include "combat/FloatingTextManager.hpp"
#include "core/RecipeManager.hpp"
#include "ui/TimeBar.hpp"
#include "Util/Renderer.hpp"
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>

// 將 CardManager 的三種延遲任務（採集、戰鬥、合成）集中管理（SRP）
class TaskScheduler {
public:
    using SpawnFn  = std::function<void(const std::string&, float, float, float)>;
    using RemoveFn = std::function<void(std::shared_ptr<Card>)>;

    struct PendingGather {
        std::weak_ptr<Card> character;
        std::weak_ptr<Card> structure;
        bool        exhausted    = false;
        std::string spawnName;
        float spawnX = 0, spawnY = 0, spawnScale = 0.05f;
        float timeLeftMs = 10000.0f;
        float totalMs    = 10000.0f; // 用於存檔還原讀條進度
        std::unique_ptr<TimeBar> bar;
    };

    struct PendingCombat {
        struct FighterEntry {
            std::weak_ptr<Card> fighter;
            float timer = 0.0f;
        };
        std::weak_ptr<Card>              target;
        std::vector<FighterEntry>        fighters;
        float                            targetTimer = 0.0f;
        std::unique_ptr<CombatArena>     arena;
    };

    struct PendingCraft {
        std::weak_ptr<Card>              stackBottom;
        std::vector<std::weak_ptr<Card>> allCards;
        std::string outputName;
        float spawnX = 0, spawnY = 0, spawnScale = 0.05f;
        float timeLeftMs = 0.0f;
        float totalMs    = 0.0f;
        std::unique_ptr<TimeBar> bar;
    };

    TaskScheduler(Util::Renderer& renderer, std::mt19937& rng,
                  RecipeManager& recipes, SpawnFn spawnFn, RemoveFn removeFn);

    void UpdateGathers(float dtMs);
    void UpdateCombats(float dtMs, const std::shared_ptr<Card>& dragging);
    void UpdateCrafts(float dtMs);

    void AddGather(PendingGather pg);
    void AddCraft(PendingCraft  pc);

    // 清空所有延遲任務（返回選單 / Game Over 用）
    // 會析構各任務內的 TimeBar / CombatArena，自動從 Renderer 移除
    void ClearAll() {
        m_Gathers.clear();
        m_Combats.clear();
        m_Crafts.clear();
    }

    // ── 存檔用 access ────────────────────────────────────────────────
    const std::vector<PendingGather>& GetGathers() const { return m_Gathers; }
    const std::vector<PendingCombat>& GetCombats() const { return m_Combats; }
    const std::vector<PendingCraft>&  GetCrafts()  const { return m_Crafts;  }

    // 若目標已有戰鬥則加入，否則新建；回傳是否成功（false 表示戰鬥員已在其他戰鬥中）
    bool JoinOrCreateCombat(const std::shared_ptr<Card>& target,
                            const std::shared_ptr<Card>& fighter);

    bool IsFighterBusy(const std::shared_ptr<Card>& fighter) const;
    bool HasCraftForBottom(const std::shared_ptr<Card>& bottom) const;

    // 同步所有戰鬥場地的世界座標（Pan / Zoom 時呼叫）
    void MoveCombatArenas(glm::vec2 delta);
    void ScaleCombatArenas(float ratio, glm::vec2 pivot);

    // 浮動文字管理器（供外部同步 Pan/Zoom）
    FloatingTextManager& GetFloatingText() { return m_FloatingText; }

private:
    void ApplyHitEffects(const std::shared_ptr<Card>& attacker,
                         const std::shared_ptr<Card>& directHit,
                         int dmgDealt,
                         const std::vector<std::shared_ptr<Card>>& fighters,
                         const std::weak_ptr<Card>& combatTarget,
                         bool attackerIsFighter);

    void ApplyPassiveEffects(const std::shared_ptr<Card>& attacker,
                             const std::shared_ptr<Card>& directHit,
                             const std::vector<std::shared_ptr<Card>>& fighters,
                             const std::weak_ptr<Card>& combatTarget,
                             bool attackerIsFighter);

    std::vector<PendingGather>  m_Gathers;
    std::vector<PendingCombat>  m_Combats;
    std::vector<PendingCraft>   m_Crafts;

    Util::Renderer&     m_Renderer;
    std::mt19937&       m_Rng;
    RecipeManager&      m_Recipes;
    SpawnFn             m_SpawnFn;
    RemoveFn            m_RemoveFn;
    FloatingTextManager m_FloatingText;
};

#endif // STACKLANDS_TASKSCHEDULER_HPP
