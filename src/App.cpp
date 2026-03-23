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

    bool isStartHover = m_BtnStart->UpdateHover(mousePos);
    bool isExitHover = m_BtnExit->UpdateHover(mousePos);
    bool isOptionsHover = m_BtnOptions->UpdateHover(mousePos);
    bool isCardWikiHover = m_BtnCardWiki->UpdateHover(mousePos);
    bool isModsHover = m_BtnMods->UpdateHover(mousePos);

    if (isStartHover && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        LOG_INFO("Start Button Clicked!");

        auto greenBG = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/greenBG.png");
        m_MainMenuBG->SetDrawable(greenBG);

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
        m_DescriptionBarImage->SetZIndex(99);
        m_Renderer.AddChild(m_DescriptionBarImage);

        m_resourseBarImage = std::make_shared<BackgroundImage>();
        auto resourseBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png");
        m_resourseBarImage->SetDrawable(resourseBar);
        m_resourseBarImage->SetScale({0.55, 0.75});
        m_resourseBarImage->m_Transform.translation = {260, 335};
        m_resourseBarImage->SetZIndex(99);
        m_Renderer.AddChild(m_resourseBarImage);

        m_timeBarImage = std::make_shared<BackgroundImage>();
        auto timeBar = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png");
        m_timeBarImage->SetDrawable(timeBar);
        m_timeBarImage->SetScale({0.75, 0.75});
        m_timeBarImage->m_Transform.translation = {500, 335};
        m_timeBarImage->SetZIndex(99);
        m_Renderer.AddChild(m_timeBarImage);

        m_play = std::make_shared<MenuButton>(615, 335, 0.06, 0.06, 20, 20, "/Image/button/play.png");
        for (auto& obj : m_play->GetGameObjects()) {
            m_Renderer.AddChild(obj);
        }

        m_BtnStart->HideAll();
        m_BtnExit->HideAll();
        m_BtnOptions->HideAll();
        m_BtnMods->HideAll();
        m_BtnCardWiki->HideAll();
        m_MainMenuImage->SetVisible(false);

        m_CurrentState = State::GAME_INIT;
    }

    if (isExitHover && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        LOG_INFO("Exit Button Clicked!");
        m_CurrentState = State::END;
    }
}

void App::GameInit() {
    float test_scale = 0.2f;

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
    mousePos = Util::Input::GetCursorPosition();

    // --- 攝影機控制 ---
    if (Util::Input::IfScroll()) {
        glm::vec2 scroll = Util::Input::GetScrollDistance();
        if (scroll.y > 0 && m_GameFieldImage->m_Transform.scale.x < 2.0f) {
            m_GameFieldImage->SetScale({m_GameFieldImage->m_Transform.scale.x + 0.05f, m_GameFieldImage->m_Transform.scale.y + 0.05f});
        }
        else if (scroll.y < 0 && m_GameFieldImage->m_Transform.scale.x > 0.5f) {
            m_GameFieldImage->SetScale({m_GameFieldImage->m_Transform.scale.x - 0.05f, m_GameFieldImage->m_Transform.scale.y - 0.05f});
        }
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::W)) m_GameFieldImage->m_Transform.translation.y -= 1;
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) m_GameFieldImage->m_Transform.translation.y += 1;
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) m_GameFieldImage->m_Transform.translation.x += 1;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) m_GameFieldImage->m_Transform.translation.x -= 1;

    if (m_play->UpdateHover(mousePos) && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        // 時間切換邏輯...
    }

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