//
// Created by m0938 on 2026/3/20.
//
#include "CardManager.hpp"
#include "CardFactory.hpp"
#include "EventManager.hpp"
#include "RecipeManager.hpp"
#include "CharacterCard.hpp"
#include "EquipmentCard.hpp"
#include "AnimalCard.hpp"
#include "MonsterCard.hpp"
#include "AttackResolver.hpp"
#include "CardPack.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>

#include "WarehouseCard.hpp"
#include "CoinChest.hpp"
#include "Hotpot.hpp"
#include "ResourceChest.hpp"
#include "LocationCard.hpp"

#include "Util/Logger.hpp"

CardType StringToCardType(const std::string& typeStr) {
    if (typeStr == "CHARACTER") return CardType::CHARACTER;
    if (typeStr == "RESOURCE") return CardType::RESOURCE;
    if (typeStr == "COIN") return CardType::COIN;
    if (typeStr == "PACK") return CardType::PACK;
    if (typeStr == "EQUIPMENT") return CardType::EQUIPMENT;
    if (typeStr == "BUILDING") return CardType::BUILDING;
    if (typeStr == "STRUCTURE") return CardType::STRUCTURE;
    if (typeStr == "FOOD")    return CardType::FOOD;
    if (typeStr == "ANIMAL")   return CardType::ANIMAL;
    if (typeStr == "MONSTER")  return CardType::MONSTER;
    if (typeStr == "LOCATION") return CardType::LOCATION;
    if (typeStr == "IDEA")     return CardType::IDEA;
    return CardType::BASIC;
}

EquipSlot StringToEquipSlot(const std::string& slotStr) {
    if (slotStr == "HEAD" || slotStr == "Head") return EquipSlot::HEAD;
    if (slotStr == "HAND" || slotStr == "Hand") return EquipSlot::HAND;
    if (slotStr == "BODY" || slotStr == "Body") return EquipSlot::BODY;
    return EquipSlot::NONE;
}

