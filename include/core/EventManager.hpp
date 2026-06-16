//
// Created by user on 2026/3/27.
//

#ifndef STACKLANDS_EVENTMANAGER_HPP
#define STACKLANDS_EVENTMANAGER_HPP

#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "ui/BackgroundImage.hpp"
#include "cards/Card.hpp"
#include "ui/UIManager.hpp"

class CardManager;  // 前置宣告：避免循環 include

class EventManager {
public:

    enum class GameTime {
        NORMAL,
        FAST,
        PAUSE,
    };

    float tick  = 0;
    float month = 1;

    GameTime GetGameState() const { return m_GameTime; }

    // 取得本幀已套用倍率的 dtMs（PAUSE=0、NORMAL=1x、FAST=2x）
    // 動物 / 怪物 / 其他需要遊戲時間的系統應改用這個，而非 Util::Time::GetDeltaTimeMs()
    static float GetScaledDtMs() { return s_ScaledDtMs; }

    // ── 縮放邊界常數 ──────────────────────────────────────
    static constexpr float ZOOM_MIN      = 0.5f;
    static constexpr float ZOOM_MAX      = 4.0f;
    static constexpr float ZOOM_FACTOR   = 1.08f;  // 乘法縮放：每次滾輪 ×1.08
    static constexpr float WASD_SPEED    = 3.0f;

    EventManager() = default;

    // 重置月份與進度條（Game Over 時呼叫）
    void Reset() {
        tick  = 0;
        month = 1;
        m_GameTime   = GameTime::NORMAL;
        is_Pausing   = false;
        m_RequestExit = false;
    }

    // 綁定需要跟著移動/縮放的場景背景圖
    void SetGameField(std::shared_ptr<BackgroundImage> gameField);

    // 綁定 UIManager（不擁有，只借用）
    void SetUIManager(UIManager* uiManager) { m_UIManager = uiManager; }

    // 綁定 CardManager（月底結算時呼叫其 OnMonthEnd）
    void SetCardManager(CardManager* cardManager) { m_CardManager = cardManager; }

    // 每幀呼叫
    // mousePos        — 當前滑鼠座標
    // isDraggingCard  — CardManager 是否正在拖曳卡片 (若是，則不能拖曳地圖)
    // cards           — 場上所有卡片 (移動/縮放時一起搬)
    void Update(glm::vec2 mousePos,
                bool isDraggingCard,
                std::vector<std::shared_ptr<Card>>& cards);

    // 查詢目前是否正在拖曳地圖
    bool IsDraggingMap() const { return m_IsDraggingMap; }

    // 暫停選單按下「返回選單」後會變成 true，App 讀取後應切換狀態
    bool IsRequestingExit() const { return m_RequestExit; }

    // 取得目前縮放倍率
    float GetZoomRatio() const { return m_ZoomRatio; }

private:
    void HandlePan(glm::vec2 mousePos,
                   bool isDraggingCard,
                   std::vector<std::shared_ptr<Card>>& cards);

    void HandleZoom(std::vector<std::shared_ptr<Card>>& cards);

    void HandleWASD(std::vector<std::shared_ptr<Card>>& cards);

    // 移動所有物件（背景 + 卡片）
    void MoveAll(glm::vec2 delta, std::vector<std::shared_ptr<Card>>& cards);

    // 切換遊戲時間速度
    void SwitchGameState();

    // 暫停選單
    void ESCMenu();
    bool is_Pausing = false;

    std::shared_ptr<BackgroundImage> m_GameField;

    bool      m_IsDraggingMap = false;
    bool      m_RequestExit   = false;
    glm::vec2 m_LastMousePos  = {0.f, 0.f};
    float     m_ZoomRatio     = 1.0f;  // 累積縮放倍率，供生成新卡片時使用

    GameTime m_GameTime = GameTime::NORMAL;

    UIManager*   m_UIManager   = nullptr;
    CardManager* m_CardManager = nullptr;

    static float s_ScaledDtMs;  // 每幀更新；經過 GameTime 倍率調整後的 dtMs
};

#endif //STACKLANDS_EVENTMANAGER_HPP
