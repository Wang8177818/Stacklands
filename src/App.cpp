#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "CharacterCard.hpp"
#include "Sellslot.hpp"

// ─────────────────────────────────────────────────────────────
void App::Start() {
    LOG_TRACE("Start");

    m_UIManager    = std::make_unique<UIManager>(m_Renderer);
    m_CardManager  = std::make_unique<CardManager>(m_Renderer);
    m_EventManager = std::make_unique<EventManager>();
    m_SellSlot     = std::make_shared<SellSlot>(-250, 210);

    m_UIManager->InitMenu();

    m_CurrentState = State::MAIN_MENU;
}

// ─────────────────────────────────────────────────────────────
void App::MainMenu() {
    LOG_TRACE("MainMenu");

    mousePos = Util::Input::GetCursorPosition();
    LOG_DEBUG("{} {}", mousePos.x, mousePos.y);

    auto event = m_UIManager->UpdateMenu(mousePos);

    switch (event) {
        case UIManager::MenuEvent::START_GAME:
            LOG_INFO("Start Button Clicked!");
            m_UIManager->TransitionToGame();
            // 遊戲 UI 建好後，把 GameField 與 UIManager 交給 EventManager
            m_EventManager->SetGameField(m_UIManager->GetGameFieldImage());
            m_EventManager->SetUIManager(m_UIManager.get());
            m_CurrentState = State::GAME_INIT;
            break;

        case UIManager::MenuEvent::EXIT:
            LOG_INFO("Exit Button Clicked!");
            m_CurrentState = State::END;
            break;

        default:
            break;
    }

    m_Renderer.Update();
}

// ─────────────────────────────────────────────────────────────
void App::GameInit() {
    //卡牌大小去App.hpp調basic_scale
    m_CardManager->AddCard(m_SellSlot);
    m_Renderer.Update();
    // 讀 json
    m_CardManager->LoadCardDatabase(RESOURCE_DIR"/Data/Cards.json");
    m_CardManager->LoadPackDatabase(RESOURCE_DIR"/Data/Packs.json");
    m_CardManager->LoadProfessionRecipes(RESOURCE_DIR"/Data/Profession.json");
    m_CardManager->LoadCraftingRecipes(RESOURCE_DIR"/Data/Recipe.json");

    m_CardManager->SpawnCardByName("Villager", basic_scale);
    m_CardManager->SpawnCardByName("Chicken",basic_scale);

    m_CardManager->SpawnPackByName("A New World", basic_scale);
    m_CurrentState = State::UPDATE;
}

// ─────────────────────────────────────────────────────────────
void App::Update() {
    mousePos = Util::Input::GetCursorPosition();

    // ── 鏡頭移動 & 地圖縮放（完全委託給 EventManager）────
    auto cards = m_CardManager->GetAllCards();
    m_EventManager->Update(mousePos, m_CardManager->isDraggingCard(), cards);

    // ── 卡片更新 ──────────────────────────────────────────
    m_CardManager->SetZoomRatio(m_EventManager->GetZoomRatio());
    switch (m_EventManager->GetGameState()) {
        case EventManager::GameTime::PAUSE:  m_CardManager->SetTimeScale(0.0f); break;
        case EventManager::GameTime::FAST:   m_CardManager->SetTimeScale(2.0f); break;
        case EventManager::GameTime::NORMAL:
        default:                             m_CardManager->SetTimeScale(1.0f); break;
    }
    m_CardManager->Update(mousePos);

    // ── 卡片數量 UI 更新 ─────────────────────────────────
    m_UIManager->UpdateCardCount(m_CardManager->GetCardCount(),m_CardManager->GetMaxCardCount());

    // ── 金幣數量 UI 更新 ─────────────────────────────────
    m_UIManager->UpdateCoinCount(m_CardManager->GetCoinCount());

    // ── 食物數量 UI 更新 ─────────────────────────────────
    m_UIManager->UpdateFood(m_CardManager->GetTotalFoodSupply(),m_CardManager->GetNeededFoodCount());

    // ── 懸停卡片敘述更新 ─────────────────────────────────
    // 暫停時不顯示懸停敘述（敘述欄底圖也已經被隱藏）
    if (m_EventManager->GetGameState() == EventManager::GameTime::PAUSE) {
        m_UIManager->UpdateDescriptionName("");
        m_UIManager->UpdateDescriptionText("");
    } else {
        std::shared_ptr<Card> hoveredCard = nullptr;
        int highestZ = -9999;
        for (auto& card : m_CardManager->GetAllCards()) {
            if (card->GetType() == CardType::INTERACT) continue;
            if (card->IsMouseHovering(mousePos)) {
                int z = card->GetGameObjects()[0]->GetZIndex();
                if (z > highestZ) {
                    highestZ = z;
                    hoveredCard = card;
                }
            }
        }
        m_UIManager->UpdateDescriptionName(
            hoveredCard ? hoveredCard->GetName() : "");
        m_UIManager->UpdateDescriptionText(
            hoveredCard ? hoveredCard->GetDescription() : "");
    }

    int sellPrice = m_SellSlot->GetTotalPrice();
    if (sellPrice > 0) {
        float spawnScale = basic_scale * m_EventManager->GetZoomRatio();
        // 生成一Coin作為堆疊底部
        auto topCoin = m_CardManager->SpawnCardByName("Coin", spawnScale);
        for (int i = 1; i < sellPrice; i++) {
            auto newCoin = m_CardManager->SpawnCardByName("Coin", spawnScale);
            topCoin->SetCardAbove(newCoin);
            newCoin->SetCardBelow(topCoin);
            topCoin = newCoin;
        }
        auto soldCards = m_SellSlot->PopAllCards();
        for (auto& card : soldCards) {
            card->OnSold();
            m_CardManager->RemoveCard(card);
        }
    }

    m_Renderer.Update();


    if (m_EventManager->IsRequestingExit() || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// ─────────────────────────────────────────────────────────────
void App::End() {
    LOG_TRACE("End");
}
