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

    State GetCurrentState() const { return m_CurrentState; }

    void Start();

    void MainMenu();

    void Update();

    void GameInit();

    void End(); // NOLINT(readability-convert-member-functions-to-static)

    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    //鼠標位置

private:
    void ValidTask();
    State m_CurrentState = State::START;
    Util::Renderer m_Renderer;

    std::shared_ptr<BackgroundImage> m_MainMenuImage;
    std::shared_ptr<MenuButton> m_BtnStart;
    std::shared_ptr<MenuButton> m_BtnExit;

    // 所有卡片的陣列
    std::vector<std::shared_ptr<Card>> m_Cards;
    // 正在被滑鼠抓著」(如果沒抓東西就是 nullptr)
    std::shared_ptr<Card> m_DraggingCard = nullptr;
};

#endif
