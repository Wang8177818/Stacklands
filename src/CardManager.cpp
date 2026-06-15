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

// ─────────────────────────────────────────────────────────────
// 存檔：把場上所有非 INTERACT 卡序列化為 JSON
//   - 每張卡分配一個 index（在 cards array 中的位置）
//   - 用 above_idx 表示 CardAbove 鏈（-1 = 鏈尾）
//   - 部分型別把多餘 state 寫進子物件 "state"
// ─────────────────────────────────────────────────────────────
nlohmann::json CardManager::ToJson() const {
    using nlohmann::json;
    // 1. 收集所有要存的卡（排除 INTERACT、排除疊在 INTERACT 上的 Coin —
    //    BlankSlot 上的硬幣由 App 另存避免重複）
    std::vector<std::shared_ptr<Card>> savable;
    for (auto& c : m_Cards) {
        if (c->GetType() == CardType::INTERACT) continue;
        if (c->GetType() == CardType::COIN) {
            // 找鏈最下方：若鏈底接到 INTERACT 卡，整鏈跳過
            auto bottom = c;
            while (bottom->GetCardBelow()) bottom = bottom->GetCardBelow();
            if (bottom->GetType() == CardType::INTERACT) continue;
        }
        savable.push_back(c);
    }
    // index 對應表
    std::unordered_map<Card*, int> idx;
    for (std::size_t i = 0; i < savable.size(); ++i) idx[savable[i].get()] = static_cast<int>(i);

    json cards = json::array();
    for (auto& c : savable) {
        json e;
        e["name"]  = c->GetName();
        e["type"]  = static_cast<int>(c->GetType());
        e["x"]     = c->GetX();
        e["y"]     = c->GetY();
        e["scale"] = c->GetScale();
        // above index（若鏈中下一張也在 savable 內）
        auto above = c->GetCardAbove();
        if (above && idx.count(above.get()))
            e["above"] = idx[above.get()];
        else
            e["above"] = -1;

        // 型別專屬 state
        json st = json::object();
        if (c->GetType() == CardType::FOOD) {
            st["nutrition"] = std::static_pointer_cast<FoodCard>(c)->GetNutritionValue();
        } else if (c->GetType() == CardType::STRUCTURE) {
            st["resourceCount"] = std::static_pointer_cast<StructureCard>(c)->GetResourceCount();
        } else if (c->GetType() == CardType::CHARACTER ||
                   c->GetType() == CardType::ANIMAL ||
                   c->GetType() == CardType::MONSTER) {
            auto cc = std::dynamic_pointer_cast<CombatCard>(c);
            if (cc) {
                st["health"] = cc->GetHealth();
                // 狀態效果計時器（只存非 0 的）
                json effects = json::object();
                if (cc->GetStunTimerMs()         > 0) effects["stun"]    = cc->GetStunTimerMs();
                if (cc->GetBleedTimerMs()        > 0) effects["bleed"]   = cc->GetBleedTimerMs();
                if (cc->GetPoisonTimerMs()       > 0) effects["poison"]  = cc->GetPoisonTimerMs();
                if (cc->GetInvulnerableTimerMs() > 0) effects["invul"]   = cc->GetInvulnerableTimerMs();
                if (cc->GetFrenzyTimerMs()       > 0) effects["frenzy"]  = cc->GetFrenzyTimerMs();
                if (!effects.empty()) st["effects"] = effects;

                // 角色才存裝備（動物 / 怪物無裝備）
                if (c->GetType() == CardType::CHARACTER) {
                    json eq = json::array();
                    bool hasAny = false;
                    for (const auto& e : cc->GetAllEquipData()) {
                        if (e.name.empty()) {
                            eq.push_back(std::string{});
                        } else {
                            eq.push_back(e.name);
                            hasAny = true;
                        }
                    }
                    if (hasAny) st["equips"] = eq;
                }
                // 動物存特殊能力冷卻
                if (c->GetType() == CardType::ANIMAL) {
                    auto ac = std::static_pointer_cast<AnimalCard>(c);
                    const float t = ac->GetAbilityTimer();
                    if (t > 0) st["abilityTimer"] = t;
                }
            }
        } else if (c->GetType() == CardType::BUILDING) {
            const std::string& n = c->GetName();
            if (n == "Coin Chest") {
                st["stored"] = std::static_pointer_cast<CoinChest>(c)->GetStored();
            } else if (n == "Hotpot") {
                st["stored"] = std::static_pointer_cast<Hotpot>(c)->GetStored();
            } else if (n == "Resource Chest") {
                auto rc = std::static_pointer_cast<ResourceChest>(c);
                st["stored"]     = rc->GetStored();
                st["storedName"] = rc->GetStoredName();
            }
        } else if (c->GetType() == CardType::PACK) {
            auto pk = std::static_pointer_cast<CardPack>(c);
            st["remaining"] = pk->GetCardsRemaining();
            json poolNames = json::array();
            for (const auto& d : pk->GetContentPool()) poolNames.push_back(d.name);
            st["pool"] = poolNames;
        }
        if (!st.empty()) e["state"] = st;

        cards.push_back(e);
    }

    // ── 進行中的延遲任務 ────────────────────────────────────────
    auto lookupIdx = [&](const std::shared_ptr<Card>& c) -> int {
        if (!c) return -1;
        auto it = idx.find(c.get());
        return it != idx.end() ? it->second : -1;
    };

    json pendingGathers = json::array();
    for (const auto& g : m_Tasks.GetGathers()) {
        int ci = lookupIdx(g.character.lock());
        int si = lookupIdx(g.structure.lock());
        if (ci < 0 || si < 0) continue; // 缺一邊就跳過
        pendingGathers.push_back({
            {"char", ci}, {"struct", si},
            {"exhausted", g.exhausted},
            {"spawnName", g.spawnName},
            {"spawnX", g.spawnX}, {"spawnY", g.spawnY}, {"spawnScale", g.spawnScale},
            {"timeLeftMs", g.timeLeftMs}, {"totalMs", g.totalMs}
        });
    }

    json pendingCrafts = json::array();
    for (const auto& c : m_Tasks.GetCrafts()) {
        int bi = lookupIdx(c.stackBottom.lock());
        if (bi < 0) continue;
        json allIdx = json::array();
        for (const auto& w : c.allCards) {
            int i = lookupIdx(w.lock());
            if (i >= 0) allIdx.push_back(i);
        }
        pendingCrafts.push_back({
            {"bottom", bi},
            {"all", allIdx},
            {"outputName", c.outputName},
            {"spawnX", c.spawnX}, {"spawnY", c.spawnY}, {"spawnScale", c.spawnScale},
            {"timeLeftMs", c.timeLeftMs}, {"totalMs", c.totalMs}
        });
    }

    json pendingCombats = json::array();
    for (const auto& cb : m_Tasks.GetCombats()) {
        int ti = lookupIdx(cb.target.lock());
        if (ti < 0) continue;
        json fighters = json::array();
        for (const auto& f : cb.fighters) {
            int fi = lookupIdx(f.fighter.lock());
            if (fi >= 0) fighters.push_back({{"f", fi}, {"timer", f.timer}});
        }
        if (fighters.empty()) continue;
        pendingCombats.push_back({
            {"target", ti},
            {"fighters", fighters},
            {"targetTimer", cb.targetTimer}
        });
    }

    json root;
    root["maxCardCount"]   = m_MaxCardCount;
    root["cards"]          = cards;
    root["pendingGathers"] = pendingGathers;
    root["pendingCrafts"]  = pendingCrafts;
    root["pendingCombats"] = pendingCombats;
    return root;
}

