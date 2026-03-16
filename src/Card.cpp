//
// Created by m0938 on 2026/3/13.
//

#include "Card.hpp"

// 在建構子接收 scale 參數，並存入 m_Scale
Card::Card(float x, float y, const std::string& name, int sellValue, CardType type, float scale)
    : m_X(x), m_Y(y), m_Name(name), m_SellValue(sellValue), m_Type(type), m_Scale(scale), m_IsDragging(false)
{
    // 1. 建立底圖物件
    m_Background = std::make_shared<Util::GameObject>();
    m_Background->SetZIndex(10); // 預設圖層高度

    // 2. 建立圖示物件
    m_Icon = std::make_shared<Util::GameObject>();
    m_Icon->SetZIndex(11); // 圖示要蓋在底圖上

    // 3. 建立名稱文字物件
    m_NameText = std::make_shared<Util::GameObject>();

    // 【防護】確保字體大小為整數，且至少大於 10，避免引擎崩潰
    int fontSize = static_cast<int>(500 * m_Scale);
    if (fontSize < 10) fontSize = 10;

    m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", fontSize, m_Name, Util::Color(0, 0, 0)));
    m_NameText->SetZIndex(11); // 文字在最上層

    // ===== 根據傳入的 scale 設定縮放 =====
    glm::vec2 card_scale = {m_Scale, m_Scale};
    m_Background->m_Transform.scale = card_scale;
    m_Icon->m_Transform.scale = card_scale * glm::vec2{0.7f, 0.7f};
    m_NameText->m_Transform.scale = card_scale;

    float baseWidth = 1200.0f;
    float baseHeight = 1800.0f;

    m_Width = baseWidth * m_Scale;
    m_Height = baseHeight * m_Scale;

    // 初始化座標位置
    UpdateVisualPositions();
}

void Card::SetBackgroundImage(const std::string& imagePath) {
    if (m_Background) {
        m_Background->SetDrawable(std::make_shared<Util::Image>(imagePath));
    }
}

void Card::SetIconImage(const std::string& imagePath) {
    if (m_Icon) {
        m_Icon->SetDrawable(std::make_shared<Util::Image>(imagePath));
    }
}

void Card::Update() {
    if (m_IsDragging) {
        // --- 正在被玩家拖曳 ---
        glm::vec2 mousePos = Util::Input::GetCursorPosition();
        m_X = mousePos.x + m_DragOffset.x;
        m_Y = mousePos.y + m_DragOffset.y;

        UpdateVisualPositions();
    }
    else if (m_CardBelow != nullptr) {
        // --- 【新增】沒有被拖曳，但底下有卡片：吸附並跟隨！ ---
        // X 座標完全對齊底下的卡片
        m_X = m_CardBelow->m_X;
        // Y 座標往下偏移一點點 (例如卡牌高度的 15%)，這樣才看得到底下的卡
        m_Y = m_CardBelow->m_Y - (m_Height * 0.15f);

        // 圖層自動比底下的卡高一層，確保不會被蓋住
        int baseZ = 10; // 基礎 Z-Index
        // 簡單的圖層遞增邏輯，如果需要更嚴謹未來可以再改
        m_Background->SetZIndex(m_CardBelow->m_Background->GetZIndex() + 3);
        m_Icon->SetZIndex(m_CardBelow->m_Icon->GetZIndex() + 3);
        m_NameText->SetZIndex(m_CardBelow->m_NameText->GetZIndex() + 3);

        UpdateVisualPositions();
    }
}

bool Card::IsMouseHovering(glm::vec2 mousePos) {
    // 計算卡牌的四個邊界
    float left = m_X - m_Width / 2;
    float right = m_X + m_Width / 2;
    float top = m_Y + m_Height / 2;
    float bottom = m_Y - m_Height / 2;

    // 判斷滑鼠座標是否落在邊界內
    return (mousePos.x >= left && mousePos.x <= right &&
            mousePos.y >= bottom && mousePos.y <= top);
}

bool Card::IsOverlapping(std::shared_ptr<Card> otherCard) {
    if (!otherCard) return false;

    // 我的邊界
    float l1 = m_X - m_Width / 2;
    float r1 = m_X + m_Width / 2;
    float t1 = m_Y + m_Height / 2;
    float b1 = m_Y - m_Height / 2;

    // 對方的邊界 (為了避免碰到一點點邊緣就吸附，可以把對方的判定範圍稍微縮小一點)
    float padding = m_Width * 0.2f;
    float l2 = otherCard->m_X - otherCard->m_Width / 2 + padding;
    float r2 = otherCard->m_X + otherCard->m_Width / 2 - padding;
    float t2 = otherCard->m_Y + otherCard->m_Height / 2 - padding;
    float b2 = otherCard->m_Y - otherCard->m_Height / 2 + padding;

    // 如果沒有在上下左右的外部，就是重疊了！
    return !(l1 > r2 || r1 < l2 || t1 < b2 || b1 > t2);
}

void Card::StartDragging(glm::vec2 mousePos) {
    m_IsDragging = true;

    // 計算滑鼠點擊點與卡牌中心的距離差
    m_DragOffset = glm::vec2(m_X, m_Y) - mousePos;

    // 把正在拖曳的卡牌移到最上層，才不會被其他卡牌擋住
    m_Background->SetZIndex(50);
    m_Icon->SetZIndex(51);
    m_NameText->SetZIndex(51);
}

void Card::StopDragging() {
    m_IsDragging = false;

    // 放開後恢復原本的圖層高度
    m_Background->SetZIndex(10);
    m_Icon->SetZIndex(11);
    m_NameText->SetZIndex(12);
}

// ==========================================
// 內部工具與預設方法
// ==========================================
void Card::UpdateVisualPositions() {
    // 1. 更新底圖位置 (正中心)
    if (m_Background) {
        m_Background->m_Transform.translation = glm::vec2(m_X, m_Y);
    }

    // 2. 更新圖示位置 (相對排版)
    // 根據卡牌整體高度 m_Height 來算相對位置，不管怎麼縮放都不會跑版
    if (m_Icon) {
        float iconOffsetY = m_Height * 0;
        m_Icon->m_Transform.translation = glm::vec2(m_X, m_Y + iconOffsetY);
    }

    // 3. 更新文字位置 (相對排版)
    if (m_NameText) {
        float textOffsetY = m_Height * 0.25f;
        m_NameText->m_Transform.translation = glm::vec2(m_X, m_Y + textOffsetY);
        float textWidth = m_NameText->GetScaledSize().x;

        float padding = m_Width * 0.2f;
        float textX = (m_X - m_Width / 2.0f) + (textWidth / 2.0f) + padding;
        m_NameText->m_Transform.translation = glm::vec2(textX, m_Y + textOffsetY);
    }
}

// 回傳 GameObject 給 App 渲染
std::vector<std::shared_ptr<Util::GameObject>> Card::GetGameObjects() {
    std::vector<std::shared_ptr<Util::GameObject>> objs;

    if (m_Background) objs.push_back(m_Background);
    if (m_Icon) objs.push_back(m_Icon);
    if (m_NameText) objs.push_back(m_NameText);

    return objs;
}

bool Card::OnStacked(std::shared_ptr<Card> /*cardAbove*/) {
    return false; // 預設不接受堆疊
}

void Card::OnMonthEnd() {
    // 預設什麼都不做
}