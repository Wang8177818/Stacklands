#pragma once
#ifndef STACKLANDS_COMBATARENA_HPP
#define STACKLANDS_COMBATARENA_HPP

#include "Card.hpp"
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

// 戰鬥視覺場域：在戰鬥開始時展開背景、排列雙方卡片、播放攻擊衝刺動畫
class CombatArena {
public:
    CombatArena(Util::Renderer& renderer,
                const std::vector<std::weak_ptr<Card>>& fighters,
                const std::weak_ptr<Card>& target);
    ~CombatArena();

    void Update(float dtMs);

    // 戰鬥中途新增戰鬥員
    void AddFighter(const std::shared_ptr<Card>& fighter);

    // 觸發攻擊衝刺動畫（fighter → target / target → 指定 fighter）
    void TriggerAttack(const std::shared_ptr<Card>& attacker);
    void TriggerCounter(const std::shared_ptr<Card>& target,
                        const std::shared_ptr<Card>& chosenFighter);

    // 同步世界座標（Pan / Zoom 時呼叫）
    void MoveBy(glm::vec2 delta);
    void ScaleAroundPivot(float ratio, glm::vec2 pivot);

private:
    // 座標常數（world units；basic_scale=0.1 → 卡片約 170px 寬）
    static constexpr float SLOT_SPACING = 105.0f;  // 每個槽位水平間距
    static constexpr float FIGHTER_Y   = -95.0f;   // 角色行偏移（中心下方）
    static constexpr float TARGET_Y    =  95.0f;   // 敵人行偏移（中心上方）
    static constexpr float PAD_X       =  70.0f;   // 背景左右留白
    static constexpr float PAD_Y       =  70.0f;   // 背景上下留白
    static constexpr float LERP_SPEED  =  10.0f;   // 歸位插值速度
    static constexpr float ATK_SPEED   = 800.0f;   // 衝刺速度 (px/s)
    static constexpr float RTN_SPEED   = 520.0f;   // 回位速度 (px/s)
    static constexpr float BG_IMG_W    = 850.0f;  // 底圖原始寬度 (px)
    static constexpr float BG_IMG_H    = 1250.0f; // 底圖原始高度 (px)
    static constexpr int   BG_Z        = 5;       // Z 低於卡片(10)

    struct Slot {
        std::weak_ptr<Card> card;
        glm::vec2           home;
    };

    struct AttackAnim {
        std::weak_ptr<Card> card;
        glm::vec2           home;
        glm::vec2           aim;
        bool                returning = false;
        bool                done      = false;
    };

    void ComputeLayout();
    void ResizeBackground();
    void LerpToward(const std::shared_ptr<Card>& card, glm::vec2 target, float dtMs);
    bool HasActiveAnim(const std::shared_ptr<Card>& card) const;

    Util::Renderer&                   m_Renderer;
    std::shared_ptr<Util::GameObject> m_Bg;
    std::vector<Slot>                 m_Fighters;
    Slot                              m_Target;
    std::vector<AttackAnim>           m_Anims;
    glm::vec2                         m_Center;
};

#endif // STACKLANDS_COMBATARENA_HPP
