#include "CheatMenu.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/Logger.hpp"
#include <algorithm>

CheatMenu::CheatMenu(Util::Renderer& renderer)
    : m_Renderer(renderer) {}

namespace {
// CardType → 類別顯示名稱
const char* TypeLabel(CardType t) {
    switch (t) {
        case CardType::CHARACTER: return "── 人物 ──";
        case CardType::RESOURCE:  return "── 資源 ──";
        case CardType::BUILDING:  return "── 建築 ──";
        case CardType::STRUCTURE: return "── 結構 ──";
        case CardType::FOOD:      return "── 食物 ──";
        case CardType::EQUIPMENT: return "── 裝備 ──";
        case CardType::ANIMAL:    return "── 動物 ──";
        case CardType::MONSTER:   return "── 怪物 ──";
        case CardType::IDEA:      return "── 點子 ──";
        case CardType::LOCATION:  return "── 地點 ──";
        case CardType::COIN:      return "── 硬幣 ──";
        case CardType::PACK:      return "── 卡包 ──";
        default:                  return "── 其他 ──";
    }
}
// 類別顯示順序
const std::vector<CardType> kTypeOrder = {
    CardType::CHARACTER, CardType::ANIMAL,    CardType::MONSTER,
    CardType::RESOURCE,  CardType::FOOD,      CardType::EQUIPMENT,
    CardType::BUILDING,  CardType::STRUCTURE, CardType::LOCATION,
    CardType::IDEA,      CardType::COIN,      CardType::PACK,
};
} // namespace

void CheatMenu::SetCardEntries(const std::vector<std::pair<std::string, CardType>>& entries) {
    // 清理舊的
    for (auto& e : m_Entries) {
        if (e.text) {
            e.text->SetVisible(false);
            e.text->m_Transform.translation = {-9999, -9999};
        }
    }
    m_Entries.clear();
    m_Built = false;
    m_ScrollOffset = 0.0f;

    // 依類別 bucket 化（名稱已在傳入時排序）
    std::unordered_map<int, std::vector<std::string>> buckets;
    for (const auto& [name, type] : entries) {
        buckets[static_cast<int>(type)].push_back(name);
    }

    // 依 kTypeOrder 輸出：每個非空類別先加 header，再加該類所有卡
    for (CardType t : kTypeOrder) {
        auto it = buckets.find(static_cast<int>(t));
        if (it == buckets.end() || it->second.empty()) continue;
        Entry header;
        header.name     = TypeLabel(t);
        header.isHeader = true;
        m_Entries.push_back(std::move(header));
        for (auto& n : it->second) {
            Entry e;
            e.name = n;
            m_Entries.push_back(std::move(e));
        }
    }

    float contentHeight = static_cast<float>(m_Entries.size()) * ROW_HEIGHT;
    float viewHeight = PANEL_TOP - PANEL_BOTTOM - TITLE_HEIGHT;
    m_MaxScroll = std::max(0.0f, contentHeight - viewHeight);

    if (m_Visible) Build();
}

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
        m_Highlight->m_Transform.scale = {PANEL_W - 16.0f - SCROLLBAR_WIDTH,
                                          ROW_HEIGHT * ROW_HIT_RATIO};
        m_Highlight->SetVisible(false);
        m_Renderer.AddChild(m_Highlight);
    }

    // 拉條軌道（深色背板，z 同面板上方一層）
    if (!m_ScrollTrack) {
        m_ScrollTrack = std::make_shared<BackgroundImage>();
        m_ScrollTrack->SetDrawable(std::make_shared<Util::Image>(
            RESOURCE_DIR"/Image/button/darker_bg.png"));
        m_ScrollTrack->SetZIndex(Z_TRACK);
        m_Renderer.AddChild(m_ScrollTrack);
    }
    // 拉條拖把（白色：1×1 純白），z 一定要高於軌道才看得到
    if (!m_ScrollThumb) {
        m_ScrollThumb = std::make_shared<BackgroundImage>();
        m_ScrollThumb->SetDrawable(std::make_shared<Util::Image>(
            RESOURCE_DIR"/Image/background/timebarWhite.png"));
        m_ScrollThumb->SetZIndex(Z_THUMB);
        m_Renderer.AddChild(m_ScrollThumb);
    }

    // 建立每個 entry 的文字物件（類別標題用黃色字）
    for (auto& entry : m_Entries) {
        if (!entry.text) {
            entry.text = std::make_shared<Util::GameObject>();
            const Util::Color color = entry.isHeader
                ? Util::Color(255, 220, 100)   // 類別標題：金黃
                : Util::Color(255, 255, 255);  // 卡片名稱：白
            entry.text->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjhbd.ttc", static_cast<int>(TEXT_SIZE),
                entry.name, color));
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
    UpdateScrollbar();
}

