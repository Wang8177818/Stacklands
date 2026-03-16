#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "BackgroundImage.hpp"
#include "Button.hpp"
#include "Core/Context.hpp"

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
        m_CurrentState = State::UPDATE;
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
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}
