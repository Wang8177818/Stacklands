//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_CARDDATA_HPP
#define STACKLANDS_CARDDATA_HPP

#pragma once
#include <string>
#include <vector>
#include "cards/Card.hpp"        // CardType, EquipSlot (also transitively includes EffectData.hpp)
#include "data/EffectData.hpp"  // explicit include for EffectData

// 裝備插槽資料（CharacterCard / MonsterCard 共用）
struct EquipSlotData {
    std::string name;
    int   bonusAtk       = 0;
    int   bonusHp        = 0;
    int   bonusDef       = 0;
    float bonusAtkSpd    = 0.0f;
    float bonusHitChance = 0.0f;
    std::vector<EffectData> effects;  // 裝備附帶的特效
};

// 生成卡片所需的資料
struct CardSpawnData {
    std::string name;
    int sellValue;
    CardType type;
    std::string iconPath;
    float scale, attackSpeed, hitChance;
    int health, attack, defense, foodConsumption; // 0 代表非人物卡
    std::string description;

    EquipSlot equipSlot = EquipSlot::NONE;

    // 食物卡專用
    int nutritionValue = 0;

    // 結構卡專用
    int resourceCount = 0;
    float time = 10.0f;
    std::vector<std::pair<std::string, int>> spawnCards;

    // 動物卡專用
    std::vector<std::pair<std::string, int>> dropCards;
    std::string abilityName;
    float abilityCooldown = 0.0f;

    // 地點卡專用
    int maxGathers = 0;
    std::vector<std::pair<int, std::string>> guaranteedDrops;

    // 背景覆寫（空字串 = 使用各類型預設背景）
    std::string backgroundPath;

    // 戰鬥特效列表
    std::vector<EffectData> effects;
};

struct Recipe {
    std::vector<std::string> requiredCards;
    std::string resultCardName;
    float craftTime;
};

#endif //STACKLANDS_CARDDATA_HPP
