#include "CombatCard.hpp"
#include "GameConstants.hpp"

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

void CombatCard::TakeDamage(int dmg) {
    m_CurrentHealth -= dmg;
    if (m_CurrentHealth < 0) m_CurrentHealth = 0;
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
                                  float bonusAtkSpd, float bonusHitChance) {
    auto& e          = m_Equips[static_cast<int>(slot)];
    e.name           = name;
    e.bonusAtk       = bonusAtk;
    e.bonusHp        = bonusHp;
    e.bonusDef       = bonusDef;
    e.bonusAtkSpd    = bonusAtkSpd;
    e.bonusHitChance = bonusHitChance;
    RecalculateStats();
}

void CombatCard::ClearEquipment(EquipSlot slot) {
    m_Equips[static_cast<int>(slot)] = {};
    RecalculateStats();
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
    if (m_HealthText)
        RebuildLabelText(m_HealthText, std::to_string(m_CurrentHealth), HealthTextColor());
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
