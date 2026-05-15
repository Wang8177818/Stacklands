//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_CARDDATA_HPP
#define STACKLANDS_CARDDATA_HPP

#pragma once
#include <string>
#include "Card.hpp"

// 裝備插槽資料（CharacterCard / MonsterCard 共用）
struct EquipSlotData {
    std::string name;
    int   bonusAtk       = 0;
    int   bonusHp        = 0;
    int   bonusDef       = 0;
    float bonusAtkSpd    = 0.0f;
    float bonusHitChance = 0.0f;
};

// 生成卡片所需的資料
struct CardSpawnData {
    std::string name;
    int sellValue;
    CardType type;
    std::string iconPath;
    float scale, attackSpeed, hitChance;
    int health, attack, defense, foodConsumption; // 0 代表非人物卡
    std::string description;   // 卡片敘述文字

    EquipSlot equipSlot = EquipSlot::NONE;

    // 食物卡專用
    int nutritionValue = 0;

    // 結構卡專用：
    int resourceCount = 0;
    float time = 10.0f;
    std::vector<std::pair<std::string, int>> spawnCards; // {卡片名稱, 權重}

    // 動物卡專用：
    std::vector<std::pair<std::string, int>> dropCards;  // 死亡掉落 {卡片名稱, 權重}
    std::string abilityName;                             // 特殊能力識別字 (produce_egg 等)
    float abilityCooldown = 0.0f;                        // 能力冷卻秒數
};

struct Recipe {
    // 1. 需要的卡片名稱清單 (例如: {"Villager", "Wood", "Stone"})
    std::vector<std::string> requiredCards;

    // 2. 合成出來的結果名稱
    std::string resultCardName;

    // 3. 需要花費的時間 (秒)
    float craftTime;
};


#endif //STACKLANDS_CARDDATA_HPP