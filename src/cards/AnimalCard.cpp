#include "cards/AnimalCard.hpp"
#include "core/EventManager.hpp"
#include "data/GameConstants.hpp"

AnimalCard::AnimalCard(float x, float y, const std::string& name, const std::string& iconPath,
                        float scale,
                        int health, int attack, int defense,
                        float attackSpeed, float hitChance,
                        const std::vector<std::pair<std::string, int>>& dropCards,
                        const std::string& abilityName, float abilityCooldown,
                        ISpawnListener* listener)
    : WanderingCard(x, y, name, 0, CardType::ANIMAL, scale,
                    health, attack, defense, attackSpeed, hitChance,
                    dropCards,
                    WanderParams{5.0f, 3000.0f, 6000.0f, -80.0f, 80.0f}),
      m_AbilityCooldown(abilityCooldown * 1000.0f),
      m_AbilityName(abilityName),
      m_Listener(listener)
{
    SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Animal.png");
    SetIconImage(iconPath);
    m_HealthText = InitLabelText(std::to_string(health), HealthTextColor());
    UpdateVisualPositions();
}

void AnimalCard::Update() {
    Card::Update();
    float dtMs = EventManager::GetScaledDtMs();

    // 特殊能力計時（戰鬥中仍繼續冷卻）
    if (m_AbilityCooldown > 0.0f && m_Listener) {
        m_AbilityTimer += dtMs;
        if (m_AbilityTimer >= m_AbilityCooldown) {
            m_AbilityTimer = 0.0f;
            ProcessSpecialAbility();
        }
    }

    UpdateWander(dtMs);
}

void AnimalCard::ProcessSpecialAbility() {
    if      (m_AbilityName == "produce_egg")  m_Listener->OnSpawn("Egg",  m_X, m_Y);
    else if (m_AbilityName == "produce_milk") m_Listener->OnSpawn("Milk", m_X, m_Y);
    else if (m_AbilityName == "produce_poop") m_Listener->OnSpawn("Poop", m_X, m_Y);
}
