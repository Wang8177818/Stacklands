//
// Created by user on 2026/3/27.
//

#include "EventManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// ─────────────────────────────────────────────────────────────
void EventManager::SetGameField(std::shared_ptr<BackgroundImage> gameField) {
    m_GameField = gameField;
}

// ─────────────────────────────────────────────────────────────
void EventManager::Update(glm::vec2 mousePos,
                           bool isDraggingCard,
                           std::vector<std::shared_ptr<Card>>& cards) {
    if (!m_GameField) return;

    HandlePan(mousePos, isDraggingCard, cards);
    HandleZoom(cards);
    HandleWASD(cards);
}

// ─────────────────────────────────────────────────────────────
void EventManager::HandlePan(glm::vec2 mousePos,
                               bool isDraggingCard,
                               std::vector<std::shared_ptr<Card>>& cards) {
    if (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB) && !isDraggingCard) {
        if (!m_IsDraggingMap) {
            m_IsDraggingMap = true;
            m_LastMousePos  = mousePos;
        } else {
            glm::vec2 delta = mousePos - m_LastMousePos;
            if (delta.x != 0.f || delta.y != 0.f) {
                MoveAll(delta, cards);
            }
            m_LastMousePos = mousePos;
        }
    } else {
        m_IsDraggingMap = false;
    }
}

// ─────────────────────────────────────────────────────────────
// 滾輪縮放
// 以畫面中心 (0,0) 為錨點，讓背景圖與卡片的邏輯座標同步等比縮放。
// 卡片透過 ScaleAroundPivot() 同時修改 m_X, m_Y 與 m_Scale，
// 確保 UpdateVisualPositions() 每幀覆寫時位置仍然正確。
void EventManager::HandleZoom(std::vector<std::shared_ptr<Card>>& cards) {
    if (!Util::Input::IfScroll()) return;

    glm::vec2 scroll   = Util::Input::GetScrollDistance();
    float     oldScale = m_GameField->m_Transform.scale.x;
    float     newScale = oldScale;

    if      (scroll.y > 0 && oldScale < ZOOM_MAX) newScale = oldScale + ZOOM_STEP;
    else if (scroll.y < 0 && oldScale > ZOOM_MIN) newScale = oldScale - ZOOM_STEP;
    else return;

    const float     ratio = newScale / oldScale;
    const glm::vec2 pivot = {0.f, 0.f}; // 畫面中心為縮放錨點

    // 背景圖：縮放 + 錨點等比位移
    m_GameField->SetScale({newScale, newScale});
    m_GameField->m_Transform.translation =
        pivot + (m_GameField->m_Transform.translation - pivot) * ratio;

    // 卡片：透過邏輯座標介面同步，避免被 UpdateVisualPositions() 覆蓋
    for (auto& card : cards) {
        card->ScaleAroundPivot(ratio, pivot);
    }

    LOG_DEBUG("Zoom {} -> {:.3f}", (scroll.y > 0 ? "in " : "out"), newScale);
}

// ─────────────────────────────────────────────────────────────
void EventManager::HandleWASD(std::vector<std::shared_ptr<Card>>& cards) {
    glm::vec2 delta = {0.f, 0.f};

    if (Util::Input::IsKeyPressed(Util::Keycode::W)) delta.y -= WASD_SPEED;
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) delta.y += WASD_SPEED;
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) delta.x += WASD_SPEED;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) delta.x -= WASD_SPEED;

    if (delta.x != 0.f || delta.y != 0.f) {
        MoveAll(delta, cards);
    }
}

// ─────────────────────────────────────────────────────────────
// 同時移動背景圖與所有卡片（Pan / WASD 共用）
// 卡片透過 MoveBy() 修改 m_X, m_Y，而非直接動 GameObject translation。
void EventManager::MoveAll(glm::vec2 delta,
                             std::vector<std::shared_ptr<Card>>& cards) {
    m_GameField->m_Transform.translation += delta;

    for (auto& card : cards) {
        card->MoveBy(delta);
    }
}
