#include "UIManager.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Core/Context.hpp"
#include "App.hpp"

// ─────────────────────────────────────────────────────────────
UIManager::UIManager(Util::Renderer& renderer) : m_Renderer(renderer) {}

// ─────────────────────────────────────────────────────────────
// 輔助：把按鈕的所有 GameObject 一次加入 Renderer
void UIManager::AddButtonToRenderer(std::shared_ptr<MenuButton> btn) {
    for (auto& obj : btn->GetGameObjects()) {
        m_Renderer.AddChild(obj);
    }
}

// ─────────────────────────────────────────────────────────────
void UIManager::InitMenu() {
    // ── 視窗尺寸 ──────────────────────────────────────────
    auto instance   = Core::Context::GetInstance();
    float winW      = instance->GetWindowWidth();
    float winH      = instance->GetWindowHeight();

    // ── 主選單背景圖 ──────────────────────────────────────
    m_MainMenuBG = std::make_shared<BackgroundImage>();
    float baseW  = 1150.f, baseH = 720.f;
    m_MainMenuBG->SetScale({winW / baseW, winH / baseH});
    m_Renderer.AddChild(m_MainMenuBG);

    m_MainMenuImage = std::make_shared<BackgroundImage>();
    m_MainMenuImage->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/stacklandsMenuMain.png"));
    m_MainMenuImage->m_Transform.translation = glm::vec2(-470, -110);
    m_MainMenuImage->m_Transform.scale= {0.5f, 0.5f};
    m_Renderer.AddChild(m_MainMenuImage);

    m_PauseMenuImage = std::make_shared<BackgroundImage>();
    m_PauseMenuImage->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/pauseMenu.png"));
    m_PauseMenuImage->m_Transform.translation = glm::vec2(-470, -190);
    m_PauseMenuImage->m_Transform.scale= {0.5f, 0.5f};
    m_PauseMenuImage->SetZIndex(98);
    m_PauseMenuImage->SetVisible(false);
    m_Renderer.AddChild(m_PauseMenuImage);

    // ── 選單按鈕 ──────────────────────────────────────────
    m_BtnStart    = std::make_shared<MenuButton>(-562, -80,  20, 100, 20, "開始新遊戲", true, 5);
    m_BtnExit     = std::make_shared<MenuButton>(-572, -315, 20,  80, 20, "離開遊戲",   true, 4);
    m_BtnOptions  = std::make_shared<MenuButton>(-592, -200, 20,  40, 20, "選項",       true, 2);
    m_BtnCardWiki = std::make_shared<MenuButton>(-572, -160, 20,  80, 20, "卡片百科",   true, 4);
    m_BtnMods     = std::make_shared<MenuButton>(-592, -240, 20,  40, 20, "模組",       true, 2);

    m_Continue    = std::make_shared<MenuButton>(-592, -120, 20,  40, 20, "繼續",       true, 2);
    m_Continue->HideAll();

    m_ReturnToMenu = std::make_shared<MenuButton>(-574, -310, 20,  80, 20, "返回選單",       true, 4);
    m_ReturnToMenu->HideAll();

    AddButtonToRenderer(m_BtnStart);
    AddButtonToRenderer(m_BtnExit);
    AddButtonToRenderer(m_BtnOptions);
    AddButtonToRenderer(m_BtnCardWiki);
    AddButtonToRenderer(m_BtnMods);
    AddButtonToRenderer(m_Continue);
    AddButtonToRenderer(m_ReturnToMenu);
}

// ─────────────────────────────────────────────────────────────
UIManager::MenuEvent UIManager::UpdateMenu(glm::vec2 mousePos) {
    bool isStartHover    = m_BtnStart   ->UpdateHover(mousePos);
    bool isExitHover     = m_BtnExit    ->UpdateHover(mousePos);
    bool isOptionsHover  = m_BtnOptions ->UpdateHover(mousePos);
    bool isCardWikiHover = m_BtnCardWiki->UpdateHover(mousePos);
    bool isModsHover     = m_BtnMods    ->UpdateHover(mousePos);

    if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        if (isStartHover)    return MenuEvent::START_GAME;
        if (isExitHover)     return MenuEvent::EXIT;
        if (isOptionsHover)  return MenuEvent::OPTIONS;
        if (isCardWikiHover) return MenuEvent::CARD_WIKI;
        if (isModsHover)     return MenuEvent::MODS;
    }
    return MenuEvent::NONE;
}

