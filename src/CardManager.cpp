//
// Created by m0938 on 2026/3/20.
//
#include "CardManager.hpp"
#include "CardFactory.hpp"
#include "RecipeManager.hpp"
#include "CharacterCard.hpp"
#include "AnimalCard.hpp"
#include "CardPack.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>

#include "WarehouseCard.hpp"

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
    if (typeStr == "ANIMAL")  return CardType::ANIMAL;
    if (typeStr == "MONSTER") return CardType::MONSTER;
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
        data.attackSpeed     = item.value("attackSpeed", 0.0f);
        data.hitChance       = item.value("hitChance", 0.0f);
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
    std::vector<CardSpawnData> actualPool;
    for (const auto& cardName : tmpl.pool) {
        if (m_CardDatabase.count(cardName)) {
            CardSpawnData poolData = m_CardDatabase[cardName];

            poolData.scale = scale;

            actualPool.push_back(poolData);
        }
    }

    auto pack = std::make_shared<CardPack>(
        x, y, tmpl.name, tmpl.sellValue, tmpl.iconPath, scale * 0.6, tmpl.totalCards, actualPool);

    AddCard(pack);
}

CardManager::CardManager(Util::Renderer& renderer) : m_Renderer(renderer) {
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

std::shared_ptr<Card> CardManager::CreateCardFromData(float x, float y, const CardSpawnData& data) {
    auto spawnCb = [this](const std::string& name, float sx, float sy) {
        std::uniform_real_distribution<float> off(-50.0f, 50.0f);
        SpawnCardByName(name, m_ZoomRatio * 0.05f,
                        sx + off(m_RandomGenerator),
                        sy + off(m_RandomGenerator));
    };

    auto newCard = CardFactory::Create(x, y, data, m_MaxCardCount, spawnCb);
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
                SpawnCardByName(drop, card->GetScale(),
                                card->GetX() + off(m_RandomGenerator),
                                card->GetY() + off(m_RandomGenerator));
            }
            RemoveCard(card);
        }
    }

    // 3. 更新卡片動畫與跟隨
    for (auto& card : m_Cards) {
        card->Update();
    }

    // 計時任務
    {
        // 套用時間流速倍率（PAUSE=0 → 完全停住合成 / 採集 / 讀條動畫）
        float dtMs = static_cast<float>(Util::Time::GetDeltaTimeMs()) * m_TimeScale;

        // 採集等待
        for (auto it = m_PendingGathers.begin(); it != m_PendingGathers.end(); ) {
            auto ch = it->character.lock();
            auto st = it->structure.lock();
            // 中斷：任一邊消失，或玩家把角色從結構上拖開
            if (!ch || !st || ch->GetCardBelow() != st || st->GetCardAbove() != ch) {
                it = m_PendingGathers.erase(it);
                continue;
            }

            // 讀條跟著結構卡（stackBottom）+ 推進進度
            if (it->bar) {
                const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * st->GetScale();
                it->bar->SetPosition({st->GetX(), st->GetY() + barOffsetY});
                it->bar->Update(dtMs);
            }

            it->timeLeftMs -= dtMs;
            if (it->timeLeftMs <= 0.0f) {
                // 角色與結構分離
                if (auto st = it->structure.lock()) st->SetCardAbove(nullptr);
                if (auto ch = it->character.lock())  ch->SetCardBelow(nullptr);
                // 結構耗盡則移除
                if (it->exhausted) {
                    if (auto st = it->structure.lock())
                        RemoveCard(st);
                }
                // 生成採集到的卡片
                if (!it->spawnName.empty()) {
                    std::uniform_real_distribution<float> distOff(-60.f, 60.f);
                    SpawnCardByName(it->spawnName, it->spawnScale,
                                    it->spawnX + distOff(m_RandomGenerator),
                                    it->spawnY + distOff(m_RandomGenerator));
                }
                it = m_PendingGathers.erase(it);
            } else {
                ++it;
            }
        }

        // 合成等待
        for (auto it = m_PendingCrafts.begin(); it != m_PendingCrafts.end(); ) {
            auto bottom = it->stackBottom.lock();
            if (!bottom) { it = m_PendingCrafts.erase(it); continue; }

            // 每幀驗證配方仍成立（玩家可能抽走某張卡 → 中斷）
            // erase 會解構 PendingCraft → unique_ptr<TimeBar> 析構 → 自動移出 Renderer
            float verifyTime = 0.0f;
            if (m_RecipeManager.CheckCrafting(bottom, verifyTime) != it->outputName) {
                it = m_PendingCrafts.erase(it);
                continue;
            }

            // 讀條跟著 stackBottom + 推進進度
            if (it->bar) {
                const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * bottom->GetScale();
                it->bar->SetPosition({bottom->GetX(), bottom->GetY() + barOffsetY});
                it->bar->Update(dtMs);
            }

            it->timeLeftMs -= dtMs;
            if (it->timeLeftMs <= 0.0f) {
                float sx = bottom->GetX(), sy = bottom->GetY(), ss = bottom->GetScale();
                std::vector<std::shared_ptr<Card>> toDelete;
                for (auto cur = bottom; cur; cur = cur->GetCardAbove()) {
                    if (cur->GetType() != CardType::CHARACTER && cur->GetType() != CardType::BUILDING)
                        toDelete.push_back(cur);
                }
                for (auto cur = bottom; cur; ) {
                    auto next = cur->GetCardAbove();
                    cur->SetCardAbove(nullptr);
                    cur->SetCardBelow(nullptr);
                    cur = next;
                }
                for (auto& c : toDelete) RemoveCard(c);
                SpawnCardByName(it->outputName, ss, sx, sy);
                it = m_PendingCrafts.erase(it);
            } else {
                ++it;
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
        if (targetToPick != nullptr) {
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

                    // Gather（採集：角色堆疊在結構卡上，等待 10s 後分離並生成資源卡）
                    if (targetCard->GetType() == CardType::STRUCTURE &&
                        m_DraggingCard->GetType() == CardType::CHARACTER) {

                        auto structure = std::static_pointer_cast<StructureCard>(targetCard);
                        std::string spawnName = structure->Gather(m_RandomGenerator);

                        // 將角色堆疊在結構卡上方
                        targetCard->SetCardAbove(m_DraggingCard);
                        m_DraggingCard->SetCardBelow(targetCard);

                        PendingGather pg;
                        pg.character  = m_DraggingCard;
                        pg.structure  = targetCard;
                        pg.exhausted  = structure->IsExhausted();
                        pg.spawnName  = spawnName;
                        pg.spawnX     = structure->GetX();
                        pg.spawnY     = structure->GetY();
                        pg.spawnScale = m_DraggingCard->GetScale();
                        pg.timeLeftMs = GameConstants::GATHER_TIME_MS;

                        const float gatherSec = GameConstants::GATHER_TIME_MS / 1000.0f;
                        const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * targetCard->GetScale();
                        pg.bar = std::make_unique<TimeBar>(
                            m_Renderer,
                            glm::vec2{targetCard->GetX(), targetCard->GetY() + barOffsetY},
                            glm::vec2{GameConstants::CRAFT_BAR_BLACK_W, GameConstants::CRAFT_BAR_BLACK_H},
                            glm::vec2{GameConstants::CRAFT_BAR_WHITE_W, GameConstants::CRAFT_BAR_WHITE_H},
                            gatherSec,
                            GameConstants::CRAFT_BAR_Z);
                        pg.bar->Start();

                        m_PendingGathers.push_back(std::move(pg));
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
                        if (!craftOutput.empty()) {
                            // 避免同一堆疊重複排隊
                            bool alreadyPending = false;
                            for (const auto& pc : m_PendingCrafts) {
                                if (pc.stackBottom.lock() == stackBot) { alreadyPending = true; break; }
                            }
                            if (!alreadyPending) {
                                PendingCraft pc;
                                pc.stackBottom = stackBot;
                                for (auto cur = stackBot; cur; cur = cur->GetCardAbove())
                                    pc.allCards.push_back(cur);
                                pc.outputName  = craftOutput;
                                pc.spawnX      = stackBot->GetX();
                                pc.spawnY      = stackBot->GetY();
                                pc.spawnScale  = stackBot->GetScale();
                                pc.timeLeftMs  = craftTime * 1000.0f;
                                pc.totalMs     = pc.timeLeftMs;

                                // 讀條放在 stackBottom 上方
                                const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * stackBot->GetScale();
                                pc.bar = std::make_unique<TimeBar>(
                                    m_Renderer,
                                    glm::vec2{stackBot->GetX(), stackBot->GetY() + barOffsetY},
                                    glm::vec2{GameConstants::CRAFT_BAR_BLACK_W, GameConstants::CRAFT_BAR_BLACK_H},
                                    glm::vec2{GameConstants::CRAFT_BAR_WHITE_W, GameConstants::CRAFT_BAR_WHITE_H},
                                    craftTime,
                                    GameConstants::CRAFT_BAR_Z);
                                pc.bar->Start();

                                m_PendingCrafts.push_back(std::move(pc));
                            }
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
        // 跳過 INTERACT type
        if (cardA->GetType() == CardType::INTERACT) continue;
        // 跳過正在拖曳的卡片及其整個堆疊
        if (m_DraggingCard && InSameStack(cardA, m_DraggingCard)) continue;

        for (size_t j = i + 1; j < m_Cards.size(); j++) {
            auto& cardB = m_Cards[j];
            if (cardB->GetType() == CardType::INTERACT) continue;
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