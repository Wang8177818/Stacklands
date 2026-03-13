//
// Created by user on 2026/3/11.
//

#ifndef STACKLANDS_BUTTON_HPP
#define STACKLANDS_BUTTON_HPP

#pragma once
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class MenuButton {
public:
    // 建構子：設定按鈕的位置、大小、文字內容與資源路徑
    MenuButton(float x, float y, float textsize,float width, float height, const std::string& textStr, bool bold) {
        // 記錄原始座標與計算邊界
        m_OriginalX = x;
        m_OriginalY = y;
        m_Left = x - width / 2;
        m_Right = x + width / 2;
        m_Top = y + height / 2;
        m_Bottom = y - height / 2;
        m_textsize = textsize;

        // 1. 建立深色背景 (預設藏在畫面外)
        m_DarkBg = std::make_shared<Util::GameObject>();
        m_DarkBg->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/button/dark_bg.png"));
        m_DarkBg->SetZIndex(5);

        // 2. 建立文字
        m_Text = std::make_shared<Util::GameObject>();
        if (bold == true) {
            m_Text->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", textsize, textStr, Util::Color(0, 0, 0)));
        } else {
            m_Text->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", textsize, textStr, Util::Color(0, 0, 0)));
        }

        m_Text->SetZIndex(6); // 文字蓋在深色背景上
        m_Text->m_Transform.translation = glm::vec2(x, y);

        // 3. 建立底線 (預設藏在畫面外)
        m_Underline = std::make_shared<Util::GameObject>();
        m_Underline->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/button/underline.png"));
        m_Underline->SetZIndex(6);
        m_Underline->m_Transform.translation = glm::vec2(x - textsize/2.5, y - textsize/1.5);
        m_Underline->m_Transform.scale = {textsize*5, 1};
    }

    // 更新按鈕狀態，回傳 true 代表滑鼠正在上面
    bool UpdateHover(glm::vec2 mousePos) {
        bool isHover = (mousePos.x >= m_Left && mousePos.x <= m_Right &&
                        mousePos.y >= m_Bottom && mousePos.y <= m_Top);

        if (isHover) {
            // 滑鼠移入：顯示深色背景與底線
            m_DarkBg->SetVisible(true);
            m_Underline->SetVisible(true);
        } else {
            // 滑鼠移出：隱藏
            m_DarkBg->SetVisible(false);
            m_Underline->SetVisible(false);
        }
        return isHover;
    }

    // 切換場景時，把所有東西藏起來
    void HideAll() {
        m_DarkBg->m_Transform.translation = glm::vec2(m_OriginalX, -9999);
        m_Text->m_Transform.translation = glm::vec2(m_OriginalX, -9999);
        m_Underline->m_Transform.translation = glm::vec2(m_OriginalX, -9999);
    }

    // 讓 App 可以取得這些物件並加入 Renderer
    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() {
        return {m_DarkBg, m_Text, m_Underline};
    }

private:
    float m_OriginalX, m_OriginalY;
    float m_Left, m_Right, m_Top, m_Bottom, m_textsize;
    std::shared_ptr<Util::GameObject> m_DarkBg;
    std::shared_ptr<Util::GameObject> m_Text;
    std::shared_ptr<Util::GameObject> m_Underline;
};

#endif //STACKLANDS_BUTTON_HPP