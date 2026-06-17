//
// Created by m0938 on 2026/3/20.
//
#include "CardManager.hpp"
#include "CardFactory.hpp"
#include "EffectData.hpp"
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
#include <limits>

#include "WarehouseCard.hpp"
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

        // 背景覆寫
        data.backgroundPath = item.value("backgroundPath", "");

        // 結構化特效列表
        data.effects.clear();
        if (item.contains("effects")) {
            for (const auto& e : item["effects"]) {
                EffectData ed;
                std::string typeStr = e.value("type", "");
                if      (typeStr == "Stun")         ed.type = EffectData::Type::Stun;
                else if (typeStr == "Bleed")        ed.type = EffectData::Type::Bleed;
                else if (typeStr == "Poison")       ed.type = EffectData::Type::Poison;
                else if (typeStr == "CriticalHit")  ed.type = EffectData::Type::CriticalHit;
                else if (typeStr == "Lifesteal")    ed.type = EffectData::Type::Lifesteal;
                else if (typeStr == "DamageAll")    ed.type = EffectData::Type::DamageAll;
                else if (typeStr == "DamageRandom") ed.type = EffectData::Type::DamageRandom;
                else if (typeStr == "Heal")         ed.type = EffectData::Type::Heal;
                else if (typeStr == "Frenzy")       ed.type = EffectData::Type::Frenzy;
                else if (typeStr == "Invulnerable") ed.type = EffectData::Type::Invulnerable;
                else continue;

                std::string tgtStr = e.value("target", "target");
                if      (tgtStr == "target")             ed.target = EffectData::Target::DirectTarget;
                else if (tgtStr == "self")               ed.target = EffectData::Target::Self;
                else if (tgtStr == "random_enemy")       ed.target = EffectData::Target::RandomEnemy;
                else if (tgtStr == "all_enemies")        ed.target = EffectData::Target::AllEnemies;
                else if (tgtStr == "random_friendly")    ed.target = EffectData::Target::RandomFriendly;
                else if (tgtStr == "friendly_lowest_hp") ed.target = EffectData::Target::FriendlyLowestHp;
                else if (tgtStr == "all_friendlies")     ed.target = EffectData::Target::AllFriendlies;

                ed.chance   = e.value("chance",   0.0f) / 100.0f;
                ed.duration = e.value("duration", 5.0f);
                ed.value    = e.value("value",    2);
                ed.passive  = e.value("passive",  false);
                data.effects.push_back(ed);
            }
        }

        // 地點卡專用
        data.maxGathers = item.value("maxGathers", 0);
        if (item.contains("guaranteedDrops")) {
            for (const auto& entry : item["guaranteedDrops"]) {
                int         count    = entry.value("count", 0);
                std::string cardName = entry.value("name", "");
                if (count > 0 && !cardName.empty())
                    data.guaranteedDrops.push_back({count, cardName});
            }
        }

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
                  auto card = SpawnCardByName(name, scale, x, y);
                  TryAutoStack(card);
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

    // 2. 收集所有食物卡（用 m_Cards 順序）
    std::vector<std::shared_ptr<FoodCard>> foods;
    for (auto& c : m_Cards)
        if (c->GetType() == CardType::FOOD)
            foods.push_back(std::static_pointer_cast<FoodCard>(c));

    // 3. 依優先順序餵食：每人從食物池逐單位扣 nutrition
    std::vector<std::shared_ptr<CharacterCard>> starved;
    std::size_t foodIdx = 0;
    int totalConsumed = 0;
    for (auto& chr : chars) {
        int need = chr->GetFoodConsumption();
        while (need > 0 && foodIdx < foods.size()) {
            int taken = foods[foodIdx]->ConsumeNutrition(need);
            need          -= taken;
            totalConsumed += taken;
            if (foods[foodIdx]->GetNutritionValue() == 0) ++foodIdx;
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

void CardManager::ClearAllCards() {
    for (auto& card : m_Cards) {
        for (auto& obj : card->GetGameObjects()) {
            obj->SetVisible(false);
            obj->m_Transform.translation = glm::vec2(-9999, -9999);
        }
    }
    m_Cards.clear();
    m_DraggingCard = nullptr;
}

void CardManager::OnSpawn(const std::string& name, float x, float y) {
    std::uniform_real_distribution<float> off(-50.0f, 50.0f);
    auto card = SpawnCardByName(name, m_ZoomRatio * 0.05f,
                                x + off(m_RandomGenerator),
                                y + off(m_RandomGenerator));
    TryAutoStack(card);
}

std::vector<std::string> CardManager::GetAllCardNames() const {
    std::vector<std::string> names;
    names.reserve(m_CardDatabase.size());
    for (const auto& pair : m_CardDatabase)
        names.push_back(pair.first);
    std::sort(names.begin(), names.end());
    return names;
}

void CardManager::TryAutoStack(const std::shared_ptr<Card>& newCard) {
    if (!newCard) return;
    if (!newCard->CanStackOnto()) return;

    constexpr float SEARCH_RADIUS_SQ = 200.0f * 200.0f;

    std::shared_ptr<Card> bestTarget = nullptr;
    float bestDist = SEARCH_RADIUS_SQ;

    for (auto& card : m_Cards) {
        if (card == newCard) continue;
        if (card->GetType() == CardType::INTERACT) continue;
        if (!card->IsHitboxActive()) continue;
        if (card->GetName() != newCard->GetName()) continue;

        float dx = card->GetX() - newCard->GetX();
        float dy = card->GetY() - newCard->GetY();
        float distSq = dx * dx + dy * dy;

        if (distSq < bestDist) {
            bestDist = distSq;
            bestTarget = card;
        }
    }

    if (!bestTarget) return;

    // 找到目標堆疊的最頂端
    auto topCard = bestTarget;
    while (topCard->GetCardAbove()) topCard = topCard->GetCardAbove();

    // 檢查是否允許疊加
    if (!topCard->OnStacked(newCard)) return;

    // 直接將新卡位置對齊到頂端卡片，避免推擠
    float dx = topCard->GetX() - newCard->GetX();
    float dy = topCard->GetY() - newCard->GetY();
    if (dx != 0.f || dy != 0.f) {
        newCard->MoveBy({dx, dy});
    }

    // 建立疊加連結
    topCard->SetCardAbove(newCard);
    newCard->SetCardBelow(topCard);
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
                auto dropCard = SpawnCardByName(drop, card->GetScale(),
                                                card->GetX() + off(m_RandomGenerator),
                                                card->GetY() + off(m_RandomGenerator));
                TryAutoStack(dropCard);
            }
            RemoveCard(card);
        }
    }

    // 2.5 讓怪物朝最近的角色移動，並在重疊時主動觸發戰鬥
    {
        // 收集所有角色卡
        std::vector<std::shared_ptr<Card>> characters;
        for (auto& card : m_Cards) {
            if (card->GetType() == CardType::CHARACTER)
                characters.push_back(card);
        }

        for (auto& card : m_Cards) {
            if (card->GetType() != CardType::MONSTER) continue;
            auto monster = std::static_pointer_cast<MonsterCard>(card);

            if (monster->IsInCombat()) continue;

            if (characters.empty()) {
                monster->ClearChaseTarget();
                continue;
            }

            // 找最近的角色
            float bestDist = std::numeric_limits<float>::max();
            std::shared_ptr<Card> nearest = nullptr;
            for (auto& ch : characters) {
                float dx = ch->GetX() - monster->GetX();
                float dy = ch->GetY() - monster->GetY();
                float dist = dx * dx + dy * dy;
                if (dist < bestDist) {
                    bestDist = dist;
                    nearest = ch;
                }
            }

            monster->SetChaseTarget(nearest->GetX(), nearest->GetY());

            // 怪物與角色重疊時主動觸發戰鬥
            if (monster->IsOverlapping(nearest)) {
                m_Tasks.JoinOrCreateCombat(monster, nearest);
            }
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
        ClampCardToField(m_DraggingCard);

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

                        // 從資料庫查出裝備的特效
                        std::vector<EffectData> equipEffects;
                        if (m_CardDatabase.count(equipName))
                            equipEffects = m_CardDatabase[equipName].effects;

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
                                equip->GetBonusHitChance(),
                                equipEffects
                            };

                            // 切斷堆疊連結，確保 PendingCraft 的 weak_ptr 立即失效，
                            // 避免合成任務抓住 ghost character 造成卡牌卡死
                            if (auto above = charCard->GetCardAbove()) above->SetCardBelow(nullptr);
                            if (auto below = charCard->GetCardBelow()) below->SetCardAbove(nullptr);
                            charCard->SetCardAbove(nullptr);
                            charCard->SetCardBelow(nullptr);

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
                                                     equip->GetBonusHitChance(),
                                                     equipEffects);
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
                            exhausted    = location->IsExhausted();
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

    // 每幀將卡片約束在場地範圍內
    for (auto& card : m_Cards) {
        if (card == m_DraggingCard) continue; // 拖曳中的不管
        if (card->GetType() == CardType::INTERACT) continue;
        ClampCardToField(card);
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

            if (overlapX <= overlapY) {
                float push = overlapX * 0.5f;
                cardA->MoveBy({dx >= 0.f ?  push : -push, 0.f});
                cardB->MoveBy({dx >= 0.f ? -push :  push, 0.f});
            } else {
                float push = overlapY * 0.5f;
                cardA->MoveBy({0.f, dy >= 0.f ?  push : -push});
                cardB->MoveBy({0.f, dy >= 0.f ? -push :  push});
            }
        }
    }
}

void CardManager::ClampCardToField(const std::shared_ptr<Card>& card) {
    if (!m_Field) return;

    // 場地的世界中心 & 半寬半高
    glm::vec2 fieldPos  = m_Field->m_Transform.translation;
    glm::vec2 fieldSize = m_Field->GetScaledSize();
    float fieldHalfW = fieldSize.x * 0.5f;
    float fieldHalfH = fieldSize.y * 0.5f;

    // 卡片半寬半高
    float cardHalfW = card->GetWidth()  * 0.5f;
    float cardHalfH = card->GetHeight() * 0.5f;

    // 場地邊界（卡片中心允許的範圍）
    float minX = fieldPos.x - fieldHalfW + cardHalfW;
    float maxX = fieldPos.x + fieldHalfW - cardHalfW;
    float minY = fieldPos.y - fieldHalfH + cardHalfH;
    float maxY = fieldPos.y + fieldHalfH - cardHalfH;

    float cx = card->GetX();
    float cy = card->GetY();
    float nx = std::clamp(cx, minX, maxX);
    float ny = std::clamp(cy, minY, maxY);

    if (nx != cx || ny != cy) {
        card->MoveBy({nx - cx, ny - cy});
    }
}