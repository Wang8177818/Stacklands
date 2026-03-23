//
// Created by m0938 on 2026/3/20.
//
#include "CardManager.hpp"
#include "CharacterCard.hpp"
#include "CardPack.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include <algorithm>

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

std::shared_ptr<Card> CardManager::CreateCardFromData(float x, float y, const CardSpawnData& data) {
    std::shared_ptr<Card> newCard = nullptr;

    if (data.type == CardType::CHARACTER) {
        newCard = std::make_shared<CharacterCard>(x, y, data.name, data.sellValue, data.iconPath, data.scale, data.health, data.attack);
    }else if (data.type == CardType::RESOURCE){
        std::make_shared<CharacterCard>(x, y, data.name, data.sellValue, data.iconPath, data.scale);
    }else {
        newCard = std::make_shared<Card>(x, y, data.name, data.sellValue, data.type, data.scale);
    }

    // 建立完成後自動加入管理陣列並渲染
    if (newCard) {
        AddCard(newCard);
    }
    return newCard;
}

void CardManager::Update(glm::vec2 mousePos) {
    // 1. 清理空卡包
    m_Cards.erase(std::remove_if(m_Cards.begin(), m_Cards.end(),
        [](const std::shared_ptr<Card>& card) {
            if (card->GetType() == CardType::PACK) {
                return std::static_pointer_cast<CardPack>(card)->IsEmpty();
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

                            // 直接呼叫工廠，它內部已經寫好 AddCard 自動加入了！
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

                    if (targetCard->OnStacked(m_DraggingCard) == false) {
                        break;
                    }

                    targetCard->SetCardAbove(m_DraggingCard);
                    m_DraggingCard->SetCardBelow(targetCard);
                    break;
                }
            }
        }

        m_DraggingCard = nullptr;
    }
}