void CheatMenu::UpdateScrollbar() {
    if (!m_ScrollTrack || !m_ScrollThumb) return;

    const float trackTop    = PANEL_TOP - TITLE_HEIGHT;
    const float trackBottom = PANEL_BOTTOM;
    const float trackHeight = trackTop - trackBottom;
    const float trackX      = PANEL_X + PANEL_W * 0.5f - SCROLLBAR_WIDTH * 0.5f;
    const float trackY      = (trackTop + trackBottom) * 0.5f;

    // 軌道（永遠固定大小，貼右邊）
    m_ScrollTrack->m_Transform.translation = {trackX, trackY};
    m_ScrollTrack->m_Transform.scale       = {SCROLLBAR_WIDTH, trackHeight};
    m_ScrollTrack->SetVisible(m_Visible);

    // 拖把：高度依「可視 / 總內容」比例；位置依 scrollOffset
    const float viewHeight    = trackHeight;
    const float contentHeight = static_cast<float>(m_Entries.size()) * ROW_HEIGHT;
    const float ratio = (contentHeight > viewHeight && contentHeight > 0.f)
                       ? viewHeight / contentHeight : 1.f;
    const float thumbHeight = std::max(THUMB_MIN_HEIGHT, ratio * trackHeight);
    const float thumbYMax = trackTop    - thumbHeight * 0.5f;
    const float thumbYMin = trackBottom + thumbHeight * 0.5f;
    const float scrollRatio = (m_MaxScroll > 0.f) ? (m_ScrollOffset / m_MaxScroll) : 0.f;
    const float thumbY = thumbYMax - scrollRatio * (thumbYMax - thumbYMin);

    m_ScrollThumb->m_Transform.translation = {trackX, thumbY};
    m_ScrollThumb->m_Transform.scale       = {SCROLLBAR_WIDTH, thumbHeight};
    // 內容不需要滾動時隱藏拖把
    m_ScrollThumb->SetVisible(m_Visible && m_MaxScroll > 0.f);
}

bool CheatMenu::IsMouseOnThumb(glm::vec2 mousePos) const {
    if (!m_ScrollThumb || m_MaxScroll <= 0.f || !m_Visible) return false;
    const float tx = m_ScrollThumb->m_Transform.translation.x;
    const float ty = m_ScrollThumb->m_Transform.translation.y;
    const float halfW = m_ScrollThumb->m_Transform.scale.x * 0.5f;
    const float halfH = m_ScrollThumb->m_Transform.scale.y * 0.5f;
    return mousePos.x >= tx - halfW && mousePos.x <= tx + halfW &&
           mousePos.y >= ty - halfH && mousePos.y <= ty + halfH;
}

