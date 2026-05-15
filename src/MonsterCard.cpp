//
// Created by m0938 on 2026/5/15.
//

#include "MonsterCard.hpp"
#include "GameConstants.hpp"
#include "Util/Time.hpp"
#include <random>
#include <numeric>
#include <cmath>

static std::mt19937 s_MonsterRng{ std::random_device{}() };

void MonsterCard::Update() {
    Card::Update();

    float dtMs = static_cast<float>(Util::Time::GetDeltaTimeMs());
    m_MoveTimer += dtMs;

    // 在戰鬥中或被疊住時停止移動
    if (m_InCombat || m_CardBelow != nullptr) return;

    // 平滑插值移動（指數型緩出，與幀率無關）
    if (m_IsMoving) {
        const float dtSec = dtMs / 1000.0f;
        const float speed = 3.0f;
        const float alpha = 1.0f - std::exp(-speed * dtSec);
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

    // 計時：到時間則設定新目標
    if (m_MoveTimer >= m_MoveCooldown) {
        m_MoveTimer = 0.0f;
        std::uniform_real_distribution<float> cooldownDist(4000.0f, 8000.0f);
        m_MoveCooldown = cooldownDist(s_MonsterRng);

        std::uniform_real_distribution<float> offset(-100.0f, 100.0f);
        m_TargetX  = m_X + offset(s_MonsterRng);
        m_TargetY  = m_Y + offset(s_MonsterRng);
        m_IsMoving = true;
    }
}

std::string MonsterCard::RollDrop() const {
    if (m_DropCards.empty()) return "";

    int total = 0;
    for (auto& [name, w] : m_DropCards) total += w;
    if (total <= 0) return "";

    std::uniform_int_distribution<int> dist(0, total - 1);
    int roll = dist(s_MonsterRng);
    for (auto& [name, w] : m_DropCards) {
        roll -= w;
        if (roll < 0) return name;
    }
    return m_DropCards.back().first;
}
