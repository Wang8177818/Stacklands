#ifndef APP_HPP
#define APP_HPP

#include "ui/BackgroundImage.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "Util/Input.hpp"
#include "Util/Renderer.hpp"
#include "ui/Button.hpp"
#include "cards/Card.hpp"
#include <random>
#include "cards/CardPack.hpp"
#include "core/CardManager.hpp"
#include "ui/UIManager.hpp"
#include "core/EventManager.hpp"
#include "cards/Sellslot.hpp"
#include "cards/BlankSlot.hpp"
#include <vector>

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
    void SetAppState(State state) {
        m_CurrentState = state;
    }

    void Start();
    void MainMenu();
    void GameInit();
    void Update();
    void End();

    glm::vec2 mousePos = Util::Input::GetCursorPosition();

private:
    float basic_scale = 0.1f;
    State    m_CurrentState = State::START;

    Util::Renderer m_Renderer;

    // ── 子系統 ────────────────────────────────────────────
    std::unique_ptr<CardManager>  m_CardManager;
    std::unique_ptr<UIManager>    m_UIManager;
    std::unique_ptr<EventManager> m_EventManager;
    std::shared_ptr<SellSlot> m_SellSlot;
    std::vector<std::shared_ptr<BlankSlot>> m_BlankSlots;  // SellSlot 右側 8 個無功能格子

    bool m_HadCharacters = false; // 場上曾經出現過角色後才啟用 Game Over 檢查
};

#endif
