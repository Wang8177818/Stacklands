//
// Created by m0938 on 2026/4/14.
//

#ifndef STACKLANDS_STRUCTURECARD_HPP
#define STACKLANDS_STRUCTURECARD_HPP

#pragma once
#include <string>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Image.hpp"

class StructureCard : public Card {
public:
    // spawnCards {{"Wood", 70} {"Stone", 30} 70% 木頭、30% 石頭
    StructureCard(float x, float y, const std::string& name, int sellValue,
                  const std::string& iconPath,
                  int resourceCount = 5,
                  const std::vector<std::pair<std::string, int>>& spawnCards = {},
                  float scale = 1.0f)
        : Card(x, y, name, sellValue, CardType::STRUCTURE, scale),
          m_ResourceCount(resourceCount),
          m_SpawnCards(spawnCards)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Structures.png");
        SetIconImage(iconPath);

        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));

        m_PriceText = std::make_shared<Util::GameObject>();
        m_PriceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(sellValue), Util::Color(100, 111, 128)));
        m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);

        m_ResourceText = std::make_shared<Util::GameObject>();
        m_ResourceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(m_ResourceCount), Util::Color(100, 111, 128)));
        m_ResourceText->m_Transform.scale = {m_Scale, m_Scale};
        m_ResourceText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::CHARACTER;
    }

    // 採集 減少剩餘資源次數依權重隨機回傳一張卡片名稱
    std::string Gather(std::mt19937& rng) {
        if (m_ResourceCount <= 0 || m_SpawnCards.empty()) return "";
        --m_ResourceCount;
        RefreshResourceText();
        return PickByWeight(rng);
    }

    bool IsExhausted() const { return m_ResourceCount <= 0; }
    int  GetResourceCount() const { return m_ResourceCount; }

    void SetResourceCount(int count) { m_ResourceCount = count; RefreshResourceText(); }
    void SetSpawnCards(const std::vector<std::pair<std::string, int>>& cards) { m_SpawnCards = cards; }

    void SetScale(float scale) override {
        Card::SetScale(scale);
        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        if (m_PriceText) {
            m_PriceText->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjh.ttc", fontSize,
                std::to_string(m_SellValue), Util::Color(100, 111, 128)));
            m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
        }
        if (m_ResourceText) {
            m_ResourceText->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjh.ttc", fontSize,
                std::to_string(m_ResourceCount), Util::Color(100, 111, 128)));
            m_ResourceText->m_Transform.scale = {m_Scale, m_Scale};
        }
    }

    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();

        if (m_PriceText) {
            m_PriceText->m_Transform.translation =
                glm::vec2(m_X + m_Width * -0.32f, m_Y + m_Height * -0.3f);
            m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
        if (m_ResourceText) {
            // 顯示在右下角，與售價錯開
            m_ResourceText->m_Transform.translation =
                glm::vec2(m_X + m_Width * 0.32f, m_Y + m_Height * -0.3f);
            m_ResourceText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_PriceText)    m_PriceText->SetZIndex(42);
        if (m_ResourceText) m_ResourceText->SetZIndex(42);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_PriceText)    m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);
        if (m_ResourceText) m_ResourceText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText)    objs.push_back(m_PriceText);
        if (m_ResourceText) objs.push_back(m_ResourceText);
        return objs;
    }

protected:
    std::shared_ptr<Util::GameObject> m_PriceText;
    std::shared_ptr<Util::GameObject> m_ResourceText;

    int m_ResourceCount;                                      // 剩餘資源數
    std::vector<std::pair<std::string, int>> m_SpawnCards;    // {卡片名, 權重}

private:
    void RefreshResourceText() {
        if (!m_ResourceText) return;
        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        m_ResourceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(m_ResourceCount), Util::Color(255, 200, 80)));
        m_ResourceText->m_Transform.scale = {m_Scale, m_Scale};
    }

    // 根據權重 隨機抽一張
    std::string PickByWeight(std::mt19937& rng) const {
        int totalWeight = 0;
        for (const auto& entry : m_SpawnCards)
            totalWeight += entry.second;

        if (totalWeight <= 0) return m_SpawnCards.front().first;

        std::uniform_int_distribution<int> dist(0, totalWeight - 1);
        int roll = dist(rng);

        int accumulated = 0;
        for (const auto& entry : m_SpawnCards) {
            accumulated += entry.second;
            if (roll < accumulated)
                return entry.first;
        }
        return m_SpawnCards.back().first; // 保底
    }
};

#endif //STACKLANDS_STRUCTURECARD_HPP
