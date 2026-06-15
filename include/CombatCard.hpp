#pragma once
#ifndef STACKLANDS_COMBATCARD_HPP
#define STACKLANDS_COMBATCARD_HPP

#include "Card.hpp"
#include "CardData.hpp"
#include "EffectData.hpp"
#include <algorithm>
#include <array>
#include <vector>

class CombatCard : public Card {
public:
    CombatCard(float x, float y, const std::string& name, int sellValue,
               CardType type, float scale,
               int health, int attack, int defense,
               float attackSpeed, float hitChance);

    // ── 戰鬥介面 ─────────────────────────────────────────────────────
    void  TakeDamage(int dmg)    override;
    bool  IsDead()         const override { return m_CurrentHealth <= 0; }
    int   GetAttack()      const override { return m_CombatAttack; }
    int   GetDefense()     const override { return m_CombatDefense; }
    float GetAttackSpeed() const override;
    float GetHitChance()   const override { return m_CombatHitChance; }
    int   GetHealth()      const override { return m_CurrentHealth; }
    int   GetMaxHealth()   const          { return m_CombatHealth; }

    // 給存檔回復用：直接設定 current HP（不過 Invul / 結算）
    void  SetCurrentHealth(int hp);

    // ── 狀態計時器 getter / setter（存檔用，毫秒）────────────────────
    float GetStunTimerMs()          const { return m_StunTimerMs;        }
    float GetBleedTimerMs()         const { return m_BleedTimerMs;       }
    float GetPoisonTimerMs()        const { return m_PoisonTimerMs;      }
    float GetInvulnerableTimerMs()  const { return m_InvulnerableTimerMs;}
    float GetFrenzyTimerMs()        const { return m_FrenzyTimerMs;      }
    void  SetStunTimerMs(float v)         { m_StunTimerMs        = std::max(0.0f, v); }
    void  SetBleedTimerMs(float v)        { m_BleedTimerMs       = std::max(0.0f, v); }
    void  SetPoisonTimerMs(float v)       { m_PoisonTimerMs      = std::max(0.0f, v); }
    void  SetInvulnerableTimerMs(float v) { m_InvulnerableTimerMs= std::max(0.0f, v); }
    void  SetFrenzyTimerMs(float v)       { m_FrenzyTimerMs      = std::max(0.0f, v); }

    int   GetBaseAttack()  const { return m_BaseAttack; }
    int   GetBaseHealth()  const { return m_BaseHealth; }
    int   GetBaseDefense() const { return m_BaseDefense; }

    // ── 暈眩介面 ─────────────────────────────────────────────────────
    bool  IsStunned()                    const override { return m_StunTimerMs > 0.0f; }
    void  ApplyStun(float durationMs)          override;
    void  UpdateStun(float dtMs)               override;
    void  UpdateCombatStates(float dtMs)       override;

    // ── 其他狀態 ─────────────────────────────────────────────────────
    bool  IsBleed()         const { return m_BleedTimerMs > 0.0f; }
    bool  IsPoison()        const { return m_PoisonTimerMs > 0.0f; }
    bool  IsInvulnerable()  const { return m_InvulnerableTimerMs > 0.0f; }
    bool  IsFrenzy()        const { return m_FrenzyTimerMs > 0.0f; }

    void  ApplyBleed(float durationMs);
    void  ApplyPoison(float durationMs);
    void  ApplyInvulnerable(float durationMs);
    void  ApplyFrenzy(float durationMs);
    void  HealBy(int amount);

    // ── 特效列表 ─────────────────────────────────────────────────────
    const std::vector<EffectData>& GetEffects() const override { return m_Effects; }
    void SetEffects(const std::vector<EffectData>& effects) override;

    // ── 裝備系統（CharacterCard 主要使用）──────────────────────────
    void StoreEquipment(EquipSlot slot, const std::string& name,
                        int bonusAtk, int bonusHp, int bonusDef,
                        float bonusAtkSpd, float bonusHitChance,
                        const std::vector<EffectData>& effects = {});
    void ClearEquipment(EquipSlot slot);
    void RecalculateStats();
    const std::string&                  GetEquipName(EquipSlot slot)    const;
    const EquipSlotData&                GetEquipSlotData(EquipSlot slot) const;
    const std::array<EquipSlotData, 4>& GetAllEquipData()               const;
    void SetAllEquipData(const std::array<EquipSlotData, 4>& data);

    // ── 視覺更新 ─────────────────────────────────────────────────────
    void StartDragging(glm::vec2 mousePos) override;
    void StopDragging()                    override;
    void SetScale(float scale)             override;
    void UpdateVisualPositions()           override;
    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override;

protected:
    virtual Util::Color HealthTextColor() const { return Util::Color(255, 255, 255); }

    int   m_BaseHealth, m_BaseAttack, m_BaseDefense;
    float m_BaseAttackSpeed, m_BaseHitChance;

    int   m_CombatHealth;
    int   m_CombatAttack;
    int   m_CombatDefense;
    float m_CombatAttackSpeed;
    float m_CombatHitChance;
    int   m_CurrentHealth;

    // 狀態計時器
    float m_StunTimerMs        = 0.0f;
    float m_BleedTimerMs       = 0.0f;
    float m_BleedTickMs        = 0.0f; // 距下次 bleed tick 的剩餘時間
    float m_PoisonTimerMs      = 0.0f;
    float m_PoisonTickMs       = 0.0f;
    float m_InvulnerableTimerMs = 0.0f;
    float m_FrenzyTimerMs      = 0.0f;

    std::vector<EffectData>            m_BaseEffects; // 角色自身效果（來自 JSON）
    std::vector<EffectData>            m_Effects;     // 合併後效果（base + 裝備）
    std::array<EquipSlotData, 4>       m_Equips{};
    std::shared_ptr<Util::GameObject>  m_HealthText;

    // 重建 m_Effects = m_BaseEffects + 所有裝備的 effects
    void RebuildEffects();
};

#endif // STACKLANDS_COMBATCARD_HPP
