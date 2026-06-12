#include "WanderingCard.hpp"
#include "GameConstants.hpp"
#include <cmath>
#include <random>
#include <numeric>

std::mt19937 WanderingCard::s_Rng{ std::random_device{}() };

WanderingCard::WanderingCard(float x, float y, const std::string& name, int sellValue,
                              CardType type, float scale,
                              int health, int attack, int defense,
                              float attackSpeed, float hitChance,
                              const std::vector<std::pair<std::string, int>>& dropCards,
                              WanderParams wander)
    : CombatCard(x, y, name, sellValue, type, scale, health, attack, defense, attackSpeed, hitChance),
      m_WanderParams(wander),
      m_TargetX(x), m_TargetY(y),
      m_DropCards(dropCards)
{
    std::uniform_real_distribution<float> d(m_WanderParams.cooldownMin, m_WanderParams.cooldownMax);
    m_MoveCooldown = d(s_Rng);
}

void WanderingCard::StartDragging(glm::vec2 mousePos) {
    CombatCard::StartDragging(mousePos);
    m_TargetX  = m_X;
    m_TargetY  = m_Y;
    m_IsMoving = false;
}

void WanderingCard::MoveBy(glm::vec2 delta) {
    Card::MoveBy(delta);
    m_TargetX += delta.x;
    m_TargetY += delta.y;
}

void WanderingCard::ScaleAroundPivot(float ratio, glm::vec2 pivot) {
    Card::ScaleAroundPivot(ratio, pivot);
    m_TargetX = pivot.x + (m_TargetX - pivot.x) * ratio;
    m_TargetY = pivot.y + (m_TargetY - pivot.y) * ratio;
}

void WanderingCard::UpdateWander(float dtMs) {
    m_MoveTimer += dtMs;

    // 戰鬥中、被拖曳中、或疊在卡上時不移動，但計時繼續累積
    if (m_InCombat || m_IsDragging || m_CardBelow != nullptr) return;

    // 主動追擊：覆寫隨機漫遊，朝目標座標前進
    if (m_HasSeek) {
        const float dtSec = dtMs / 1000.0f;
        const float alpha = 1.0f - std::exp(-m_WanderParams.speed * dtSec);
        m_X += (m_SeekX - m_X) * alpha;
        m_Y += (m_SeekY - m_Y) * alpha;
        m_TargetX  = m_X;      // 同步隨機漫遊目標，取消追擊後不會瞬移
        m_TargetY  = m_Y;
        m_IsMoving = false;
        m_MoveTimer = 0.0f;    // 重置漫遊冷卻
        UpdateVisualPositions();
        return;
    }

    if (m_IsMoving) {
        const float dtSec = dtMs / 1000.0f;
        const float alpha = 1.0f - std::exp(-m_WanderParams.speed * dtSec);
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

    if (m_MoveTimer >= m_MoveCooldown) {
        m_MoveTimer = 0.0f;
        std::uniform_real_distribution<float> cdDist(m_WanderParams.cooldownMin, m_WanderParams.cooldownMax);
        m_MoveCooldown = cdDist(s_Rng);
        std::uniform_real_distribution<float> offDist(m_WanderParams.offsetMin, m_WanderParams.offsetMax);
        m_TargetX  = m_X + offDist(s_Rng);
        m_TargetY  = m_Y + offDist(s_Rng);
        m_IsMoving = true;
    }
}

std::string WanderingCard::RollDrop() const {
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
