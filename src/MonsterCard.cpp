#include "MonsterCard.hpp"
#include "EventManager.hpp"

MonsterCard::MonsterCard(float x, float y, const std::string& name, int sellValue,
                          const std::string& iconPath, float scale,
                          int health, int attack, int defense,
                          float attackSpeed, float hitChance,
                          const std::vector<std::pair<std::string, int>>& dropCards)
    : WanderingCard(x, y, name, sellValue, CardType::MONSTER, scale,
                    health, attack, defense, attackSpeed, hitChance,
                    dropCards,
                    WanderParams{3.0f, 4000.0f, 8000.0f, -100.0f, 100.0f})
{
    SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Monster.png");
    SetIconImage(iconPath);
    m_HealthText = InitLabelText(std::to_string(health), HealthTextColor());
    UpdateVisualPositions();
}

void MonsterCard::Update() {
    Card::Update();
    UpdateWander(EventManager::GetScaledDtMs());
}
