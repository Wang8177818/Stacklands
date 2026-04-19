//
// Created by m0938 on 2026/4/18.
//

#ifndef STACKLANDS_GAMECONSTANTS_HPP
#define STACKLANDS_GAMECONSTANTS_HPP

#pragma once

namespace GameConstants {

    // ── 卡牌基礎尺寸 ──────────────────────────────────────────────
    constexpr float BASE_CARD_WIDTH  = 850.0f;
    constexpr float BASE_CARD_HEIGHT = 1250.0f;

    // ── 文字大小基數（fontSize = max(1, SCALE * m_Scale)）────────
    constexpr int CARD_FONT_SCALE = 1000;  // 卡片文字
    constexpr int PACK_FONT_SCALE = 3000;  // 卡包文字

    // ── 標籤位置偏移比例（相對卡牌寬/高）───────────────────────────
    constexpr float PRICE_OFFSET_X     = -0.32f; // 售價 X
    constexpr float PRICE_OFFSET_Y     = -0.3f;  // 售價 Y
    constexpr float SECONDARY_OFFSET_X =  0.32f; // 營養值 X (Y跟PRICE用)
    constexpr float HEALTH_OFFSET_X    =  0.34f; // 血量 (Y跟PRICE用)

    // ── Z-Index ─────────────────────────────────────────────────
    constexpr int Z_BACKGROUND  = 10; // 卡牌底圖
    constexpr int Z_ICON        = 11; // 圖、文字
    constexpr int Z_DRAG_BG     = 40; // 拖曳：底圖
    constexpr int Z_DRAG_TEXT   = 41; // 拖曳：圖/名稱
    constexpr int Z_DRAG_EXTRA  = 42; // 拖曳：文字

    // ── 卡牌縮放係數 ──────────────────────────────────────────────
    constexpr float ICON_SCALE_FACTOR = 0.6f;

    // ── 堆疊動畫參數 ──────────────────────────────────────────────
    constexpr float STACK_OFFSET_Y = -0.15f; // 堆疊垂直偏移（相對自身高度）
    constexpr int   STACK_Z_STEP   = 3;     // 每層 Z 增量
    constexpr float FOLLOW_SPEED   = 0.3f;  // 延遲跟隨

    // ── 遊戲時間 ──────────────────────────────────────────────────
    constexpr float MONTH_DURATION_MS  = 120000.0f; // 一個月的毫秒數
    constexpr float PROGRESS_BAR_WIDTH = 265.0f;    // 月份進度條像素寬度

    // ── 採集時間 ──────────────────────────────────────────────────
    constexpr float GATHER_TIME_MS = 10000.0f; // 採集等待時間

} // namespace GameConstants

// ── 字體路徑 ──────────────────────────────────────────────────────
#define FONT_REGULAR RESOURCE_DIR"/Font/msjh.ttc"
#define FONT_BOLD    RESOURCE_DIR"/Font/msjhbd.ttc"

#endif // STACKLANDS_GAMECONSTANTS_HPP
