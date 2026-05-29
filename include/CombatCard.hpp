#pragma once
#ifndef STACKLANDS_COMBATCARD_HPP
#define STACKLANDS_COMBATCARD_HPP

#include "Card.hpp"
#include "CardData.hpp"
#include <array>

// 所有有戰鬥能力的卡片共用基底類別
// 統一管理：5種戰鬥屬性（含裝備加成計算）、血量顯示文字
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
    float GetAttackSpeed() const override { return m_CombatAttackSpeed; }
    float GetHitChance()   const override { return m_CombatHitChance; }
    int   GetHealth()      const override { return m_CurrentHealth; }
    int   GetMaxHealth()   const          { return m_CombatHealth; }

    int   GetBaseAttack()  const { return m_BaseAttack; }
    int   GetBaseHealth()  const { return m_BaseHealth; }
    int   GetBaseDefense() const { return m_BaseDefense; }

    // ── 裝備系統（CharacterCard 主要使用）──────────────────────────
    void StoreEquipment(EquipSlot slot, const std::string& name,
                        int bonusAtk, int bonusHp, int bonusDef,
                        float bonusAtkSpd, float bonusHitChance);
    void ClearEquipment(EquipSlot slot);
    void RecalculateStats();
    const std::string&                  GetEquipName(EquipSlot slot)    const;
    const EquipSlotData&                GetEquipSlotData(EquipSlot slot) const;
    const std::array<EquipSlotData, 4>& GetAllEquipData()               const;
    void SetAllEquipData(const std::array<EquipSlotData, 4>& data);

    // ── 視覺更新（統一處理 m_HealthText）─────────────────────────────
    void StartDragging(glm::vec2 mousePos) override;
    void StopDragging()                    override;
    void SetScale(float scale)             override;
    void UpdateVisualPositions()           override;
    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override;

protected:
    // 子類別覆寫以改變血量文字顏色（CharacterCard=白、MonsterCard=紅、AnimalCard=黑）
    virtual Util::Color HealthTextColor() const { return Util::Color(255, 255, 255); }

    int   m_BaseHealth, m_BaseAttack, m_BaseDefense;
    float m_BaseAttackSpeed, m_BaseHitChance;

    int   m_CombatHealth;       // 最大 HP（基礎 + 裝備加成）
    int   m_CombatAttack;
    int   m_CombatDefense;
    float m_CombatAttackSpeed;
    float m_CombatHitChance;
    int   m_CurrentHealth;

    std::array<EquipSlotData, 4>       m_Equips{};
    std::shared_ptr<Util::GameObject>  m_HealthText;   // 由子類別在建構子中初始化
};

#endif // STACKLANDS_COMBATCARD_HPP