// ─────────────────────────────────────────────────────────────
// 讀檔：清空現有可變卡（保留 INTERACT slots）後依 JSON 重建
// ─────────────────────────────────────────────────────────────
void CardManager::LoadFromJson(const nlohmann::json& j, float spawnScale) {
    using nlohmann::json;
    // 1. 移除所有非 INTERACT 卡
    std::vector<std::shared_ptr<Card>> toRemove;
    for (auto& c : m_Cards)
        if (c->GetType() != CardType::INTERACT) toRemove.push_back(c);
    for (auto& c : toRemove) RemoveCard(c);

    // 2. 重置 maxCardCount（Warehouse 重建時會再 +14）
    m_MaxCardCount = 50;

    // 3. 重建卡片（先建立全部，之後再串鏈）
    std::vector<std::shared_ptr<Card>> rebuilt;
    if (!j.contains("cards") || !j["cards"].is_array()) return;
    const auto& arr = j["cards"];
    rebuilt.reserve(arr.size());

    for (const auto& e : arr) {
        const std::string name = e.value("name", std::string{});
        const int   typeInt = e.value("type", static_cast<int>(CardType::BASIC));
        const auto  type    = static_cast<CardType>(typeInt);
        const float x  = e.value("x", 0.0f);
        const float y  = e.value("y", 0.0f);
        const float s  = e.value("scale", spawnScale);

        std::shared_ptr<Card> card;
        if (type == CardType::PACK) {
            // 卡包要用 SpawnPackByName（PackDatabase 才有 totalCards / pool）
            SpawnPackByName(name, s, x, y);
            // SpawnPackByName 把卡包加到 m_Cards 最後一張
            if (!m_Cards.empty()) card = m_Cards.back();
        } else {
            card = SpawnCardByName(name, s, x, y);
        }
        rebuilt.push_back(card);

        if (!card) continue;
        if (!e.contains("state")) continue;
        const auto& st = e["state"];

        if (card->GetType() == CardType::FOOD && st.contains("nutrition")) {
            std::static_pointer_cast<FoodCard>(card)->SetNutritionValue(st["nutrition"].get<int>());
        } else if (card->GetType() == CardType::STRUCTURE && st.contains("resourceCount")) {
            std::static_pointer_cast<StructureCard>(card)->SetResourceCount(st["resourceCount"].get<int>());
        } else if (card->GetType() == CardType::CHARACTER ||
                   card->GetType() == CardType::ANIMAL ||
                   card->GetType() == CardType::MONSTER) {
            auto cc = std::dynamic_pointer_cast<CombatCard>(card);
            if (cc && st.contains("health")) cc->SetCurrentHealth(st["health"].get<int>());
            // 角色裝備還原：依名稱查 m_CardDatabase 取 bonus
            if (cc && card->GetType() == CardType::CHARACTER && st.contains("equips")) {
                const auto& eq = st["equips"];
                for (std::size_t i = 0; i < eq.size() && i < 4; ++i) {
                    const std::string eqName = eq[i].is_string() ? eq[i].get<std::string>() : "";
                    if (eqName.empty()) continue;
                    auto it = m_CardDatabase.find(eqName);
                    if (it == m_CardDatabase.end()) continue;
                    const CardSpawnData& d = it->second;
                    cc->StoreEquipment(static_cast<EquipSlot>(i), eqName,
                                       d.attack, d.health, d.defense,
                                       d.attackSpeed, d.hitChance);
                }
                if (st.contains("health")) cc->SetCurrentHealth(st["health"].get<int>());
            }
            // 狀態效果還原
            if (cc && st.contains("effects")) {
                const auto& eff = st["effects"];
                if (eff.contains("stun"))   cc->SetStunTimerMs(eff["stun"].get<float>());
                if (eff.contains("bleed"))  cc->SetBleedTimerMs(eff["bleed"].get<float>());
                if (eff.contains("poison")) cc->SetPoisonTimerMs(eff["poison"].get<float>());
                if (eff.contains("invul"))  cc->SetInvulnerableTimerMs(eff["invul"].get<float>());
                if (eff.contains("frenzy")) cc->SetFrenzyTimerMs(eff["frenzy"].get<float>());
            }
            // 動物冷卻還原
            if (card->GetType() == CardType::ANIMAL && st.contains("abilityTimer")) {
                std::static_pointer_cast<AnimalCard>(card)
                    ->SetAbilityTimer(st["abilityTimer"].get<float>());
            }
        } else if (card->GetType() == CardType::BUILDING) {
            const std::string& n = card->GetName();
            if (n == "Coin Chest" && st.contains("stored")) {
                std::static_pointer_cast<CoinChest>(card)->Deposit(st["stored"].get<int>());
            } else if (n == "Hotpot" && st.contains("stored")) {
                std::static_pointer_cast<Hotpot>(card)->Deposit(st["stored"].get<int>());
            } else if (n == "Resource Chest" && st.contains("stored") && st.contains("storedName")) {
                const std::string sn = st["storedName"].get<std::string>();
                const int cnt = st["stored"].get<int>();
                std::string icon;
                if (!sn.empty()) {
                    auto it = m_CardDatabase.find(sn);
                    if (it != m_CardDatabase.end()) icon = it->second.iconPath;
                }
                std::static_pointer_cast<ResourceChest>(card)->RestoreState(sn, cnt, icon);
            }
        } else if (card->GetType() == CardType::PACK &&
                   st.contains("remaining") && st.contains("pool")) {
            auto pk = std::static_pointer_cast<CardPack>(card);
            std::vector<CardSpawnData> pool;
            for (const auto& pname : st["pool"]) {
                if (!pname.is_string()) continue;
                auto it = m_CardDatabase.find(pname.get<std::string>());
                if (it == m_CardDatabase.end()) continue;
                CardSpawnData d = it->second;
                d.scale = s;
                pool.push_back(d);
            }
            pk->RestoreState(st["remaining"].get<int>(), pool);
        }
    }

    // 4. 串鏈：用 above index
    for (std::size_t i = 0; i < arr.size() && i < rebuilt.size(); ++i) {
        int aboveIdx = arr[i].value("above", -1);
        if (aboveIdx < 0 || aboveIdx >= static_cast<int>(rebuilt.size())) continue;
        if (!rebuilt[i] || !rebuilt[aboveIdx]) continue;
        rebuilt[i]->SetCardAbove(rebuilt[aboveIdx]);
        rebuilt[aboveIdx]->SetCardBelow(rebuilt[i]);
    }

    // 5. 套用 JSON 內的 maxCardCount（覆蓋 Warehouse 重建時的累加）
    if (j.contains("maxCardCount")) m_MaxCardCount = j["maxCardCount"].get<int>();

    // 6. 還原進行中的延遲任務（gathers / crafts / combats）
    auto getCard = [&](int i) -> std::shared_ptr<Card> {
        if (i < 0 || i >= static_cast<int>(rebuilt.size())) return nullptr;
        return rebuilt[i];
    };

    // 共用：建立讀條（以 anchor 卡片座標為基準）
    auto makeBar = [&](const std::shared_ptr<Card>& anchor,
                       float durationMs, float elapsedMs) {
        if (!anchor) return std::unique_ptr<TimeBar>{};
        const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * anchor->GetScale();
        auto bar = std::make_unique<TimeBar>(
            m_Renderer,
            glm::vec2{anchor->GetX(), anchor->GetY() + barOffsetY},
            glm::vec2{GameConstants::CRAFT_BAR_BLACK_W, GameConstants::CRAFT_BAR_BLACK_H},
            glm::vec2{GameConstants::CRAFT_BAR_WHITE_W, GameConstants::CRAFT_BAR_WHITE_H},
            durationMs / 1000.0f,
            GameConstants::CRAFT_BAR_Z);
        bar->Start();
        if (elapsedMs > 0) bar->Update(elapsedMs); // 將白條推到當前進度
        return bar;
    };

    if (j.contains("pendingGathers")) {
        for (const auto& g : j["pendingGathers"]) {
            auto ch = getCard(g.value("char",   -1));
            auto st = getCard(g.value("struct", -1));
            if (!ch || !st) continue;
            TaskScheduler::PendingGather pg;
            pg.character  = ch;
            pg.structure  = st;
            pg.exhausted  = g.value("exhausted", false);
            pg.spawnName  = g.value("spawnName", std::string{});
            pg.spawnX     = g.value("spawnX", 0.0f);
            pg.spawnY     = g.value("spawnY", 0.0f);
            pg.spawnScale = g.value("spawnScale", spawnScale);
            pg.totalMs    = g.value("totalMs",    10000.0f);
            pg.timeLeftMs = g.value("timeLeftMs", pg.totalMs);
            pg.bar = makeBar(st, pg.totalMs, pg.totalMs - pg.timeLeftMs);
            // 重連堆疊鏈：角色疊在結構上方
            st->SetCardAbove(ch);
            ch->SetCardBelow(st);
            m_Tasks.AddGather(std::move(pg));
        }
    }

    if (j.contains("pendingCrafts")) {
        for (const auto& c : j["pendingCrafts"]) {
            auto bottom = getCard(c.value("bottom", -1));
            if (!bottom) continue;
            TaskScheduler::PendingCraft pc;
            pc.stackBottom = bottom;
            if (c.contains("all")) {
                for (const auto& i : c["all"]) {
                    auto cc = getCard(i.is_number_integer() ? i.get<int>() : -1);
                    if (cc) pc.allCards.push_back(cc);
                }
            }
            pc.outputName = c.value("outputName", std::string{});
            pc.spawnX     = c.value("spawnX",     0.0f);
            pc.spawnY     = c.value("spawnY",     0.0f);
            pc.spawnScale = c.value("spawnScale", spawnScale);
            pc.totalMs    = c.value("totalMs",    10000.0f);
            pc.timeLeftMs = c.value("timeLeftMs", pc.totalMs);
            pc.bar = makeBar(bottom, pc.totalMs, pc.totalMs - pc.timeLeftMs);
            m_Tasks.AddCraft(std::move(pc));
        }
    }

    if (j.contains("pendingCombats")) {
        // 用 JoinOrCreateCombat 重啟戰鬥（無法保留 timer，已 reset）
        for (const auto& cb : j["pendingCombats"]) {
            auto target = getCard(cb.value("target", -1));
            if (!target) continue;
            if (cb.contains("fighters")) {
                for (const auto& f : cb["fighters"]) {
                    auto fighter = getCard(f.value("f", -1));
                    if (fighter) m_Tasks.JoinOrCreateCombat(target, fighter);
                }
            }
        }
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
    SpawnCardByName(name, m_ZoomRatio * 0.05f,
                    x + off(m_RandomGenerator),
                    y + off(m_RandomGenerator));
}

std::vector<std::string> CardManager::GetAllCardNames() const {
    std::vector<std::string> names;
    names.reserve(m_CardDatabase.size());
    for (const auto& pair : m_CardDatabase)
        names.push_back(pair.first);
    std::sort(names.begin(), names.end());
    return names;
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

    // 2.5 怪物主動追擊最近的角色，接觸後開戰
    for (auto& card : m_Cards) {
        if (card->GetType() != CardType::MONSTER) continue;
        auto monster = std::static_pointer_cast<MonsterCard>(card);

        // 戰鬥中 / 拖曳中 / 堆疊中 / 無碰撞箱 → 不主動行動
        if (monster->IsInCombat() || monster == m_DraggingCard ||
            monster->GetCardBelow() || monster->GetCardAbove() ||
            !monster->IsHitboxActive()) {
            monster->ClearSeekTarget();
            continue;
        }

        // 找最近且可被攻擊的角色（排除拖曳中、已在戰鬥中即碰撞箱關閉者）
        std::shared_ptr<Card> nearest;
        float bestDist2 = 0.0f;
        for (auto& other : m_Cards) {
            if (other->GetType() != CardType::CHARACTER) continue;
            if (other == m_DraggingCard || !other->IsHitboxActive()) continue;
            float dx = other->GetX() - monster->GetX();
            float dy = other->GetY() - monster->GetY();
            float d2 = dx * dx + dy * dy;
            if (!nearest || d2 < bestDist2) { nearest = other; bestDist2 = d2; }
        }

        // 場上沒有角色才隨機漫遊，否則持續主動追擊（不再受偵測範圍限制）
        if (!nearest) { monster->ClearSeekTarget(); continue; }

        // 接觸（重疊）即開戰，否則持續靠近
        if (monster->IsOverlapping(nearest)) {
            monster->ClearSeekTarget();
            m_Tasks.JoinOrCreateCombat(monster, nearest);
        } else {
            monster->SetSeekTarget(nearest->GetX(), nearest->GetY());
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
                        pg.totalMs    = gatherTimeMs;

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

    // 推擠後再次約束邊界，避免被推出場地
    for (auto& card : m_Cards) {
        if (card == m_DraggingCard) continue;
        if (card->GetType() == CardType::INTERACT) continue;
        ClampCardToField(card);
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