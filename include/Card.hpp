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
    BASIC,      // 空卡
    RESOURCE,   // 資源卡 (木頭、石頭等)
    CHARACTER,  // 人物卡 (村民、民兵等)
    BUILDING,   // 功能/建築卡 (箱子、房子等)
    FOOD,       // 食物卡 (漿果、蘋果等)
    PACK,       // 卡包
    COIN,       // 金幣卡
    EQUIPMENT   // 裝備卡
};

enum class EquipSlot {
    NONE,
    HEAD,
    HAND,
    BODY
};

class Card {
public:
    Card(float x, float y, const std::string& name, int sellValue, CardType type, float scale = 1.0f);

    virtual ~Card() = default;

    std::string GetName() const { return m_Name; }

    void SetBackgroundImage(const std::string& imagePath);

    void SetIconImage(const std::string& imagePath);

    virtual void Update();

    // 當卡牌被疊上另一張卡時觸發
    virtual bool OnStacked(std::shared_ptr<Card> cardAbove);

    // 月底結算時觸發 (例如扣除食物、產生新卡牌等)
    virtual void OnMonthEnd();

    // 檢查滑鼠是否停留在這張卡牌上
    virtual bool IsMouseHovering(glm::vec2 mousePos);
    bool IsOverlapping(std::shared_ptr<Card> otherCard);

    // 拖曳控制
    virtual void StartDragging(glm::vec2 mousePos);
    virtual void StopDragging();

    // 取得卡牌屬性
    int GetSellValue() const { return m_SellValue; }
    CardType GetType() const { return m_Type; }

    void SetCardBelow(std::shared_ptr<Card> card) { m_CardBelow = card; }
    void SetCardAbove(std::shared_ptr<Card> card) { m_CardAbove = card; }

    //取得疊在自己下方的卡片
    std::shared_ptr<Card> GetCardBelow() const { return m_CardBelow; }

    std::shared_ptr<Card> GetCardAbove() const { return m_CardAbove; }

    // 取得所有負責顯示的 GameObject，交給 Renderer 繪製
    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects();
    float GetX() const { return m_X; }
    float GetY() const { return m_Y; }

    void SetScale(float scale);

    // EventManager 用：直接移動邏輯座標
    void MoveBy(glm::vec2 delta) {
        m_X += delta.x;
        m_Y += delta.y;
        UpdateVisualPositions();
    }

    // EventManager 用：以指定 pivot 為錨點縮放並重新定位
    void ScaleAroundPivot(float ratio, glm::vec2 pivot) {
        m_X = pivot.x + (m_X - pivot.x) * ratio;
        m_Y = pivot.y + (m_Y - pivot.y) * ratio;
        SetScale(m_Scale * ratio); // 同步更新視覺縮放與碰撞尺寸
    }

protected:
    CardType m_Type;
    std::string m_Name;
    int m_SellValue;

    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<Util::GameObject> m_Icon;
    std::shared_ptr<Util::GameObject> m_NameText;

    float m_X, m_Y;
    float m_Scale;  // 記錄這張卡牌的縮放比例
    float m_Width;  // 實際碰撞寬度
    float m_Height; // 實際碰撞高度

    // 拖曳狀態
    bool m_IsDragging;
    glm::vec2 m_DragOffset;

    // 堆疊指標
    std::shared_ptr<Card> m_CardBelow = nullptr;
    std::shared_ptr<Card> m_CardAbove = nullptr;

    // 更新 GameObject 實際座標的內部方法
    virtual void UpdateVisualPositions();
};

#endif //STACKLANDS_CARD_HPP