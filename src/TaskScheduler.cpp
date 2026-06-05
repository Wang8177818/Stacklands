#include "TaskScheduler.hpp"
#include "CombatArena.hpp"
#include "AnimalCard.hpp"
#include "MonsterCard.hpp"
#include "CharacterCard.hpp"
#include "StructureCard.hpp"
#include "LocationCard.hpp"
#include "AttackResolver.hpp"
#include "GameConstants.hpp"
#include <algorithm>

TaskScheduler::TaskScheduler(Util::Renderer& renderer, std::mt19937& rng,
                             RecipeManager& recipes, SpawnFn spawnFn, RemoveFn removeFn)
    : m_Renderer(renderer), m_Rng(rng), m_Recipes(recipes),
      m_SpawnFn(std::move(spawnFn)), m_RemoveFn(std::move(removeFn)) {}

// ── Gather ────────────────────────────────────────────────────────────────────

void TaskScheduler::AddGather(PendingGather pg) {
    m_Gathers.push_back(std::move(pg));
}

void TaskScheduler::UpdateGathers(float dtMs) {
    for (auto it = m_Gathers.begin(); it != m_Gathers.end(); ) {
        auto ch = it->character.lock();
        auto st = it->structure.lock();
        if (!ch || !st || ch->GetCardBelow() != st || st->GetCardAbove() != ch) {
            it = m_Gathers.erase(it);
            continue;
        }

        if (it->bar) {
            const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * st->GetScale();
            it->bar->SetPosition({st->GetX(), st->GetY() + barOffsetY});
            it->bar->Update(dtMs);
        }

        it->timeLeftMs -= dtMs;
        if (it->timeLeftMs <= 0.0f) {
            if (!it->spawnName.empty()) {
                std::uniform_real_distribution<float> off(-60.f, 60.f);
                m_SpawnFn(it->spawnName, it->spawnScale,
                          it->spawnX + off(m_Rng),
                          it->spawnY + off(m_Rng));
            }

            ch = it->character.lock();
            st = it->structure.lock();

            if (it->exhausted || !ch || !st ||
                ch->GetCardBelow() != st || st->GetCardAbove() != ch) {
                if (ch && st) { ch->SetCardBelow(nullptr); st->SetCardAbove(nullptr); }
                if (it->exhausted && st) m_RemoveFn(st);
                it = m_Gathers.erase(it);
            } else {
                if (st->GetType() == CardType::STRUCTURE) {
                    auto structure   = std::static_pointer_cast<StructureCard>(st);
                    it->spawnName  = structure->Gather(m_Rng);
                    it->exhausted  = structure->IsExhausted();
                    it->timeLeftMs = GameConstants::GATHER_TIME_MS;
                } else {
                    auto location    = std::static_pointer_cast<LocationCard>(st);
                    it->spawnName  = location->Explore(m_Rng);
                    it->exhausted  = false;
                    it->timeLeftMs = location->GetExploreTimeMs();
                }
                it->spawnX = st->GetX();
                it->spawnY = st->GetY();

                const float gatherSec  = it->timeLeftMs / 1000.0f;
                const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * st->GetScale();
                it->bar = std::make_unique<TimeBar>(
                    m_Renderer,
                    glm::vec2{st->GetX(), st->GetY() + barOffsetY},
                    glm::vec2{GameConstants::CRAFT_BAR_BLACK_W, GameConstants::CRAFT_BAR_BLACK_H},
                    glm::vec2{GameConstants::CRAFT_BAR_WHITE_W, GameConstants::CRAFT_BAR_WHITE_H},
                    gatherSec, GameConstants::CRAFT_BAR_Z);
                it->bar->Start();
                ++it;
            }
        } else {
            ++it;
        }
    }
}

// ── Combat ────────────────────────────────────────────────────────────────────

bool TaskScheduler::IsFighterBusy(const std::shared_ptr<Card>& fighter) const {
    for (const auto& pc : m_Combats)
        for (const auto& fe : pc.fighters)
            if (fe.fighter.lock() == fighter) return true;
    return false;
}

