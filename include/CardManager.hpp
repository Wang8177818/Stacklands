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
#include  "CoinChest.hpp"
#include  "Hotpot.hpp"
#include  "ResourceChest.hpp"
#include "ISpawnListener.hpp"
#include "TaskScheduler.hpp"
using json = nlohmann::json;

class CardManager : public ISpawnListener {
public:
    // ISpawnListener 介面：動物卡觸發特殊能力時呼叫
    void OnSpawn(const std::string& name, float x, float y) override;
    // 建構子：需要接收 App 的 Renderer 才能把卡片畫在畫面上
    CardManager(Util::Renderer& renderer);

    // 每一幀的更新邏輯 (取代原本 App::Update 裡一大堆的卡片邏輯)
    void Update(glm::vec2 mousePos);

    // 把卡片加入管理清單，並同時交給 Renderer 渲染
    void AddCard(std::shared_ptr<Card> card);
    void RemoveCard(std::shared_ptr<Card> target);
    void ClearAllCards();

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

    // 取得資料庫中所有卡片名稱（供作弊選單使用）
    std::vector<std::string> GetAllCardNames() const;

    // 同步當前縮放倍率（由 App 每幀呼叫）
    void SetZoomRatio(float ratio) { m_ZoomRatio = ratio; }

    // 同步戰鬥場地世界座標（Pan / Zoom 時由 EventManager 呼叫）
    void MoveCombatArenas(glm::vec2 delta)               { m_Tasks.MoveCombatArenas(delta); }
    void ScaleCombatArenas(float ratio, glm::vec2 pivot) { m_Tasks.ScaleCombatArenas(ratio, pivot); }

    // 設定遊戲場地邊界（供卡片位置約束使用）
    void SetFieldBounds(const std::shared_ptr<Util::GameObject>& field) { m_Field = field; }

    // 取得場上角色卡數量
    int GetCharacterCount() const {
        int count = 0;
        for (auto& card : m_Cards)
            if (card->GetType() == CardType::CHARACTER) count++;
        return count;
    }

    // 取得當前場上卡片數量（排除 SellSlot 等 INTERACT 類型）
    int GetCardCount() const {
        int count = 0;
        for (auto& card : m_Cards) {
            if (card->GetType() == CardType::INTERACT) continue;
            if (card->GetType() == CardType::COIN)     continue;
            ++count;
            if (card->GetType() == CardType::BUILDING &&
                card->GetName() == "Resource Chest") {
                count += std::static_pointer_cast<ResourceChest>(card)->GetStored();
            }
        }
        return count;
    }

    // 取得卡片持有上限
    int  GetMaxCardCount() const { return m_MaxCardCount; }
    bool IsCardFull()      const { return GetCardCount() >= m_MaxCardCount; }

    // 月底結算：扣除人物的食物消耗
    void OnMonthEnd();

    // 取得場上所有人物卡每月需消耗的食物總數
    int GetNeededFoodCount() const {
        int total = 0;
        for (auto& card : m_Cards) {
            if (card->GetType() == CardType::CHARACTER)
                total += std::static_pointer_cast<CharacterCard>(card)->GetFoodConsumption();
        }
        return total;
    }

    // 取得場上所有食物卡提供的食物總量（含 Hotpot 內儲存的）
    int GetTotalFoodSupply() const {
        int total = 0;
        for (auto& card : m_Cards) {
            if (card->GetType() == CardType::FOOD) {
                total += std::static_pointer_cast<FoodCard>(card)->GetNutritionValue();
            } else if (card->GetType() == CardType::BUILDING &&
                       card->GetName() == "Hotpot") {
                total += std::static_pointer_cast<Hotpot>(card)->GetStored();
            }
        }
        return total;
    }

    // 取得金幣數量（場上的 Coin + Coin Chest 內儲存的）
    int GetCoinCount() {
        int count = 0;
        for (auto& card : GetAllCards()) {
            if (card->GetType() == CardType::COIN) {
                ++count;
            } else if (card->GetType() == CardType::BUILDING &&
                       card->GetName() == "Coin Chest") {
                count += std::static_pointer_cast<CoinChest>(card)->GetStored();
            }
        }
        return count;
    }

private:
    Util::Renderer& m_Renderer; // 參考到 App 的 Renderer

    int m_MaxCardCount = 50; // 預設上限

    std::vector<std::shared_ptr<Card>> m_Cards;
    std::shared_ptr<Card> m_DraggingCard = nullptr;

    std::mt19937 m_RandomGenerator;
    RecipeManager m_RecipeManager;
    TaskScheduler m_Tasks;

    // 將原本在 App 裡的 static 變數變成 Manager 的私人變數
    glm::vec2 m_ClickStartPos = {0, 0};
    std::chrono::time_point<std::chrono::steady_clock> m_LastClickTime;
    std::shared_ptr<Card> m_LastClickedCard = nullptr;

    float m_ZoomRatio = 1.0f; // 當前累積縮放倍率，用於卡包開出卡片時套用正確大小

    // 遊戲場地 GameObject（用於計算邊界）
    std::shared_ptr<Util::GameObject> m_Field;

    // 將卡片位置約束在場地範圍內
    void ClampCardToField(const std::shared_ptr<Card>& card);

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