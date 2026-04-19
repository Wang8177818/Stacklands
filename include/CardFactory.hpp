//
// CardFactory.hpp — 負責依 CardSpawnData 建立對應卡牌物件
// 將建立邏輯從 CardManager 拆出，CardManager 不再需要知道每種子類別
// Created during Phase 1 refactoring, 2026/4/18
//

#ifndef STACKLANDS_CARDFACTORY_HPP
#define STACKLANDS_CARDFACTORY_HPP

#pragma once
#include "Card.hpp"
#include "CardData.hpp"
#include <memory>

class CardFactory {
public:
    static std::shared_ptr<Card> Create(float x, float y, const CardSpawnData& data);
};

#endif // STACKLANDS_CARDFACTORY_HPP
