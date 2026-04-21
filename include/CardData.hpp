//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_CARDDATA_HPP
#define STACKLANDS_CARDDATA_HPP

#pragma once
#include <string>
#include "Card.hpp"

// 這個結構用來儲存生成一張卡片所需的「所有配方」
struct CardSpawnData {
    std::string name;
    int sellValue;
    CardType type;
    std::string iconPath;
    float scale;
    int health, attack;        // 0 代表非人物卡
    int foodConsumption = 0;   // 每月消耗食物數（人物卡專用）
    std::string description;   // 卡片敘述文字

    EquipSlot equipSlot = EquipSlot::NONE;

    // 食物卡專用
    int nutritionValue = 0;

    // 結構卡專用：可採集次數與採集後依權重生成的卡片清單
    int resourceCount = 0;
    std::vector<std::pair<std::string, int>> spawnCards; // {卡片名稱, 權重}
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