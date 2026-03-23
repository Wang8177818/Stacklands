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

    // 1. 設定外觀 (這裡共用 Card.jpg，您也可以準備專門的卡包圖)
    SetBackgroundImage(RESOURCE_DIR"/Image/card/Card.jpg");
    SetIconImage(iconPath);

    // 2. 建立數量文字 (右上角)
    m_CountText = std::make_shared<Util::GameObject>();
    // 文字大小調小一點，例如基础 300
    int fontSize = static_cast<int>(300 * m_Scale);
    if (fontSize < 10) fontSize = 10;

    m_CountText->SetDrawable(std::make_shared<Util::Text>(
        RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(m_CardsRemaining), Util::Color(1, 1, 1))); // 白色文字
    m_CountText->SetZIndex(m_Background->GetZIndex() + 2); // 名字文字通常是 +2，我們文字蓋在上面
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

    // 更新數量文字座標到卡片右上角
    if (m_CountText) {
        // 卡牌中心 + 卡牌寬度的一半(再往內縮一點 padding)
        float textX = m_X + (m_Width / 2.0f) * 0.85f;
        // 卡牌中心 + 卡牌高度的一半
        float textY = m_Y + (m_Height / 2.0f) * 0.85f;
        m_CountText->m_Transform.translation = glm::vec2(textX, textY);
    }
}

std::vector<std::shared_ptr<Util::GameObject>> CardPack::GetGameObjects() {
    std::vector<std::shared_ptr<Util::GameObject>> objs = Card::GetGameObjects();
    if (m_CountText) objs.push_back(m_CountText);
    return objs;
}

bool CardPack::OnStacked(std::shared_ptr<Card> /*cardAbove*/) {
    return false; // 卡包上面預設拒絕堆疊其他東西
}