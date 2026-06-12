#pragma once
#ifndef STACKLANDS_FLOATINGTEXTMANAGER_HPP
#define STACKLANDS_FLOATINGTEXTMANAGER_HPP

#include "EffectData.hpp"
#include "GameConstants.hpp"
#include "Util/Color.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

// 戰鬥中觸發特效時，在目標卡片上方彈出對應英文文字（向上飄浮後消失）
class FloatingTextManager {
public:
    explicit FloatingTextManager(Util::Renderer& renderer);

    // 根據效果類型在指定位置生成浮動文字
    void Spawn(EffectData::Type type, float x, float y);

    // 生成自訂文字（暴擊、傷害數字等）
    void SpawnCustom(const std::string& text, float x, float y,
                     const Util::Color& color);

    // 每幀更新：移動文字、移除超時文字
    void Update(float dtMs);

    // 同步世界座標
    void MoveBy(glm::vec2 delta);
    void ScaleAroundPivot(float ratio, glm::vec2 pivot);

private:
    static constexpr float LIFETIME_MS  = 1200.0f; // 存活時間
    static constexpr float RISE_SPEED   = 60.0f;   // 向上飄浮速度 (px/s)
    static constexpr int   TEXT_Z       = 92;       // 高於卡片與讀條
    static constexpr int   FONT_SIZE    = 600;      // 基礎字體大小

    struct Entry {
        std::shared_ptr<Util::GameObject> obj;
        float elapsed = 0.0f;
    };

    // 效果類型 → 顯示文字 + 顏色
    static std::string   EffectLabel(EffectData::Type type);
    static Util::Color   EffectColor(EffectData::Type type);

    Util::Renderer&    m_Renderer;
    std::vector<Entry> m_Entries;
};

#endif // STACKLANDS_FLOATINGTEXTMANAGER_HPP
