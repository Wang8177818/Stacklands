#include "cards/CombatCard.hpp"
#include "data/GameConstants.hpp"
#include <algorithm>

CombatCard::CombatCard(float x, float y, const std::string& name, int sellValue,
                        CardType type, float scale,
                        int health, int attack, int defense,
                        float attackSpeed, float hitChance)
    : Card(x, y, name, sellValue, type, scale),
      m_BaseHealth(health), m_BaseAttack(attack), m_BaseDefense(defense),
      m_BaseAttackSpeed(attackSpeed), m_BaseHitChance(hitChance),
      m_CombatHealth(health), m_CombatAttack(attack), m_CombatDefense(defense),
      m_CombatAttackSpeed(attackSpeed), m_CombatHitChance(hitChance),
      m_CurrentHealth(health)
{}

float CombatCard::GetAttackSpeed() const {
    return m_FrenzyTimerMs > 0.0f ? m_CombatAttackSpeed * 0.5f : m_CombatAttackSpeed;
}

void CombatCard::ApplyStun(float durationMs) {
    if (durationMs > m_StunTimerMs) m_StunTimerMs = durationMs;
}
void CombatCard::UpdateStun(float dtMs) {
    if (m_StunTimerMs > 0.0f) m_StunTimerMs = std::max(0.0f, m_StunTimerMs - dtMs);
}

void CombatCard::ApplyBleed(float durationMs) {
    if (durationMs > m_BleedTimerMs) {
        m_BleedTimerMs = durationMs;
        if (m_BleedTickMs <= 0.0f) m_BleedTickMs = 1000.0f;
    }
}
void CombatCard::ApplyPoison(float durationMs) {
    if (durationMs > m_PoisonTimerMs) {
        m_PoisonTimerMs = durationMs;
        if (m_PoisonTickMs <= 0.0f) m_PoisonTickMs = 1000.0f;
    }
}
void CombatCard::ApplyInvulnerable(float durationMs) {
    if (durationMs > m_InvulnerableTimerMs) m_InvulnerableTimerMs = durationMs;
}
void CombatCard::ApplyFrenzy(float durationMs) {
    if (durationMs > m_FrenzyTimerMs) m_FrenzyTimerMs = durationMs;
}
void CombatCard::SetEffects(const std::vector<EffectData>& effects) {
    m_BaseEffects = effects;
    RebuildEffects();
}

void CombatCard::RebuildEffects() {
    m_Effects = m_BaseEffects;
    for (const auto& slot : m_Equips)
        for (const auto& eff : slot.effects)
            m_Effects.push_back(eff);
}

void CombatCard::HealBy(int amount) {
    m_CurrentHealth = std::min(m_CurrentHealth + amount, m_CombatHealth);
    if (m_HealthText)
        RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), HealthTextColor());
}

void CombatCard::UpdateCombatStates(float dtMs) {
    UpdateStun(dtMs);

    if (m_BleedTimerMs > 0.0f) {
        m_BleedTimerMs = std::max(0.0f, m_BleedTimerMs - dtMs);
        m_BleedTickMs -= dtMs;
        if (m_BleedTickMs <= 0.0f) {
            m_BleedTickMs += 1000.0f;
            TakeDamage(1);
        }
    }
    if (m_PoisonTimerMs > 0.0f) {
        m_PoisonTimerMs = std::max(0.0f, m_PoisonTimerMs - dtMs);
        m_PoisonTickMs -= dtMs;
        if (m_PoisonTickMs <= 0.0f) {
            m_PoisonTickMs += 1000.0f;
            TakeDamage(1);
        }
    }
    if (m_InvulnerableTimerMs > 0.0f)
        m_InvulnerableTimerMs = std::max(0.0f, m_InvulnerableTimerMs - dtMs);
    if (m_FrenzyTimerMs > 0.0f)
        m_FrenzyTimerMs = std::max(0.0f, m_FrenzyTimerMs - dtMs);
}

