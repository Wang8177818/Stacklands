#include "cards/MonsterCard.hpp"
#include "core/EventManager.hpp"
#include <cmath>

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
    float dtMs = EventManager::GetScaledDtMs();

    if (!m_HasTarget) {
        // 場上無角色時，隨機漫遊
        UpdateWander(dtMs);
        return;
    }

    // 有追蹤目標時：使用與 UpdateWander 相同的間隔性移動方式
    // 但移動方向朝角色而非隨機

    m_MoveTimer += dtMs;

    // 戰鬥中、被拖曳中、或疊在卡上時不移動，但計時繼續累積
    if (m_InCombat || m_IsDragging || m_CardBelow != nullptr) return;

    // 平滑移動到目標位置（與 UpdateWander 相同的插值邏輯）
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

    // 冷卻結束時，朝角色方向移動一段距離
    if (m_MoveTimer >= m_MoveCooldown) {
        m_MoveTimer = 0.0f;
        std::uniform_real_distribution<float> cdDist(m_WanderParams.cooldownMin, m_WanderParams.cooldownMax);
        m_MoveCooldown = cdDist(s_Rng);

        // 計算朝角色方向的偏移量（移動一段固定距離，而非直接到達）
        float dx = m_ChaseX - m_X;
        float dy = m_ChaseY - m_Y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > 1.0f) {
            // 每次朝角色方向移動 60~100 像素
            std::uniform_real_distribution<float> stepDist(60.0f, 100.0f);
            float step = stepDist(s_Rng);
            if (step > dist) step = dist; // 不超過實際距離

            m_TargetX = m_X + (dx / dist) * step;
            m_TargetY = m_Y + (dy / dist) * step;
            m_IsMoving = true;
        }
    }
}
