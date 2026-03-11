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

    auto test = std::make_shared<Util::GameObject>();
    auto testbutton = std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/btnSeleted.png");
    test->SetDrawable(testbutton);

    mousePos = Util::Input::GetCursorPosition();
    //LOG_DEBUG("{} {}", mousePos.x, mousePos.y);

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