void CombatCard::TakeDamage(int dmg) {
    if (m_InvulnerableTimerMs > 0.0f) return;
    m_CurrentHealth -= dmg;
    if (m_CurrentHealth < 0) m_CurrentHealth = 0;
    if (m_HealthText)
        RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), HealthTextColor());
}

void CombatCard::SetCurrentHealth(int hp) {
    if (hp < 0) hp = 0;
    if (hp > m_CombatHealth) hp = m_CombatHealth;
    m_CurrentHealth = hp;
    if (m_HealthText)
        RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), HealthTextColor());
}

void CombatCard::RecalculateStats() {
    int   totalAtk = m_BaseAttack;
    int   totalHp  = m_BaseHealth;
    int   totalDef = m_BaseDefense;
    float totalSpd = m_BaseAttackSpeed;
    float totalHit = m_BaseHitChance;
    for (const auto& e : m_Equips) {
        totalAtk += e.bonusAtk;
        totalHp  += e.bonusHp;
        totalDef += e.bonusDef;
        totalSpd += e.bonusAtkSpd;
        totalHit += e.bonusHitChance;
    }
    m_CombatAttack      = totalAtk;
    m_CombatHealth      = totalHp;
    m_CombatDefense     = totalDef;
    m_CombatAttackSpeed = totalSpd;
    m_CombatHitChance   = totalHit;
    if (m_CurrentHealth > m_CombatHealth) m_CurrentHealth = m_CombatHealth;
    UpdateVisualPositions();
}

void CombatCard::StoreEquipment(EquipSlot slot, const std::string& name,
                                  int bonusAtk, int bonusHp, int bonusDef,
                                  float bonusAtkSpd, float bonusHitChance,
                                  const std::vector<EffectData>& effects) {
    auto& e          = m_Equips[static_cast<int>(slot)];
    e.name           = name;
    e.bonusAtk       = bonusAtk;
    e.bonusHp        = bonusHp;
    e.bonusDef       = bonusDef;
    e.bonusAtkSpd    = bonusAtkSpd;
    e.bonusHitChance = bonusHitChance;
    e.effects        = effects;
    RecalculateStats();
    RebuildEffects();
}

void CombatCard::ClearEquipment(EquipSlot slot) {
    m_Equips[static_cast<int>(slot)] = {};
    RecalculateStats();
    RebuildEffects();
}

const std::string& CombatCard::GetEquipName(EquipSlot slot) const {
    return m_Equips[static_cast<int>(slot)].name;
}

const EquipSlotData& CombatCard::GetEquipSlotData(EquipSlot slot) const {
    return m_Equips[static_cast<int>(slot)];
}

const std::array<EquipSlotData, 4>& CombatCard::GetAllEquipData() const {
    return m_Equips;
}

void CombatCard::SetAllEquipData(const std::array<EquipSlotData, 4>& data) {
    m_Equips = data;
    RecalculateStats();
    RebuildEffects();
}

void CombatCard::StartDragging(glm::vec2 mousePos) {
    Card::StartDragging(mousePos);
    if (m_HealthText) m_HealthText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
}

void CombatCard::StopDragging() {
    Card::StopDragging();
    if (m_HealthText) m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
}

void CombatCard::SetScale(float scale) {
    Card::SetScale(scale);
    // 血量文字只更新 transform 縮放，不重建紋理（避免縮放卡頓）
    if (m_HealthText)
        m_HealthText->m_Transform.scale = {m_Scale, m_Scale};
}

void CombatCard::UpdateVisualPositions() {
    Card::UpdateVisualPositions();
    if (m_HealthText) {
        m_HealthText->m_Transform.translation = glm::vec2(
            m_X + m_Width  * GameConstants::HEALTH_OFFSET_X,
            m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
        m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
        RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), HealthTextColor());
    }
}

std::vector<std::shared_ptr<Util::GameObject>> CombatCard::GetGameObjects() {
    auto objs = Card::GetGameObjects();
    if (m_HealthText) objs.push_back(m_HealthText);
    return objs;
}
