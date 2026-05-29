#include "UIManager.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Core/Context.hpp"
#include "App.hpp"

namespace {
// 以單字為單位插入 \n。SDL_TTF 的 TTF_RenderUTF8_Blended_Wrapped 會處理換行。
// maxLinePx：surface 寬度（像素）；fontSize：字級。英文平均字元寬約 fontSize*0.5。
std::string WrapEnglishText(const std::string& text, int maxLinePx, int fontSize) {
    const float avgCharPx = fontSize * 0.5f;
    const int maxCharsPerLine = std::max(1, static_cast<int>(maxLinePx / avgCharPx));

    std::string result;
    int lineLen = 0;
    std::size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '\n') {
            result += '\n';
            lineLen = 0;
            ++i;
            continue;
        }
        std::size_t wordEnd = i;
        while (wordEnd < text.size() && text[wordEnd] != ' ' && text[wordEnd] != '\n') {
            ++wordEnd;
        }
        int wordLen = static_cast<int>(wordEnd - i);

        if (lineLen > 0 && lineLen + 1 + wordLen > maxCharsPerLine) {
            result += '\n';
            lineLen = 0;
        } else if (lineLen > 0) {
            result += ' ';
            ++lineLen;
        }
        result.append(text, i, wordLen);
        lineLen += wordLen;
        i = wordEnd;

        while (i < text.size() && text[i] == ' ') ++i;
    }
    return result;
}
} // namespace

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
    m_Menu.bg = std::make_shared<BackgroundImage>();
    float baseW  = 1150.f, baseH = 720.f;
    m_Menu.bg->SetScale({winW / baseW, winH / baseH});
    m_Renderer.AddChild(m_Menu.bg);

    m_Menu.panel = std::make_shared<BackgroundImage>();
    m_Menu.panel->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/stacklandsMenuMain.png"));
    m_Menu.panel->m_Transform.translation = glm::vec2(-470, -110);
    m_Menu.panel->m_Transform.scale= {0.5f, 0.5f};
    m_Renderer.AddChild(m_Menu.panel);

    m_Pause.image = std::make_shared<BackgroundImage>();
    m_Pause.image->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/pauseMenu.png"));
    m_Pause.image->m_Transform.translation = glm::vec2(-470, -190);
    m_Pause.image->m_Transform.scale= {0.5f, 0.5f};
    m_Pause.image->SetZIndex(98);
    m_Pause.image->SetVisible(false);
    m_Renderer.AddChild(m_Pause.image);

    // ── 選單按鈕 ──────────────────────────────────────────
    m_Menu.btnStart    = std::make_shared<MenuButton>(-562, -80,  20, 100, 20, "開始新遊戲", true, 5);
    m_Menu.btnExit     = std::make_shared<MenuButton>(-572, -315, 20,  80, 20, "離開遊戲",   true, 4);
    m_Menu.btnOptions  = std::make_shared<MenuButton>(-592, -200, 20,  40, 20, "選項",       true, 2);
    m_Menu.btnCardWiki = std::make_shared<MenuButton>(-572, -160, 20,  80, 20, "卡片百科",   true, 4);
    m_Menu.btnMods     = std::make_shared<MenuButton>(-592, -240, 20,  40, 20, "模組",       true, 2);

    m_Pause.btnContinue    = std::make_shared<MenuButton>(-592, -120, 20,  40, 20, "繼續",       true, 2);
    m_Pause.btnContinue->HideAll();

    m_Pause.btnReturnToMenu = std::make_shared<MenuButton>(-574, -310, 20,  80, 20, "返回選單",       true, 4);
    m_Pause.btnReturnToMenu->HideAll();

    AddButtonToRenderer(m_Menu.btnStart);
    AddButtonToRenderer(m_Menu.btnExit);
    AddButtonToRenderer(m_Menu.btnOptions);
    AddButtonToRenderer(m_Menu.btnCardWiki);
    AddButtonToRenderer(m_Menu.btnMods);
    AddButtonToRenderer(m_Pause.btnContinue);
    AddButtonToRenderer(m_Pause.btnReturnToMenu);
}

