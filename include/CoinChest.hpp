//
// CoinChest.hpp — Coin Chest 建築卡
// 功能：
//   - 疊在上方的 Coin 會被自動吸收（由 CardManager 每幀掃描處理）
//   - 容量上限 100 枚
//   - 右鍵點擊：吐出 10 枚（不足則吐出全部）
//   - 儲存的金幣數量直接用「售價」呈現（賣掉 chest 可取回全部）
//

#ifndef STACKLANDS_COINCHEST_HPP
#define STACKLANDS_COINCHEST_HPP

#include "BuildingCard.hpp"
#include <algorithm>

class CoinChest final : public BuildingCard {
public:
    static constexpr int MAX_COINS       = 100;
    static constexpr int WITHDRAW_AMOUNT = 10;

    CoinChest(float x, float y, int /*sellValue 忽略*/,
              const std::string& iconPath, float scale)
        : BuildingCard(x, y, "Coin Chest", 0, iconPath, scale)
    {
        // 起始售價 = 0（無存幣）；BuildingCard 已建立 m_PriceText，這裡刷新一下就好
        RefreshPriceText();
    }

    int  GetStored() const { return m_SellValue; }
    bool IsFull()    const { return m_SellValue >= MAX_COINS; }
    bool IsEmpty()   const { return m_SellValue <= 0; }

    // 嘗試存入 amount 枚，回傳實際存入的數量
    int Deposit(int amount) {
        const int take = std::min(amount, MAX_COINS - m_SellValue);
        m_SellValue += take;
        RefreshPriceText();
        return take;
    }

    // 嘗試取出 amount 枚，回傳實際取出的數量
    int Withdraw(int amount) {
        const int give = std::min(amount, m_SellValue);
        m_SellValue -= give;
        RefreshPriceText();
        return give;
    }

    // 只接受 Coin 疊上來（疊上後 CardManager 會自動吸收）
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::COIN;
    }

private:
    void RefreshPriceText() {
        if (m_PriceText)
            RebuildLabelText(m_PriceText, std::to_string(m_SellValue),
                             Util::Color(100, 111, 128));
    }
};

#endif // STACKLANDS_COINCHEST_HPP