bool TaskScheduler::JoinOrCreateCombat(const std::shared_ptr<Card>& target,
                                       const std::shared_ptr<Card>& fighter) {
    if (IsFighterBusy(fighter)) return false;

    for (auto& pc : m_Combats) {
        if (pc.target.lock() == target) {
            pc.fighters.push_back({fighter, fighter->GetAttackSpeed() * 1000.0f});
            if (pc.arena) pc.arena->AddFighter(fighter);
            fighter->SetHitboxActive(false);
            return true;
        }
    }

    PendingCombat pc;
    pc.target      = target;
    pc.targetTimer = target->GetAttackSpeed() * 1000.0f;
    pc.fighters.push_back({fighter, fighter->GetAttackSpeed() * 1000.0f});
    if (target->GetType() == CardType::MONSTER)
        std::static_pointer_cast<MonsterCard>(target)->SetInCombat(true);
    else if (target->GetType() == CardType::ANIMAL)
        std::static_pointer_cast<AnimalCard>(target)->SetInCombat(true);
    fighter->SetHitboxActive(false);
    target->SetHitboxActive(false);
    pc.arena = std::make_unique<CombatArena>(
        m_Renderer,
        std::vector<std::weak_ptr<Card>>{fighter},
        std::weak_ptr<Card>(target));
    m_Combats.push_back(std::move(pc));
    return true;
}

void TaskScheduler::UpdateCombats(float dtMs, const std::shared_ptr<Card>& dragging) {
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    for (auto it = m_Combats.begin(); it != m_Combats.end(); ) {
        auto target = it->target.lock();
        if (!target) { it = m_Combats.erase(it); continue; }

        for (auto& fe : it->fighters) {
            auto f = fe.fighter.lock();
            if (f && f == dragging) f->SetHitboxActive(true);
        }
        it->fighters.erase(std::remove_if(it->fighters.begin(), it->fighters.end(),
            [&](const PendingCombat::FighterEntry& fe) {
                auto f = fe.fighter.lock();
                return !f || f == dragging;
            }), it->fighters.end());

        if (it->fighters.empty()) {
            target->SetHitboxActive(true);
            if (target->GetType() == CardType::MONSTER)
                std::static_pointer_cast<MonsterCard>(target)->SetInCombat(false);
            else if (target->GetType() == CardType::ANIMAL)
                std::static_pointer_cast<AnimalCard>(target)->SetInCombat(false);
            it = m_Combats.erase(it);
            continue;
        }

        if (it->arena) it->arena->Update(dtMs);

        for (auto& fe : it->fighters) {
            auto fighter = fe.fighter.lock();
            if (!fighter) continue;
            fe.timer -= dtMs;
            if (fe.timer <= 0.0f) {
                fe.timer += fighter->GetAttackSpeed() * 1000.0f;
                if (it->arena) it->arena->TriggerAttack(fighter);
                if (Combat::IsHit(fighter->GetHitChance(), dist01(m_Rng))) {
                    bool bonusDmg = dist01(m_Rng) < 0.5f;
                    bool pierce   = dist01(m_Rng) < 0.5f;
                    int dmg = Combat::ResolveDamage(fighter->GetAttack(), target->GetDefense(), bonusDmg, pierce);
                    target->TakeDamage(dmg);
                }
            }
        }

        it->targetTimer -= dtMs;
        if (it->targetTimer <= 0.0f && !it->fighters.empty()) {
            it->targetTimer += target->GetAttackSpeed() * 1000.0f;
            std::uniform_int_distribution<int> idx(0, static_cast<int>(it->fighters.size()) - 1);
            auto fighter = it->fighters[idx(m_Rng)].fighter.lock();
            if (fighter) {
                if (it->arena) it->arena->TriggerCounter(target);
                if (Combat::IsHit(target->GetHitChance(), dist01(m_Rng))) {
                    bool bonusDmg = dist01(m_Rng) < 0.5f;
                    bool pierce   = dist01(m_Rng) < 0.5f;
                    int dmg = Combat::ResolveDamage(target->GetAttack(), fighter->GetDefense(), bonusDmg, pierce);
                    fighter->TakeDamage(dmg);
                }
            }
        }

        // 偵測並處理死亡
        bool targetDied = target->IsDead();

        std::shared_ptr<Card> deadFighterCard;
        {
            std::vector<std::shared_ptr<Card>> deadFighters;
            for (auto& fe : it->fighters) {
                auto f = fe.fighter.lock();
                if (f && f->IsDead()) deadFighters.push_back(f);
            }
            for (auto& df : deadFighters) {
                if (!deadFighterCard) deadFighterCard = df;
                m_RemoveFn(df);
            }
            it->fighters.erase(std::remove_if(it->fighters.begin(), it->fighters.end(),
                [](const PendingCombat::FighterEntry& fe) {
                    auto f = fe.fighter.lock();
                    return !f || f->IsDead();
                }), it->fighters.end());
        }

        if (deadFighterCard) {
            const float fx = deadFighterCard->GetX();
            const float fy = deadFighterCard->GetY();
            const float fs = deadFighterCard->GetScale();

            if (deadFighterCard->GetType() == CardType::ANIMAL) {
                auto animal = std::static_pointer_cast<AnimalCard>(deadFighterCard);
                std::string drop = animal->RollDrop();
                if (!drop.empty()) {
                    std::uniform_real_distribution<float> off(-60.0f, 60.0f);
                    m_SpawnFn(drop, fs, fx + off(m_Rng), fy + off(m_Rng));
                }
            } else if (deadFighterCard->GetType() == CardType::CHARACTER) {
                auto chr = std::static_pointer_cast<CharacterCard>(deadFighterCard);
                std::uniform_real_distribution<float> off(-80.0f, 80.0f);
                for (const auto& e : chr->GetAllEquipData()) {
                    if (e.name.empty()) continue;
                    m_SpawnFn(e.name, fs, fx + off(m_Rng), fy + off(m_Rng));
                }
                m_SpawnFn("Corpse", fs, fx, fy);
            }

            if (auto below = deadFighterCard->GetCardBelow()) below->SetCardAbove(nullptr);
            if (auto above = deadFighterCard->GetCardAbove()) above->SetCardBelow(nullptr);
            deadFighterCard->SetCardBelow(nullptr);
            deadFighterCard->SetCardAbove(nullptr);
        }

        if (targetDied) {
            for (auto& fe : it->fighters) {
                if (auto f = fe.fighter.lock()) f->SetHitboxActive(true);
            }
            std::string drop;
            if (target->GetType() == CardType::MONSTER)
                drop = std::static_pointer_cast<MonsterCard>(target)->RollDrop();
            else if (target->GetType() == CardType::ANIMAL)
                drop = std::static_pointer_cast<AnimalCard>(target)->RollDrop();
            if (!drop.empty()) {
                std::uniform_real_distribution<float> off(-60.f, 60.f);
                m_SpawnFn(drop, target->GetScale(),
                          target->GetX() + off(m_Rng),
                          target->GetY() + off(m_Rng));
            }
            m_RemoveFn(target);
            it = m_Combats.erase(it);
        } else if (it->fighters.empty()) {
            target->SetHitboxActive(true);
            if (target->GetType() == CardType::MONSTER)
                std::static_pointer_cast<MonsterCard>(target)->SetInCombat(false);
            else if (target->GetType() == CardType::ANIMAL)
                std::static_pointer_cast<AnimalCard>(target)->SetInCombat(false);
            it = m_Combats.erase(it);
        } else {
            ++it;
        }
    }
}

