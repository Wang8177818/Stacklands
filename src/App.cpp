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
        m_CurrentState = State::UPDATE;
    }

    if (Util::Input::IsKeyUp(Util::Keycode::SPACE)) {
        auto newImage = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/testBackground.png");
        m_MainMenuImage->SetDrawable(newImage);
        m_CurrentState = State::UPDATE;
    }
    //IF 按下遊戲開始跳到Update
}

void App::Update() {
    m_Renderer.Update();
    
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
