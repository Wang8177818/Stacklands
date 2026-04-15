//
// Created by user on 2026/3/27.
//

#include "EventManager.hpp"

#include "../PTSD/example/include/App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

// ─────────────────────────────────────────────────────────────
void EventManager::SetGameField(std::shared_ptr<BackgroundImage> gameField) {
    m_GameField = gameField;
}

// ─────────────────────────────────────────────────────────────
void EventManager::Update(glm::vec2 mousePos,
                           bool isDraggingCard,
                           std::vector<std::shared_ptr<Card>>& cards) {
    if (!m_GameField) return;

    HandlePan(mousePos, isDraggingCard, cards);
    HandleZoom(cards);
    HandleWASD(cards);
    SwitchGameState();
    ESCMenu();
}

// ─────────────────────────────────────────────────────────────
void EventManager::HandlePan(glm::vec2 mousePos,
                               bool isDraggingCard,
                               std::vector<std::shared_ptr<Card>>& cards) {
    if (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB) && !isDraggingCard) {
        if (!m_IsDraggingMap) {
            m_IsDraggingMap = true;
            m_LastMousePos  = mousePos;
        } else {
            glm::vec2 delta = mousePos - m_LastMousePos;
            if (delta.x != 0.f || delta.y != 0.f) {
                MoveAll(delta, cards);
            }
            m_LastMousePos = mousePos;
        }
    } else {
        m_IsDraggingMap = false;
    }
}

// ─────────────────────────────────────────────────────────────
// 滾輪縮放
// 以畫面中心 (0,0) 為錨點，讓背景圖與卡片的邏輯座標同步等比縮放。
// 卡片透過 ScaleAroundPivot() 同時修改 m_X, m_Y 與 m_Scale，
// 確保 UpdateVisualPositions() 每幀覆寫時位置仍然正確。
void EventManager::HandleZoom(std::vector<std::shared_ptr<Card>>& cards) {
    if (!Util::Input::IfScroll()) return;

    glm::vec2 scroll   = Util::Input::GetScrollDistance();
    float     oldScale = m_GameField->m_Transform.scale.x;
    float     newScale = oldScale;

    if      (scroll.y > 0 && oldScale < ZOOM_MAX) newScale = oldScale + ZOOM_STEP;
    else if (scroll.y < 0 && oldScale > ZOOM_MIN) newScale = oldScale - ZOOM_STEP;
    else return;

    const float     ratio = newScale / oldScale;
    const glm::vec2 pivot = {0.f, 0.f}; // 畫面中心為縮放錨點

    // 背景圖：縮放 + 錨點等比位移
    m_GameField->SetScale({newScale, newScale});
    m_GameField->m_Transform.translation =
        pivot + (m_GameField->m_Transform.translation - pivot) * ratio;

    // 卡片：透過邏輯座標介面同步，避免被 UpdateVisualPositions() 覆蓋
    for (auto& card : cards) {
        card->ScaleAroundPivot(ratio, pivot);
    }

    // 記錄累積縮放倍率 讓後續生成的新卡片能以正確大小出現
    m_ZoomRatio *= ratio;

    LOG_DEBUG("Zoom {} -> {:.3f}", (scroll.y > 0 ? "in " : "out"), newScale);
}

// ─────────────────────────────────────────────────────────────
void EventManager::HandleWASD(std::vector<std::shared_ptr<Card>>& cards) {
    glm::vec2 delta = {0.f, 0.f};

    if (Util::Input::IsKeyPressed(Util::Keycode::W)) delta.y -= WASD_SPEED;
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) delta.y += WASD_SPEED;
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) delta.x += WASD_SPEED;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) delta.x -= WASD_SPEED;

    if (delta.x != 0.f || delta.y != 0.f) {
        MoveAll(delta, cards);
    }
}

// ─────────────────────────────────────────────────────────────
// 同時移動背景圖與所有卡片（Pan / WASD 共用）
// 卡片透過 MoveBy() 修改 m_X, m_Y，而非直接動 GameObject translation。
void EventManager::MoveAll(glm::vec2 delta, std::vector<std::shared_ptr<Card>>& cards) {
    m_GameField->m_Transform.translation += delta;

    for (auto& card : cards) {
        card->MoveBy(delta);
    }
}

void EventManager::SwitchGameState() {
    if (!m_UIManager) return;

    auto mousePos = Util::Input::GetCursorPosition();

    auto runTimeBar = m_UIManager->GetRunTimeBar();
    auto pauseText  = m_UIManager->GetPauseText();
    auto playButton = m_UIManager->GetPlayButton();

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

    if (tick < 120000) {
        switch (GetGameState()) {
        case GameTime::NORMAL:
            pauseText->SetVisible(false);
            runTimeBar->SetScale({tick * 265/120000, 35.f});
            tick += Util::Time::GetDeltaTimeMs();

            break;
        case GameTime::FAST:
            runTimeBar->SetScale({tick * 265/120000, 35.f});
            tick += Util::Time::GetDeltaTimeMs()*2;
            break;
        case GameTime::PAUSE:
            if (!is_Pausing) {
                pauseText->SetVisible(true);
            }
            break;
        }
    }

}

void EventManager::ESCMenu() {
    auto mousePos = Util::Input::GetCursorPosition();
    auto PauseMenu = m_UIManager->GetPauseMenu();
    auto DescriptionBar = m_UIManager->GetDescriptionBar();
    auto runTimeBar = m_UIManager->GetRunTimeBar();
    auto playButton = m_UIManager->GetPlayButton();
    auto resourseBar = m_UIManager->GetResourseBar();
    auto timeBar = m_UIManager->GetTimeBar();
    auto continueButton = m_UIManager->GetContinueButton();
    auto returnToMenuButton = m_UIManager->GetReturnToMenuButton();
    auto month = m_UIManager->GetMonth();

    // 進入暫停選單後開始更新按鈕
    if (is_Pausing) {
        auto event = m_UIManager->UpdatePauseMenu(mousePos);

        switch (event) {
            case UIManager::MenuEvent::CONTINUE:
                m_GameTime = GameTime::NORMAL;
                is_Pausing = false;

                PauseMenu->SetVisible(false);
                continueButton->HideAll();

                DescriptionBar->SetVisible(true);
                runTimeBar->SetVisible(true);
                resourseBar->SetVisible(true);
                timeBar->SetVisible(true);
                playButton->ShowAll();
                break;
            case UIManager::MenuEvent::BACKTOMENU:
                m_RequestExit = true;
                break;
        }
    }

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        if (!is_Pausing) {
            m_GameTime = GameTime::PAUSE;
            is_Pausing = true;

            PauseMenu->SetVisible(true);
            continueButton->ShowAll();
            returnToMenuButton->ShowAll();

            DescriptionBar->SetVisible(false);
            runTimeBar->SetVisible(false);
            resourseBar->SetVisible(false);
            timeBar->SetVisible(false);
            month->SetVisible(false);
            playButton->HideAll();
        } else {
            m_GameTime = GameTime::NORMAL;
            is_Pausing = false;

            PauseMenu->SetVisible(false);
            continueButton->HideAll();
            returnToMenuButton->HideAll();

            DescriptionBar->SetVisible(true);
            runTimeBar->SetVisible(true);
            resourseBar->SetVisible(true);
            timeBar->SetVisible(true);
            month->SetVisible(true);
            playButton->ShowAll();
        }
    }
}