// ─────────────────────────────────────────────────────────────
UIManager::MenuEvent UIManager::UpdateMenu(glm::vec2 mousePos) {
    bool isStartHover    = m_Menu.btnStart   ->UpdateHover(mousePos);
    bool isExitHover     = m_Menu.btnExit    ->UpdateHover(mousePos);
    bool isOptionsHover  = m_Menu.btnOptions ->UpdateHover(mousePos);
    bool isCardWikiHover = m_Menu.btnCardWiki->UpdateHover(mousePos);
    bool isModsHover     = m_Menu.btnMods    ->UpdateHover(mousePos);

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
    bool isContinueHover = m_Pause.btnContinue   ->UpdateHover(mousePos);
    bool isReturnToMenuButtonHover = m_Pause.btnReturnToMenu->UpdateHover(mousePos);

    if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        if (isContinueHover) return MenuEvent::CONTINUE;
        if (isReturnToMenuButtonHover) return MenuEvent::BACKTOMENU;
    }
    return MenuEvent::NONE;
}

// ─────────────────────────────────────────────────────────────
void UIManager::TransitionToGame() {
    // ── 隱藏選單 ──────────────────────────────────────────
    m_Menu.panel->SetVisible(false);
    m_Menu.btnStart   ->HideAll();
    m_Menu.btnExit    ->HideAll();
    m_Menu.btnOptions ->HideAll();
    m_Menu.btnCardWiki->HideAll();
    m_Menu.btnMods    ->HideAll();

    // ── 換成遊戲背景 ──────────────────────────────────────
    m_Menu.bg->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/greenBG.png"));

    // ── 遊戲欄位圖 ────────────────────────────────────────
    m_HUD.field = std::make_shared<BackgroundImage>();
    m_HUD.field->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/gameField.png"));
    m_HUD.field->SetScale({0.7f, 0.7f});
    m_Renderer.AddChild(m_HUD.field);

    // ── 描述欄 ────────────────────────────────────────────
    m_HUD.descBar = std::make_shared<BackgroundImage>();
    m_HUD.descBar->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/descriptionBar.png"));
    m_HUD.descBar->SetScale({0.75f, 0.75f});
    m_HUD.descBar->m_Transform.translation = {-510.f, -240.f};
    m_HUD.descBar->SetZIndex(98);
    m_Renderer.AddChild(m_HUD.descBar);

    // ── 資源欄 ────────────────────────────────────────────
    m_HUD.resourceBar = std::make_shared<BackgroundImage>();
    m_HUD.resourceBar->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png"));
    m_HUD.resourceBar->SetScale({0.55f, 0.75f});
    m_HUD.resourceBar->m_Transform.translation = {260.f, 335.f};
    m_HUD.resourceBar->SetZIndex(98);
    m_Renderer.AddChild(m_HUD.resourceBar);

    // ── 時間欄 ────────────────────────────────────────────
    m_HUD.timeBar = std::make_shared<BackgroundImage>();
    m_HUD.timeBar->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/topBar.png"));
    m_HUD.timeBar->SetScale({0.75f, 0.75f});
    m_HUD.timeBar->m_Transform.translation = {500.f, 335.f};
    m_HUD.timeBar->SetZIndex(98);
    m_Renderer.AddChild(m_HUD.timeBar);

    // ── 進度條 ────────────────────────────────────────────
    m_HUD.runTimeBar = std::make_shared<BackgroundImage>();
    m_HUD.runTimeBar->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/button/darker_bg.png"));
    m_HUD.runTimeBar->SetPivot({-0.5f, 0.f});
    m_HUD.runTimeBar->SetScale({0.f, 35.f});
    m_HUD.runTimeBar->m_Transform.translation = {368.f, 335.f};
    m_HUD.runTimeBar->SetZIndex(99);
    m_Renderer.AddChild(m_HUD.runTimeBar);

    // ── 月份 ────────────────────────────────────────────
    m_HUD.month = std::make_shared<Util::GameObject>();
    m_HUD.month->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 35, "1月", Util::Color(0, 0, 0)));
    m_HUD.month->m_Transform.translation = glm::vec2(395, 335);
    m_HUD.month->m_Transform.scale= {0.5f, 0.5f};
    m_HUD.month->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.month);

    // ── 暫停文字 ──────────────────────────────────────────
    m_HUD.pauseText = std::make_shared<Util::GameObject>();
    m_HUD.pauseText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 50, "暫停。", Util::Color(0, 0, 0, 50)));
    m_HUD.pauseText->SetVisible(false);
    m_HUD.pauseText->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.pauseText);

    // ── 播放按鈕 ──────────────────────────────────────────
    m_HUD.playButton = std::make_shared<MenuButton>(615.f, 335.f, 0.06f, 0.06f, 20.f, 20.f, "/Image/button/play.png");
    AddButtonToRenderer(m_HUD.playButton);

    // ── 卡片數量圖標 ──────────────────────────────────────────
    m_HUD.cardIcon = std::make_shared<BackgroundImage>();
    m_HUD.cardIcon->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/cardicon.png"));
    m_HUD.cardIcon->m_Transform.translation = glm::vec2(340, 335);
    m_HUD.cardIcon->m_Transform.scale= {0.35f, 0.35f};
    m_HUD.cardIcon->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.cardIcon);

    // ── 卡片數量 ────────────────────────────────────────────
    m_HUD.cardCount = std::make_shared<Util::GameObject>();
    m_HUD.cardCount->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 35, "0/50", Util::Color(0, 0, 0)));
    m_HUD.cardCount->m_Transform.translation = glm::vec2(305, 335);
    m_HUD.cardCount->SetPivot({0.5,0});
    m_HUD.cardCount->m_Transform.scale= {0.5f, 0.5f};
    m_HUD.cardCount->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.cardCount);

    // ── 金幣數量圖標 ──────────────────────────────────────────
    m_HUD.coinIcon = std::make_shared<BackgroundImage>();
    m_HUD.coinIcon->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/Coin.png"));
    m_HUD.coinIcon->m_Transform.translation = glm::vec2(255, 335);
    m_HUD.coinIcon->m_Transform.scale= {0.04f, 0.04f};
    m_HUD.coinIcon->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.coinIcon);

    // ── 金幣數量 ────────────────────────────────────────────
    m_HUD.coinCount = std::make_shared<Util::GameObject>();
    m_HUD.coinCount->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 35, "0", Util::Color(0, 0, 0)));
    m_HUD.coinCount->m_Transform.translation = glm::vec2(235, 335);
    m_HUD.coinCount->SetPivot({0.5,0});
    m_HUD.coinCount->m_Transform.scale= {0.5f, 0.5f};
    m_HUD.coinCount->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.coinCount);

    // ── 食物數量圖標 ──────────────────────────────────────────
    m_HUD.foodIcon = std::make_shared<BackgroundImage>();
    m_HUD.foodIcon->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/foodicon.png"));
    m_HUD.foodIcon->m_Transform.translation = glm::vec2(210, 335);
    m_HUD.foodIcon->m_Transform.scale= {0.04f, 0.04f};
    m_HUD.foodIcon->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.foodIcon);

    // ── 食物數量 ────────────────────────────────────────────
    m_HUD.foodCount = std::make_shared<Util::GameObject>();
    m_HUD.foodCount->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 35, "0/0", Util::Color(0, 0, 0)));
    m_HUD.foodCount->m_Transform.translation = glm::vec2(185, 335);
    m_HUD.foodCount->SetPivot({0.5,0});
    m_HUD.foodCount->m_Transform.scale= {0.5f, 0.5f};
    m_HUD.foodCount->SetZIndex(100);
    m_Renderer.AddChild(m_HUD.foodCount);

    // ── 卡片名稱（顯示於敘述欄上方）────────────────────────
    m_HUD.descName = std::make_shared<Util::GameObject>();
    {
        auto initDrawable = std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjhbd.ttc", 32, " ", Util::Color(0, 0, 0));
        m_HUD.descName->SetDrawable(initDrawable);
        m_HUD.descName->SetPivot({-initDrawable->GetSize().x / 2.f, 0.f});
    }
    m_HUD.descName->m_Transform.translation = glm::vec2(-630, -150); // 位置可自行調整
    m_HUD.descName->m_Transform.scale = {0.5f, 0.5f};
    m_HUD.descName->SetZIndex(100);
    m_HUD.descName->SetVisible(false);
    m_Renderer.AddChild(m_HUD.descName);

    // ── 敘述文字 ────────────────────────────────────────────
    m_HUD.descText = std::make_shared<Util::GameObject>();
    {
        auto initDrawable = std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjhbd.ttc", 28, " ", Util::Color(0, 0, 0));
        m_HUD.descText->SetDrawable(initDrawable);
        m_HUD.descText->SetPivot({-initDrawable->GetSize().x / 2.f, 0.f});
    }
    m_HUD.descText->m_Transform.translation = glm::vec2(-630, -190);
    m_HUD.descText->m_Transform.scale= {0.5f, 0.5f};
    m_HUD.descText->SetZIndex(100);
    m_HUD.descText->SetVisible(false);
    m_Renderer.AddChild(m_HUD.descText);
}

