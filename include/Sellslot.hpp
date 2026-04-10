//
// Created by m0938 on 2026/4/10.
//

#ifndef STACKLANDS_SELLSLOT_HPP
#define STACKLANDS_SELLSLOT_HPP

#pragma once

#include "Card.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "Card.hpp"
#include "CardManager.hpp"

class SellSlot : public Card{
public:
    SellSlot(float x, float y,
             const std::string& name = "0",
             int sellValue = 0,
             float scale = 0.1f) : Card(x, y, name, sellValue, CardType::CHARACTER, scale){

        m_Background = std::make_shared<Util::GameObject>(
        std::make_unique<Util::Image>(RESOURCE_DIR"/Image/card/Sellslot.png"), 1);
        m_Background->m_Transform.translation = { m_X, m_Y };
        m_Background->SetVisible(true);

        int fontSize = static_cast<int>(1000 * m_Scale);
        if (fontSize < 22) fontSize = 22;

        m_NameText->SetZIndex(m_Background->GetZIndex() + 1);
        m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(sellValue), Util::Color(255, 255, 255)));

        glm::vec2 card_scale = {m_Scale, m_Scale};
        m_Background->m_Transform.scale = card_scale * 2.0f;
        m_NameText->m_Transform.scale = card_scale;

        UpdateVisualPositions();
    }


    virtual std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() override
    {
        return Card::GetGameObjects();
    }

    void StartDragging(glm::vec2 mousePos) override{};
    void StopDragging() override{};

    int GetTotalPrice(){
        if (GetCardBelow() != nullptr)
        {
            int value = 0;
            std::vector<std::shared_ptr<Card>> allcards;
            std::shared_ptr<Card> temp = GetCardBelow();
            do{
                allcards.push_back(temp);
                value += temp->GetSellValue();
                temp = temp->GetCardBelow();
            }while (temp != nullptr);
            return value;
        }
        return 0;
    };

protected:
    int m_price = 0;

};

#endif // STACKLANDS_SELLSLOT_HPP