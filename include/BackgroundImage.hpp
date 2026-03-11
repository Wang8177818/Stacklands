//
// Created by user on 2026/3/11.
//

#ifndef STACKLANDS_BACKGROUNDIMAGE_HPP
#define STACKLANDS_BACKGROUNDIMAGE_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

class BackgroundImage : public Util::GameObject {

public:
    BackgroundImage() : GameObject(
        std::make_unique<Util::Image>(RESOURCE_DIR"/Image/background/stacklandsMenu.png"), 0) {}

    // ===== 新增這個函式來控制圖片大小 =====
    void SetScale(const glm::vec2& scale) {
        m_Transform.scale = scale;
    }
};


#endif //STACKLANDS_BACKGROUNDIMAGE_HPP