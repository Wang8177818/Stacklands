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

    m_CardManager->SpawnCardByName("Villager", basic_scale);
    m_CardManager->SpawnCardByName("Militia",  basic_scale);
    m_CardManager->SpawnCardByName("Wood",     basic_scale);
    m_CardManager->SpawnCardByName("Coin",     basic_scale);

    m_CardManager->SpawnPackByName("A New World", basic_scale);
    m_CardManager->SpawnPackByName("Seeking Wisdom", basic_scale);
    m_CurrentState = State::UPDATE;
}

// ─────────────────────────────────────────────────────────────
void App::Update() {
    mousePos = Util::Input::GetCursorPosition();

    // ── 鏡頭移動 & 地圖縮放（完全委託給 EventManager）────
    auto cards = m_CardManager->GetAllCards();
    m_EventManager->Update(mousePos, m_CardManager->isDraggingCard(), cards);

    // ── 卡片更新 ──────────────────────────────────────────
    m_CardManager->Update(mousePos);
    int sellPrice = m_SellSlot->GetTotalPrice();
    if (sellPrice > 0) {
        // 生成第一枚 Coin 作為堆疊底部
        auto topCoin = m_CardManager->SpawnCardByName("Coin", basic_scale);
        for (int i = 1; i < sellPrice; i++) {
            auto newCoin = m_CardManager->SpawnCardByName("Coin", basic_scale);
            topCoin->SetCardAbove(newCoin);
            newCoin->SetCardBelow(topCoin);
            topCoin = newCoin;
        }
        auto soldCards = m_SellSlot->PopAllCards();
        for (auto& card : soldCards) {
            m_CardManager->RemoveCard(card);
        }
    }

    m_Renderer.Update();

    /*
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
    */
}

// ─────────────────────────────────────────────────────────────
void App::End() {
    LOG_TRACE("End");
}
