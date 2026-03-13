//
// Created by m0938 on 2026/3/13.
//

#ifndef STACKLANDS_CARD_HPP
#define STACKLANDS_CARD_HPP

#pragma once

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/Input.hpp"
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

enum class CardType {
    BASIC,      // 基礎卡 (例如金幣)
    RESOURCE,   // 資源卡 (木頭、石頭等)
    CHARACTER,  // 人物卡 (村民、民兵等)
    BUILDING,   // 功能/建築卡 (箱子、房子等)
    FOOD        // 食物卡 (漿果、蘋果等)
};

class Card {
public:
    Card(float x, float y, const std::string& name, int sellValue, CardType type);

    // 虛擬解構子 (確保子類別被刪除時，記憶體能正確釋放)
    virtual ~Card() = default;
    //設定背景種類(黃、藍)
    void SetBackgroundImage(const std::string& imagePath);
    // 切換卡牌圖示
    void SetIconImage(const std::string& imagePath);

    virtual void Update();

    // 當卡牌被疊上另一張卡時觸發
    // 回傳 bool 代表這個堆疊是否被允許 (例如：食物不能疊在木頭上)
    virtual bool OnStacked(std::shared_ptr<Card> cardAbove);

    // 月底結算時觸發 (例如扣除食物、產生新卡牌等)
    virtual void OnMonthEnd();

    // 檢查滑鼠是否停留在這張卡牌上
    bool IsMouseHovering(glm::vec2 mousePos);

    // 拖曳控制
    void StartDragging(glm::vec2 mousePos);
    void StopDragging();

    // 取得卡牌屬性
    int GetSellValue() const { return m_SellValue; }
    CardType GetType() const { return m_Type; }

    // ==========================================
    // 堆疊關係 (Linked List 結構)
    // ==========================================
    void SetCardBelow(std::shared_ptr<Card> card) { m_CardBelow = card; }
    void SetCardAbove(std::shared_ptr<Card> card) { m_CardAbove = card; }
    std::shared_ptr<Card> GetCardAbove() const { return m_CardAbove; }

    // 取得所有負責顯示的 GameObject，交給 Renderer 繪製
    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects();

protected:
    CardType m_Type;
    std::string m_Name;
    int m_SellValue;

    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<Util::GameObject> m_Icon;
    std::shared_ptr<Util::GameObject> m_NameText;

    float m_X, m_Y;
    float m_Width = 120.0f;  // 卡牌預設寬度
    float m_Height = 180.0f; // 卡牌預設高度

    // 拖曳狀態
    bool m_IsDragging;
    glm::vec2 m_DragOffset;

    // 堆疊指標
    std::shared_ptr<Card> m_CardBelow = nullptr;
    std::shared_ptr<Card> m_CardAbove = nullptr;

    // 更新 GameObject 實際座標的內部方法
    void UpdateVisualPositions();
};

#endif //STACKLANDS_CARD_HPP