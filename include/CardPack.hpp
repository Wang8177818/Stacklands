//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_CARDPACK_HPP
#define STACKLANDS_CARDPACK_HPP

#pragma once
#include "Card.hpp"
#include "CardData.hpp"
#include <vector>
#include <memory>

class CardPack : public Card {
public:
    // 建構子接收：基本卡片參數 + 總張數 + 卡池內容清單
    CardPack(float x, float y, const std::string& name, int sellValue,
             const std::string& iconPath, float scale, int totalCards,
             std::vector<CardSpawnData> contents);

    virtual ~CardPack() = default;

    // 嘗試抽出一張卡牌資料 (若抽完回傳 nullptr)
    std::shared_ptr<CardSpawnData> SpawnNext();

    // 檢查卡包是否已經空了
    bool IsEmpty() const { return m_CardsRemaining <= 0; }

    // === Overrides ===
    // 人物卡不應該疊在卡包上面 (雖然人物卡本身 OnStacked 回傳 false，這裡做雙重保險)
    virtual bool OnStacked(std::shared_ptr<Card> cardAbove) override;
    // 需要 Override 才能顯示數量文字
    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override;
    // 雖然數量文字在右上角不太需要更新位置，但 Overload 起來比較標準
    virtual void UpdateVisualPositions() override;

protected:
    int m_CardsRemaining;
    std::vector<CardSpawnData> m_ContentPool; // 這個包裡的內容配方
    std::shared_ptr<Util::GameObject> m_CountText; // 右上角的剩餘張數文字
};

#endif //STACKLANDS_CARDPACK_HPP