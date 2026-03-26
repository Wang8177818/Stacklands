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

    virtual bool OnStacked(std::shared_ptr<Card> cardAbove) override;
    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override;
    virtual void UpdateVisualPositions() override;

    virtual void StartDragging(glm::vec2 mousePos) override;
    virtual void StopDragging() override;

protected:
    int m_CardsRemaining;
    std::vector<CardSpawnData> m_ContentPool; // 這個包裡的內容配方
    std::shared_ptr<Util::GameObject> m_CountText; // 右上角的剩餘張數文字
};

#endif //STACKLANDS_CARDPACK_HPP