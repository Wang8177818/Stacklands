//
// Hotpot.hpp — Hotpot 建築卡
// 功能：
//   - 疊在上方的 Food 會被自動吸收（CardManager 每幀掃描處理）
//   - 吸收量 = FoodCard 的 nutritionValue
//   - 玩家無法手動取出（僅村民月底結算時消耗）
//   - 儲存量顯示在卡片右下角（與 FoodCard nutrition 同位置、同顏色）
//

#ifndef STACKLANDS_HOTPOT_HPP
#define STACKLANDS_HOTPOT_HPP

#include "BuildingCard.hpp"

class Hotpot final : public BuildingCard {
public:
    Hotpot(float x, float y, int sellValue,
           const std::string& iconPath, float scale)
        : BuildingCard(x, y, "Hotpot", sellValue, iconPath, scale)
    {
        // 右下角的 nutrition 顯示（橘色，與 FoodCard 一致）
        m_NutritionText = InitLabelText(std::to_string(m_StoredFood),
                                        Util::Color(0, 0, 0));
        UpdateVisualPositions();
    }

    int  GetStored() const { return m_StoredFood; }

    // 存入指定量的食物 nutrition（無上限）
    void Deposit(int amount) {
        if (amount <= 0) return;
        m_StoredFood += amount;
        RefreshNutritionText();
    }

    // 內部用：月底結算讓村民取食物（不對玩家提供 UI）
    int Withdraw(int amount) {
        const int give = std::min(amount, m_StoredFood);
        m_StoredFood -= give;
        RefreshNutritionText();
        return give;
    }

    // 只接受 Food 疊上來（疊上後 CardManager 會自動吸收）
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::FOOD;
    }

    void SetScale(float scale) override {
        BuildingCard::SetScale(scale);
        RefreshNutritionText();
    }

    void UpdateVisualPositions() override {
        BuildingCard::UpdateVisualPositions();
        if (m_NutritionText) {
            m_NutritionText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::SECONDARY_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_NutritionText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    void StartDragging(glm::vec2 mousePos) override {
        BuildingCard::StartDragging(mousePos);
        if (m_NutritionText) m_NutritionText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    void StopDragging() override {
        BuildingCard::StopDragging();
        if (m_NutritionText) m_NutritionText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = BuildingCard::GetGameObjects();
        if (m_NutritionText) objs.push_back(m_NutritionText);
        return objs;
    }

private:
    void RefreshNutritionText() {
        if (m_NutritionText)
            RebuildLabelText(m_NutritionText, std::to_string(m_StoredFood),
                             Util::Color(0, 0, 0));
    }

    int m_StoredFood = 0;
    std::shared_ptr<Util::GameObject> m_NutritionText;
};

#endif // STACKLANDS_HOTPOT_HPP
