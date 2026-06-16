#ifndef STACKLANDS_CARDFACTORY_HPP
#define STACKLANDS_CARDFACTORY_HPP

#pragma once
#include "cards/Card.hpp"
#include "data/CardData.hpp"
#include "data/ISpawnListener.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

class CardFactory {
public:
    using SpawnCb   = ISpawnListener*;
    using CreatorFn = std::function<std::shared_ptr<Card>(float, float, const CardSpawnData&, int&, SpawnCb)>;

    static std::shared_ptr<Card> Create(float x, float y, const CardSpawnData& data,
                                        int& maxCardCount, SpawnCb listener = nullptr);

    // 允許在不修改 CardFactory 本身的情況下新增卡片類型（OCP）
    static void Register(CardType type, CreatorFn creator);

private:
    static std::unordered_map<CardType, CreatorFn>& Registry();
};

#endif // STACKLANDS_CARDFACTORY_HPP
