//
// Created by m0938 on 2026/3/13.
//

#ifndef STACKLANDS_CHARACTERCARD_HPP
#define STACKLANDS_CHARACTERCARD_HPP

#pragma once
#include <array>
#include <string>
#include <algorithm>
#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Image.hpp"
#include "EquipmentCard.hpp"

class CharacterCard : public Card {
public:
    CharacterCard(float x, float y, const std::string& name, int sellValue,
                  const std::string& iconPath, float scale, int health = 1, int attack = 1)
        : Card(x, y, name, sellValue, CardType::CHARACTER, scale),
          health(health), attack(attack)
    {
        SetBackgroundImage(RESOURCE_DIR"/Image/card/Card_Character.png");
        SetIconImage(iconPath);

        m_HealthText = std::make_shared<Util::GameObject>();
        int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
        m_HealthText->SetDrawable(std::make_shared<Util::Text>(
            RESOURCE_DIR"/Font/msjh.ttc", fontSize,
            std::to_string(health), Util::Color(255, 255, 255)));
        m_HealthText->m_Transform.scale = {m_Scale, m_Scale};
        m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);

        UpdateVisualPositions();
    }

    // ── 裝備槽（存名稱字串，空字串 = 未裝備）────────────────────
    // 索引 = (int)EquipSlot：[0=NONE不用, 1=HEAD, 2=HAND, 3=BODY]

    // 儲存裝備名稱到對應插槽
    void StoreEquipment(EquipSlot slot, const std::string& name) {
        m_EquipNames[static_cast<int>(slot)] = name;
    }

    // 取得某插槽的裝備名稱（空字串 = 未裝備）
    const std::string& GetEquipName(EquipSlot slot) const {
        return m_EquipNames[static_cast<int>(slot)];
    }

    // 取得全部插槽（轉職時整批轉移用）
    const std::array<std::string, 4>& GetEquipNames() const {
        return m_EquipNames;
    }

    // 整批設定插槽（轉職後寫入新角色用）
    void SetEquipNames(const std::array<std::string, 4>& names) {
        m_EquipNames = names;
    }

    // ── 縮放（同步重建 HealthText 字體）────────────────────────
    void SetScale(float scale) override {
        Card::SetScale(scale);
        if (m_HealthText) {
            int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
            m_HealthText->SetDrawable(std::make_shared<Util::Text>(
                RESOURCE_DIR"/Font/msjh.ttc", fontSize,
                std::to_string(health), Util::Color(255, 255, 255)));
            m_HealthText->m_Transform.scale = {m_Scale, m_Scale};
        }
    }

    // ── 月底結算 ──────────────────────────────────────────────
    void OnMonthEnd() override {}

    // ── 視覺更新 ──────────────────────────────────────────────
    virtual void UpdateVisualPositions() override {
        Card::UpdateVisualPositions();
        if (m_HealthText) {
            m_HealthText->m_Transform.translation =
                glm::vec2(m_X + m_Width * 0.34f, m_Y + m_Height * -0.3f);
            m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
        }
    }

    virtual void StopDragging() override {
        Card::StopDragging();
        if (m_HealthText) m_HealthText->SetZIndex(m_Background->GetZIndex() + 1);
    }

    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override {
        auto objs = Card::GetGameObjects();
        if (m_HealthText) objs.push_back(m_HealthText);
        return objs;
    }

    // ── 堆疊規則 ──────────────────────────────────────────────
    // 只接受裝備卡；插槽衝突檢查由 CardManager 在消耗前處理
    bool OnStacked(std::shared_ptr<Card> cardAbove) override {
        return cardAbove->GetType() == CardType::EQUIPMENT;
    }

    // ── 基礎攻擊力 ────────────────────────────────────────────
    int GetBaseAttack() const { return attack; }

protected:
    std::shared_ptr<Util::GameObject> m_HealthText;
    int   health = 1;
    int   attack = 1;
    float attackSpeed = 1.0f;

    // 裝備插槽（存名稱，空字串 = 空槽）
    std::array<std::string, 4> m_EquipNames = {"", "", "", ""};
};

#endif //STACKLANDS_CHARACTERCARD_HPP
