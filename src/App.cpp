#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "BackgroundImage.hpp"
#include "Button.hpp"
#include "Core/Context.hpp"
#include "Card.hpp"
#include "CharacterCard.hpp"

void App::Start() {
    LOG_TRACE("Start");

    //===新增Menu按鈕===
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

    m_CardManager = std::make_unique<CardManager>(m_Renderer);

    m_CurrentState = State::MAIN_MENU;

    // 使用當前時間初始化隨機產生器種子

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
        m_GameFieldImage->SetScale({0.7, 0.7});
        m_Renderer.AddChild(m_GameFieldImage);

        m_DescriptionBarImage = std::make_shared<BackgroundImage>();
        auto descriptionBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/descriptionBar.png");
        m_DescriptionBarImage->SetDrawable(descriptionBar);
        m_DescriptionBarImage->SetScale({0.75, 0.75});
        m_DescriptionBarImage->m_Transform.translation = {-510, -240};
        m_DescriptionBarImage->SetZIndex(98);
        m_Renderer.AddChild(m_DescriptionBarImage);

        m_resourseBarImage = std::make_shared<BackgroundImage>();
        auto resourseBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png");
        m_resourseBarImage->SetDrawable(resourseBar);
        m_resourseBarImage->SetScale({0.55, 0.75});
        m_resourseBarImage->m_Transform.translation = {260, 335};
        m_resourseBarImage->SetZIndex(98);
        m_Renderer.AddChild(m_resourseBarImage);

        m_timeBarImage = std::make_shared<BackgroundImage>();
        auto timeBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png");
        m_timeBarImage->SetDrawable(timeBar);
        m_timeBarImage->SetScale({0.75, 0.75});
        m_timeBarImage->m_Transform.translation = {500, 335};
        m_timeBarImage->SetZIndex(98);
        m_Renderer.AddChild(m_timeBarImage);

        m_runTimeBarImage = std::make_shared<BackgroundImage>();
        auto runtimeBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/button/dark_bg.png");
        m_runTimeBarImage->SetDrawable(runtimeBar);
        m_runTimeBarImage->SetPivot({-0.5,0});
        m_runTimeBarImage->SetScale({0, 35});
        m_runTimeBarImage->m_Transform.translation = {368, 335};
        m_runTimeBarImage->SetZIndex(99);
        m_Renderer.AddChild(m_runTimeBarImage);

        m_pauseText = std::make_shared<Util::GameObject>();
        m_pauseText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 50, "暫停。", Util::Color(0, 0, 0, 50)));
        m_pauseText->SetVisible(false);
        m_pauseText->SetZIndex(100);
        m_Renderer.AddChild(m_pauseText);

        m_play = std::make_shared<MenuButton>(615, 335, 0.06, 0.06, 20, 20, "/Image/button/play.png");
        for (auto& obj : m_play->GetGameObjects()) {
            m_Renderer.AddChild(obj);
        }

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

void App::GameInit() {
    float test_scale = 0.05f;

    // 使用 Manager 統一加入卡片
    auto villager1 = std::make_shared<CharacterCard>(-150, 0, "Villager", 3, RESOURCE_DIR"/Image/card/character/Villager.png", test_scale, 1, 1);
    auto villager2 = std::make_shared<CharacterCard>(150, 0, "Militia", 5, RESOURCE_DIR"/Image/card/character/Militia.png", test_scale, 2, 1);
    auto res = std::make_shared<CharacterCard>(100, 0, "Wood", 1, RESOURCE_DIR"/Image/card/resource/Wood.png", test_scale);
    m_CardManager->AddCard(villager1);
    m_CardManager->AddCard(villager2);
    m_CardManager->AddCard(res);

    std::vector<CardSpawnData> pool;
    pool.push_back({"Baby", 5, CardType::CHARACTER, RESOURCE_DIR"/Image/card/character/Baby.png", test_scale, 2});
    pool.push_back({"Builder", 5, CardType::CHARACTER, RESOURCE_DIR"/Image/card/character/Builder.png", test_scale, 5});

    auto startPack = std::make_shared<CardPack>(0, 150, "Starter Pack", 0, RESOURCE_DIR"/Image/card/Pack.png", test_scale, 5, pool);
    m_CardManager->AddCard(startPack);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    //遊戲內邏輯 寫這裡 暫停跳出去
    mousePos = Util::Input::GetCursorPosition();

    switch (GetGameState()) {
        case GameTime::NORMAL:
            m_pauseText->SetVisible(false);
            m_runTimeBarImage->SetScale({m_runTimeBarImage->GetScaledSize().x + 0.05, 35});
            tick += 0.05;
            break;
        case GameTime::FAST:
            m_runTimeBarImage->SetScale({m_runTimeBarImage->GetScaledSize().x + 0.15, 35});
            tick += 0.15;
            break;
        case GameTime::PAUSE:
            m_pauseText->SetVisible(true);
            break;
    }

    // 1. 先確認玩家這一幀「有沒有」轉動滾輪
    if (Util::Input::IfScroll()) {
        // 2. 取得滾動的數值
        glm::vec2 scroll = Util::Input::GetScrollDistance();
        // 3. 判斷方向並執行縮放 (記得讀取的是 X 軸)
        if (scroll.y > 0) {
            // 往上滾：放大地圖
            LOG_DEBUG("往上滾！放大！");
            if (m_GameFieldImage->m_Transform.scale.x < 2.0f) {
                m_GameFieldImage->SetScale({m_GameFieldImage->m_Transform.scale.x + 0.01f,
                                            m_GameFieldImage->m_Transform.scale.y + 0.01f});
            }
        }
        else if (scroll.y < 0) {
            // 往下滾：縮小地圖
            LOG_DEBUG("往下滾！縮小！");
            if (m_GameFieldImage->m_Transform.scale.x > 0.5f) {
                m_GameFieldImage->SetScale({m_GameFieldImage->m_Transform.scale.x - 0.01f,
                                            m_GameFieldImage->m_Transform.scale.y - 0.01f});
            }
        }
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::W)) m_GameFieldImage->m_Transform.translation.y -= 1;
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) m_GameFieldImage->m_Transform.translation.y += 1;
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) m_GameFieldImage->m_Transform.translation.x += 1;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) m_GameFieldImage->m_Transform.translation.x -= 1;

    if (m_play->UpdateHover(mousePos) && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        switch (GetGameState()) {
            case GameTime::NORMAL:
                m_play->ChangeImage("/Image/button/fastforward.png");
                m_GameTime = GameTime::FAST;
                break;
            case GameTime::FAST:
                m_play->ChangeImage("/Image/button/pause.png");
                m_GameTime = GameTime::PAUSE;
                break;
            case GameTime::PAUSE:
                m_play->ChangeImage("/Image/button/play.png");
                m_GameTime = GameTime::NORMAL;
                break;
        }
    }

    // ==========================================
    // 2. 判斷滑鼠「按下左鍵」：抓起卡片
    // ==========================================

    // ===【核心瘦身】把滑鼠座標丟給管理器，讓它自己去搞定所有卡牌互動！ ===
    m_CardManager->Update(mousePos);

    m_Renderer.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}