//
// Created by m0938 on 2026/5/8.
//

#include "AnimalCard.hpp"
#include "GameConstants.hpp"
#include "Util/Time.hpp"
#include <random>
#include <numeric>
#include <cmath>

static std::mt19937 s_Rng{ std::random_device{}() };

AnimalCard::AnimalCard(float x, float y,
                       const std::string& name,
                       const std::string& iconPath,
                       float scale,
                       int health, int attack, int defense,
                       float attackSpeed, float hitChance,
                       const std::vector<std::pair<std::string, int>>& dropCards,
                       const std::string& abilityName,
                       float abilityCooldown,
                       std::function<void(const std::string&, float, float)> spawnCallback)
    : Card(x, y, name, 0, CardType::ANIMAL, scale),
      m_Health(health), m_MaxHealth(health), m_Attack(attack),
      m_Defense(defense), m_AttackSpeed(attackSpeed), m_HitChance(hitChance),
      m_AbilityCooldown(abilityCooldown * 1000.0f),
      m_AbilityName(abilityName),
      m_DropCards(dropCards),
      m_SpawnCallback(std::move(spawnCallback)),
      m_TargetX(x), m_TargetY(y)
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

    // 平滑插值移動（指數型緩出，與幀率無關）
    if (m_IsMoving) {
        const float dtSec  = dtMs / 1000.0f;
        const float speed  = 5.0f;                          // 越大越快
        const float alpha  = 1.0f - std::exp(-speed * dtSec);
        m_X += (m_TargetX - m_X) * alpha;
        m_Y += (m_TargetY - m_Y) * alpha;

        float dx = m_TargetX - m_X, dy = m_TargetY - m_Y;
        if (dx * dx + dy * dy < 1.0f) {
            m_X = m_TargetX;
            m_Y = m_TargetY;
            m_IsMoving = false;
        }
        UpdateVisualPositions();
    }

    // 計時：到時間則設定新目標（不再立即瞬移）
    if (m_MoveTimer >= m_MoveCooldown) {
        m_MoveTimer = 0.0f;
        std::uniform_real_distribution<float> moveDist(3000.0f, 6000.0f);
        m_MoveCooldown = moveDist(s_Rng);

        std::uniform_real_distribution<float> offset(-80.0f, 80.0f);
        m_TargetX = m_X + offset(s_Rng);
        m_TargetY = m_Y + offset(s_Rng);
        m_IsMoving = true;
    }
}

void AnimalCard::StartDragging(glm::vec2 mousePos) {
    Card::StartDragging(mousePos);
    m_TargetX  = m_X;
    m_TargetY  = m_Y;
    m_IsMoving = false;
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
