#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "BackgroundImage.hpp"
#include "Button.hpp"
#include "Core/Context.hpp"
#include "Card.hpp"
#include"CharacterCard.hpp"

void App::Start() {
    LOG_TRACE("Start");

    //===新增遊戲開始按鈕===
    m_BtnStart = std::make_shared<MenuButton>(-562, -80, 20, 100, 50, "開始新遊戲", true);
    for (auto& obj : m_BtnStart->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }
    //===================

    auto Instance = Core::Context::GetInstance();
    float windowWidth = Instance->GetWindowWidth();
    float windowHeight = Instance->GetWindowHeight();

    m_MainMenuImage = std::make_shared<BackgroundImage>();
    float MainMenuWidth = 2560;
    float MainMenuHeight = 1600;
    float MainMenuScaleX = windowWidth / MainMenuWidth;
    float MainMenuScaleY = windowHeight / MainMenuHeight;
    m_MainMenuImage->SetScale({MainMenuScaleX, MainMenuScaleY});
    m_Renderer.AddChild(m_MainMenuImage);

    m_CurrentState = State::MAIN_MENU;
}

void App::MainMenu() {
    LOG_TRACE("MainMenu");
    m_Renderer.Update();

    mousePos = Util::Input::GetCursorPosition();
    LOG_DEBUG("{} {}", mousePos.x, mousePos.y);

    // 更新按鈕的 Hover 狀態
    bool isStartHover = m_BtnStart->UpdateHover(mousePos);

    // 如果滑鼠在按鈕上，且按下了左鍵
    if (isStartHover && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        LOG_INFO("Start Button Clicked!");

        // 1. 換成遊戲中的背景
        auto newBg = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/testBackground.png");
        m_MainMenuImage->SetDrawable(newBg);

        // 2. 隱藏主選單的 UI
        m_BtnStart->HideAll();

        // 3. 進入遊戲更新迴圈
        m_CurrentState = State::GAME_INIT;
    }

    // if (Util::Input::IsKeyUp(Util::Keycode::SPACE)) {
    //     auto newImage = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/testBackground.png");
    //     m_MainMenuImage->SetDrawable(newImage);
    //     m_CurrentState = State::GAME_INIT;
    // }
    //IF 按下遊戲開始跳到Gameinit
}

void App::GameInit() { // 遊戲初始化

    auto villager1 = std::make_shared<CharacterCard>(
        -150, 0, "Villager", 3, RESOURCE_DIR"/Image/card/character/Villager.png",0.1f);
    auto villager2 = std::make_shared<CharacterCard>(
        150, 0, "Militia", 5, RESOURCE_DIR"/Image/card/character/Militia.png", 0.1f);

    // 把卡片收編進我們的陣列裡
    m_Cards.push_back(villager1);
    m_Cards.push_back(villager2);

    // 迴圈把所有卡片的 GameObject 交給 Renderer 畫出來
    for (auto& card : m_Cards) {
        for (auto& obj : card->GetGameObjects()) {
            m_Renderer.AddChild(obj);
        }
    }

    auto newImage = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/testBackground.png");
    m_MainMenuImage->SetDrawable(newImage);

    m_CurrentState = State::UPDATE;
}

void App::Update() { //遊戲內邏輯 寫這裡 暫停跳出去
mousePos = Util::Input::GetCursorPosition();

    for (auto& card : m_Cards) {
        card->Update();
    }

    // ==========================================
    // 2. 判斷滑鼠「按下左鍵」：抓起卡片
    // ==========================================
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB) && m_DraggingCard == nullptr) {
        std::shared_ptr<Card> targetToPick = nullptr;
        int highestZ = -9999;

        // 掃描所有卡片，找出滑鼠底下「圖層最高」的那一張
        for (auto& card : m_Cards) {
            if (card->IsMouseHovering(mousePos)) {
                // 透過 GetGameObjects() 取得卡牌底圖目前的 Z-Index
                int currentZ = card->GetGameObjects()[0]->GetZIndex();
                if (currentZ > highestZ) {
                    highestZ = currentZ;
                    targetToPick = card; // 鎖定最高層的這張卡
                }
            }
        }

        // 如果有抓到卡片
        if (targetToPick != nullptr) {
            m_DraggingCard = targetToPick;

            // 【斷開連結】如果這張卡原本疊在別人上面，拿起來時要分手
            if (m_DraggingCard->GetCardBelow() != nullptr) {
                m_DraggingCard->GetCardBelow()->SetCardAbove(nullptr);
                m_DraggingCard->SetCardBelow(nullptr);
            }

            m_DraggingCard->StartDragging(mousePos);
        }
    }

    // ==========================================
    // 3. 判斷滑鼠「放開左鍵」：放下並嘗試堆疊
    // ==========================================
    if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_DraggingCard != nullptr) {
        m_DraggingCard->StopDragging();

        // 尋找有沒有疊到其他卡片上
        for (auto it = m_Cards.rbegin(); it != m_Cards.rend(); ++it) {
            auto targetCard = *it;

            // 確保不是自己，且兩張卡有重疊
            if (targetCard != m_DraggingCard && m_DraggingCard->IsOverlapping(targetCard)) {

                // 【新增防呆】檢查 targetCard 是不是已經疊在我自己身上了？
                // 如果目標卡片是跟著我一起被拿起來的小弟，就不可以疊上去，否則會形成無限死結！
                bool isSelfStack = false;
                auto checkCard = m_DraggingCard->GetCardAbove();
                while (checkCard != nullptr) {
                    if (checkCard == targetCard) {
                        isSelfStack = true;
                        break;
                    }
                    checkCard = checkCard->GetCardAbove();
                }

                if (isSelfStack) {
                    continue; // 這是自己人，跳過這張，繼續檢查下一張有沒有重疊
                }

                // 【關鍵】尋找該牌堆的「最頂層」卡片！
                while (targetCard->GetCardAbove() != nullptr) {
                    targetCard = targetCard->GetCardAbove();
                }

                // 互相綁定！
                targetCard->SetCardAbove(m_DraggingCard);
                m_DraggingCard->SetCardBelow(targetCard);

                LOG_INFO("卡牌堆疊成功！");
                break; // 疊到一張就夠了
            }
        }

        m_DraggingCard = nullptr;
    }

    m_Renderer.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}
