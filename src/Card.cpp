//
// Created by m0938 on 2026/3/13.
//

#include "Card.hpp"

// 在建構子接收 scale 參數，並存入 m_Scale
Card::Card(float x, float y, const std::string& name, int sellValue, CardType type, float scale)
    : m_X(x), m_Y(y), m_Name(name), m_SellValue(sellValue), m_Type(type), m_Scale(scale), m_IsDragging(false)
{
    //底圖
    m_Background = std::make_shared<Util::GameObject>();
    m_Background->SetZIndex(10); // 預設圖層高度

    //圖示
    m_Icon = std::make_shared<Util::GameObject>();
    m_Icon->SetZIndex(11); // 圖示要蓋在底圖上

    //名稱
    m_NameText = std::make_shared<Util::GameObject>();
    
    //字體大小少大於22
    int fontSize = static_cast<int>(1000 * m_Scale);
    if (fontSize < 22) fontSize = 22;
    m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", fontSize, m_Name, Util::Color(0, 0, 0)));
    m_NameText->SetZIndex(11);

    //縮放
    glm::vec2 card_scale = {m_Scale, m_Scale};
    m_Background->m_Transform.scale = card_scale * 2.0f;
    m_Icon->m_Transform.scale = card_scale * 0.6f;
    m_NameText->m_Transform.scale = card_scale;

    float baseWidth = 850.0f;
    float baseHeight = 1250.0f;

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
        glm::vec2 mousePos = Util::Input::GetCursorPosition();
        m_X = mousePos.x + m_DragOffset.x;
        m_Y = mousePos.y + m_DragOffset.y;

        UpdateVisualPositions();
    }
    // 吸附並跟隨
    else if (m_CardBelow != nullptr) {
        // 1. 先算出「目標座標」(底卡的中心點往下偏移 10%)
        float targetX = m_CardBelow->m_X;
        float targetY = m_CardBelow->m_Y - (m_Height * 0.15f);

        //移動延遲
        float followSpeed = 0.3f;
        m_X += (targetX - m_X) * followSpeed;
        m_Y += (targetY - m_Y) * followSpeed;

        // 3. 圖層調整
        int baseZ = 10;
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

    // 對方的邊界
    float padding = m_Width * 0.2f; // 容錯
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
    m_Background->SetZIndex(40);
    m_Icon->SetZIndex(41);
    m_NameText->SetZIndex(41);
}

void Card::StopDragging() {
    m_IsDragging = false;
    // 放開後恢復原本的圖層高度
    m_Background->SetZIndex(10);
    m_Icon->SetZIndex(11);
    m_NameText->SetZIndex(12);
}

void Card::UpdateVisualPositions() { // 排版
    if (m_Background) {
        m_Background->m_Transform.translation = glm::vec2(m_X, m_Y);
    }

    if (m_Icon) {
        float iconOffsetY = m_Height * 0;
        m_Icon->m_Transform.translation = glm::vec2(m_X, m_Y + iconOffsetY);
    }

    if (m_NameText) {
        float textOffsetY = m_Height * 0.35;
        float textWidth = m_NameText->GetScaledSize().x;

        float padding = m_Width * 0.1f;
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

void Card::SetScale(float scale) {
    m_Scale = scale;
    glm::vec2 card_scale = {m_Scale, m_Scale};
    m_Background->m_Transform.scale = card_scale * 2.0f;
    m_Icon->m_Transform.scale       = card_scale * 0.6f;
    m_NameText->m_Transform.scale   = card_scale;

    float baseWidth  = 850.0f;
    float baseHeight = 1250.0f;
    m_Width  = baseWidth  * m_Scale;
    m_Height = baseHeight * m_Scale;

    UpdateVisualPositions();
}

bool Card::OnStacked(std::shared_ptr<Card> /*cardAbove*/) {
    return false; // 預設不接受堆疊
}

void Card::OnMonthEnd() {
    // 預設什麼都不做
}