//
// Created by m0938 on 2026/5/22.
//

#ifndef STACKLANDS_LOCATIONCARD_HPP
#define STACKLANDS_LOCATIONCARD_HPP

#pragma once
#include <string>
#include <vector>
#include <random>
#include "Card.hpp"
#include "GameConstants.hpp"

class LocationCard : public Card {
public:
    LocationCard(float x, float y, const std::string& name, int sellValue,
                 const std::string& iconPath,
                 float exploreTimeSec,
                 const std::vector<std::pair<std::string, int>>& spawnCards,
                 float scale = 1.0f,
                 int maxGathers = 0,
                 const std::vector<std::pair<int, std::string>>& guaranteedDrops = {})
        : Card(x, y, name, sellValue, CardType::LOCATION, scale),
          m_ExploreTimeMs(exploreTimeSec * 1000.0f),
          m_SpawnCards(spawnCards),
          m_MaxGathers(maxGathers),
          m_GatherCount(0),
          m_GuaranteedDrops(guaranteedDrops)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Location.png");
        SetIconImage(iconPath);
        m_PriceText = InitLabelText(std::to_string(sellValue), Util::Color(100, 111, 128));
        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::CHARACTER;
    }

    std::string Explore(std::mt19937& rng) {
        ++m_GatherCount;
        for (const auto& [count, cardName] : m_GuaranteedDrops) {
            if (m_GatherCount == count) return cardName;
        }
        return PickByWeight(rng);
    }

    bool  IsExhausted()       const { return m_MaxGathers > 0 && m_GatherCount >= m_MaxGathers; }
    float GetExploreTimeMs()  const { return m_ExploreTimeMs; }

    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_PriceText)
            RebuildLabelText(m_PriceText, std::to_string(m_SellValue), Util::Color(100, 111, 128));
    }

    void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_PriceText) {
            m_PriceText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::PRICE_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText) m_PriceText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText) m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_PriceText;
    float m_ExploreTimeMs;
    std::vector<std::pair<std::string, int>> m_SpawnCards;
    int  m_MaxGathers;
    int  m_GatherCount;
    std::vector<std::pair<int, std::string>> m_GuaranteedDrops;

private:
    std::string PickByWeight(std::mt19937& rng) const {
        if (m_SpawnCards.empty()) return "";
        int total = 0;
        for (const auto& [name, w] : m_SpawnCards) total += w;
        if (total <= 0) return m_SpawnCards.front().first;
        std::uniform_int_distribution<int> dist(0, total - 1);
        int roll = dist(rng);
        int acc  = 0;
        for (const auto& [name, w] : m_SpawnCards) {
            acc += w;
            if (roll < acc) return name;
        }
        return m_SpawnCards.back().first;
    }
};

#endif // STACKLANDS_LOCATIONCARD_HPP
