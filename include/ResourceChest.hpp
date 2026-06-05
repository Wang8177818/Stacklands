//
// ResourceChest.hpp — Resource Chest 建築卡
// 功能：
//   - 只能存放單一類型的 RESOURCE 卡，最多 100 張
//   - 首次存入時鎖定類型；之後只接受同類型
//   - 卡片中心會顯示該 resource 的 icon
//   - 計數顯示在右下角（與 FoodCard nutrition 同位置，灰色）
//

#ifndef STACKLANDS_RESOURCECHEST_HPP
#define STACKLANDS_RESOURCECHEST_HPP

#include "BuildingCard.hpp"
#include "Util/Image.hpp"
#include <algorithm>
#include <string>

class ResourceChest final : public BuildingCard {
public:
    static constexpr int MAX_COUNT       = 100;
    static constexpr int WITHDRAW_AMOUNT = 10;
    static constexpr float STORED_ICON_SCALE = 0.6f;  // 相對 m_Scale

    ResourceChest(float x, float y, int sellValue,
                  const std::string& iconPath, float scale)
        : BuildingCard(x, y, "Resource Chest", sellValue, iconPath, scale)
    {
        // 中心：儲存資源的 icon 疊加（建立時不可見，存入後才打開）
        m_StoredIcon = std::make_shared<Util::GameObject>();
        m_StoredIcon->SetZIndex(m_Background->GetZIndex() + 1);
        m_StoredIcon->SetVisible(false);

        // 右下：數量
        m_CountText = InitLabelText(std::to_string(m_StoredCount),
                                    Util::Color(100, 111, 128));
        UpdateVisualPositions();
    }

    int  GetStored()          const { return m_StoredCount; }
    bool IsFull()             const { return m_StoredCount >= MAX_COUNT; }
    bool IsEmpty()            const { return m_StoredCount <= 0; }
    const std::string& GetStoredName() const { return m_StoredName; }

    // 是否接受該名稱的 resource 存入（容量未滿且類型相符 / 尚未鎖定）
    bool CanAccept(const std::string& name) const {
        if (IsFull()) return false;
        return m_StoredName.empty() || m_StoredName == name;
    }

    // 存入一張 resource（首次存入會鎖定類型並設定中心 icon）
    // 回傳是否成功存入
    bool Deposit(const std::string& name, const std::string& iconPath) {
        if (!CanAccept(name)) return false;
        if (m_StoredName.empty()) {
            m_StoredName = name;
            m_StoredIcon->SetDrawable(std::make_shared<Util::Image>(iconPath));
            m_StoredIcon->m_Transform.scale = {m_Scale * STORED_ICON_SCALE,
                                               m_Scale * STORED_ICON_SCALE};
            m_StoredIcon->SetVisible(true);
        }
        ++m_StoredCount;
        RefreshCountText();
        return true;
    }

    // 取出 amount 張；回傳實際取出的數量。歸 0 時自動解鎖類型 + 隱藏 icon
    int Withdraw(int amount) {
        const int give = std::min(amount, m_StoredCount);
        m_StoredCount -= give;
        RefreshCountText();
        if (m_StoredCount == 0) {
            m_StoredName.clear();
            if (m_StoredIcon) m_StoredIcon->SetVisible(false);
        }
        return give;
    }

    // 只接受 RESOURCE 疊上來；其餘規則交由 CardManager 用 CanAccept 過濾
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::RESOURCE &&
               CanAccept(cardAbove->GetName());
    }

    void SetScale(float scale) override {
        BuildingCard::SetScale(scale);
        if (m_StoredIcon)
            m_StoredIcon->m_Transform.scale = {m_Scale * STORED_ICON_SCALE,
                                               m_Scale * STORED_ICON_SCALE};
        RefreshCountText();
    }

    void UpdateVisualPositions() override {
        BuildingCard::UpdateVisualPositions();
        if (m_StoredIcon) {
            m_StoredIcon->m_Transform.translation = glm::vec2(m_X, m_Y);
        }
        if (m_CountText) {
            m_CountText->m_Transform.translation = glm::vec2(
                m_X + m_Width  * GameConstants::SECONDARY_OFFSET_X,
                m_Y + m_Height * GameConstants::PRICE_OFFSET_Y);
            m_CountText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    void StartDragging(glm::vec2 mousePos) override {
        BuildingCard::StartDragging(mousePos);
        if (m_StoredIcon) m_StoredIcon->SetZIndex(GameConstants::Z_DRAG_TEXT);
        if (m_CountText)  m_CountText->SetZIndex(GameConstants::Z_DRAG_EXTRA);
    }

    void StopDragging() override {
        BuildingCard::StopDragging();
        if (m_StoredIcon) m_StoredIcon->SetZIndex(m_Background->GetZIndex() + 1);
        if (m_CountText)  m_CountText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = BuildingCard::GetGameObjects();
        if (m_StoredIcon) objs.push_back(m_StoredIcon);
        if (m_CountText)  objs.push_back(m_CountText);
        return objs;
    }

private:
    void RefreshCountText() {
        if (m_CountText)
            RebuildLabelText(m_CountText, std::to_string(m_StoredCount),
                             Util::Color(100, 111, 128));
    }

    int         m_StoredCount = 0;
    std::string m_StoredName;            // 空 = 還沒鎖定類型
    std::shared_ptr<Util::GameObject> m_StoredIcon;
    std::shared_ptr<Util::GameObject> m_CountText;
};

#endif // STACKLANDS_RESOURCECHEST_HPP
