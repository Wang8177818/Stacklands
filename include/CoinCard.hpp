//
// Created by m0938 on 2026/3/26.
//

#ifndef STACKLANDS_COINCARD_HPP
#define STACKLANDS_COINCARD_HPP

#pragma once
#include "Card.hpp"
#include "Util/Text.hpp"

class CoinCard : public Card {
public:
    // 金幣建構子：我們把預設價值設為 1 (m_SellValue = 1)
    CoinCard(float x, float y, float scale = 1.0f)
        : Card(x, y, "Coin", 1, CardType::COIN, scale) {

        // 請準備一張金幣的圖片 (例如 Coin.png) 放在 Image/card 裡面
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Coin.png");
        if (m_Icon) m_Icon->SetVisible(false);

        // 調整物理大小 (跟資源卡差不多)
        float baseWidth = 750.0f;
        float baseHeight = 1100.0f;
        m_Width = baseWidth * m_Scale;
        m_Height = baseHeight * m_Scale;

        // 建立顯示總金額的文字物件
        m_TotalValueText = std::make_shared<Util::GameObject>();
        int fontSize = static_cast<int>(500 * m_Scale);
        if (fontSize < 20) fontSize = 20;

        m_TotalValueText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize, "1", Util::Color(255, 215, 0))); // 金黃色
        m_TotalValueText->m_Transform.scale = {m_Scale, m_Scale};

        UpdateVisualPositions();
    }

    // ==========================================
    // 核心 1：堆疊限制 (只能被金幣疊)
    // ==========================================
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        // 如果想疊上來的卡片也是金幣，就放行！
        if (cardAbove->GetType() == CardType::COIN) {
            return true;
        }
        // 未來如果是「金幣盒」，那是金幣疊在金幣盒上，邏輯會寫在 CoinChest::OnStacked 裡面，所以這裡不用改
        return false;
    }

    // ==========================================
    // 核心 2：動態計算整疊金幣的總價值
    // ==========================================
    int GetTotalStackValue() {
        int total = m_SellValue; // 先算自己的價值 (1)

        // 往上找：把所有疊在上面的卡片價值加起來
        auto current = m_CardAbove;
        while (current != nullptr) {
            total += current->GetSellValue();
            current = current->GetCardAbove();
        }

        // 往下找：把所有墊在下面的卡片價值加起來
        current = m_CardBelow;
        while (current != nullptr) {
            total += current->GetSellValue();
            current = current->GetCardBelow();
        }
        return total;
    }

    // ==========================================
    // 核心 3：每幀更新 UI 顯示
    // ==========================================
    virtual void Update() override {
        Card::Update();

        if (m_TotalValueText) {
            // 【Stacklands 視覺特性】：只有「最底下」的那張金幣才負責顯示整疊的總數！
            if (m_CardBelow == nullptr) {
                m_TotalValueText->SetVisible(true);
                int currentTotal = GetTotalStackValue();
                // 動態更新數字
                m_TotalValueText->SetDrawable(std::make_shared<Util::Text>(
                    RESOURCE_DIR"/Font/msjh.ttc", static_cast<int>(500 * m_Scale),
                    std::to_string(currentTotal), Util::Color(255, 215, 0)));
            } else {
                // 如果我下面還有金幣，我就把我的數字藏起來
                m_TotalValueText->SetVisible(false);
            }
        }
    }

    // ==========================================
    // UI 排版與渲染覆寫
    // ==========================================
    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_TotalValueText) {
            // 顯示在右下角
            float textOffsetX = m_Width * 0.25f;
            float textOffsetY = m_Height * -0.35f;
            m_TotalValueText->m_Transform.translation = glm::vec2(m_X + textOffsetX, m_Y + textOffsetY);
            m_TotalValueText->SetZIndex(m_Background->GetZIndex() + 2);
        }
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_TotalValueText) objs.push_back(m_TotalValueText);
        return objs;
    }

    virtual void StartDragging(glm::vec2 mousePos) override {
        Card::StartDragging(mousePos);
        if (m_TotalValueText) m_TotalValueText->SetZIndex(42);
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_TotalValueText) m_TotalValueText->SetZIndex(m_Background->GetZIndex() + 2);
    }

protected:
    std::shared_ptr<Util::GameObject> m_TotalValueText;
};

#endif //STACKLANDS_COINCARD_HPP