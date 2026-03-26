//
// Created by m0938 on 2026/3/20.
//

#ifndef STACKLANDS_CARDMANAGER_HPP
#define STACKLANDS_CARDMANAGER_HPP

#pragma once
#include <vector>
#include <memory>
#include <chrono>
#include <random>
#include <glm/glm.hpp>
#include "Card.hpp"
#include "CardData.hpp"
#include "Util/Renderer.hpp"
#include "CoinCard.hpp"

class CardManager {
public:
    // 建構子：需要接收 App 的 Renderer 才能把卡片畫在畫面上
    CardManager(Util::Renderer& renderer);

    // 每一幀的更新邏輯 (取代原本 App::Update 裡一大堆的卡片邏輯)
    void Update(glm::vec2 mousePos);

    // 把卡片加入管理清單，並同時交給 Renderer 渲染
    void AddCard(std::shared_ptr<Card> card);

    // 卡片工廠
    std::shared_ptr<Card> CreateCardFromData(float x, float y, const CardSpawnData& data);

private:
    Util::Renderer& m_Renderer; // 參考到 App 的 Renderer

    std::vector<std::shared_ptr<Card>> m_Cards;
    std::shared_ptr<Card> m_DraggingCard = nullptr;

    std::mt19937 m_RandomGenerator;

    // 將原本在 App 裡的 static 變數變成 Manager 的私人變數
    glm::vec2 m_ClickStartPos = {0, 0};
    std::chrono::time_point<std::chrono::steady_clock> m_LastClickTime;
    std::shared_ptr<Card> m_LastClickedCard = nullptr;
};

#endif //STACKLANDS_CARDMANAGER_HPP