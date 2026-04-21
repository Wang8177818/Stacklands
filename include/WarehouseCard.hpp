//
// Created by user on 2026/4/17.
//

#ifndef STACKLANDS_WAREHOUSECARD_HPP
#define STACKLANDS_WAREHOUSECARD_HPP

#include "BuildingCard.hpp"

class WarehouseCard final : public BuildingCard {
public:
    WarehouseCard(float x, float y, int sellValue,
                  const std::string& iconPath, float scale,
                  int& maxCardCount)
        : BuildingCard(x, y, "Warehouse", sellValue, iconPath, scale)
        , m_MaxCardCount(maxCardCount)   // 存起來供 OnSold 使用
    {
        m_MaxCardCount += 14;
    }

    void OnSold() override {
        m_MaxCardCount -= 14;
    }

    // 在這裡覆寫 OnStacked、OnMonthEnd、Update 等

private:
    int& m_MaxCardCount;   // 指向 CardManager::m_MaxCardCount
};

#endif //STACKLANDS_WAREHOUSECARD_HPP
