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
    // --- 1. 文字按鈕建構子 ---
    MenuButton(float x, float y, float textsize,float width, float height, const std::string& textStr, bool bold, int words) {
        m_IsImageButton = false; // 標記為文字按鈕

        m_OriginalX = x;
        m_OriginalY = y;
        m_Left = x - width / 2;
        m_Right = x + width / 2;
        m_Top = y + height / 2;
        m_Bottom = y - height / 2;
        m_textsize = textsize;

        // ... (保留您原本建立 m_DarkBg, m_Text, m_Underline 的程式碼) ...
        m_DarkBg = std::make_shared<Util::GameObject>();
        m_DarkBg->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/button/dark_bg.png"));
        m_DarkBg->SetZIndex(99);
        m_DarkBg->m_Transform.scale = {300, textsize*2};
        m_DarkBg->m_Transform.translation = {-470, y};

        m_Text = std::make_shared<Util::GameObject>();
        if (bold == true) {
            m_Text->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjhbd.ttc", textsize, textStr, Util::Color(0, 0, 0)));
        } else {
            m_Text->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/msjh.ttc", textsize, textStr, Util::Color(0, 0, 0)));
        }
        m_Text->SetZIndex(100);
        m_Text->m_Transform.translation = glm::vec2(x, y);

        m_Underline = std::make_shared<Util::GameObject>();
        m_Underline->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/button/underline.png"));
        m_Underline->SetZIndex(100);
        m_Underline->m_Transform.translation = glm::vec2(x - textsize/3, y - textsize/1.5);
        m_Underline->m_Transform.scale = {words*textsize, 1};
    }

    // --- 2. 圖片按鈕建構子 ---
    MenuButton(float x, float y,float scaleX, float scaleY, float width, float height, const std::string& path) {
        m_IsImageButton = true; // 標記為圖片按鈕

        m_OriginalX = x; // 最好也把 OriginalX/Y 存起來，以備 HideAll 需要
        m_OriginalY = y;
        m_Left = x - width / 2;
        m_Right = x + width / 2;
        m_Top = y + height / 2;
        m_Bottom = y - height / 2;

        m_Image = std::make_shared<Util::GameObject>();
        m_Image->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR + path));
        m_Image->SetZIndex(100);
        m_Image->m_Transform.translation = {x, y};
        m_Image->m_Transform.scale = {scaleX, scaleY};
    }

    // 更新按鈕狀態
    bool UpdateHover(glm::vec2 mousePos) {
        bool isHover = (mousePos.x >= m_Left && mousePos.x <= m_Right &&
                        mousePos.y >= m_Bottom && mousePos.y <= m_Top);

        // 只有文字按鈕才需要顯示/隱藏深色背景和底線
        if (!m_IsImageButton) {
            if (isHover) {
                m_DarkBg->SetVisible(true);
                m_Underline->SetVisible(true);
            } else {
                m_DarkBg->SetVisible(false);
                m_Underline->SetVisible(false);
            }
        } else {
            // 如果是圖片按鈕，您可以在這裡加別的 Hover 特效
            // 例如：放大一點點 -> m_Image->m_Transform.scale = ...
        }
        return isHover;
    }

    // 隱藏
    void HideAll() {
        if (!m_IsImageButton) {
            /*
            m_DarkBg->m_Transform.translation = glm::vec2(m_OriginalX, -9999);
            m_Text->m_Transform.translation = glm::vec2(m_OriginalX, -9999);
            m_Underline->m_Transform.translation = glm::vec2(m_OriginalX, -9999);
            */
            m_DarkBg->SetVisible(false);
            m_Text->SetVisible(false);
            m_Underline->SetVisible(false);
        } else {
            //m_Image->m_Transform.translation = glm::vec2(m_OriginalX, -9999);
            m_Image->SetVisible(false);
        }
    }

    // 顯示
    void ShowAll() {
        if (!m_IsImageButton) {
            m_DarkBg->SetVisible(true);
            m_Text->SetVisible(true);
            m_Underline->SetVisible(true);
        } else {
            m_Image->SetVisible(true);
        }
    }

    // 【關鍵修復點】動態決定回傳哪些物件
    std::vector<std::shared_ptr<Util::GameObject>> GetGameObjects() {
        if (m_IsImageButton) {
            return {m_Image}; // 圖片按鈕只回傳 Image
        } else {
            return {m_DarkBg, m_Text, m_Underline}; // 文字按鈕回傳這三個
        }
    }

    // 專門給圖片按鈕換圖用的方法
    void ChangeImage(const std::string& path) {
        if (m_IsImageButton && m_Image != nullptr) {
            auto newImage = std::make_shared<Util::Image>(RESOURCE_DIR + path);
            m_Image->SetDrawable(newImage);
        }
    }

private:
    bool m_IsImageButton; // 新增一個布林值來分辨按鈕類型

    float m_OriginalX, m_OriginalY;
    float m_Left, m_Right, m_Top, m_Bottom, m_textsize;

    std::shared_ptr<Util::GameObject> m_DarkBg;
    std::shared_ptr<Util::GameObject> m_Text;
    std::shared_ptr<Util::GameObject> m_Underline;
    std::shared_ptr<Util::GameObject> m_Image;
};

#endif //STACKLANDS_BUTTON_HPP