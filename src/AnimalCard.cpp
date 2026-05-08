//
// Created by m0938 on 2026/5/8.
//

#include "AnimalCard.hpp"
#include "GameConstants.hpp"
#include "Util/Time.hpp"
#include <random>
#include <numeric>

static std::mt19937 s_Rng{ std::random_device{}() };

AnimalCard::AnimalCard(float x, float y,
                       const std::string& name,
                       const std::string& iconPath,
                       float scale,
                       int health, int attack,
                       const std::vector<std::pair<std::string, int>>& dropCards,
                       const std::string& abilityName,
                       float abilityCooldown,
                       std::function<void(const std::string&, float, float)> spawnCallback)
    : Card(x, y, name, 0, CardType::ANIMAL, scale),
      m_Health(health), m_Attack(attack),
      m_AbilityCooldown(abilityCooldown * 1000.0f), // 轉毫秒
      m_AbilityName(abilityName),
      m_DropCards(dropCards),
      m_SpawnCallback(std::move(spawnCallback))
{
    SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Animal.png");
    SetIconImage(iconPath);

    m_HealthText = InitLabelText(std::to_string(m_Health), Util::Color(0, 0, 0));

    // 隨機初始移動冷卻 3~6 秒
    std::uniform_real_distribution<float> moveDist(3000.0f, 6000.0f);
    m_MoveCooldown = moveDist(s_Rng);

    UpdateVisualPositions();
}

void AnimalCard::Update() {
    Card::Update();

    float dtMs = static_cast<float>(Util::Time::GetDeltaTimeMs());

    m_MoveTimer += dtMs;

    if (m_AbilityCooldown > 0.0f && m_SpawnCallback) {
        m_AbilityTimer += dtMs;
        if (m_AbilityTimer >= m_AbilityCooldown) {
            m_AbilityTimer = 0.0f;
            ProcessSpecialAbility();
        }
    }

    if (m_IsDragging || m_CardBelow != nullptr) return;

    if (m_MoveTimer >= m_MoveCooldown) {
        m_MoveTimer = 0.0f;
        std::uniform_real_distribution<float> moveDist(3000.0f, 6000.0f);
        m_MoveCooldown = moveDist(s_Rng);

        std::uniform_real_distribution<float> offset(-80.0f, 80.0f);
        MoveBy({ offset(s_Rng), offset(s_Rng) });
        Card::UpdateVisualPositions();
    }
}

void AnimalCard::ProcessSpecialAbility() {
    if (m_AbilityName == "produce_egg") {
        m_SpawnCallback("Egg", m_X, m_Y);
    }
    else if (m_AbilityName == "produce_milk") {
        m_SpawnCallback("Milk", m_X, m_Y);
    }
    else if (m_AbilityName == "produce_poop") {
        m_SpawnCallback("Poop", m_X, m_Y);
    }
}

void AnimalCard::TakeDamage(int dmg) {
    m_Health -= dmg;
    if (m_Health < 0) m_Health = 0;
    UpdateHealthText();
}

void AnimalCard::UpdateHealthText() {
    RebuildLabelText(m_HealthText, std::to_string(m_Health), Util::Color(0, 0, 0));
}

void AnimalCard::UpdateVisualPositions() {
    Card::UpdateVisualPositions();
    if (m_HealthText) {
        m_HealthText->m_Transform.translation = glm::vec2(
            m_X + m_Width  * GameConstants::HEALTH_OFFSET_X,
            m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
        m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
        RebuildLabelText(m_HealthText, std::to_string(m_Health), Util::Color(0, 0, 0));
    }
}

void AnimalCard::SetScale(float scale) {
    Card::SetScale(scale);
    if (m_HealthText) RebuildLabelText(m_HealthText, std::to_string(m_Health), Util::Color(0, 0, 0));
}

std::string AnimalCard::RollDrop() const {
    if (m_DropCards.empty()) return "";

    int total = 0;
    for (auto& [name, w] : m_DropCards) total += w;
    if (total <= 0) return "";

    std::uniform_int_distribution<int> dist(0, total - 1);
    int roll = dist(s_Rng);
    for (auto& [name, w] : m_DropCards) {
        roll -= w;
        if (roll < 0) return name;
    }
    return m_DropCards.back().first;
}