UIManager::MenuEvent UIManager::UpdatePauseMenu(glm::vec2 mousePos) {
    bool isContinueHover = m_Continue   ->UpdateHover(mousePos);
    bool isReturnToMenuButtonHover = m_ReturnToMenu->UpdateHover(mousePos);

    if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        if (isContinueHover) return MenuEvent::CONTINUE;
        if (isReturnToMenuButtonHover) return MenuEvent::BACKTOMENU;
    }
    return MenuEvent::NONE;
}

// ─────────────────────────────────────────────────────────────
void UIManager::TransitionToGame() {
    // ── 隱藏選單 ──────────────────────────────────────────
    m_MainMenuImage->SetVisible(false);
    m_BtnStart   ->HideAll();
    m_BtnExit    ->HideAll();
    m_BtnOptions ->HideAll();
    m_BtnCardWiki->HideAll();
    m_BtnMods    ->HideAll();

    // ── 換成遊戲背景 ──────────────────────────────────────
    m_MainMenuBG->SetDrawable(
        std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/greenBG.png"));

    // ── 遊戲欄位圖 ────────────────────────────────────────
    m_GameFieldImage = std::make_shared<BackgroundImage>();
    m_GameFieldImage->SetDrawable(
        std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/gameField.png"));
    m_GameFieldImage->SetScale({0.7f, 0.7f});
    m_Renderer.AddChild(m_GameFieldImage);

    // ── 描述欄 ────────────────────────────────────────────
    m_DescriptionBarImage = std::make_shared<BackgroundImage>();
    m_DescriptionBarImage->SetDrawable(
        std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/descriptionBar.png"));
    m_DescriptionBarImage->SetScale({0.75f, 0.75f});
    m_DescriptionBarImage->m_Transform.translation = {-510.f, -240.f};
    m_DescriptionBarImage->SetZIndex(98);
    m_Renderer.AddChild(m_DescriptionBarImage);

    // ── 資源欄 ────────────────────────────────────────────
    m_ResourceBarImage = std::make_shared<BackgroundImage>();
    m_ResourceBarImage->SetDrawable(
        std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png"));
    m_ResourceBarImage->SetScale({0.55f, 0.75f});
    m_ResourceBarImage->m_Transform.translation = {260.f, 335.f};
    m_ResourceBarImage->SetZIndex(98);
    m_Renderer.AddChild(m_ResourceBarImage);

    // ── 時間欄 ────────────────────────────────────────────
    m_TimeBarImage = std::make_shared<BackgroundImage>();
    m_TimeBarImage->SetDrawable(
        std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png"));
    m_TimeBarImage->SetScale({0.75f, 0.75f});
    m_TimeBarImage->m_Transform.translation = {500.f, 335.f};
    m_TimeBarImage->SetZIndex(98);
    m_Renderer.AddChild(m_TimeBarImage);

    // ── 進度條 ────────────────────────────────────────────
    m_RunTimeBarImage = std::make_shared<BackgroundImage>();
    m_RunTimeBarImage->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/button/darker_bg.png"));
    m_RunTimeBarImage->SetPivot({-0.5f, 0.f});
    m_RunTimeBarImage->SetScale({0.f, 35.f});
    m_RunTimeBarImage->m_Transform.translation = {368.f, 335.f};
    m_RunTimeBarImage->SetZIndex(99);
    m_Renderer.AddChild(m_RunTimeBarImage);

    // ── 月份 ────────────────────────────────────────────
    m_Month = std::make_shared<Util::GameObject>();
    m_Month->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 35, "1月", Util::Color(0, 0, 0)));
    m_Month->m_Transform.translation = glm::vec2(395, 335);
    m_Month->m_Transform.scale= {0.5f, 0.5f};
    m_Month->SetZIndex(100);
    m_Renderer.AddChild(m_Month);

    // ── 暫停文字 ──────────────────────────────────────────
    m_PauseText = std::make_shared<Util::GameObject>();
    m_PauseText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 50, "暫停。", Util::Color(0, 0, 0, 50)));
    m_PauseText->SetVisible(false);
    m_PauseText->SetZIndex(100);
    m_Renderer.AddChild(m_PauseText);

    // ── 播放按鈕 ──────────────────────────────────────────
    m_PlayButton = std::make_shared<MenuButton>(
        615.f, 335.f, 0.06f, 0.06f, 20.f, 20.f, "/Image/button/play.png");
    AddButtonToRenderer(m_PlayButton);
}
