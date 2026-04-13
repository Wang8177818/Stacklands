#ifndef APP_HPP
#define APP_HPP

#include "BackgroundImage.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "Util/Input.hpp"
#include "Util/Renderer.hpp"
#include "Button.hpp"
#include "Card.hpp"
#include <random>
#include "CardPack.hpp"
#include "CardManager.hpp"
#include "UIManager.hpp"
#include "EventManager.hpp"
#include "Sellslot.hpp"

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

    State    GetCurrentState() const { return m_CurrentState; }
    GameTime GetGameState()    const { return m_GameTime; }

    void Start();
    void MainMenu();
    void GameInit();
    void Update();
    void End();

    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    float tick  = 0;
    float month = 1;

private:
    float basic_scale = 0.05f;
    State    m_CurrentState = State::START;
    GameTime m_GameTime     = GameTime::NORMAL;

    Util::Renderer m_Renderer;

    // ── 子系統 ────────────────────────────────────────────
    std::unique_ptr<CardManager>  m_CardManager;
    std::unique_ptr<UIManager>    m_UIManager;
    std::unique_ptr<EventManager> m_EventManager;
    std::shared_ptr<SellSlot> m_SellSlot;

};

#endif
