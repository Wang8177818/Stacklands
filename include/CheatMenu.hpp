#ifndef STACKLANDS_CHEATMENU_HPP
#define STACKLANDS_CHEATMENU_HPP

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "BackgroundImage.hpp"

class CheatMenu {
public:
    CheatMenu(Util::Renderer& renderer);

    // 設定可生成的卡片名稱清單
    void SetCardNames(const std::vector<std::string>& names);

    // 每幀更新：處理滾動與懸停，回傳被點擊的卡片名稱（空字串 = 無點擊）
    std::string Update(glm::vec2 mousePos);

    // 開關作弊選單
    void Toggle();
    bool IsVisible() const { return m_Visible; }

    // 隱藏全部（Game Over 回選單時用）
    void Hide();

private:
    Util::Renderer& m_Renderer;
    bool m_Visible = false;
    bool m_Built   = false;  // 是否已建立 UI 物件

    // 面板配置
    static constexpr float PANEL_X      =  500.0f;  // 面板中心 X
    static constexpr float PANEL_W      =  220.0f;  // 面板寬度
    static constexpr float PANEL_TOP    =  280.0f;  // 面板頂端 Y
    static constexpr float PANEL_BOTTOM = -280.0f;  // 面板底端 Y
    static constexpr float ROW_HEIGHT   =   28.0f;  // 每列高度
    static constexpr float TEXT_SIZE    =   22.0f;  // 文字大小
    static constexpr int   Z_PANEL      =  98;      // 面板 Z（引擎 far clip = 100）
    static constexpr int   Z_TEXT       =  99;      // 文字 Z
    static constexpr int   Z_HOVER      =  99;      // 懸停高亮 Z

    // 面板背景
    std::shared_ptr<BackgroundImage> m_PanelBg;

    // 標題
    std::shared_ptr<Util::GameObject> m_Title;

    // 每個卡片按鈕的資料
    struct Entry {
        std::string name;
        std::shared_ptr<Util::GameObject> text;
    };
    std::vector<Entry> m_Entries;

    // 懸停高亮條
    std::shared_ptr<BackgroundImage> m_Highlight;

    // 滾動偏移（像素）
    float m_ScrollOffset = 0.0f;
    float m_MaxScroll    = 0.0f;

    // 標題列高
    static constexpr float TITLE_HEIGHT = 32.0f;

    void Build();
    void UpdatePositions();
    void SetAllVisible(bool v);

    // 判斷滑鼠是否在面板範圍內
    bool IsMouseInPanel(glm::vec2 mousePos) const;
};

#endif // STACKLANDS_CHEATMENU_HPP
