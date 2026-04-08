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
    int health, attack; // 0 代表非人物卡

    EquipSlot equipSlot = EquipSlot::NONE;
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