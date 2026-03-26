//
// Created by m0938 on 2026/3/20.
//

#include "CardPack.hpp"
#include "Util/Text.hpp"

CardPack::CardPack(float x, float y, const std::string& name, int sellValue,
                   const std::string& iconPath, float scale, int totalCards,
                   std::vector<CardSpawnData> contents)
    : Card(x, y, name, sellValue, CardType::PACK, scale),
      m_CardsRemaining(totalCards), m_ContentPool(contents)
{
    glm::vec2 card_scale = {m_Scale, m_Scale};

    SetBackgroundImage(RESOURCE_DIR"/Image/card/Pack.png");
    SetIconImage(iconPath);

    m_CountText = std::make_shared<Util::GameObject>();

    int nameSize = static_cast<int>(1000 * m_Scale);
    if (nameSize < 22) nameSize = 22;
    m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", nameSize, m_Name, Util::Color(255, 255, 255  )));


    int fontSize = static_cast<int>(1000 * m_Scale);
    if (fontSize < 22) fontSize = 22;

    m_CountText->SetDrawable(std::make_shared<Util::Text>(
        RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(m_CardsRemaining), Util::Color(1, 1, 1))); // 白色文字
    m_CountText->SetZIndex(m_Background->GetZIndex() + 1);
    m_CountText->m_Transform.scale = card_scale;

    UpdateVisualPositions();
}

std::shared_ptr<CardSpawnData> CardPack::SpawnNext() {
    if (m_CardsRemaining <= 0) return nullptr;

    m_CardsRemaining--;

    // 更新文字顯示
    if (m_CountText) {
        m_CountText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", static_cast<int>(300 * m_Scale),
            std::to_string(m_CardsRemaining), Util::Color(1, 1, 1)));
    }

    // 實作您定義的「出現卡片可以訂」: 隨機選取卡池中的一個配方
    // (這裡是簡單的平均機率隨機，未來可以優化權重系統)
    int randomIndex = rand() % m_ContentPool.size();

    // 回傳一份資料配方的拷貝
    return std::make_shared<CardSpawnData>(m_ContentPool[randomIndex]);
}

void CardPack::UpdateVisualPositions() {
    // 基礎排版
    Card::UpdateVisualPositions();
    if (m_Icon){
        float iconOffsetX = m_Width * 0.08f;
        float iconOffsetY = m_Height * -0.05f;
        m_Icon->m_Transform.translation = glm::vec2(m_X + iconOffsetX ,m_Y + iconOffsetY);
    }

    if (m_CountText) {
        float countOffsetX = m_Width * -0.7f;
        float countOffsetY = m_Height * 0.7f;
        m_CountText->m_Transform.translation = glm::vec2(m_X + countOffsetX ,m_Y + countOffsetY);
    }

    if (m_NameText){
        float nameOffsetX = m_Width * 0.08f;
        float nameOffsetY = m_Height * -0.5f;
        m_NameText->m_Transform.translation = glm::vec2(m_X + nameOffsetX ,m_Y + nameOffsetY);
    }
}

std::vector<std::shared_ptr<Util::GameObject>> CardPack::GetGameObjects() {
    std::vector<std::shared_ptr<Util::GameObject>> objs = Card::GetGameObjects();
    if (m_CountText) objs.push_back(m_CountText);
    return objs;
}

bool CardPack::OnStacked(std::shared_ptr<Card> /*cardAbove*/) {
    return false;
}

void CardPack::StartDragging(glm::vec2 mousePos) {
    Card::StartDragging(mousePos);

    if (m_CountText) {
        m_CountText->SetZIndex(42);
    }
}

void CardPack::StopDragging() {
    Card::StopDragging();

    if (m_CountText) {
        m_CountText->SetZIndex(m_Background->GetZIndex() + 1);
    }
}