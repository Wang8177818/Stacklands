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
#include "CheatMenu.hpp"

class UIManager {
public:
    enum class MenuEvent {
        NONE,
        START_GAME,
        EXIT,
        OPTIONS,
        CARD_WIKI,
        MODS,
        CONTINUE,
        BACKTOMENU,
        LOAD_GAME,   // 主選單按下「繼續遊戲(X月)」載入存檔
    };

    UIManager(Util::Renderer& renderer);
    ~UIManager() = default;

    // ── 選單階段 ──────────────────────────────────────────
    // saveMonth：若有存檔則顯示「繼續遊戲(N月)」按鈕；< 0 = 無存檔
    void InitMenu(int saveMonth = -1);
    MenuEvent UpdateMenu(glm::vec2 mousePos);
    MenuEvent UpdatePauseMenu(glm::vec2 mousePos);
    void TransitionToGame();
    void TransitionToMenu();

    // ── Game Over 畫面 ────────────────────────────────────
    // monthsPlayed：要顯示的「遊玩 N 月」
    void ShowGameOver(int monthsPlayed);
    void HideGameOver();
    // 回傳 true 表示點擊了「返回選單」
    bool UpdateGameOver(glm::vec2 mousePos);

    // 主選單「繼續遊戲」按鈕從此不再顯示（存檔被刪後呼叫）
    void ClearLoadGameButton();

    // 重建主選單「繼續遊戲(N月)」按鈕（剛存檔後呼叫，月份標籤刷新）
    void RefreshLoadGameButton(int month);

    // ── 遊戲 UI 階段 ──────────────────────────────────────
    std::shared_ptr<BackgroundImage>   GetGameFieldImage()      const { return m_HUD.field; }
    std::shared_ptr<Util::GameObject>  GetPauseText()           const { return m_HUD.pauseText; }
    std::shared_ptr<BackgroundImage>   GetRunTimeBar()          const { return m_HUD.runTimeBar; }
    std::shared_ptr<MenuButton>        GetPlayButton()          const { return m_HUD.playButton; }
    std::shared_ptr<BackgroundImage>   GetPauseMenu()           const { return m_Pause.image; }
    std::shared_ptr<BackgroundImage>   GetDescriptionBar()      const { return m_HUD.descBar; }
    std::shared_ptr<BackgroundImage>   GetResourseBar()         const { return m_HUD.resourceBar; }
    std::shared_ptr<BackgroundImage>   GetTimeBar()             const { return m_HUD.timeBar; }
    std::shared_ptr<Util::GameObject>  GetMonth()               const { return m_HUD.month; }
    std::shared_ptr<MenuButton>        GetContinueButton()      const { return m_Pause.btnContinue; }
    std::shared_ptr<MenuButton>        GetReturnToMenuButton()  const { return m_Pause.btnReturnToMenu; }
    std::shared_ptr<BackgroundImage>   GetCardIcon()            const { return m_HUD.cardIcon; }
    std::shared_ptr<Util::GameObject>  GetCardCount()           const { return m_HUD.cardCount; }
    std::shared_ptr<BackgroundImage>   GetCoinIcon()            const { return m_HUD.coinIcon; }
    std::shared_ptr<Util::GameObject>  GetCoinCount()           const { return m_HUD.coinCount; }
    std::shared_ptr<BackgroundImage>   GetFoodIcon()            const { return m_HUD.foodIcon; }
    std::shared_ptr<Util::GameObject>  GetFoodCount()           const { return m_HUD.foodCount; }
    std::shared_ptr<Util::GameObject>  GetDescriptionText()     const { return m_HUD.descText; }
    std::shared_ptr<Util::GameObject>  GetDescriptionName()     const { return m_HUD.descName; }

    void UpdateCardCount(int current, int max);
    void UpdateCoinCount(int current);
    void UpdateFood(int current, int need);
    void UpdateMonth(int month);
    void UpdateDescriptionText(const std::string& text);
    void UpdateDescriptionName(const std::string& name);

    // 作弊選單
    CheatMenu& GetCheatMenu() { return m_CheatMenu; }

private:
    Util::Renderer& m_Renderer;

    // 主選單資源
    struct MainMenuUI {
        std::shared_ptr<BackgroundImage> bg;
        std::shared_ptr<BackgroundImage> panel;
        std::shared_ptr<MenuButton> btnStart;
        std::shared_ptr<MenuButton> btnLoadGame;   // 繼續遊戲(N月)
        std::shared_ptr<MenuButton> btnExit;
        std::shared_ptr<MenuButton> btnOptions;
        std::shared_ptr<MenuButton> btnCardWiki;
        std::shared_ptr<MenuButton> btnMods;
    } m_Menu;

    // 暫停選單資源
    struct PauseMenuUI {
        std::shared_ptr<BackgroundImage> image;
        std::shared_ptr<MenuButton>      btnContinue;
        std::shared_ptr<MenuButton>      btnReturnToMenu;
    } m_Pause;

    // Game Over 畫面資源
    struct GameOverUI {
        std::shared_ptr<BackgroundImage>  panel;
        std::shared_ptr<Util::GameObject> title;
        std::shared_ptr<Util::GameObject> subtitle;
        std::shared_ptr<MenuButton>       btnReturn;
        bool initialized = false;
    } m_GameOver;

    // 遊戲 HUD 資源
    struct GameHUD {
        std::shared_ptr<BackgroundImage>  field;
        std::shared_ptr<BackgroundImage>  descBar;
        std::shared_ptr<BackgroundImage>  resourceBar;
        std::shared_ptr<BackgroundImage>  timeBar;
        std::shared_ptr<BackgroundImage>  runTimeBar;
        std::shared_ptr<BackgroundImage>  cardIcon;
        std::shared_ptr<BackgroundImage>  coinIcon;
        std::shared_ptr<BackgroundImage>  foodIcon;
        std::shared_ptr<Util::GameObject> pauseText;
        std::shared_ptr<Util::GameObject> month;
        std::shared_ptr<Util::GameObject> cardCount;
        std::shared_ptr<Util::GameObject> coinCount;
        std::shared_ptr<Util::GameObject> foodCount;
        std::shared_ptr<Util::GameObject> descName;
        std::shared_ptr<Util::GameObject> descText;
        std::shared_ptr<MenuButton>       playButton;
    } m_HUD;

    void AddButtonToRenderer(std::shared_ptr<MenuButton> btn);

    CheatMenu m_CheatMenu;
};

#endif //STACKLANDS_UIMANAGER_HPP
