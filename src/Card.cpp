//
// Created by m0938 on 2026/3/13.
//

#include "Card.hpp"
#include <algorithm>

Card::Card(float x, float y, const std::string& name, int sellValue, CardType type, float scale)
    : m_X(x), m_Y(y), m_Name(name), m_SellValue(sellValue), m_Type(type), m_Scale(scale), m_IsDragging(false)
{
    //底圖
    m_Background = std::make_shared<Util::GameObject>();
    m_Background->SetZIndex(GameConstants::Z_BACKGROUND);

    //圖示
    m_Icon = std::make_shared<Util::GameObject>();
    m_Icon->SetZIndex(GameConstants::Z_ICON);

    //名稱
    m_NameText = InitLabelText(m_Name, Util::Color(0, 0, 0));

    //縮放
    glm::vec2 card_scale = {m_Scale, m_Scale};
    m_Background->m_Transform.scale = card_scale * 2.0f;
    m_Icon->m_Transform.scale       = card_scale * GameConstants::ICON_SCALE_FACTOR;

    m_Width  = GameConstants::BASE_CARD_WIDTH  * m_Scale;
    m_Height = GameConstants::BASE_CARD_HEIGHT * m_Scale;

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
        // 目標座標
        float targetX = m_CardBelow->m_X;
        float targetY = m_CardBelow->m_Y + (m_Height * GameConstants::STACK_OFFSET_Y);

        // 移動延遲
        m_X += (targetX - m_X) * GameConstants::FOLLOW_SPEED;
        m_Y += (targetY - m_Y) * GameConstants::FOLLOW_SPEED;

        // 圖層調整
        m_Background->SetZIndex(m_CardBelow->m_Background->GetZIndex() + GameConstants::STACK_Z_STEP);
        m_Icon->SetZIndex(m_CardBelow->m_Icon->GetZIndex()             + GameConstants::STACK_Z_STEP);
        m_NameText->SetZIndex(m_CardBelow->m_NameText->GetZIndex()     + GameConstants::STACK_Z_STEP);

        UpdateVisualPositions();
    }
}

bool Card::IsMouseHovering(glm::vec2 mousePos) {
    float left   = m_X - m_Width  / 2;
    float right  = m_X + m_Width  / 2;
    float top    = m_Y + m_Height / 2;
    float bottom = m_Y - m_Height / 2;

    return (mousePos.x >= left && mousePos.x <= right &&
            mousePos.y >= bottom && mousePos.y <= top);
}

bool Card::IsOverlapping(std::shared_ptr<Card> otherCard) {
    if (!otherCard) return false;

    float l1 = m_X - m_Width  / 2;
    float r1 = m_X + m_Width  / 2;
    float t1 = m_Y + m_Height / 2;
    float b1 = m_Y - m_Height / 2;

    float l2 = otherCard->m_X - otherCard->m_Width  / 2;
    float r2 = otherCard->m_X + otherCard->m_Width  / 2;
    float t2 = otherCard->m_Y + otherCard->m_Height / 2;
    float b2 = otherCard->m_Y - otherCard->m_Height / 2;

    return !(l1 > r2 || r1 < l2 || t1 < b2 || b1 > t2);
}

void Card::StartDragging(glm::vec2 mousePos) {
    m_IsDragging = true;
    m_DragOffset = glm::vec2(m_X, m_Y) - mousePos;

    m_Background->SetZIndex(GameConstants::Z_DRAG_BG);
    m_Icon->SetZIndex(GameConstants::Z_DRAG_TEXT);
    m_NameText->SetZIndex(GameConstants::Z_DRAG_TEXT);
}

void Card::StopDragging() {
    m_IsDragging = false;
    m_Background->SetZIndex(GameConstants::Z_BACKGROUND);
    m_Icon->SetZIndex(GameConstants::Z_ICON);
    m_NameText->SetZIndex(GameConstants::Z_ICON);
}

void Card::UpdateVisualPositions() {
    if (m_Background) {
        m_Background->m_Transform.translation = glm::vec2(m_X, m_Y);
    }

    if (m_Icon) {
        m_Icon->m_Transform.translation = glm::vec2(m_X, m_Y);
    }

    if (m_NameText) {
        float textOffsetY = m_Height * 0.35f;
        float textWidth   = m_NameText->GetScaledSize().x;
        float padding     = m_Width * 0.1f;
        float textX       = (m_X - m_Width / 2.0f) + (textWidth / 2.0f) + padding;
        m_NameText->m_Transform.translation = glm::vec2(textX, m_Y + textOffsetY);
    }
}

std::vector<std::shared_ptr<Util::GameObject>> Card::GetGameObjects() {
    std::vector<std::shared_ptr<Util::GameObject>> objs;
    if (m_Background) objs.push_back(m_Background);
    if (m_Icon)       objs.push_back(m_Icon);
    if (m_NameText)   objs.push_back(m_NameText);
    return objs;
}

void Card::SetScale(float scale) {
    m_Scale = scale;
    glm::vec2 card_scale = {m_Scale, m_Scale};
    m_Background->m_Transform.scale = card_scale * 2.0f;
    m_Icon->m_Transform.scale       = card_scale * GameConstants::ICON_SCALE_FACTOR;

    // 重建名稱文字以維持與卡牌大小成固定比例
    RebuildLabelText(m_NameText, m_Name, Util::Color(0, 0, 0));

    m_Width  = GameConstants::BASE_CARD_WIDTH  * m_Scale;
    m_Height = GameConstants::BASE_CARD_HEIGHT * m_Scale;

    UpdateVisualPositions();
}

bool Card::OnStacked(std::shared_ptr<Card> /*cardAbove*/) {
    return false;
}

void Card::OnMonthEnd() {
}

std::shared_ptr<Util::GameObject> Card::InitLabelText(
    const std::string& text, const Util::Color& color, int zOffset)
{
    auto obj = std::make_shared<Util::GameObject>();
    int fontSize = std::max(1, static_cast<int>(GameConstants::CARD_FONT_SCALE * m_Scale));
    obj->SetDrawable(std::make_shared<Util::Text>(FONT_REGULAR, fontSize, text, color));
    obj->m_Transform.scale = {m_Scale, m_Scale};
    obj->SetZIndex(m_Background->GetZIndex() + zOffset);
    return obj;
}

void Card::RebuildLabelText(std::shared_ptr<Util::GameObject>& obj,
                             const std::string& text, const Util::Color& color)
{
    if (!obj) return;
    int fontSize = std::max(1, static_cast<int>(GameConstants::CARD_FONT_SCALE * m_Scale));
    obj->SetDrawable(std::make_shared<Util::Text>(FONT_REGULAR, fontSize, text, color));
    obj->m_Transform.scale = {m_Scale, m_Scale};
}
