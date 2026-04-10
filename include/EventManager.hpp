//
// Created by user on 2026/3/27.
//

#ifndef STACKLANDS_EVENTMANAGER_HPP
#define STACKLANDS_EVENTMANAGER_HPP

#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "BackgroundImage.hpp"
#include "Card.hpp"
#include "UIManager.hpp"

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

    // ── 縮放邊界常數 ──────────────────────────────────────
    static constexpr float ZOOM_MIN      = 0.5f;
    static constexpr float ZOOM_MAX      = 2.0f;
    static constexpr float ZOOM_STEP     = 0.01f;
    static constexpr float WASD_SPEED    = 3.0f;

    EventManager() = default;

    // 綁定需要跟著移動/縮放的場景背景圖
    void SetGameField(std::shared_ptr<BackgroundImage> gameField);

    // 綁定 UIManager（不擁有，只借用）
    void SetUIManager(UIManager* uiManager) { m_UIManager = uiManager; }

    // 每幀呼叫
    // mousePos        — 當前滑鼠座標
    // isDraggingCard  — CardManager 是否正在拖曳卡片 (若是，則不能拖曳地圖)
    // cards           — 場上所有卡片 (移動/縮放時一起搬)
    void Update(glm::vec2 mousePos,
                bool isDraggingCard,
                std::vector<std::shared_ptr<Card>>& cards);

    // 查詢目前是否正在拖曳地圖
    bool IsDraggingMap() const { return m_IsDraggingMap; }

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
    glm::vec2 m_LastMousePos  = {0.f, 0.f};

    GameTime m_GameTime = GameTime::NORMAL;

    UIManager* m_UIManager = nullptr;
};

#endif //STACKLANDS_EVENTMANAGER_HPP
