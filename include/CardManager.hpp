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
#include  "CharacterCard.hpp"
#include  "StructureCard.hpp"
#include  "FoodCard.hpp"
#include  "TimeBar.hpp"
#include  "MonsterCard.hpp"
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

    // 時間流速倍率（PAUSE=0, NORMAL=1, FAST=2 等）：影響合成 / 採集 / 讀條
    void SetTimeScale(float scale) { m_TimeScale = scale; }

    // 取得當前場上卡片數量（排除 SellSlot 等 INTERACT 類型）
    int GetCardCount() const {
        int count = 0;
        for (auto& card : m_Cards)
            if (card->GetType() != CardType::INTERACT)
                count++;
        return count;
    }

    // 取得卡片持有上限
    int  GetMaxCardCount() const { return m_MaxCardCount; }
    bool IsCardFull()      const { return GetCardCount() >= m_MaxCardCount; }

    // 取得場上所有人物卡每月需消耗的食物總數
    int GetNeededFoodCount() const {
        int total = 0;
        for (auto& card : m_Cards) {
            if (card->GetType() == CardType::CHARACTER)
                total += std::static_pointer_cast<CharacterCard>(card)->GetFoodConsumption();
        }
        return total;
    }

    // 取得場上所有食物卡提供的食物總量
    int GetTotalFoodSupply() const {
        int total = 0;
        for (auto& card : m_Cards) {
            if (card->GetType() == CardType::FOOD)
                total += std::static_pointer_cast<FoodCard>(card)->GetNutritionValue();
        }
        return total;
    }

    // 取得金幣數量
    int GetCoinCount() {
        int count = 0;
        for (auto& card : GetAllCards()) {
            if (card->GetType() == CardType::COIN)
                count++;
        }
        return count;
    }

private:
    // 延遲採集任務（角色堆疊在結構卡上，等待 timeLeftMs 後生成卡片再分離）
    struct PendingGather {
        std::weak_ptr<Card> character;
        std::weak_ptr<Card> structure;
        bool exhausted = false;
        std::string spawnName;
        float spawnX, spawnY, spawnScale;
        float timeLeftMs = 10000.0f;
        std::unique_ptr<TimeBar> bar;   // 顯示用讀條（解構時自動移出 Renderer）
    };

    // 戰鬥任務（CHARACTER / ANIMAL 對 MONSTER 的持續戰鬥）
    struct PendingCombat {
        std::weak_ptr<Card> fighter;   // CHARACTER 或 ANIMAL
        std::weak_ptr<Card> monster;
        float fighterTimer = 0.0f;     // ms 倒數至下一次攻擊
        float monsterTimer = 0.0f;
    };

    // 延遲合成任務（配方匹配後等待 timeLeftMs 再消耗材料生成成品）
    struct PendingCraft {
        std::weak_ptr<Card> stackBottom;
        std::vector<std::weak_ptr<Card>> allCards;
        std::string outputName;
        float spawnX, spawnY, spawnScale;
        float timeLeftMs;
        float totalMs = 0.f;             // 整段配方所需毫秒（用於進度條）
        std::unique_ptr<TimeBar> bar;    // 顯示用讀條（解構時自動移出 Renderer）
    };

    Util::Renderer& m_Renderer; // 參考到 App 的 Renderer

    int m_MaxCardCount = 50; // 預設上限

    std::vector<std::shared_ptr<Card>> m_Cards;
    std::shared_ptr<Card> m_DraggingCard = nullptr;
    std::vector<PendingGather>  m_PendingGathers;
    std::vector<PendingCraft>   m_PendingCrafts;
    std::vector<PendingCombat>  m_PendingCombats;

    std::mt19937 m_RandomGenerator;

    // 將原本在 App 裡的 static 變數變成 Manager 的私人變數
    glm::vec2 m_ClickStartPos = {0, 0};
    std::chrono::time_point<std::chrono::steady_clock> m_LastClickTime;
    std::shared_ptr<Card> m_LastClickedCard = nullptr;

    float m_ZoomRatio = 1.0f; // 當前累積縮放倍率，用於卡包開出卡片時套用正確大小
    float m_TimeScale = 1.0f; // 計時任務的時間流速倍率（暫停=0）
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