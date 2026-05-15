//
// Created by m0938 on 2026/5/15.
//

#ifndef STACKLANDS_ATTACKRESOLVER_HPP
#define STACKLANDS_ATTACKRESOLVER_HPP

#pragma once

namespace Combat {

inline int BlockValue(int defense) {
    if (defense <= 0) return 0;
    if (defense <= 2) return 1;
    if (defense <= 5) return 2;
    return 3;
}

// 單次攻擊最終傷害
//   rawDmg      : 攻擊方的基礎傷害值
//   defense     : 防禦方的 defense int
//   bonusDmgRoll: 50% 機率額外 +1 傷害
//   pierceRoll  : 50% 機率穿透（即使全擋仍造成 1 傷害）
inline int ResolveDamage(int rawDmg, int defense, bool bonusDmgRoll, bool pierceRoll) {
    int dmg   = rawDmg + (bonusDmgRoll ? 1 : 0);
    int block = BlockValue(defense);
    dmg -= block;
    if (dmg <= 0) return pierceRoll ? 1 : 0;
    return dmg;
}

// 命中判定：roll < hitChance 則命中
inline bool IsHit(float hitChance, float roll) {
    return roll < hitChance;
}

} // namespace Combat

#endif // STACKLANDS_ATTACKRESOLVER_HPP
