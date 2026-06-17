#include "combat/FloatingTextManager.hpp"
#include <algorithm>
#include <cmath>

FloatingTextManager::FloatingTextManager(Util::Renderer& renderer)
    : m_Renderer(renderer) {}

std::string FloatingTextManager::EffectLabel(EffectData::Type type) {
    switch (type) {
        case EffectData::Type::Stun:         return "Stun";
        case EffectData::Type::Bleed:        return "Bleed";
        case EffectData::Type::Poison:       return "Poison";
        case EffectData::Type::CriticalHit:  return "Crit!";
        case EffectData::Type::Lifesteal:    return "Lifesteal";
        case EffectData::Type::DamageAll:    return "AoE";
        case EffectData::Type::DamageRandom: return "Hit";
        case EffectData::Type::Heal:         return "Heal";
        case EffectData::Type::Frenzy:       return "Frenzy";
        case EffectData::Type::Invulnerable: return "Shield";
        default:                             return "Effect";
    }
}

Util::Color FloatingTextManager::EffectColor(EffectData::Type type) {
    switch (type) {
        case EffectData::Type::Stun:         return {255, 255,   0}; // 黃色
        case EffectData::Type::Bleed:        return {220,  20,  20}; // 紅色
        case EffectData::Type::Poison:       return { 50, 205,  50}; // 綠色
        case EffectData::Type::CriticalHit:  return {255, 165,   0}; // 橙色
        case EffectData::Type::Lifesteal:    return {200,  50, 200}; // 紫色
        case EffectData::Type::DamageAll:    return {255,  80,  80}; // 淺紅
        case EffectData::Type::DamageRandom: return {255, 100, 100}; // 淺紅
        case EffectData::Type::Heal:         return { 50, 255, 120}; // 亮綠
        case EffectData::Type::Frenzy:       return {255, 120,   0}; // 深橙
        case EffectData::Type::Invulnerable: return {100, 180, 255}; // 天藍
        default:                             return {255, 255, 255};
    }
}

void FloatingTextManager::Spawn(EffectData::Type type, float x, float y) {
    SpawnCustom(EffectLabel(type), x, y, EffectColor(type));
}

void FloatingTextManager::SpawnCustom(const std::string& text, float x, float y,
                                       const Util::Color& color) {
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(std::make_shared<Util::Text>(
        FONT_BOLD, FONT_SIZE, text, color));
    obj->m_Transform.translation = {x, y};
    obj->m_Transform.scale = {0.05f, 0.05f};
    obj->SetZIndex(static_cast<float>(TEXT_Z));

    m_Renderer.AddChild(obj);
    m_Entries.push_back({obj, 0.0f});
}

void FloatingTextManager::Clear() {
    for (auto& e : m_Entries) {
        e.obj->SetVisible(false);
        m_Renderer.RemoveChild(e.obj);
    }
    m_Entries.clear();
}

void FloatingTextManager::Update(float dtMs) {
    float dtSec = dtMs * 0.001f;

    for (auto& e : m_Entries) {
        e.elapsed += dtMs;

        // 向上飄浮
        e.obj->m_Transform.translation.y += RISE_SPEED * dtSec;

        // 後半段漸隱（縮小 + 透明化模擬）
        float ratio = e.elapsed / LIFETIME_MS;
        if (ratio > 0.5f) {
            float fade = 1.0f - (ratio - 0.5f) * 2.0f; // 1→0
            fade = std::max(0.0f, fade);
            e.obj->m_Transform.scale = {0.05f * fade, 0.05f * fade};
        }
    }

    // 移除超時的文字
    for (auto& e : m_Entries) {
        if (e.elapsed >= LIFETIME_MS) {
            e.obj->SetVisible(false);
            m_Renderer.RemoveChild(e.obj);
        }
    }
    m_Entries.erase(
        std::remove_if(m_Entries.begin(), m_Entries.end(),
            [&](const Entry& e) { return e.elapsed >= LIFETIME_MS; }),
        m_Entries.end());
}

void FloatingTextManager::MoveBy(glm::vec2 delta) {
    for (auto& e : m_Entries)
        e.obj->m_Transform.translation += delta;
}

void FloatingTextManager::ScaleAroundPivot(float ratio, glm::vec2 pivot) {
    for (auto& e : m_Entries) {
        e.obj->m_Transform.translation =
            pivot + (e.obj->m_Transform.translation - pivot) * ratio;
        e.obj->m_Transform.scale *= ratio;
    }
}