void CheatMenu::SetAllVisible(bool v) {
    if (m_PanelBg)   m_PanelBg->SetVisible(v);
    if (m_Title)     m_Title->SetVisible(v);
    if (m_Highlight) m_Highlight->SetVisible(false);
    if (m_ScrollTrack) m_ScrollTrack->SetVisible(v);
    if (m_ScrollThumb) m_ScrollThumb->SetVisible(v && m_MaxScroll > 0.f);

    if (v) {
        UpdatePositions();
    } else {
        for (auto& e : m_Entries)
            if (e.text) e.text->SetVisible(false);
        m_IsDraggingThumb = false;
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

    // 滾動（滑鼠滾輪）— 要先檢查 IfScroll，因為 PTSD 的 GetScrollDistance
    // 不會每幀清零，直接讀會持續累加上次的值
    if (Util::Input::IfScroll() && IsMouseInPanel(mousePos)) {
        float scroll = Util::Input::GetScrollDistance().y;
        if (scroll != 0.0f) {
            // 滾輪向上 → scroll.y > 0，期望「往上看」= 列表往上拉 = scrollOffset 減少
            m_ScrollOffset -= scroll * ROW_HEIGHT * 3.0f;
            m_ScrollOffset = std::clamp(m_ScrollOffset, 0.0f, m_MaxScroll);
            UpdatePositions();
        }
    }

    // ── 拉條拖曳 ────────────────────────────────────────────
    // 結束拖曳（先檢查，避免本幀的 KeyUp 同時觸發卡片點擊）
    if (m_IsDraggingThumb && Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        m_IsDraggingThumb = false;
        return "";
    }
    // 拖曳中：每幀依 mouse.y 計算新 scrollOffset
    if (m_IsDraggingThumb) {
        const float trackTop    = PANEL_TOP - TITLE_HEIGHT;
        const float trackBottom = PANEL_BOTTOM;
        const float trackHeight = trackTop - trackBottom;
        const float viewHeight    = trackHeight;
        const float contentHeight = static_cast<float>(m_Entries.size()) * ROW_HEIGHT;
        const float ratio = (contentHeight > viewHeight && contentHeight > 0.f)
                           ? viewHeight / contentHeight : 1.f;
        const float thumbHeight = std::max(THUMB_MIN_HEIGHT, ratio * trackHeight);
        const float thumbYMax = trackTop    - thumbHeight * 0.5f;
        const float thumbYMin = trackBottom + thumbHeight * 0.5f;
        const float thumbRange = thumbYMax - thumbYMin;

        if (thumbRange > 0.f && m_MaxScroll > 0.f) {
            const float targetY = std::clamp(mousePos.y - m_DragGrabOffsetY,
                                             thumbYMin, thumbYMax);
            const float r = (thumbYMax - targetY) / thumbRange;
            m_ScrollOffset = std::clamp(r * m_MaxScroll, 0.f, m_MaxScroll);
            UpdatePositions();
        }
        return "";
    }
    // 開始拖曳？
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB) && IsMouseOnThumb(mousePos)) {
        m_IsDraggingThumb = true;
        m_DragGrabOffsetY = mousePos.y - m_ScrollThumb->m_Transform.translation.y;
        return "";
    }

    // 懸停 & 點擊
    float listTop = PANEL_TOP - TITLE_HEIGHT;
    std::string clickedName;
    bool anyHover = false;

    for (size_t i = 0; i < m_Entries.size(); ++i) {
        if (m_Entries[i].isHeader) continue;  // 類別標題不能點

        float y = listTop - ROW_HEIGHT * 0.5f
                  - static_cast<float>(i) * ROW_HEIGHT
                  + m_ScrollOffset;
        bool inView = (y > PANEL_BOTTOM + ROW_HEIGHT * 0.3f) &&
                      (y < listTop - ROW_HEIGHT * 0.3f);
        if (!inView) continue;

        // 點擊區比 ROW_HEIGHT 小一圈、且避開右側拉條
        const float hitHalfW = (PANEL_W - 16.0f - SCROLLBAR_WIDTH) * 0.5f;
        const float hitHalfH = ROW_HEIGHT * ROW_HIT_RATIO * 0.5f;
        // 列以面板中心扣掉拉條寬的方式置中
        const float rowCenterX = PANEL_X - SCROLLBAR_WIDTH * 0.5f;
        bool hover = mousePos.x >= rowCenterX - hitHalfW &&
                     mousePos.x <= rowCenterX + hitHalfW &&
                     mousePos.y >= y - hitHalfH &&
                     mousePos.y <= y + hitHalfH;

        if (hover) {
            anyHover = true;
            m_Highlight->m_Transform.translation = {rowCenterX, y};
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
