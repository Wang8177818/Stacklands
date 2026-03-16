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

    //===新增Menu按鈕=== aaa
    m_BtnStart = std::make_shared<MenuButton>(-562, -80, 20, 100, 20, "開始新遊戲", true, 5);
    for (auto& obj : m_BtnStart->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }

    m_BtnExit = std::make_shared<MenuButton>(-572, -315, 20, 80, 20, "離開遊戲", true, 4);
    for (auto& obj : m_BtnExit->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }

    m_BtnOptions = std::make_shared<MenuButton>(-592, -200, 20, 40, 20, "選項", true, 2);
    for (auto& obj : m_BtnOptions->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }

    m_BtnCardWiki = std::make_shared<MenuButton>(-572, -160, 20, 80, 20, "卡片百科", true, 4);
    for (auto& obj : m_BtnCardWiki->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }

    m_BtnMods = std::make_shared<MenuButton>(-592, -240, 20, 40, 20, "模組", true, 2);
    for (auto& obj : m_BtnMods->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }
    //===================

    //===選單背景===
    auto Instance = Core::Context::GetInstance();
    float windowWidth = Instance->GetWindowWidth();
    float windowHeight = Instance->GetWindowHeight();

    m_MainMenuBG = std::make_shared<BackgroundImage>();
    float MainMenuWidth = 1150;
    float MainMenuHeight = 720;
    float MainMenuScaleX = windowWidth / MainMenuWidth;
    float MainMenuScaleY = windowHeight / MainMenuHeight;
    m_MainMenuBG->SetScale({MainMenuScaleX, MainMenuScaleY});
    m_Renderer.AddChild(m_MainMenuBG);
    //======

    m_MainMenuImage = std::make_shared<BackgroundImage>();
    m_MainMenuImage->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/stacklandsMenuMain.png"));
    m_MainMenuImage->m_Transform.translation = glm::vec2(-470,-110);
    m_MainMenuImage->m_Transform.scale = {0.5, 0.5};
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
    bool isExitHover = m_BtnExit->UpdateHover(mousePos);
    bool isOptionsHover = m_BtnOptions->UpdateHover(mousePos);
    bool isCardWikiHover = m_BtnCardWiki->UpdateHover(mousePos);
    bool isModsHover = m_BtnMods->UpdateHover(mousePos);

    // 如果滑鼠在按鈕上，且按下了左鍵
    if (isStartHover && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        LOG_INFO("Start Button Clicked!");

        // 1. 換成遊戲中的背景
        auto greenBG = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/greenBG.png");
        m_MainMenuBG->SetDrawable(greenBG);

        /*
        m_GameWhiteBG = std::make_shared<BackgroundImage>();
        auto whiteBG = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/whiteBG.png");
        m_GameWhiteBG->SetScale({5, 3});
        m_GameWhiteBG->SetZIndex(1);
        m_GameWhiteBG->SetDrawable(whiteBG);
        m_Renderer.AddChild(m_GameWhiteBG);

        m_GameBlackBG = std::make_shared<BackgroundImage>();
        auto blackBG = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/blackBG.png");
        m_GameBlackBG->SetScale({4.95, 2.95});
        m_GameBlackBG->SetZIndex(2);
        m_GameBlackBG->SetDrawable(blackBG);
        m_Renderer.AddChild(m_GameBlackBG);

        m_GameLightGreenBG = std::make_shared<BackgroundImage>();
        auto lightGreenBG = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/lightgreen.png");
        m_GameLightGreenBG->SetScale({4.9, 2.33});
        m_GameLightGreenBG->m_Transform.translation = {0,-37};
        m_GameLightGreenBG->SetZIndex(3);
        m_GameLightGreenBG->SetDrawable(lightGreenBG);
        m_Renderer.AddChild(m_GameLightGreenBG);

        m_GameDarkGreenBG = std::make_shared<BackgroundImage>();
        auto darkGreenBG = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/darkgreen.png");
        m_GameDarkGreenBG->SetScale({4.9, 0.55});
        m_GameDarkGreenBG->m_Transform.translation = {0,150};
        m_GameDarkGreenBG->SetZIndex(3);
        m_GameDarkGreenBG->SetDrawable(darkGreenBG);
        m_Renderer.AddChild(m_GameDarkGreenBG);
        */
        m_GameFieldImage = std::make_shared<BackgroundImage>();
        auto gamefieldImage = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/gameField.png");
        m_GameFieldImage->SetDrawable(gamefieldImage);
        m_GameFieldImage->SetScale({0.5, 0.5});
        m_Renderer.AddChild(m_GameFieldImage);

        m_DescriptionBarImage = std::make_shared<BackgroundImage>();
        auto descriptionBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/descriptionBar.png");
        m_DescriptionBarImage->SetDrawable(descriptionBar);
        m_DescriptionBarImage->SetScale({0.75, 0.75});
        m_DescriptionBarImage->m_Transform.translation = {-510, -240};
        m_Renderer.AddChild(m_DescriptionBarImage);

        m_resourseBarImage = std::make_shared<BackgroundImage>();
        auto resourseBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png");
        m_resourseBarImage->SetDrawable(resourseBar);
        m_resourseBarImage->SetScale({0.75, 0.75});
        m_resourseBarImage->m_Transform.translation = {510, 240};
        m_Renderer.AddChild(m_resourseBarImage);

        // 2. 隱藏主選單的 UI
        m_BtnStart->HideAll();
        m_BtnExit->HideAll();
        m_BtnOptions->HideAll();
        m_BtnMods->HideAll();
        m_BtnCardWiki->HideAll();
        m_MainMenuImage->SetVisible(false);

        // 3. 進入遊戲更新迴圈
        m_CurrentState = State::GAME_INIT;
    }

    if (isExitHover && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        LOG_INFO("Exit Button Clicked!");
        m_CurrentState = State::END;
    }
}

void App::Update() {
    m_Renderer.Update();

    auto scroll = Util::Input::GetScrollDistance();
    if (Util::Input::IfScroll()) {
        if (scroll.x > 0 && m_GameFieldImage->m_Transform.scale.x < 2) {
            m_GameFieldImage->SetScale({m_GameFieldImage.get()->m_Transform.scale.x + 0.005, m_GameFieldImage.get()->m_Transform.scale.y + 0.005});
        }
        if (scroll.x < 0 && m_GameFieldImage.get()->m_Transform.scale.x > 0.5) {
            m_GameFieldImage->SetScale({m_GameFieldImage.get()->m_Transform.scale.x - 0.005, m_GameFieldImage.get()->m_Transform.scale.y - 0.005});
        }
    }

    if (Util::Input::IsKeyPressed(Util::Keycode::W)) {
        m_GameFieldImage->m_Transform.translation = {m_GameFieldImage.get()->m_Transform.translation.x, m_GameFieldImage.get()->m_Transform.translation.y - 1};
    }

    /*
     * Do not touch the code below as they serve the purpose for
     * closing the window.
     */
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
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
