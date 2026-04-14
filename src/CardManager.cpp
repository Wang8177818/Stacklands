//
// Created by m0938 on 2026/3/20.
//
#include "CardManager.hpp"
#include "RecipeManager.hpp"
#include "CharacterCard.hpp"
#include "CardPack.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include "Util/Logger.hpp"

CardType StringToCardType(const std::string& typeStr) {
    if (typeStr == "CHARACTER") return CardType::CHARACTER;
    if (typeStr == "RESOURCE") return CardType::RESOURCE;
    if (typeStr == "COIN") return CardType::COIN;
    if (typeStr == "PACK") return CardType::PACK;
    if (typeStr == "EQUIPMENT") return CardType::EQUIPMENT;
    if (typeStr == "BUILDING") return CardType::BUILDING;
    if (typeStr == "STRUCTURE") return CardType::STRUCTURE;
    return CardType::BASIC;
}

EquipSlot StringToEquipSlot(const std::string& slotStr) {
    if (slotStr == "HEAD") return EquipSlot::HEAD;
    if (slotStr == "HAND") return EquipSlot::HAND;
    if (slotStr == "BODY") return EquipSlot::BODY;
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

    // 遍歷陣列，存入字典 (Key 是卡片名字，Value 是配方)
    for (const auto& item : j) {
        CardSpawnData data;
        // 使用 .value() 加上預設值，就算 JSON 漏寫某個欄位也不會崩潰！
        data.name      = item.value("name", "Unknown");
        data.sellValue = item.value("sellValue", 0);
        data.type      = StringToCardType(item.value("type", "BASIC"));

        std::string rawPath = item.value("iconPath", "");
        data.iconPath  = rawPath.empty() ? "" : RESOURCE_DIR + rawPath;

        data.health    = item.value("health", 0); // resource 不用寫hp
        data.scale     = 0.05f;

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
    std::shared_ptr<Card> newCard = nullptr;

    if (data.type == CardType::CHARACTER) {
        newCard = std::make_shared<CharacterCard>(x, y, data.name, data.sellValue, data.iconPath, data.scale, data.health, data.attack);
    }else if (data.type == CardType::RESOURCE){
        newCard = std::make_shared<ResourceCard>(x, y, data.name, data.sellValue, data.iconPath, data.scale);
    }else if (data.type == CardType::COIN){
        newCard = std::make_shared<CoinCard>(x, y, data.scale);
    }else if (data.type == CardType::EQUIPMENT) {
        newCard = std::make_shared<EquipmentCard>(x, y, data.name, data.sellValue, data.iconPath, data.attack, data.health, data.equipSlot, data.scale);
    }else if (data.type == CardType::BUILDING) {
        newCard = std::make_shared<BuildingCard>(x, y, data.name, data.sellValue, data.iconPath, data.scale);
    }else if (data.type == CardType::STRUCTURE) {
        newCard = std::make_shared<StructureCard>(x, y, data.name, data.sellValue, data.iconPath, data.scale);
    }else {
        newCard = std::make_shared<Card>(x, y, data.name, data.sellValue, data.type, data.scale);
    }

    // 建立完成後自動加入管理陣列並渲染
    if (newCard) {
        AddCard(newCard);
    }
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

    // 2. 更新卡片動畫與跟隨
    for (auto& card : m_Cards) {
        card->Update();
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
        if (m_DraggingCard->GetType() != CardType::PACK) {
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

                    // ── 裝備卡放到角色卡：消耗裝備並處理轉職 ────────
                    if (m_DraggingCard->GetType() == CardType::EQUIPMENT &&
                        targetCard->GetType() == CardType::CHARACTER) {

                        auto equip    = std::static_pointer_cast<EquipmentCard>(m_DraggingCard);
                        auto charCard = std::static_pointer_cast<CharacterCard>(targetCard);
                        const std::string equipName = equip->GetName();

                        std::string outputName = m_RecipeManager.CheckProfession(equipName);

                        if (!outputName.empty()) {
                            // ── 觸發轉職 ──────────────────────────────────
                            float spawnX     = charCard->GetX();
                            float spawnY     = charCard->GetY();
                            float spawnScale = charCard->GetScale();

                            // 繼承舊角色插槽，將 HAND 改為本次觸發裝備
                            auto newSlots = charCard->GetEquipNames();
                            const std::string& oldHand =
                                newSlots[static_cast<int>(EquipSlot::HAND)];

                            // 若舊 HAND 有裝備名稱，在旁邊重新生成實體卡
                            if (!oldHand.empty()) {
                                std::uniform_real_distribution<float> dist(-80.f, 80.f);
                                SpawnCardByName(oldHand, spawnScale,
                                                spawnX + dist(m_RandomGenerator),
                                                spawnY + dist(m_RandomGenerator));
                            }
                            newSlots[static_cast<int>(EquipSlot::HAND)] = equipName;

                            // 刪除觸發裝備實體卡與舊角色卡
                            auto dragging = m_DraggingCard;
                            m_DraggingCard = nullptr;
                            RemoveCard(dragging);
                            RemoveCard(std::static_pointer_cast<Card>(charCard));

                            // 生成新職業卡並設定插槽
                            auto newCardBase = SpawnCardByName(outputName, spawnScale, spawnX, spawnY);
                            if (newCardBase && newCardBase->GetType() == CardType::CHARACTER) {
                                std::static_pointer_cast<CharacterCard>(newCardBase)
                                    ->SetEquipNames(newSlots);
                            }
                        } else {
                            // ── 非轉職：插槽已佔用則推開，否則儲存並刪除實體卡 ──
                            const std::string& existing =
                                charCard->GetEquipName(equip->GetEquipSlot());
                            if (!existing.empty()) {
                                // 插槽已有裝備，推開
                                float dx = m_DraggingCard->GetX() - charCard->GetX();
                                float dy = m_DraggingCard->GetY() - charCard->GetY();
                                float overlapX = (m_DraggingCard->GetWidth()  + charCard->GetWidth())  * 0.5f - std::abs(dx);
                                float overlapY = (m_DraggingCard->GetHeight() + charCard->GetHeight()) * 0.5f - std::abs(dy);
                                if (overlapX > 0 && overlapY > 0) {
                                    if (overlapX <= overlapY)
                                        m_DraggingCard->MoveBy({dx >= 0.f ? overlapX : -overlapX, 0.f});
                                    else
                                        m_DraggingCard->MoveBy({0.f, dy >= 0.f ? overlapY : -overlapY});
                                }
                            } else {
                                charCard->StoreEquipment(equip->GetEquipSlot(), equipName);
                                auto dragging = m_DraggingCard;
                                m_DraggingCard = nullptr;
                                RemoveCard(dragging);
                            }
                        }
                        break;
                    }

                    // ── 一般堆疊（非裝備→角色）────────────────────────
                    targetCard->SetCardAbove(m_DraggingCard);
                    m_DraggingCard->SetCardBelow(targetCard);

                    // 堆疊完成後檢查合成配方
                    {
                        auto stackBot = m_DraggingCard;
                        while (stackBot->GetCardBelow()) stackBot = stackBot->GetCardBelow();

                        std::string craftOutput = m_RecipeManager.CheckCrafting(stackBot);
                        if (!craftOutput.empty()) {
                            float spawnX     = stackBot->GetX();
                            float spawnY     = stackBot->GetY();
                            float spawnScale = stackBot->GetScale();

                            // 分類：CHARACTER / BUILDING 保留，其餘消耗
                            std::vector<std::shared_ptr<Card>> toDelete;
                            std::vector<std::shared_ptr<Card>> toKeep;
                            for (auto cur = stackBot; cur; cur = cur->GetCardAbove()) {
                                if (cur->GetType() == CardType::CHARACTER ||
                                    cur->GetType() == CardType::BUILDING)
                                    toKeep.push_back(cur);
                                else
                                    toDelete.push_back(cur);
                            }

                            // 全部先斷鏈
                            for (auto cur = stackBot; cur; ) {
                                auto next = cur->GetCardAbove();
                                cur->SetCardAbove(nullptr);
                                cur->SetCardBelow(nullptr);
                                cur = next;
                            }

                            m_DraggingCard = nullptr;
                            for (auto& c : toDelete) RemoveCard(c);
                            // toKeep 留在 m_Cards，由分離系統推開

                            SpawnCardByName(craftOutput, spawnScale, spawnX, spawnY);
                        }
                    }
                    break;
                }
            }
        }

        if (m_DraggingCard) m_DraggingCard = nullptr;
    }

    // 每幀分離重疊的非堆疊卡片 (除部分物件in m_Cards)
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

            // 沿重疊較小的軸各推開一半
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