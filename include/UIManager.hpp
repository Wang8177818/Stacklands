//
// Created by user on 2026/3/27.
//

#ifndef STACKLANDS_UIMANAGER_HPP
#define STACKLANDS_UIMANAGER_HPP

#include "BackgroundImage.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "Util/Input.hpp"
#include "Util/Renderer.hpp"
#include "Button.hpp"

class UIManager {
public:
    // 按鈕事件，App 根據這個決定狀態切換
    enum class MenuEvent {
        NONE,
        START_GAME,
        EXIT,
        OPTIONS,
        CARD_WIKI,
        MODS,
        CONTINUE,
    };

    UIManager(Util::Renderer& renderer);
    ~UIManager() = default;

    // ── 選單階段 ──────────────────────────────────────────
    // 初始化選單所有背景與按鈕，並加入 Renderer
    void InitMenu();

    // 每幀呼叫；處理 Hover 動畫並回傳被點擊的事件
    MenuEvent UpdateMenu(glm::vec2 mousePos);
    MenuEvent UpdatePauseMenu(glm::vec2 mousePos);

    // 進入遊戲時，隱藏所有選單物件並建立遊戲 UI
    void TransitionToGame();

    // ── 遊戲 UI 階段 ──────────────────────────────────────
    // 取得遊戲欄位圖，App::Update 需要拿它做地圖移動 / 縮放
    std::shared_ptr<BackgroundImage> GetGameFieldImage() const { return m_GameFieldImage; }

    // 取得暫停文字物件
    std::shared_ptr<Util::GameObject> GetPauseText() const { return m_PauseText; }

    // 取得進度條圖
    std::shared_ptr<BackgroundImage> GetRunTimeBar() const { return m_RunTimeBarImage; }

    // 取得播放/快進/暫停按鈕
    std::shared_ptr<MenuButton> GetPlayButton() const { return m_PlayButton; }

    // 取得暫停選單
    std::shared_ptr<BackgroundImage> GetPauseMenu() const { return m_PauseMenuImage; }

    // 取得描述欄
    std::shared_ptr<BackgroundImage> GetDescriptionBar() const { return m_DescriptionBarImage; }

    // 取得資訊欄
    std::shared_ptr<BackgroundImage> GetResourseBar() const { return m_ResourceBarImage; }

    // 取得時間欄
    std::shared_ptr<BackgroundImage> GetTimeBar() const { return m_TimeBarImage; }

    // 取得月份
    std::shared_ptr<Util::GameObject> GetMonth() const { return m_Month; }

    // 取得暫停選單:繼續
    std::shared_ptr<MenuButton> GetContinueButton() const { return m_Continue; }

private:
    Util::Renderer& m_Renderer;

    // ── 選單資源 ──────────────────────────────────────────
    std::shared_ptr<BackgroundImage> m_MainMenuBG;
    std::shared_ptr<BackgroundImage> m_MainMenuImage;
    std::shared_ptr<BackgroundImage> m_PauseMenuImage;

    std::shared_ptr<MenuButton> m_BtnStart;
    std::shared_ptr<MenuButton> m_BtnExit;
    std::shared_ptr<MenuButton> m_BtnOptions;
    std::shared_ptr<MenuButton> m_BtnCardWiki;
    std::shared_ptr<MenuButton> m_BtnMods;

    std::shared_ptr<MenuButton> m_Continue;

    // ── 遊戲 UI 資源 ─────────────────────────────────────
    std::shared_ptr<BackgroundImage> m_GameFieldImage;
    std::shared_ptr<BackgroundImage> m_DescriptionBarImage;
    std::shared_ptr<BackgroundImage> m_ResourceBarImage;
    std::shared_ptr<BackgroundImage> m_TimeBarImage;
    std::shared_ptr<BackgroundImage> m_RunTimeBarImage;
    std::shared_ptr<Util::GameObject> m_PauseText;
    std::shared_ptr<MenuButton>       m_PlayButton;

    std::shared_ptr<Util::GameObject> m_Month;

    // 內部輔助：把一個 MenuButton 的所有 GameObject 加入 Renderer
    void AddButtonToRenderer(std::shared_ptr<MenuButton> btn);
};

#endif //STACKLANDS_UIMANAGER_HPP
