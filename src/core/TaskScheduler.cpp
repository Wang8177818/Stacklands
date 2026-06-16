#include "core/TaskScheduler.hpp"
#include "combat/CombatArena.hpp"
#include "cards/AnimalCard.hpp"
#include "cards/MonsterCard.hpp"
#include "cards/CharacterCard.hpp"
#include "cards/StructureCard.hpp"
#include "cards/LocationCard.hpp"
#include "combat/AttackResolver.hpp"
#include "cards/CombatCard.hpp"
#include "data/EffectData.hpp"
#include "data/GameConstants.hpp"
#include <algorithm>

TaskScheduler::TaskScheduler(Util::Renderer& renderer, std::mt19937& rng,
                             RecipeManager& recipes, SpawnFn spawnFn, RemoveFn removeFn)
    : m_Renderer(renderer), m_Rng(rng), m_Recipes(recipes),
      m_SpawnFn(std::move(spawnFn)), m_RemoveFn(std::move(removeFn)),
      m_FloatingText(renderer) {}

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
                    it->exhausted  = location->IsExhausted();
                    it->timeLeftMs = location->GetExploreTimeMs();
                }
                it->spawnX     = st->GetX();
                it->spawnY     = st->GetY();
                it->spawnScale = st->GetScale();

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
    m_FloatingText.Update(dtMs);
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

        // 收集活著的戰鬥員（供特效目標解析用）
        std::vector<std::shared_ptr<Card>> aliveFighters;
        for (auto& fe : it->fighters)
            if (auto f = fe.fighter.lock()) aliveFighters.push_back(f);

        // 每幀更新所有戰鬥狀態（暈眩 / DoT / 無敵 / 狂暴）
        target->UpdateCombatStates(dtMs);
        for (auto& f : aliveFighters) f->UpdateCombatStates(dtMs);

        // 角色攻擊：暈眩中凍結 CD、不可出手
        for (auto& fe : it->fighters) {
            auto fighter = fe.fighter.lock();
            if (!fighter) continue;
            if (!fighter->IsStunned()) fe.timer -= dtMs;
            if (fe.timer <= 0.0f && !fighter->IsStunned()) {
                fe.timer += fighter->GetAttackSpeed() * 1000.0f;
                if (it->arena) it->arena->TriggerAttack(fighter);

                // 暴擊判定（passive 效果在 miss 時也觸發，非 passive 需先命中）
                bool isCrit = false;
                for (const auto& eff : fighter->GetEffects()) {
                    if (eff.type == EffectData::Type::CriticalHit && eff.passive)
                        if (dist01(m_Rng) < eff.chance) isCrit = true;
                }

                if (Combat::IsHit(fighter->GetHitChance(), dist01(m_Rng))) {
                    // 命中後再判定非 passive 暴擊
                    for (const auto& eff : fighter->GetEffects())
                        if (eff.type == EffectData::Type::CriticalHit && !eff.passive)
                            if (dist01(m_Rng) < eff.chance) isCrit = true;

                    bool bonusDmg = dist01(m_Rng) < 0.5f;
                    bool pierce   = dist01(m_Rng) < 0.5f;
                    int rawAtk = fighter->GetAttack() * (isCrit ? 2 : 1);
                    int dmg = Combat::ResolveDamage(rawAtk, target->GetDefense(), bonusDmg, pierce);
                    target->TakeDamage(dmg);
                    // 顯示傷害數字
                    m_FloatingText.SpawnCustom(std::to_string(dmg),
                                              target->GetX(), target->GetY(),
                                              {255, 255, 255});
                    if (isCrit)
                        m_FloatingText.Spawn(EffectData::Type::CriticalHit,
                                             target->GetX(), target->GetY());
                    ApplyHitEffects(fighter, target, dmg, aliveFighters, it->target, true);
                } else {
                    // MISS
                    m_FloatingText.SpawnCustom("MISS",
                                              target->GetX(), target->GetY(),
                                              {180, 180, 180});
                }
                // passive 非傷害特效（即使 miss 也觸發）
                ApplyPassiveEffects(fighter, target, aliveFighters, it->target, true);
            }
        }

        // 怪物反擊：暈眩中凍結 CD、不可出手
        if (!target->IsStunned()) it->targetTimer -= dtMs;
        if (it->targetTimer <= 0.0f && !it->fighters.empty() && !target->IsStunned()) {
            it->targetTimer += target->GetAttackSpeed() * 1000.0f;
            std::uniform_int_distribution<int> idx(0, static_cast<int>(it->fighters.size()) - 1);
            auto fighter = it->fighters[idx(m_Rng)].fighter.lock();
            if (fighter) {
                if (it->arena) it->arena->TriggerCounter(target, fighter);

                bool isCrit = false;
                for (const auto& eff : target->GetEffects())
                    if (eff.type == EffectData::Type::CriticalHit && eff.passive)
                        if (dist01(m_Rng) < eff.chance) isCrit = true;

                if (Combat::IsHit(target->GetHitChance(), dist01(m_Rng))) {
                    for (const auto& eff : target->GetEffects())
                        if (eff.type == EffectData::Type::CriticalHit && !eff.passive)
                            if (dist01(m_Rng) < eff.chance) isCrit = true;

                    bool bonusDmg = dist01(m_Rng) < 0.5f;
                    bool pierce   = dist01(m_Rng) < 0.5f;
                    int rawAtk = target->GetAttack() * (isCrit ? 2 : 1);
                    int dmg = Combat::ResolveDamage(rawAtk, fighter->GetDefense(), bonusDmg, pierce);
                    fighter->TakeDamage(dmg);
                    // 顯示傷害數字
                    m_FloatingText.SpawnCustom(std::to_string(dmg),
                                              fighter->GetX(), fighter->GetY(),
                                              {255, 255, 255});
                    if (isCrit)
                        m_FloatingText.Spawn(EffectData::Type::CriticalHit,
                                             fighter->GetX(), fighter->GetY());
                    ApplyHitEffects(target, fighter, dmg, aliveFighters, it->target, false);
                } else {
                    // MISS
                    m_FloatingText.SpawnCustom("MISS",
                                              fighter->GetX(), fighter->GetY(),
                                              {180, 180, 180});
                }
                ApplyPassiveEffects(target, fighter, aliveFighters, it->target, false);
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

// ── Arena world-sync ────────────────────────────────────────────────────────

void TaskScheduler::MoveCombatArenas(glm::vec2 delta) {
    for (auto& pc : m_Combats)
        if (pc.arena) pc.arena->MoveBy(delta);
    m_FloatingText.MoveBy(delta);
}

void TaskScheduler::ScaleCombatArenas(float ratio, glm::vec2 pivot) {
    for (auto& pc : m_Combats)
        if (pc.arena) pc.arena->ScaleAroundPivot(ratio, pivot);
    m_FloatingText.ScaleAroundPivot(ratio, pivot);
}

// ── Effect helpers ────────────────────────────────────────────────────────────

// 解析目標清單（fighter 側呼叫：allEnemies = {monster}, allFriendlies = fighters）
static std::vector<std::shared_ptr<Card>> ResolveTargets(
    EffectData::Target t,
    const std::shared_ptr<Card>& attacker,
    const std::shared_ptr<Card>& directHit,
    const std::vector<std::shared_ptr<Card>>& fighters,
    const std::weak_ptr<Card>& combatTarget,
    bool attackerIsFighter,
    std::mt19937& rng)
{
    auto monster = combatTarget.lock();
    std::vector<std::shared_ptr<Card>> enemies  = monster ? std::vector<std::shared_ptr<Card>>{monster} : std::vector<std::shared_ptr<Card>>{};
    const auto& friendlies = fighters;

    // 若攻擊者是怪物，敵友互換
    if (!attackerIsFighter) {
        enemies.clear();
        for (const auto& f : fighters) enemies.push_back(f);
    }

    switch (t) {
        case EffectData::Target::DirectTarget:     return {directHit};
        case EffectData::Target::Self:             return {attacker};
        case EffectData::Target::AllEnemies:       return enemies;
        case EffectData::Target::AllFriendlies:    return {friendlies.begin(), friendlies.end()};
        case EffectData::Target::RandomEnemy: {
            if (enemies.empty()) return {};
            std::uniform_int_distribution<int> d(0, static_cast<int>(enemies.size()) - 1);
            return {enemies[d(rng)]};
        }
        case EffectData::Target::RandomFriendly: {
            if (friendlies.empty()) return {};
            std::uniform_int_distribution<int> d(0, static_cast<int>(friendlies.size()) - 1);
            return {friendlies[d(rng)]};
        }
        case EffectData::Target::FriendlyLowestHp: {
            std::shared_ptr<Card> lowest;
            for (const auto& f : friendlies)
                if (!lowest || f->GetHealth() < lowest->GetHealth()) lowest = f;
            return lowest ? std::vector<std::shared_ptr<Card>>{lowest} : std::vector<std::shared_ptr<Card>>{};
        }
        default: break;
    }
    return {};
}

void TaskScheduler::ApplyHitEffects(
    const std::shared_ptr<Card>& attacker,
    const std::shared_ptr<Card>& directHit,
    int dmgDealt,
    const std::vector<std::shared_ptr<Card>>& fighters,
    const std::weak_ptr<Card>& combatTarget,
    bool attackerIsFighter)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (const auto& eff : attacker->GetEffects()) {
        if (eff.type == EffectData::Type::CriticalHit) continue; // handled separately
        if (eff.passive) continue;                                // handled by ApplyPassiveEffects
        if (dist(m_Rng) >= eff.chance) continue;

        auto targets = ResolveTargets(eff.target, attacker, directHit,
                                      fighters, combatTarget, attackerIsFighter, m_Rng);
        float durMs = eff.duration * 1000.0f;

        for (auto& tgt : targets) {
            if (!tgt) continue;
            switch (eff.type) {
                case EffectData::Type::Stun:
                    tgt->ApplyStun(durMs); break;
                case EffectData::Type::Bleed:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->ApplyBleed(durMs); break;
                case EffectData::Type::Poison:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->ApplyPoison(durMs); break;
                case EffectData::Type::Lifesteal:
                    if (auto cc = dynamic_cast<CombatCard*>(attacker.get())) cc->HealBy(dmgDealt); break;
                case EffectData::Type::DamageAll:
                case EffectData::Type::DamageRandom:
                    tgt->TakeDamage(eff.value > 0 ? eff.value : 1); break;
                case EffectData::Type::Heal:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->HealBy(eff.value); break;
                case EffectData::Type::Frenzy:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->ApplyFrenzy(durMs); break;
                case EffectData::Type::Invulnerable:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->ApplyInvulnerable(durMs); break;
                default: break;
            }
            // 浮動文字：Lifesteal 顯示在攻擊者身上，其他顯示在目標身上
            if (eff.type == EffectData::Type::Lifesteal)
                m_FloatingText.Spawn(eff.type, attacker->GetX(), attacker->GetY());
            else
                m_FloatingText.Spawn(eff.type, tgt->GetX(), tgt->GetY());
        }
    }
}

void TaskScheduler::ApplyPassiveEffects(
    const std::shared_ptr<Card>& attacker,
    const std::shared_ptr<Card>& directHit,
    const std::vector<std::shared_ptr<Card>>& fighters,
    const std::weak_ptr<Card>& combatTarget,
    bool attackerIsFighter)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (const auto& eff : attacker->GetEffects()) {
        if (!eff.passive) continue;
        if (eff.type == EffectData::Type::CriticalHit) continue;
        if (dist(m_Rng) >= eff.chance) continue;

        auto targets = ResolveTargets(eff.target, attacker, directHit,
                                      fighters, combatTarget, attackerIsFighter, m_Rng);
        float durMs = eff.duration * 1000.0f;
        for (auto& tgt : targets) {
            if (!tgt) continue;
            switch (eff.type) {
                case EffectData::Type::Bleed:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->ApplyBleed(durMs); break;
                case EffectData::Type::Poison:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->ApplyPoison(durMs); break;
                case EffectData::Type::Heal:
                    if (auto cc = dynamic_cast<CombatCard*>(tgt.get())) cc->HealBy(eff.value); break;
                case EffectData::Type::DamageAll:
                case EffectData::Type::DamageRandom:
                    tgt->TakeDamage(eff.value > 0 ? eff.value : 1); break;
                default: break;
            }
            m_FloatingText.Spawn(eff.type, tgt->GetX(), tgt->GetY());
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
