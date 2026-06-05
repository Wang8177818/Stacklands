//
// CardFactory.cpp
// Created during Phase 1 refactoring, 2026/4/18
//

#include "CardFactory.hpp"
#include "CharacterCard.hpp"
#include "ResourceCard.hpp"
#include "CoinCard.hpp"
#include "EquipmentCard.hpp"
#include "BuildingCard.hpp"
#include "WarehouseCard.hpp"
#include "MagicGlue.hpp"
#include "CoinChest.hpp"
#include "Hotpot.hpp"
#include "ResourceChest.hpp"
#include "FoodCard.hpp"
#include "StructureCard.hpp"
#include "AnimalCard.hpp"
#include "MonsterCard.hpp"
#include "LocationCard.hpp"
#include "IdeaCard.hpp"

// ── Registry ────────────────────────────────────────────────────────────────

std::unordered_map<CardType, CardFactory::CreatorFn>& CardFactory::Registry() {
    static std::unordered_map<CardType, CreatorFn> s_Registry = []() {
        std::unordered_map<CardType, CreatorFn> r;

        r[CardType::CHARACTER] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<CharacterCard>(
                x, y, d.name, d.sellValue, d.iconPath, d.scale,
                d.health, d.attack, d.defense,
                d.attackSpeed, d.hitChance, d.foodConsumption);
        };

        r[CardType::RESOURCE] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<ResourceCard>(
                x, y, d.name, d.sellValue, d.iconPath, d.scale);
        };

        r[CardType::COIN] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<CoinCard>(x, y, d.scale);
        };

        r[CardType::EQUIPMENT] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<EquipmentCard>(
                x, y, d.name, d.sellValue, d.iconPath,
                d.attack, d.health, d.defense,
                d.attackSpeed, d.hitChance, d.equipSlot, d.scale);
        };

        r[CardType::BUILDING] = [](float x, float y, const CardSpawnData& d, int& maxCardCount, SpawnCb) -> std::shared_ptr<Card> {
            if (d.name == "Warehouse")
                return std::make_shared<WarehouseCard>(
                    x, y, d.sellValue, d.iconPath, d.scale, maxCardCount);
            if (d.name == "Magic Glue")
                return std::make_shared<MagicGlue>(
                    x, y, d.sellValue, d.iconPath, d.scale);
            if (d.name == "Coin Chest")
                return std::make_shared<CoinChest>(
                    x, y, d.sellValue, d.iconPath, d.scale);
            if (d.name == "Hotpot")
                return std::make_shared<Hotpot>(
                    x, y, d.sellValue, d.iconPath, d.scale);
            if (d.name == "Resource Chest")
                return std::make_shared<ResourceChest>(
                    x, y, d.sellValue, d.iconPath, d.scale);
            return std::make_shared<BuildingCard>(
                x, y, d.name, d.sellValue, d.iconPath, d.scale);
        };

        r[CardType::FOOD] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<FoodCard>(
                x, y, d.name, d.sellValue, d.iconPath,
                d.nutritionValue, d.scale);
        };

        r[CardType::STRUCTURE] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<StructureCard>(
                x, y, d.name, d.sellValue, d.iconPath,
                d.resourceCount, d.spawnCards, d.scale);
        };

        r[CardType::ANIMAL] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb listener) {
            return std::make_shared<AnimalCard>(
                x, y, d.name, d.iconPath, d.scale,
                d.health, d.attack, d.defense,
                d.attackSpeed, d.hitChance,
                d.dropCards, d.abilityName, d.abilityCooldown,
                listener);
        };

        r[CardType::MONSTER] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<MonsterCard>(
                x, y, d.name, d.sellValue, d.iconPath, d.scale,
                d.health, d.attack, d.defense,
                d.attackSpeed, d.hitChance,
                d.dropCards);
        };

        r[CardType::LOCATION] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<LocationCard>(
                x, y, d.name, d.sellValue, d.iconPath,
                d.time, d.spawnCards, d.scale);
        };

        r[CardType::IDEA] = [](float x, float y, const CardSpawnData& d, int&, SpawnCb) {
            return std::make_shared<IdeaCard>(
                x, y, d.name, d.sellValue, d.scale);
        };

        return r;
    }();
    return s_Registry;
}

// ── Public API ───────────────────────────────────────────────────────────────

void CardFactory::Register(CardType type, CreatorFn creator) {
    Registry()[type] = std::move(creator);
}

std::shared_ptr<Card> CardFactory::Create(float x, float y, const CardSpawnData& data,
                                           int& maxCardCount, SpawnCb spawnCallback) {
    auto& reg = Registry();
    auto it = reg.find(data.type);
    if (it != reg.end())
        return it->second(x, y, data, maxCardCount, spawnCallback);

    return std::make_shared<Card>(x, y, data.name, data.sellValue, data.type, data.scale);
}