// ── Craft ─────────────────────────────────────────────────────────────────────

bool TaskScheduler::HasCraftForBottom(const std::shared_ptr<Card>& bottom) const {
    for (const auto& pc : m_Crafts)
        if (pc.stackBottom.lock() == bottom) return true;
    return false;
}

void TaskScheduler::AddCraft(PendingCraft pc) {
    m_Crafts.push_back(std::move(pc));
}

void TaskScheduler::UpdateCrafts(float dtMs) {
    for (auto it = m_Crafts.begin(); it != m_Crafts.end(); ) {
        auto bottom = it->stackBottom.lock();
        if (!bottom) { it = m_Crafts.erase(it); continue; }

        float verifyTime = 0.0f;
        if (m_Recipes.CheckCrafting(bottom, verifyTime) != it->outputName) {
            it = m_Crafts.erase(it);
            continue;
        }

        if (it->bar) {
            const float barOffsetY = GameConstants::CRAFT_BAR_OFFSET_Y * bottom->GetScale();
            it->bar->SetPosition({bottom->GetX(), bottom->GetY() + barOffsetY});
            it->bar->Update(dtMs);
        }

        it->timeLeftMs -= dtMs;
        if (it->timeLeftMs <= 0.0f) {
            float sx = bottom->GetX(), sy = bottom->GetY(), ss = bottom->GetScale();
            std::vector<std::shared_ptr<Card>> toDelete;
            for (auto cur = bottom; cur; cur = cur->GetCardAbove()) {
                if (cur->GetType() != CardType::CHARACTER && cur->GetType() != CardType::BUILDING)
                    toDelete.push_back(cur);
            }
            for (auto cur = bottom; cur; ) {
                auto next = cur->GetCardAbove();
                cur->SetCardAbove(nullptr);
                cur->SetCardBelow(nullptr);
                cur = next;
            }
            for (auto& c : toDelete) m_RemoveFn(c);
            m_SpawnFn(it->outputName, ss, sx, sy);
            it = m_Crafts.erase(it);
        } else {
            ++it;
        }
    }
}
