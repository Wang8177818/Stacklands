//
// Created by m0938 on 2026/3/20.
//

#include "CardPack.hpp"
#include "Util/Text.hpp"
#include <algorithm>

CardPack::CardPack(float x, float y, const std::string& name, int sellValue,
                   const std::string& iconPath, float scale, int totalCards,
                   std::vector<CardSpawnData> contents)
    : Card(x, y, name, sellValue, CardType::PACK, scale),
      m_CardsRemaining(totalCards), m_ContentPool(contents)
{
    glm::vec2 card_scale = {m_Scale, m_Scale};
    SetBackgroundImage(iconPath);

    m_CountText = std::make_shared<Util::GameObject>();

    int fontSize = std::max(1, static_cast<int>(3000 * m_Scale));
    m_CountText->SetDrawable(std::make_shared<Util::Text>(
        RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(m_CardsRemaining), Util::Color(1, 1, 1)));
    m_CountText->SetZIndex(m_Background->GetZIndex() + 2);
    m_CountText->m_Transform.scale = card_scale;

    UpdateVisualPositions();
}

std::shared_ptr<CardSpawnData> CardPack::SpawnNext() {
    if (m_CardsRemaining <= 0) return nullptr;
    m_CardsRemaining--;


    auto dataToSpawn = std::make_shared<CardSpawnData>(m_ContentPool.back());
    m_ContentPool.pop_back();

    if (m_CountText) {
        int fontSize = std::max(1, static_cast<int>(3000 * m_Scale));
        m_CountText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(m_ContentPool.size()), Util::Color(1, 1, 1)));
    }

    // 回傳一份資料配方
    return dataToSpawn;
}

void CardPack::UpdateVisualPositions() {
    // 基礎排版
    Card::UpdateVisualPositions();

    if (m_CountText) {
        float countOffsetX = m_Width * -0.7f;
        float countOffsetY = m_Height * 0.7f;
        m_CountText->m_Transform.translation = glm::vec2(m_X + countOffsetX ,m_Y + countOffsetY);
    }

    if (m_NameText){
        m_NameText->SetVisible(false);
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

void CardPack::SetScale(float scale) {
    Card::SetScale(scale); // 同步更新 NameText

    if (m_CountText) {
        int fontSize = std::max(1, static_cast<int>(3000 * m_Scale));
        m_CountText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(m_ContentPool.size()), Util::Color(1, 1, 1)));
        m_CountText->m_Transform.scale = {m_Scale, m_Scale};
    }
}