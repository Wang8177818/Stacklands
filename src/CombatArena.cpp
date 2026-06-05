#include "CombatArena.hpp"
#include "Util/Image.hpp"
#include <cmath>
#include <algorithm>

CombatArena::CombatArena(Util::Renderer& renderer,
                         const std::vector<std::weak_ptr<Card>>& fighters,
                         const std::weak_ptr<Card>& target)
    : m_Renderer(renderer)
{
    glm::vec2 sum{0.0f, 0.0f};
    int count = 0;
    for (const auto& fw : fighters) {
        if (auto f = fw.lock()) { sum += glm::vec2{f->GetX(), f->GetY()}; ++count; }
    }
    if (auto t = target.lock()) { sum += glm::vec2{t->GetX(), t->GetY()}; ++count; }
    m_Center = (count > 0) ? sum / static_cast<float>(count) : glm::vec2{0.0f, 0.0f};

    m_Bg = std::make_shared<Util::GameObject>(
        std::make_shared<Util::Image>(RESOURCE_DIR"/Image/background/darkgreen.png"),
        static_cast<float>(BG_Z));
    m_Bg->m_Transform.translation = m_Center;
    m_Renderer.AddChild(m_Bg);

    for (const auto& fw : fighters)
        m_Fighters.push_back({fw, glm::vec2{0.0f, 0.0f}});
    m_Target = {target, glm::vec2{0.0f, 0.0f}};

    ComputeLayout();
    ResizeBackground();
}

CombatArena::~CombatArena() {
    m_Bg->SetVisible(false);
    m_Bg->m_Transform.translation = {-9999.0f, -9999.0f};
}

void CombatArena::ComputeLayout() {
    int N = 0;
    for (const auto& s : m_Fighters)
        if (s.card.lock()) ++N;

    int i = 0;
    for (auto& s : m_Fighters) {
        if (!s.card.lock()) continue;
        float offset = (N > 1) ? (i - (N - 1) * 0.5f) * SLOT_SPACING : 0.0f;
        s.home = {m_Center.x + offset, m_Center.y + FIGHTER_Y};
        ++i;
    }
    m_Target.home = {m_Center.x, m_Center.y + TARGET_Y};
}

void CombatArena::ResizeBackground() {
    int N = 0;
    for (const auto& s : m_Fighters)
        if (s.card.lock()) ++N;
    if (N < 1) N = 1;

    float w = N * SLOT_SPACING + PAD_X * 2.0f;
    float h = (FIGHTER_Y - TARGET_Y) + PAD_Y * 2.0f;
    m_Bg->m_Transform.scale = {3.0f * w / BG_IMG_W, 3.0f * h / BG_IMG_H};
}

void CombatArena::LerpToward(const std::shared_ptr<Card>& card, glm::vec2 target, float dtMs) {
    float dx = target.x - card->GetX();
    float dy = target.y - card->GetY();
    if (dx * dx + dy * dy < 0.25f) return;
    float factor = 1.0f - std::exp(-LERP_SPEED * dtMs * 0.001f);
    card->MoveBy({dx * factor, dy * factor});
}

bool CombatArena::HasActiveAnim(const std::shared_ptr<Card>& card) const {
    for (const auto& a : m_Anims)
        if (!a.done && a.card.lock() == card) return true;
    return false;
}

void CombatArena::Update(float dtMs) {
    // Lerp non-animating cards toward their home slots
    for (const auto& s : m_Fighters) {
        auto c = s.card.lock();
        if (!c || HasActiveAnim(c)) continue;
        LerpToward(c, s.home, dtMs);
    }
    if (auto t = m_Target.card.lock()) {
        if (!HasActiveAnim(t))
            LerpToward(t, m_Target.home, dtMs);
    }

    float dt = dtMs * 0.001f;
    for (auto& a : m_Anims) {
        if (a.done) continue;
        auto c = a.card.lock();
        if (!c) { a.done = true; continue; }

        glm::vec2 dest   = a.returning ? a.home : a.aim;
        glm::vec2 pos    = {c->GetX(), c->GetY()};
        glm::vec2 dir    = dest - pos;
        float     dist   = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        float     speed  = a.returning ? RTN_SPEED : ATK_SPEED;
        float     step   = speed * dt;

        if (dist <= step) {
            c->MoveBy(dir);
            if (a.returning) {
                a.done = true;
            } else {
                a.returning = true;
            }
        } else {
            glm::vec2 norm = dir / dist;
            c->MoveBy(norm * step);
        }
    }

    m_Anims.erase(std::remove_if(m_Anims.begin(), m_Anims.end(),
        [](const AttackAnim& a) { return a.done; }), m_Anims.end());
}

void CombatArena::AddFighter(const std::shared_ptr<Card>& fighter) {
    m_Fighters.push_back({fighter, glm::vec2{0.0f, 0.0f}});
    ComputeLayout();
    ResizeBackground();
}

void CombatArena::TriggerAttack(const std::shared_ptr<Card>& attacker) {
    glm::vec2 home{0.0f, 0.0f};
    for (const auto& s : m_Fighters) {
        if (s.card.lock() == attacker) { home = s.home; break; }
    }
    m_Anims.push_back({attacker, home, m_Target.home, false, false});
}

void CombatArena::TriggerCounter(const std::shared_ptr<Card>& target) {
    glm::vec2 aim{0.0f, 0.0f};
    for (const auto& s : m_Fighters) {
        if (s.card.lock()) { aim = s.home; break; }
    }
    m_Anims.push_back({target, m_Target.home, aim, false, false});
}
