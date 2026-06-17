//
// BlankSlot.hpp — 空白格子（INTERACT 卡，視覺與 SellSlot 相同但無功能）
//

#ifndef STACKLANDS_BLANKSLOT_HPP
#define STACKLANDS_BLANKSLOT_HPP

#pragma once

#include "cards/Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <memory>
#include <glm/glm.hpp>

class BlankSlot : public Card {
public:
    BlankSlot(float x, float y,
              const std::string& name = " ",   // 空字串會讓 SDL_TTF 失敗，預設一個空白
              int buyPrice = 0,
              float scale = 0.1f)
        : Card(x, y, name.empty() ? " " : name, 0, CardType::CHARACTER, scale),
          m_BuyPrice(buyPrice) {
        this->m_Type = CardType::INTERACT;

        m_Background = std::make_shared<Util::GameObject>(
            std::make_unique<Util::Image>(RESOURCE_DIR"/Image/card/BlankSlot.png"), 1);
        m_Background->m_Transform.translation = { m_X, m_Y };
        m_Background->SetVisible(true);

        RebuildSlotText(name.empty() ? " " : name);

        glm::vec2 card_scale = { m_Scale, m_Scale };
        m_Background->m_Transform.scale = card_scale * 2.0f;
        m_NameText->m_Transform.scale   = card_scale;
        m_PriceText->m_Transform.scale  = card_scale;

        UpdateVisualPositions();
    }

    void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        // 名稱在上、價格在下
        m_NameText->m_Transform.translation  = glm::vec2(m_X, m_Y + m_Height *  0.25f);
        if (m_PriceText)
            m_PriceText->m_Transform.translation = glm::vec2(m_X, m_Y + m_Height * -0.30f);
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_PriceText) objs.push_back(m_PriceText);
        return objs;
    }

    int GetBuyPrice() const { return m_BuyPrice; }

    // 不可被滑鼠選取、不可拖曳
    bool IsMouseHovering(glm::vec2 /*mousePos*/) override { return false; }
    void StartDragging(glm::vec2 /*mousePos*/) override {}
    void StopDragging() override {}

    // 只接受 Coin 疊上來（用於購買）
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::COIN;
    }

    // 計算疊在上方的 Coin 數量
    int GetCoinCount() {
        int count = 0;
        auto temp = GetCardAbove();
        while (temp) {
            if (temp->GetType() == CardType::COIN) ++count;
            temp = temp->GetCardAbove();
        }
        return count;
    }

    // 取出並斷開所有疊在上方的硬幣，回傳待移除清單
    std::vector<std::shared_ptr<Card>> PopAllCoins() {
        std::vector<std::shared_ptr<Card>> coins;
        auto temp = GetCardAbove();
        while (temp) {
            coins.push_back(temp);
            temp = temp->GetCardAbove();
        }
        if (GetCardAbove()) GetCardAbove()->SetCardBelow(nullptr);
        SetCardAbove(nullptr);
        return coins;
    }

    // 覆寫：縮放時只更新 transform，不重建紋理（避免縮放卡頓）
    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_PriceText) m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
    }

private:
    // 以白色重建名稱與價格文字（z 拉高避免被卡片蓋掉，但 < 100 避開 far clip）
    void RebuildSlotText(const std::string& name) {
        constexpr int TEXT_Z = 90;
        // 字體比一般卡片大：CARD_FONT_SCALE(1000) → 3000
        const int fontSize = std::max(22, static_cast<int>(3000 * m_Scale));

        m_NameText->SetZIndex(TEXT_Z);
        m_NameText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            name.empty() ? " " : name, Util::Color(255, 255, 255)));

        if (!m_PriceText) m_PriceText = std::make_shared<Util::GameObject>();
        m_PriceText->SetZIndex(TEXT_Z);
        m_PriceText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjhbd.ttc", fontSize,
            std::to_string(m_BuyPrice), Util::Color(255, 255, 255)));

        const glm::vec2 card_scale = { m_Scale, m_Scale };
        m_NameText->m_Transform.scale  = card_scale;
        m_PriceText->m_Transform.scale = card_scale;
    }

protected:
    int m_BuyPrice = 0;
    std::shared_ptr<Util::GameObject> m_PriceText;
};

#endif // STACKLANDS_BLANKSLOT_HPP