// ─────────────────────────────────────────────────────────────
void UIManager::UpdateCardCount(int current, int max) {
    if (!m_HUD.cardCount) return;
    std::string text = std::to_string(current) + "/" + std::to_string(max);
    m_HUD.cardCount->SetDrawable(std::make_shared<Util::Text>(
        RESOURCE_DIR"/Font/msjhbd.ttc", 35, text, Util::Color(0, 0, 0)));
}

void UIManager::UpdateCoinCount(int current) {
    if (!m_HUD.coinCount) return;
    std::string text = std::to_string(current);
    m_HUD.coinCount->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 35, text, Util::Color(0, 0, 0)));
}

void UIManager::UpdateMonth(int month) {
    std::string text = std::to_string(month) + "月";
    m_HUD.month->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", 35, text, Util::Color(0, 0, 0)));
    m_HUD.runTimeBar->SetScale({0, 35.f});
}

void UIManager::UpdateFood(int current, int need) {
    if (!m_HUD.foodCount) return;
    std::string text = std::to_string(current) + "/" + std::to_string(need);
    m_HUD.foodCount->SetDrawable(std::make_shared<Util::Text>(
        RESOURCE_DIR"/Font/msjhbd.ttc", 35, text, Util::Color(0, 0, 0)));
}

// ─────────────────────────────────────────────────────────────
void UIManager::UpdateDescriptionName(const std::string& name) {
    if (!m_HUD.descName) return;
    if (name.empty()) {
        m_HUD.descName->SetVisible(false);
        return;
    }
    m_HUD.descName->SetVisible(true);
    auto drawable = std::make_shared<Util::Text>(
        RESOURCE_DIR"/Font/msjhbd.ttc", 32, name, Util::Color(0, 0, 0));
    m_HUD.descName->SetDrawable(drawable);
    m_HUD.descName->SetPivot({-drawable->GetSize().x / 2.f, 0.f});
}

// ─────────────────────────────────────────────────────────────
void UIManager::UpdateDescriptionText(const std::string& text) {
    if (!m_HUD.descText) return;
    if (text.empty()) {
        m_HUD.descText->SetVisible(false);
        return;
    }
    m_HUD.descText->SetVisible(true);
    // 描述欄顯示寬 333px，scale=0.5 → surface 換行寬 = 333/0.5 = 666 像素
    constexpr int WRAP_PX = 666;
    constexpr int FONT_SIZE = 28;
    std::string wrapped = WrapEnglishText(text, WRAP_PX, FONT_SIZE);
    auto drawable = std::make_shared<Util::Text>(
        RESOURCE_DIR"/Font/msjhbd.ttc", FONT_SIZE, wrapped, Util::Color(0, 0, 0));
    m_HUD.descText->SetDrawable(drawable);
    // 左對齊：pivot.x = -textWidth/2
    m_HUD.descText->SetPivot({-drawable->GetSize().x / 2.f, 0.f});
}
