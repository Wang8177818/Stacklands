#include "ui/CheatMenu.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/Logger.hpp"
#include <algorithm>

CheatMenu::CheatMenu(Util::Renderer& renderer)
    : m_Renderer(renderer) {}

void CheatMenu::SetCardNames(const std::vector<std::string>& names) {
    // 清除舊的 entry 文字物件
    for (auto& e : m_Entries) {
        if (e.text) {
            e.text->SetVisible(false);
            e.text->m_Transform.translation = {-9999, -9999};
        }
    }
    m_Entries.clear();
    m_Built = false;
    m_ScrollOffset = 0.0f;

    for (const auto& name : names) {
        Entry entry;
        entry.name = name;
        entry.text = nullptr;
        m_Entries.push_back(std::move(entry));
    }

    // 計算最大滾動量
    float contentHeight = static_cast<float>(m_Entries.size()) * ROW_HEIGHT;
    float viewHeight = PANEL_TOP - PANEL_BOTTOM - TITLE_HEIGHT;
    m_MaxScroll = std::max(0.0f, contentHeight - viewHeight);

    if (m_Visible) Build();
}

void CheatMenu::Build() {
    if (m_Built) return;
    m_Built = true;
    // 面板背景（blackBG.png = 128x128 純黑）
    if (!m_PanelBg) {
        m_PanelBg = std::make_shared<BackgroundImage>();
        m_PanelBg->SetDrawable(std::make_shared<Util::Image>(
            RESOURCE_DIR"/Image/background/blackBG.png"));
        m_PanelBg->SetZIndex(Z_PANEL);
        m_PanelBg->m_Transform.translation = {PANEL_X, 0.0f};
        // blackBG 是 128x128，scale 算出目標尺寸
        m_PanelBg->m_Transform.scale = {
            PANEL_W / 128.0f,
            (PANEL_TOP - PANEL_BOTTOM) / 128.0f};
        m_Renderer.AddChild(m_PanelBg);
    }

    // 標題
    if (!m_Title) {
        m_Title = std::make_shared<Util::GameObject>();
        m_Title->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjhbd.ttc", 26, "-- Cheat Menu --",
            Util::Color(255, 255, 100)));
        m_Title->SetZIndex(Z_TEXT);
        m_Title->m_Transform.translation = {PANEL_X, PANEL_TOP - TITLE_HEIGHT * 0.5f};
        m_Title->m_Transform.scale = {0.5f, 0.5f};
        m_Renderer.AddChild(m_Title);
    }

    // 懸停高亮條（淺米色 dark_bg.png 1x1）
    if (!m_Highlight) {
        m_Highlight = std::make_shared<BackgroundImage>();
        m_Highlight->SetDrawable(std::make_shared<Util::Image>(
            RESOURCE_DIR"/Image/button/dark_bg.png"));
        m_Highlight->SetZIndex(Z_HOVER);
        m_Highlight->m_Transform.scale = {PANEL_W - 10.0f, ROW_HEIGHT};
        m_Highlight->SetVisible(false);
        m_Renderer.AddChild(m_Highlight);
    }

    // 建立每個卡片名稱的文字物件
    for (auto& entry : m_Entries) {
        if (!entry.text) {
            entry.text = std::make_shared<Util::GameObject>();
            entry.text->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjhbd.ttc", static_cast<int>(TEXT_SIZE),
                entry.name, Util::Color(255, 255, 255)));
            entry.text->SetZIndex(Z_TEXT);
            entry.text->m_Transform.scale = {0.5f, 0.5f};
            m_Renderer.AddChild(entry.text);
        }
    }

    UpdatePositions();
    SetAllVisible(m_Visible);
}

void CheatMenu::UpdatePositions() {
    float listTop = PANEL_TOP - TITLE_HEIGHT;

    for (size_t i = 0; i < m_Entries.size(); ++i) {
        float y = listTop - ROW_HEIGHT * 0.5f
                  - static_cast<float>(i) * ROW_HEIGHT
                  + m_ScrollOffset;
        m_Entries[i].text->m_Transform.translation = {PANEL_X, y};

        bool inView = (y > PANEL_BOTTOM + ROW_HEIGHT * 0.3f) &&
                      (y < listTop - ROW_HEIGHT * 0.3f);
        m_Entries[i].text->SetVisible(m_Visible && inView);
    }
}

void CheatMenu::SetAllVisible(bool v) {
    if (m_PanelBg)   m_PanelBg->SetVisible(v);
    if (m_Title)     m_Title->SetVisible(v);
    if (m_Highlight) m_Highlight->SetVisible(false);

    if (v) {
        UpdatePositions();
    } else {
        for (auto& e : m_Entries)
            if (e.text) e.text->SetVisible(false);
    }
}

void CheatMenu::Toggle() {
    m_Visible = !m_Visible;
    if (m_Visible && !m_Built) Build();
    SetAllVisible(m_Visible);
}

void CheatMenu::Hide() {
    m_Visible = false;
    SetAllVisible(false);
}

bool CheatMenu::IsMouseInPanel(glm::vec2 mousePos) const {
    float left   = PANEL_X - PANEL_W * 0.5f;
    float right  = PANEL_X + PANEL_W * 0.5f;
    return mousePos.x >= left && mousePos.x <= right &&
           mousePos.y >= PANEL_BOTTOM && mousePos.y <= PANEL_TOP;
}

std::string CheatMenu::Update(glm::vec2 mousePos) {
    if (!m_Visible) return "";

    // 滾動
    if (IsMouseInPanel(mousePos)) {
        float scroll = Util::Input::GetScrollDistance().y;
        if (scroll != 0.0f) {
            m_ScrollOffset += scroll * ROW_HEIGHT * 3.0f;
            m_ScrollOffset = std::clamp(m_ScrollOffset, 0.0f, m_MaxScroll);
            UpdatePositions();
        }
    }

    // 懸停 & 點擊
    float listTop = PANEL_TOP - TITLE_HEIGHT;
    std::string clickedName;
    bool anyHover = false;

    for (size_t i = 0; i < m_Entries.size(); ++i) {
        float y = listTop - ROW_HEIGHT * 0.5f
                  - static_cast<float>(i) * ROW_HEIGHT
                  + m_ScrollOffset;
        bool inView = (y > PANEL_BOTTOM + ROW_HEIGHT * 0.3f) &&
                      (y < listTop - ROW_HEIGHT * 0.3f);
        if (!inView) continue;

        float left  = PANEL_X - PANEL_W * 0.5f;
        float right = PANEL_X + PANEL_W * 0.5f;
        bool hover = mousePos.x >= left && mousePos.x <= right &&
                     mousePos.y >= y - ROW_HEIGHT * 0.5f &&
                     mousePos.y <= y + ROW_HEIGHT * 0.5f;

        if (hover) {
            anyHover = true;
            m_Highlight->m_Transform.translation = {PANEL_X, y};
            m_Highlight->SetVisible(true);

            if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
                clickedName = m_Entries[i].name;
            }
        }
    }

    if (!anyHover && m_Highlight) {
        m_Highlight->SetVisible(false);
    }

    return clickedName;
}
