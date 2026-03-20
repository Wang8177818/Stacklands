//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_CARDDATA_HPP
#define STACKLANDS_CARDDATA_HPP

#pragma once
#include <string>
#include "Card.hpp"

// 這個結構用來儲存生成一張卡片所需的所有配方
struct CardSpawnData {
    std::string name;
    int sellValue;
    CardType type;
    std::string iconPath;
    float scale;
    int health; // 0 代表非人物卡

};

#endif //STACKLANDS_CARDDATA_HPP