#ifndef APP_HPP
#define APP_HPP

#include "BackgroundImage.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "Util/Input.hpp"
#include "Util/Renderer.hpp" // 新增：引入渲染器 (畫布)
#include "Button.hpp"
#include "Card.hpp"

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
    std::shared_ptr<BackgroundImage> m_runTimeBarImage;

    std::shared_ptr<MenuButton> m_play;

    std::shared_ptr<MenuButton> m_BtnStart;
    std::shared_ptr<MenuButton> m_BtnExit;
    std::shared_ptr<MenuButton> m_BtnOptions;
    std::shared_ptr<MenuButton> m_BtnCardWiki;
    std::shared_ptr<MenuButton> m_BtnMods;

    std::shared_ptr<Util::GameObject> m_pauseText;

    //===滑鼠移動鏡頭===
    bool m_IsDraggingMap = false;        // 記錄是否正在拖曳地圖
    glm::vec2 m_LastMousePos = {0, 0};   // 記錄滑鼠上一幀的位置
    //================

    // 所有卡片的陣列
    std::vector<std::shared_ptr<Card>> m_Cards;
    // 正在被滑鼠抓著」(如果沒抓東西就是 nullptr)
    std::shared_ptr<Card> m_DraggingCard = nullptr;
};

#endif
