#ifndef APP_HPP
#define APP_HPP

#include "BackgroundImage.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "Util/Input.hpp"
#include "Util/Renderer.hpp" // 新增：引入渲染器 (畫布)
#include "Button.hpp"
#include "Card.hpp"
#include <random>
#include "Card.hpp"
#include "CardPack.hpp"
#include "CardManager.hpp"

class App {
public:
    enum class State {
        START,
        MAIN_MENU,
        GAME_INIT,
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

    void GameInit();

    void End(); // NOLINT(readability-convert-member-functions-to-static)

    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    //鼠標位置
    float tick = 0;
    float month = 1;
    //遊戲內時間
private:
    void ValidTask();
    State m_CurrentState = State::START;
    GameTime m_GameTime = GameTime::NORMAL;
    Util::Renderer m_Renderer;

    std::shared_ptr<BackgroundImage> m_MainMenuBG;
    std::shared_ptr<BackgroundImage> m_GameFieldImage;
    std::shared_ptr<BackgroundImage> m_MainMenuImage;
    std::shared_ptr<BackgroundImage> m_DescriptionBarImage;
    std::shared_ptr<BackgroundImage> m_resourseBarImage;
    std::shared_ptr<BackgroundImage> m_timeBarImage;
    std::shared_ptr<BackgroundImage> m_runTimeBarImage;

    std::shared_ptr<MenuButton> m_play;

    std::shared_ptr<MenuButton> m_BtnStart;
    std::shared_ptr<MenuButton> m_BtnExit;
    std::shared_ptr<MenuButton> m_BtnOptions;
    std::shared_ptr<MenuButton> m_BtnCardWiki;
    std::shared_ptr<MenuButton> m_BtnMods;

    std::shared_ptr<Util::GameObject> m_pauseText;

    //===滑鼠移動鏡頭===
    bool m_IsDragging = false;           // 記錄是否正在拖曳
    glm::vec2 m_LastMousePos = {0, 0};   // 記錄上一幀的滑鼠位置
    //================

    // ===【核心瘦身】這行取代了原本一大串的卡片管理變數！===
    std::unique_ptr<CardManager> m_CardManager;
};


#endif
