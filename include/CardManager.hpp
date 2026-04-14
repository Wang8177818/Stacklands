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
#include  "ResourceCard.hpp"
#include "nlohmann/json.hpp"
#include "RecipeManager.hpp"
#include  "BuildingCard.hpp"
#include  "StructureCard.hpp"
using json = nlohmann::json;

class CardManager {
public:
    // 建構子：需要接收 App 的 Renderer 才能把卡片畫在畫面上
    CardManager(Util::Renderer& renderer);

    // 每一幀的更新邏輯 (取代原本 App::Update 裡一大堆的卡片邏輯)
    void Update(glm::vec2 mousePos);

    // 把卡片加入管理清單，並同時交給 Renderer 渲染
    void AddCard(std::shared_ptr<Card> card);
    void RemoveCard(std::shared_ptr<Card> target);

    // json
    void LoadCardDatabase(const std::string& filePath);
    void LoadPackDatabase(const std::string& filePath);
    void LoadProfessionRecipes(const std::string& filePath);
    void LoadCraftingRecipes(const std::string& filePath);

    std::shared_ptr<Card> SpawnCardByName(const std::string& name, float scale, float x = 0.0f, float y = 0.0f);
    void SpawnPackByName(const std::string& packName, float scale, float x = 0.0f, float y = 0.0f);
    // 卡片工廠
    std::shared_ptr<Card> CreateCardFromData(float x, float y, const CardSpawnData& data);

    // 取得所有卡片
    std::vector<std::shared_ptr<Card>> GetAllCards();

    // 是否正在拖曳卡片
    bool isDraggingCard();

    // 同步當前縮放倍率（由 App 每幀呼叫）
    void SetZoomRatio(float ratio) { m_ZoomRatio = ratio; }

private:
    Util::Renderer& m_Renderer; // 參考到 App 的 Renderer

    std::vector<std::shared_ptr<Card>> m_Cards;
    std::shared_ptr<Card> m_DraggingCard = nullptr;

    std::mt19937 m_RandomGenerator;

    // 將原本在 App 裡的 static 變數變成 Manager 的私人變數
    glm::vec2 m_ClickStartPos = {0, 0};
    std::chrono::time_point<std::chrono::steady_clock> m_LastClickTime;
    std::shared_ptr<Card> m_LastClickedCard = nullptr;

    float m_ZoomRatio = 1.0f; // 當前累積縮放倍率，用於卡包開出卡片時套用正確大小
    RecipeManager m_RecipeManager;

    // 卡牌與卡包的資料庫字典
    std::unordered_map<std::string, CardSpawnData> m_CardDatabase;

    struct PackTemplate {
        std::string name;
        int sellValue;
        std::string iconPath;
        int totalCards;
        std::vector<std::string> pool;
    };
    std::unordered_map<std::string, PackTemplate> m_PackDatabase;
};

#endif //STACKLANDS_CARDMANAGER_HPP