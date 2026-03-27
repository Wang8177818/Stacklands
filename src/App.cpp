#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "CharacterCard.hpp"
#include "ResourceCard.hpp"

// ─────────────────────────────────────────────────────────────
void App::Start() {
    LOG_TRACE("Start");

    m_UIManager    = std::make_unique<UIManager>(m_Renderer);
    m_CardManager  = std::make_unique<CardManager>(m_Renderer);
    m_EventManager = std::make_unique<EventManager>();

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
            // 遊戲 UI 建好後，把 GameField 交給 EventManager
            m_EventManager->SetGameField(m_UIManager->GetGameFieldImage());
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
    float test_scale = 0.15f;

    // 讀 json
    m_CardManager->LoadCardDatabase(RESOURCE_DIR"/Data/Cards.json");
    m_CardManager->LoadPackDatabase(RESOURCE_DIR"/Data/Packs.json");

    m_CardManager->SpawnCardByName("Villager", test_scale);
    m_CardManager->SpawnCardByName("Militia",  test_scale);
    m_CardManager->SpawnCardByName("Wood",     test_scale);
    m_CardManager->SpawnCardByName("Coin",     test_scale);

    m_CardManager->SpawnPackByName("A New World", test_scale);

    m_CurrentState = State::UPDATE;
}

// ─────────────────────────────────────────────────────────────
void App::Update() {
    mousePos = Util::Input::GetCursorPosition();

    auto runTimeBar = m_UIManager->GetRunTimeBar();
    auto pauseText  = m_UIManager->GetPauseText();
    auto playButton = m_UIManager->GetPlayButton();

    // ── 鏡頭移動 & 地圖縮放（完全委託給 EventManager）────
    auto cards = m_CardManager->getAllCards();
    m_EventManager->Update(mousePos, m_CardManager->isDraggingCard(), cards);

    // ── 時間推進 ──────────────────────────────────────────
    switch (GetGameState()) {
        case GameTime::NORMAL:
            pauseText->SetVisible(false);
            runTimeBar->SetScale({runTimeBar->GetScaledSize().x + 0.05f, 35.f});
            tick += 0.05f;
            break;
        case GameTime::FAST:
            runTimeBar->SetScale({runTimeBar->GetScaledSize().x + 0.15f, 35.f});
            tick += 0.15f;
            break;
        case GameTime::PAUSE:
            pauseText->SetVisible(true);
            break;
    }

    // ── 播放/快進/暫停按鈕 ────────────────────────────────
    if (playButton->UpdateHover(mousePos) && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        switch (GetGameState()) {
            case GameTime::NORMAL:
                playButton->ChangeImage("/Image/button/fastforward.png");
                m_GameTime = GameTime::FAST;
                break;
            case GameTime::FAST:
                playButton->ChangeImage("/Image/button/pause.png");
                m_GameTime = GameTime::PAUSE;
                break;
            case GameTime::PAUSE:
                playButton->ChangeImage("/Image/button/play.png");
                m_GameTime = GameTime::NORMAL;
                break;
        }
    }

    // ── 卡片更新 ──────────────────────────────────────────
    m_CardManager->Update(mousePos);

    m_Renderer.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// ─────────────────────────────────────────────────────────────
void App::End() {
    LOG_TRACE("End");
}
