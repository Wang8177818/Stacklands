#pragma once
#ifndef STACKLANDS_EFFECTDATA_HPP
#define STACKLANDS_EFFECTDATA_HPP

struct EffectData {
    enum class Type {
        Stun,         // 暈眩目標
        Bleed,        // 流血 DoT
        Poison,       // 中毒 DoT
        CriticalHit,  // 暴擊（傷害 x2）
        Lifesteal,    // 吸血（回復等同傷害量）
        DamageAll,    // 對所有敵人造成傷害
        DamageRandom, // 對隨機一個敵人造成傷害
        Heal,         // 回復血量
        Frenzy,       // 狂暴（攻速 x2）
        Invulnerable  // 無敵（免疫傷害）
    };
    enum class Target {
        DirectTarget,      // 被直接攻擊的目標
        Self,              // 攻擊者自身
        RandomEnemy,       // 敵方隨機一個
        AllEnemies,        // 敵方全體
        RandomFriendly,    // 友方隨機一個
        FriendlyLowestHp,  // 友方血量最低者
        AllFriendlies      // 友方全體
    };

    Type   type;
    Target target   = Target::DirectTarget;
    float  chance   = 0.0f;  // 0–1
    float  duration = 5.0f;  // 秒（Stun / Bleed / Poison / Frenzy / Invulnerable）
    int    value    = 2;     // 數值（Heal 回復量 / Damage 傷害量）
    bool   passive  = false; // true = 即使未命中也觸發（原文字前綴 *）
};

#endif // STACKLANDS_EFFECTDATA_HPP
