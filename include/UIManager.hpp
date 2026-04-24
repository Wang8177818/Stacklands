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
        BACKTOMENU,
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

    // 每幀由 App 呼叫，傳入當前卡片數與上限，更新畫面文字
    void UpdateCardCount(int current, int max);

    // 每幀由 App 呼叫，傳入當前金幣數，更新畫面文字
    void UpdateCoinCount(int current);

    // 每幀由 App 呼叫，傳入當前食物數與所需食物數，更新畫面文字
    void UpdateFood(int current, int need);

    // 當月結束時更新月份
    void UpdateMonth(int month);

    // 取得暫停選單:繼續
    std::shared_ptr<MenuButton> GetContinueButton() const { return m_Continue; }

    // 取得返回選單
    std::shared_ptr<MenuButton> GetReturnToMenuButton() const { return m_ReturnToMenu; }

    // 取得卡片數量圖標
    std::shared_ptr<BackgroundImage> GetCardIcon() const { return m_CardIcon; }

    // 取得卡片數量顯示
    std::shared_ptr<Util::GameObject> GetCardCount() const { return m_CardCount; }

    // 取得卡片數量圖標
    std::shared_ptr<BackgroundImage> GetCoinIcon() const { return m_CoinIcon; }

    // 取得金幣數量顯示
    std::shared_ptr<Util::GameObject> GetCoinCount() const { return m_CoinCount; }

    // 取得食物數量圖標
    std::shared_ptr<BackgroundImage> GetFoodIcon() const { return m_FoodIcon; }

    // 取得食物數量顯示
    std::shared_ptr<Util::GameObject> GetFoodCount() const { return m_FoodCount; }

    // 取得敘述文字
    std::shared_ptr<Util::GameObject> GetDescriptionText() const { return m_DescriptionText; }

    // 更新敘述欄文字（滑鼠懸停卡片時由 App 呼叫）
    void UpdateDescriptionText(const std::string& text);

    // 取得卡片名稱文字（顯示於敘述欄上方）
    std::shared_ptr<Util::GameObject> GetDescriptionName() const { return m_DescriptionName; }

    // 更新卡片名稱（滑鼠懸停卡片時由 App 呼叫）
    void UpdateDescriptionName(const std::string& name);


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
    std::shared_ptr<MenuButton> m_ReturnToMenu;

    // ── 遊戲 UI 資源 ─────────────────────────────────────
    std::shared_ptr<BackgroundImage> m_GameFieldImage;
    std::shared_ptr<BackgroundImage> m_DescriptionBarImage;
    std::shared_ptr<BackgroundImage> m_ResourceBarImage;
    std::shared_ptr<BackgroundImage> m_TimeBarImage;
    std::shared_ptr<BackgroundImage> m_RunTimeBarImage;
    std::shared_ptr<Util::GameObject> m_PauseText;
    std::shared_ptr<MenuButton>       m_PlayButton;

    std::shared_ptr<Util::GameObject> m_Month;
    std::shared_ptr<BackgroundImage> m_CardIcon;
    std::shared_ptr<Util::GameObject> m_CardCount;
    std::shared_ptr<BackgroundImage> m_CoinIcon;
    std::shared_ptr<Util::GameObject> m_CoinCount;
    std::shared_ptr<BackgroundImage> m_FoodIcon;
    std::shared_ptr<Util::GameObject> m_FoodCount;
    std::shared_ptr<Util::GameObject> m_DescriptionName;
    std::shared_ptr<Util::GameObject> m_DescriptionText;

    // 內部輔助：把一個 MenuButton 的所有 GameObject 加入 Renderer
    void AddButtonToRenderer(std::shared_ptr<MenuButton> btn);
};

#endif //STACKLANDS_UIMANAGER_HPP
