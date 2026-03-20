#ifndef APP_HPP
#define APP_HPP

#include "BackgroundImage.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "Util/Input.hpp"
#include "Util/Renderer.hpp" // 新增：引入渲染器 (畫布)
#include "Button.hpp"

class App {
public:
    enum class State {
        START,
        MAIN_MENU,
        UPDATE,
        END,
    };

    enum class GameTime {
        NORMAL,
        FAST,
        PAUSE,
    };

    State GetCurrentState() const { return m_CurrentState; }

    GameTime GetGameState() const { return m_GameTime; }

    void Start();

    void MainMenu();

    void Update();

    void End(); // NOLINT(readability-convert-member-functions-to-static)

    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    //鼠標位置

private:
    void ValidTask();
    State m_CurrentState = State::START;
    GameTime m_GameTime = GameTime::NORMAL;
    Util::Renderer m_Renderer;

    std::shared_ptr<BackgroundImage> m_MainMenuBG;
    /*
    std::shared_ptr<BackgroundImage> m_GameWhiteBG;
    std::shared_ptr<BackgroundImage> m_GameBlackBG;
    std::shared_ptr<BackgroundImage> m_GameLightGreenBG;
    std::shared_ptr<BackgroundImage> m_GameDarkGreenBG;
    */
    std::shared_ptr<BackgroundImage> m_GameFieldImage;
    std::shared_ptr<BackgroundImage> m_MainMenuImage;
    std::shared_ptr<BackgroundImage> m_DescriptionBarImage;
    std::shared_ptr<BackgroundImage> m_resourseBarImage;
    std::shared_ptr<BackgroundImage> m_timeBarImage;

    std::shared_ptr<MenuButton> m_play;
    std::shared_ptr<MenuButton> m_pause;
    std::shared_ptr<MenuButton> m_fastplay;

    std::shared_ptr<MenuButton> m_BtnStart;
    std::shared_ptr<MenuButton> m_BtnExit;
    std::shared_ptr<MenuButton> m_BtnOptions;
    std::shared_ptr<MenuButton> m_BtnCardWiki;
    std::shared_ptr<MenuButton> m_BtnMods;

    float zoomSize = 0;
};

#endif