bool CardManager::isDraggingCard() {
    if (m_DraggingCard != nullptr) {
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<Card>> CardManager::GetAllCards() {
    return m_Cards;
}

void CardManager::LoadCardDatabase(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("無法開啟卡牌資料檔: {}", filePath);
        return;
    }

    json j;
    file >> j;

    // 遍歷陣列
    for (const auto& item : j) {
        CardSpawnData data;
        data.name      = item.value("name", "Unknown");
        data.sellValue = item.value("sellValue", 0);
        data.type      = StringToCardType(item.value("type", "BASIC"));

        std::string rawPath = item.value("iconPath", "");
        data.iconPath  = rawPath.empty() ? "" : RESOURCE_DIR + rawPath;

        data.health          = item.value("health", 0);
        data.attack          = item.value("damage", 0) != 0 ? item.value("damage", 0) : item.value("attack", 0);
        data.defense         = item.value("defense", 0);
        data.attackSpeed     = item.value("attackSpeed", 3.0f);
        data.hitChance       = item.value("hitChance", 0.6f);
        data.time            = item.value("time", 0.0f);
        data.nutritionValue  = item.value("nutritionValue", 0);
        data.foodConsumption = item.value("food", 0);
        data.description     = item.value("description", "");
        data.scale          = 0.05f;

        if (item.contains("slot"))
            data.equipSlot = StringToEquipSlot(item.value("slot", ""));

        // 結構卡用
        data.resourceCount = item.value("resourceCount", 0);
        if (item.contains("spawnCards")) {
            for (const auto& entry : item["spawnCards"]) {
                std::string cardName = entry.value("name", "");
                int weight           = entry.value("weight", 1);
                if (!cardName.empty())
                    data.spawnCards.push_back({cardName, weight});
            }
        }

        // 動物卡用
        if (item.contains("dropCards")) {
            for (const auto& entry : item["dropCards"]) {
                std::string cardName = entry.value("name", "");
                int weight           = entry.value("weight", 1);
                if (!cardName.empty())
                    data.dropCards.push_back({cardName, weight});
            }
        }
        data.abilityName     = item.value("abilityName", "");
        data.abilityCooldown = item.value("abilityCooldown", 0.0f);

        m_CardDatabase[data.name] = data;
    }
    LOG_INFO("成功載入 {} 種卡牌資料！", m_CardDatabase.size());
}

void CardManager::LoadProfessionRecipes(const std::string& filePath) {
    m_RecipeManager.LoadProfessionRecipes(filePath);
}

void CardManager::LoadCraftingRecipes(const std::string& filePath) {
    m_RecipeManager.LoadCraftingRecipes(filePath);
}

void CardManager::LoadPackDatabase(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return;

    json j;
    file >> j;

    for (const auto& item : j) {
        PackTemplate pack;
        pack.name = item.value("name", "Unknown");
        pack.sellValue = item.value("sellValue", 0);
        pack.iconPath = RESOURCE_DIR + item["iconPath"].get<std::string>();
        pack.totalCards = item["totalCards"];

        for (const auto& cardName : item["pool"]) {
            pack.pool.push_back(cardName);
        }

        m_PackDatabase[pack.name] = pack;
    }
    LOG_INFO("成功載入 {} 種卡包資料！", m_PackDatabase.size());
}

std::shared_ptr<Card> CardManager::SpawnCardByName(const std::string& name, float scale, float x, float y) {
    if (m_CardDatabase.find(name) == m_CardDatabase.end()) {
        LOG_ERROR("找不到卡牌: {}", name);
        return nullptr;
    }

    CardSpawnData data = m_CardDatabase[name];
    data.scale = scale; // 套用當前場景的縮放比例
    return CreateCardFromData(x, y, data);
}

void CardManager::SpawnPackByName(const std::string& packName, float scale, float x, float y) {
    if (m_PackDatabase.find(packName) == m_PackDatabase.end()) return;

    PackTemplate tmpl = m_PackDatabase[packName];

    // 把 pool 裡的「名字字串」轉換成真正的「CardSpawnData 配方」
    // 存基準 scale（除掉當前 zoom），開包時 SpawnNext 後會 *= m_ZoomRatio 套回當下的縮放
    const float baseCardScale = (m_ZoomRatio > 0.0f) ? (scale / m_ZoomRatio) : scale;
    std::vector<CardSpawnData> actualPool;
    for (const auto& cardName : tmpl.pool) {
        if (m_CardDatabase.count(cardName)) {
            CardSpawnData poolData = m_CardDatabase[cardName];
            poolData.scale = baseCardScale;
            actualPool.push_back(poolData);
        }
    }

    auto pack = std::make_shared<CardPack>(
        x, y, tmpl.name, tmpl.sellValue, tmpl.iconPath, scale * 0.6, tmpl.totalCards, actualPool);

    AddCard(pack);
}

CardManager::CardManager(Util::Renderer& renderer)
    : m_Renderer(renderer),
      m_Tasks(renderer, m_RandomGenerator, m_RecipeManager,
              [this](const std::string& name, float scale, float x, float y) {
                  SpawnCardByName(name, scale, x, y);
              },
              [this](std::shared_ptr<Card> card) {
                  RemoveCard(card);
              })
{
    m_RandomGenerator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    m_LastClickTime = std::chrono::steady_clock::now();
}

void CardManager::AddCard(std::shared_ptr<Card> card) {
    if (!card) return;
    m_Cards.push_back(card);
    // 自動把卡牌身上的所有零件交給 Renderer 畫出來
    for (auto& obj : card->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }
}

// ─────────────────────────────────────────────────────────────
// 月底結算：把人物每月需消耗的食物總量從場上扣除
//   - 嬰兒 (food=1) 優先進食，再輪到成人 (food=2)
//   - 食物卡上的 nutrition 累進扣，扣到 0 才移除（不浪費）
//   - 食物不夠的人物 → 變成 Corpse
// ─────────────────────────────────────────────────────────────
void CardManager::OnMonthEnd() {
    // 1. 收集人物，按 foodConsumption 升冪排序：嬰兒優先
    std::vector<std::shared_ptr<CharacterCard>> chars;
    for (auto& c : m_Cards)
        if (c->GetType() == CardType::CHARACTER)
            chars.push_back(std::static_pointer_cast<CharacterCard>(c));

    if (chars.empty()) {
        LOG_INFO("月底結算：場上無人物，無食物消耗");
        return;
    }

    std::stable_sort(chars.begin(), chars.end(),
        [](const std::shared_ptr<CharacterCard>& a,
           const std::shared_ptr<CharacterCard>& b) {
            return a->GetFoodConsumption() < b->GetFoodConsumption();
        });

    // 2. 收集所有食物卡與 Hotpot（用 m_Cards 順序；FOOD 卡先吃完才輪 Hotpot）
    std::vector<std::shared_ptr<FoodCard>> foods;
    std::vector<std::shared_ptr<Hotpot>>   hotpots;
    for (auto& c : m_Cards) {
        if (c->GetType() == CardType::FOOD) {
            foods.push_back(std::static_pointer_cast<FoodCard>(c));
        } else if (c->GetType() == CardType::BUILDING &&
                   c->GetName() == "Hotpot") {
            hotpots.push_back(std::static_pointer_cast<Hotpot>(c));
        }
    }

    // 3. 依優先順序餵食：先吃完 FOOD 卡，再從 Hotpot 扣
    std::vector<std::shared_ptr<CharacterCard>> starved;
    std::size_t foodIdx = 0;
    std::size_t potIdx  = 0;
    int totalConsumed = 0;
    for (auto& chr : chars) {
        int need = chr->GetFoodConsumption();
        // (a) 先消耗 FOOD 卡
        while (need > 0 && foodIdx < foods.size()) {
            int taken = foods[foodIdx]->ConsumeNutrition(need);
            need          -= taken;
            totalConsumed += taken;
            if (foods[foodIdx]->GetNutritionValue() == 0) ++foodIdx;
        }
        // (b) FOOD 卡耗盡才動 Hotpot 內存
        while (need > 0 && potIdx < hotpots.size()) {
            if (hotpots[potIdx]->GetStored() <= 0) { ++potIdx; continue; }
            int taken = hotpots[potIdx]->Withdraw(need);
            need          -= taken;
            totalConsumed += taken;
            if (hotpots[potIdx]->GetStored() == 0) ++potIdx;
        }
        if (need > 0) starved.push_back(chr);
    }

    // 4. 把扣到 0 的食物卡實際移除
    int removedFoods = 0;
    for (auto& f : foods) {
        if (f->GetNutritionValue() == 0) {
            RemoveCard(f);
            ++removedFoods;
        }
    }

    // 5. 餓死的人物變屍體
    for (auto& c : starved) {
        float x = c->GetX(), y = c->GetY(), s = c->GetScale();

        // 切斷堆疊連結，避免角色被相鄰卡的 CardAbove/CardBelow 持有變成幽靈，
        // 也讓 PendingGather/PendingCraft 的中斷判定立即觸發
        if (auto below = c->GetCardBelow()) below->SetCardAbove(nullptr);
        if (auto above = c->GetCardAbove()) above->SetCardBelow(nullptr);
        c->SetCardBelow(nullptr);
        c->SetCardAbove(nullptr);

        RemoveCard(c);
        SpawnCardByName("Corpse", s, x, y);
    }

    if (!starved.empty()) {
        LOG_WARN("月底結算：消耗 {} nutrition、移除 {} 張空食物卡，飢餓死亡 {} 人",
                 totalConsumed, removedFoods, starved.size());
    } else {
        LOG_INFO("月底結算：消耗 {} nutrition、移除 {} 張空食物卡",
                 totalConsumed, removedFoods);
    }
}

void CardManager::RemoveCard(std::shared_ptr<Card> target) {
    m_Cards.erase(std::remove_if(m_Cards.begin(), m_Cards.end(),
        [&](const std::shared_ptr<Card>& card) {
            if (card == target) {
                for (auto& obj : card->GetGameObjects()) {
                    obj->SetVisible(false);
                    obj->m_Transform.translation = glm::vec2(-9999, -9999);
                }
                return true;
            }
            return false;
        }), m_Cards.end());
}

void CardManager::OnSpawn(const std::string& name, float x, float y) {
    std::uniform_real_distribution<float> off(-50.0f, 50.0f);
    SpawnCardByName(name, m_ZoomRatio * 0.05f,
                    x + off(m_RandomGenerator),
                    y + off(m_RandomGenerator));
}

std::shared_ptr<Card> CardManager::CreateCardFromData(float x, float y, const CardSpawnData& data) {
    auto newCard = CardFactory::Create(x, y, data, m_MaxCardCount, this);
    if (newCard && !data.description.empty())
        newCard->SetDescription(data.description);
    if (newCard) AddCard(newCard);
    return newCard;
}

// 判斷兩張卡是否屬於同一個堆疊（互為上下關係）
static bool InSameStack(const std::shared_ptr<Card>& a, const std::shared_ptr<Card>& b) {
    auto cur = a;
    while (cur) { if (cur == b) return true; cur = cur->GetCardAbove(); }
    cur = a->GetCardBelow();
    while (cur) { if (cur == b) return true; cur = cur->GetCardBelow(); }
    return false;
}

// 判斷一張卡所在的整個堆疊裡是否有 sticky 卡（如 Magic Glue）
static bool StackHasSticky(const std::shared_ptr<Card>& c) {
    auto cur = c;
    while (cur) { if (cur->IsSticky()) return true; cur = cur->GetCardAbove(); }
    cur = c->GetCardBelow();
    while (cur) { if (cur->IsSticky()) return true; cur = cur->GetCardBelow(); }
    return false;
}

void CardManager::Update(glm::vec2 mousePos) {
    // 1. 清理空卡包
    m_Cards.erase(std::remove_if(m_Cards.begin(), m_Cards.end(),
        [](const std::shared_ptr<Card>& card) {
            if (card->GetType() == CardType::PACK) {
                auto pack = std::static_pointer_cast<CardPack>(card);
                if (pack->IsEmpty()) {

                    for (auto& obj : pack->GetGameObjects()) {
                        obj->SetVisible(false);
                        obj->m_Transform.translation = glm::vec2(-9999, -9999);
                    }

                    return true;
                }
            }
            return false;
        }), m_Cards.end());

    // 2. 動物死亡偵測：hp <= 0 時掉落並移除
    {
        std::vector<std::shared_ptr<Card>> deadAnimals;
        for (auto& card : m_Cards) {
            if (card->GetType() == CardType::ANIMAL) {
                auto animal = std::static_pointer_cast<AnimalCard>(card);
                if (animal->IsDead()) deadAnimals.push_back(card);
            }
        }
        for (auto& card : deadAnimals) {
            auto animal = std::static_pointer_cast<AnimalCard>(card);
            std::string drop = animal->RollDrop();
            if (!drop.empty()) {
                std::uniform_real_distribution<float> off(-60.0f, 60.0f);
                SpawnCardByName(drop, card->GetScale(),
                                card->GetX() + off(m_RandomGenerator),
                                card->GetY() + off(m_RandomGenerator));
            }
            RemoveCard(card);
        }
    }

    // 3. 更新卡片動畫與跟隨
    // 快照避免 Update 內部（如動物產卵 callback）呼叫 AddCard 導致迭代器失效
    {
        auto cardsSnapshot = m_Cards;
        for (auto& card : cardsSnapshot) {
            card->Update();
        }
    }

    // 計時任務
    {
        float dtMs = EventManager::GetScaledDtMs();
        m_Tasks.UpdateGathers(dtMs);
        m_Tasks.UpdateCombats(dtMs, m_DraggingCard);
        m_Tasks.UpdateCrafts(dtMs);
    }

    // 2.5 Coin Chest：吸收疊在上方的硬幣
    // 先收集要移除的硬幣，遍歷結束後才呼叫 RemoveCard（避免在 range-for m_Cards 時 erase 而 UB）
    {
        std::vector<std::shared_ptr<Card>> coinsToRemove;
        for (auto& card : m_Cards) {
            if (card->GetType() != CardType::BUILDING) continue;
            if (card->GetName() != "Coin Chest") continue;
            auto chest = std::static_pointer_cast<CoinChest>(card);
            if (chest->IsFull()) continue;

            // 從 chest 往上掃，把直接相連的 Coin 一張一張吸收（先記住，最後才 unlink/remove）
            std::vector<std::shared_ptr<Card>> toAbsorb;
            for (auto cur = chest->GetCardAbove();
                 cur && cur->GetType() == CardType::COIN && !chest->IsFull();
                 cur = cur->GetCardAbove()) {
                if (chest->Deposit(1) > 0) toAbsorb.push_back(cur);
                else break;
            }
            for (auto& c : toAbsorb) {
                auto below = c->GetCardBelow();
                auto above = c->GetCardAbove();
                if (below) below->SetCardAbove(above);
                if (above) above->SetCardBelow(below);
                c->SetCardBelow(nullptr);
                c->SetCardAbove(nullptr);
                coinsToRemove.push_back(c);
            }
        }
        // 安全在迴圈外做實際移除
        for (auto& c : coinsToRemove) RemoveCard(c);
    }

    // 2.54 Resource Chest：吸收疊在上方的同類型 RESOURCE 卡（最多 100 張）
    {
        std::vector<std::shared_ptr<Card>> resourcesToRemove;
        for (auto& card : m_Cards) {
            if (card->GetType() != CardType::BUILDING) continue;
            if (card->GetName() != "Resource Chest") continue;
            auto chest = std::static_pointer_cast<ResourceChest>(card);
            if (chest->IsFull()) continue;

            std::vector<std::shared_ptr<Card>> toAbsorb;
            for (auto cur = chest->GetCardAbove(); cur; cur = cur->GetCardAbove()) {
                if (cur->GetType() != CardType::RESOURCE) break;
                if (!chest->CanAccept(cur->GetName())) break;
                // 從資料庫查 iconPath；查不到就不存
                auto it = m_CardDatabase.find(cur->GetName());
                if (it == m_CardDatabase.end()) break;
                if (!chest->Deposit(cur->GetName(), it->second.iconPath)) break;
                toAbsorb.push_back(cur);
            }
            for (auto& c : toAbsorb) {
                auto below = c->GetCardBelow();
                auto above = c->GetCardAbove();
                if (below) below->SetCardAbove(above);
                if (above) above->SetCardBelow(below);
                c->SetCardBelow(nullptr);
                c->SetCardAbove(nullptr);
                resourcesToRemove.push_back(c);
            }
        }
        for (auto& c : resourcesToRemove) RemoveCard(c);
    }

    // 2.55 Hotpot：吸收疊在上方的 Food（依各自 nutritionValue）
    {
        std::vector<std::shared_ptr<Card>> foodsToRemove;
        for (auto& card : m_Cards) {
            if (card->GetType() != CardType::BUILDING) continue;
            if (card->GetName() != "Hotpot") continue;
            auto pot = std::static_pointer_cast<Hotpot>(card);

            std::vector<std::shared_ptr<Card>> toAbsorb;
            for (auto cur = pot->GetCardAbove();
                 cur && cur->GetType() == CardType::FOOD;
                 cur = cur->GetCardAbove()) {
                auto food = std::static_pointer_cast<FoodCard>(cur);
                pot->Deposit(food->GetNutritionValue());
                toAbsorb.push_back(cur);
            }
            for (auto& c : toAbsorb) {
                auto below = c->GetCardBelow();
                auto above = c->GetCardAbove();
                if (below) below->SetCardAbove(above);
                if (above) above->SetCardBelow(below);
                c->SetCardBelow(nullptr);
                c->SetCardAbove(nullptr);
                foodsToRemove.push_back(c);
            }
        }
        for (auto& c : foodsToRemove) RemoveCard(c);
    }

    // 2.6 Coin Chest：右鍵點擊吐出 WITHDRAW_AMOUNT 枚硬幣
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_RB)) {
        std::shared_ptr<CoinChest> targetChest;
        float chestX = 0.f, chestY = 0.f, chestS = 1.f;
        for (auto& card : m_Cards) {
            if (card->GetType() != CardType::BUILDING) continue;
            if (card->GetName() != "Coin Chest") continue;
            if (!card->IsMouseHovering(mousePos)) continue;
            targetChest = std::static_pointer_cast<CoinChest>(card);
            chestX = card->GetX();
            chestY = card->GetY();
            chestS = card->GetScale();
            break;
        }
        // 找到後再 spawn（SpawnCardByName 會 push_back m_Cards，會讓上方 range-for 失效）
        // 吐出的硬幣串成一堆疊，避免散落
        if (targetChest) {
            const int give = targetChest->Withdraw(CoinChest::WITHDRAW_AMOUNT);
            if (give > 0) {
                auto topCoin = SpawnCardByName("Coin", chestS, chestX, chestY - 80.f);
                for (int i = 1; i < give; ++i) {
                    auto newCoin = SpawnCardByName("Coin", chestS, chestX, chestY - 80.f);
                    topCoin->SetCardAbove(newCoin);
                    newCoin->SetCardBelow(topCoin);
                    topCoin = newCoin;
                }
            }
        }

        // 2.65 Resource Chest：右鍵取出 WITHDRAW_AMOUNT 張同類資源（堆疊呈現）
        std::shared_ptr<ResourceChest> rcChest;
        std::string rcName;
        float rcX = 0.f, rcY = 0.f, rcS = 1.f;
        for (auto& card : m_Cards) {
            if (card->GetType() != CardType::BUILDING) continue;
            if (card->GetName() != "Resource Chest") continue;
            if (!card->IsMouseHovering(mousePos)) continue;
            auto chest = std::static_pointer_cast<ResourceChest>(card);
            if (chest->IsEmpty()) break; // 空 chest 不做事
            rcChest = chest;
            rcName  = chest->GetStoredName();
            rcX     = card->GetX();
            rcY     = card->GetY();
            rcS     = card->GetScale();
            break;
        }
        if (rcChest && !rcName.empty()) {
            const int give = rcChest->Withdraw(ResourceChest::WITHDRAW_AMOUNT);
            if (give > 0) {
                auto top = SpawnCardByName(rcName, rcS, rcX, rcY - 80.f);
                for (int i = 1; i < give; ++i) {
                    auto next = SpawnCardByName(rcName, rcS, rcX, rcY - 80.f);
                    top->SetCardAbove(next);
                    next->SetCardBelow(top);
                    top = next;
                }
            }
        }
    }

    // 3. 按下左鍵：抓取
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB) && m_DraggingCard == nullptr) {
        m_ClickStartPos = mousePos;

        std::shared_ptr<Card> targetToPick = nullptr;
        int highestZ = -9999;
        for (auto& card : m_Cards) {
            if (card->IsMouseHovering(mousePos)) {
                int currentZ = card->GetGameObjects()[0]->GetZIndex();
                if (currentZ > highestZ) {
                    highestZ = currentZ;
                    targetToPick = card;
                }
            }
        }
        if (targetToPick != nullptr && targetToPick->CanDrag()) {
            m_DraggingCard = targetToPick;
            if (m_DraggingCard->GetCardBelow() != nullptr) {
                m_DraggingCard->GetCardBelow()->SetCardAbove(nullptr);
                m_DraggingCard->SetCardBelow(nullptr);
            }
            m_DraggingCard->StartDragging(mousePos);
        }
    }

    // 4. 放開左鍵：放下 / 雙擊判定
    if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_DraggingCard != nullptr) {
        float moveDist = glm::distance(m_ClickStartPos, mousePos);
        bool isClick = (moveDist < 10.0f);

        // 雙擊開卡包邏輯
        if (isClick) {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastClickTime).count();

            if (duration < 300 && m_LastClickedCard == m_DraggingCard) {
                if (m_DraggingCard->GetType() == CardType::PACK) {
                    auto pack = std::static_pointer_cast<CardPack>(m_DraggingCard);
                    if (!pack->IsEmpty()) {
                        auto dataToSpawn = pack->SpawnNext();
                        if (dataToSpawn) {
                            std::uniform_real_distribution<float> distOffset(-60.0f, 60.0f);
                            float spawnX = m_DraggingCard->GetX() + distOffset(m_RandomGenerator);
                            float spawnY = m_DraggingCard->GetY() - 80.0f + distOffset(m_RandomGenerator);

                            // 套用當前縮放倍率，確保新卡片大小與場景一致
                            dataToSpawn->scale *= m_ZoomRatio;
                            CreateCardFromData(spawnX, spawnY, *dataToSpawn);
                        }
                    }
                }
                m_LastClickedCard = nullptr;
            } else {
                m_LastClickTime = now;
                m_LastClickedCard = m_DraggingCard;
            }
        } else {
            m_LastClickedCard = nullptr;
        }

        m_DraggingCard->StopDragging();

        // 堆疊邏輯
        if (m_DraggingCard->GetType() != CardType::PACK &&
            m_DraggingCard->CanStackOnto()) {
            for (auto it = m_Cards.rbegin(); it != m_Cards.rend(); ++it) {
                auto targetCard = *it;
                if (targetCard != m_DraggingCard && m_DraggingCard->IsOverlapping(targetCard)) {
                    bool isSelfStack = false;
                    auto checkCard = m_DraggingCard->GetCardAbove();
                    while (checkCard != nullptr) {
                        if (checkCard == targetCard) { isSelfStack = true; break; }
                        checkCard = checkCard->GetCardAbove();
                    }
                    if (isSelfStack) continue;

                    while (targetCard->GetCardAbove() != nullptr) {
                        targetCard = targetCard->GetCardAbove();
                    }
                    // 若無法堆疊則 移動到旁邊
                    if (targetCard->OnStacked(m_DraggingCard) == false) {
                        float dx = m_DraggingCard->GetX() - targetCard->GetX();
                        float dy = m_DraggingCard->GetY() - targetCard->GetY();
                        float overlapX = (m_DraggingCard->GetWidth()  + targetCard->GetWidth())  * 0.5f - std::abs(dx);
                        float overlapY = (m_DraggingCard->GetHeight() + targetCard->GetHeight()) * 0.5f - std::abs(dy);

                        if (overlapX > 0 && overlapY > 0) {
                            if (overlapX <= overlapY) {
                                // 沿 X 軸推開（重疊較少的方向）
                                m_DraggingCard->MoveBy({dx >= 0.f ? overlapX : -overlapX, 0.f});
                            } else {
                                // 沿 Y 軸推開
                                m_DraggingCard->MoveBy({0.f, dy >= 0.f ? overlapY : -overlapY});
                            }
                        }
                        break;
                    }

                    // 角色轉職
                    if (m_DraggingCard->GetType() == CardType::EQUIPMENT &&
                        targetCard->GetType() == CardType::CHARACTER) {

                        auto equip    = std::static_pointer_cast<EquipmentCard>(m_DraggingCard);
                        auto charCard = std::static_pointer_cast<CharacterCard>(targetCard);
                        const std::string equipName = equip->GetName();

                        std::string outputName = m_RecipeManager.CheckProfession(equipName);

                        if (!outputName.empty()) {
                            float spawnX     = charCard->GetX();
                            float spawnY     = charCard->GetY();
                            float spawnScale = charCard->GetScale();

                            // 繼承舊角色插槽，將 HAND 改為本次觸發裝備
                            auto newSlots = charCard->GetAllEquipData();
                            const std::string& oldHand =
                                newSlots[static_cast<int>(EquipSlot::HAND)].name;

                            // 若HAND有裝備 在旁邊重新生成 (換下來)
                            if (!oldHand.empty()) {
                                std::uniform_real_distribution<float> dist(-80.f, 80.f);
                                SpawnCardByName(oldHand, spawnScale,
                                                spawnX + dist(m_RandomGenerator),
                                                spawnY + dist(m_RandomGenerator));
                            }
                            newSlots[static_cast<int>(EquipSlot::HAND)] = {
                                equipName,
                                equip->GetBonusAttack(),
                                equip->GetBonusHealth(),
                                equip->GetBonusDefense(),
                                equip->GetBonusAttackSpeed(),
                                equip->GetBonusHitChance()
                            };

                            auto dragging = m_DraggingCard;
                            m_DraggingCard = nullptr;
                            RemoveCard(dragging);
                            RemoveCard(std::static_pointer_cast<Card>(charCard));

                            // 生成新角色卡並設定插槽
                            auto newCardBase = SpawnCardByName(outputName, spawnScale, spawnX, spawnY);
                            if (newCardBase && newCardBase->GetType() == CardType::CHARACTER) {
                                std::static_pointer_cast<CharacterCard>(newCardBase)
                                    ->SetAllEquipData(newSlots);
                            }
                        } else {
                            // 插槽已佔用則換下舊裝備並裝上新裝備
                            const std::string& existing =
                                charCard->GetEquipName(equip->GetEquipSlot());
                            if (!existing.empty()) {
                                std::uniform_real_distribution<float> dist(-80.f, 80.f);
                                SpawnCardByName(existing, m_DraggingCard->GetScale(),
                                                charCard->GetX() + dist(m_RandomGenerator),
                                                charCard->GetY() + dist(m_RandomGenerator));
                            }
                            charCard->StoreEquipment(equip->GetEquipSlot(), equipName,
                                                     equip->GetBonusAttack(),
                                                     equip->GetBonusHealth(),
                                                     equip->GetBonusDefense(),
                                                     equip->GetBonusAttackSpeed(),
                                                     equip->GetBonusHitChance());
                            auto dragging = m_DraggingCard;
                            m_DraggingCard = nullptr;
                            RemoveCard(dragging);
                        }
                        break;
                    }

                    // Gather（採集/探索：角色堆疊在結構卡或地點卡上）
                    if ((targetCard->GetType() == CardType::STRUCTURE ||
                         targetCard->GetType() == CardType::LOCATION) &&
                        m_DraggingCard->GetType() == CardType::CHARACTER) {

                        std::string spawnName;
                        bool  exhausted   = false;
                        float gatherTimeMs;

                        if (targetCard->GetType() == CardType::STRUCTURE) {
                            auto structure = std::static_pointer_cast<StructureCard>(targetCard);
                            spawnName    = structure->Gather(m_RandomGenerator);
                            exhausted    = structure->IsExhausted();
                            gatherTimeMs = GameConstants::GATHER_TIME_MS;
                        } else {
                            auto location = std::static_pointer_cast<LocationCard>(targetCard);
                            spawnName    = location->Explore(m_RandomGenerator);
                            exhausted    = false;
                            gatherTimeMs = location->GetExploreTimeMs();
                        }

                        // 將角色堆疊在卡上方
                        targetCard->SetCardAbove(m_DraggingCard);
                        m_DraggingCard->SetCardBelow(targetCard);

                        TaskScheduler::PendingGather pg;
                        pg.character  = m_DraggingCard;
                        pg.structure  = targetCard;
                        pg.exhausted  = exhausted;
                        pg.spawnName  = spawnName;
                        pg.spawnX     = targetCard->GetX();
                        pg.spawnY     = targetCard->GetY();
                        pg.spawnScale = m_DraggingCard->GetScale();
                        pg.timeLeftMs = gatherTimeMs;

                        const float gatherSec  = gatherTimeMs / 1000.0f;
                        const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * targetCard->GetScale();
                        pg.bar = std::make_unique<TimeBar>(
                            m_Renderer,
                            glm::vec2{targetCard->GetX(), targetCard->GetY() + barOffsetY},
                            glm::vec2{GameConstants::CRAFT_BAR_BLACK_W, GameConstants::CRAFT_BAR_BLACK_H},
                            glm::vec2{GameConstants::CRAFT_BAR_WHITE_W, GameConstants::CRAFT_BAR_WHITE_H},
                            gatherSec,
                            GameConstants::CRAFT_BAR_Z);
                        pg.bar->Start();

                        m_Tasks.AddGather(std::move(pg));
                        break;
                    }

                    // 戰鬥觸發（村民 疊到 怪物 或 動物 上）
                    if ((targetCard->GetType() == CardType::MONSTER ||
                         targetCard->GetType() == CardType::ANIMAL) &&
                        m_DraggingCard->GetType() == CardType::CHARACTER) {

                        m_Tasks.JoinOrCreateCombat(targetCard, m_DraggingCard);
                        break;
                    }

                    // 般堆疊
                    targetCard->SetCardAbove(m_DraggingCard);
                    m_DraggingCard->SetCardBelow(targetCard);

                    // 堆疊完成後檢查合成配方，符合則建立延遲合成任務
                    {
                        auto stackBot = m_DraggingCard;
                        while (stackBot->GetCardBelow()) stackBot = stackBot->GetCardBelow();

                        float craftTime = 10.0f;
                        std::string craftOutput = m_RecipeManager.CheckCrafting(stackBot, craftTime);
                        if (!craftOutput.empty() && !m_Tasks.HasCraftForBottom(stackBot)) {
                            TaskScheduler::PendingCraft pc;
                            pc.stackBottom = stackBot;
                            for (auto cur = stackBot; cur; cur = cur->GetCardAbove())
                                pc.allCards.push_back(cur);
                            pc.outputName  = craftOutput;
                            pc.spawnX      = stackBot->GetX();
                            pc.spawnY      = stackBot->GetY();
                            pc.spawnScale  = stackBot->GetScale();
                            pc.timeLeftMs  = craftTime * 1000.0f;
                            pc.totalMs     = pc.timeLeftMs;

                            const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * stackBot->GetScale();
                            pc.bar = std::make_unique<TimeBar>(
                                m_Renderer,
                                glm::vec2{stackBot->GetX(), stackBot->GetY() + barOffsetY},
                                glm::vec2{GameConstants::CRAFT_BAR_BLACK_W, GameConstants::CRAFT_BAR_BLACK_H},
                                glm::vec2{GameConstants::CRAFT_BAR_WHITE_W, GameConstants::CRAFT_BAR_WHITE_H},
                                craftTime,
                                GameConstants::CRAFT_BAR_Z);
                            pc.bar->Start();
                            m_Tasks.AddCraft(std::move(pc));
                        }
                    }
                    break;
                }
            }
        }

        if (m_DraggingCard) m_DraggingCard = nullptr;
    }

    // 每幀偵測推擠
    for (size_t i = 0; i < m_Cards.size(); i++) {
        auto& cardA = m_Cards[i];
        if (cardA->GetType() == CardType::INTERACT) continue;
        if (!cardA->IsHitboxActive()) continue;
        if (m_DraggingCard && InSameStack(cardA, m_DraggingCard)) continue;

        for (size_t j = i + 1; j < m_Cards.size(); j++) {
            auto& cardB = m_Cards[j];
            if (cardB->GetType() == CardType::INTERACT) continue;
            if (!cardB->IsHitboxActive()) continue;
            if (m_DraggingCard && InSameStack(cardB, m_DraggingCard)) continue;
            if (InSameStack(cardA, cardB)) continue;

            float dx = cardA->GetX() - cardB->GetX();
            float dy = cardA->GetY() - cardB->GetY();
            float overlapX = (cardA->GetWidth()  + cardB->GetWidth())  * 0.5f - std::abs(dx);
            float overlapY = (cardA->GetHeight() + cardB->GetHeight()) * 0.5f - std::abs(dy);

            if (overlapX <= 0 || overlapY <= 0) continue;

            // 堆疊中含 Magic Glue（sticky）→ 該邊不被推；對側吸收整段重疊量
            const bool aSticky = StackHasSticky(cardA);
            const bool bSticky = StackHasSticky(cardB);
            if (aSticky && bSticky) continue; // 兩邊都黏住，不動

            if (overlapX <= overlapY) {
                const float pushA = aSticky ? 0.f : (bSticky ? overlapX : overlapX * 0.5f);
                const float pushB = bSticky ? 0.f : (aSticky ? overlapX : overlapX * 0.5f);
                cardA->MoveBy({dx >= 0.f ?  pushA : -pushA, 0.f});
                cardB->MoveBy({dx >= 0.f ? -pushB :  pushB, 0.f});
            } else {
                const float pushA = aSticky ? 0.f : (bSticky ? overlapY : overlapY * 0.5f);
                const float pushB = bSticky ? 0.f : (aSticky ? overlapY : overlapY * 0.5f);
                cardA->MoveBy({0.f, dy >= 0.f ?  pushA : -pushA});
                cardB->MoveBy({0.f, dy >= 0.f ? -pushB :  pushB});
            }
        }
    }
}