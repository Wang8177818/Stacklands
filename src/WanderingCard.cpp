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
    RollPause();
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

// 統一的「移動一段 → 停頓 → 再移動」狀態機，追擊與隨機漫遊共用同一條路徑，
// 差別只在於每段目標如何挑選（PickNextTarget）與停頓多久（RollPause）。
void WanderingCard::UpdateWander(float dtMs) {
    // 戰鬥中、被拖曳中、或疊在卡上時完全靜止（不移動也不累積停頓）
    if (m_InCombat || m_IsDragging || m_CardBelow != nullptr) return;

    // 移動階段：朝當前段目標趨近，抵達後進入停頓並擲下一次停頓時間
    if (m_IsMoving) {
        if (StepTowardTarget(dtMs)) {
            m_IsMoving  = false;
            m_MoveTimer = 0.0f;
            RollPause();
        }
        return;
    }

    // 停頓階段：等冷卻結束後挑選下一段目標再出發
    m_MoveTimer += dtMs;
    if (m_MoveTimer < m_MoveCooldown) return;

    PickNextTarget();
    m_IsMoving = true;
}

bool WanderingCard::StepTowardTarget(float dtMs) {
    const float dtSec = dtMs / 1000.0f;
    const float alpha = 1.0f - std::exp(-m_WanderParams.speed * dtSec);
    m_X += (m_TargetX - m_X) * alpha;
    m_Y += (m_TargetY - m_Y) * alpha;
    UpdateVisualPositions();

    const float dx = m_TargetX - m_X, dy = m_TargetY - m_Y;
    if (dx * dx + dy * dy < 1.0f) {
        m_X = m_TargetX;
        m_Y = m_TargetY;
        return true;
    }
    return false;
}

void WanderingCard::PickNextTarget() {
    if (m_HasSeek) {
        // 追擊：朝角色方向只踏一段（限制單段距離），而非連續直線靠近
        const float dx = m_SeekX - m_X, dy = m_SeekY - m_Y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= m_WanderParams.seekStep || dist < 1e-4f) {
            m_TargetX = m_SeekX;
            m_TargetY = m_SeekY;
        } else {
            const float k = m_WanderParams.seekStep / dist;
            m_TargetX = m_X + dx * k;
            m_TargetY = m_Y + dy * k;
        }
    } else {
        // 隨機漫遊：在當前位置附近隨機挑一段
        std::uniform_real_distribution<float> offDist(m_WanderParams.offsetMin, m_WanderParams.offsetMax);
        m_TargetX = m_X + offDist(s_Rng);
        m_TargetY = m_Y + offDist(s_Rng);
    }
}

void WanderingCard::RollPause() {
    const float lo = m_HasSeek ? m_WanderParams.seekPauseMin : m_WanderParams.cooldownMin;
    const float hi = m_HasSeek ? m_WanderParams.seekPauseMax : m_WanderParams.cooldownMax;
    std::uniform_real_distribution<float> d(lo, hi);
    m_MoveCooldown = d(s_Rng);
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
