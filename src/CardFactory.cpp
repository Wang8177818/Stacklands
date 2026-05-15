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
#include "FoodCard.hpp"
#include "StructureCard.hpp"
#include "AnimalCard.hpp"
#include "MonsterCard.hpp"

std::shared_ptr<Card> CardFactory::Create(float x, float y, const CardSpawnData& data, int& maxCardCount,
                                           std::function<void(const std::string&, float, float)> spawnCallback) {
    switch (data.type) {
        case CardType::CHARACTER:
            return std::make_shared<CharacterCard>(
                x, y, data.name, data.sellValue, data.iconPath, data.scale,
                data.health, data.attack, data.defense,
                data.attackSpeed, data.hitChance, data.foodConsumption);

        case CardType::RESOURCE:
            return std::make_shared<ResourceCard>(
                x, y, data.name, data.sellValue, data.iconPath, data.scale);

        case CardType::COIN:
            return std::make_shared<CoinCard>(x, y, data.scale);

        case CardType::EQUIPMENT:
            return std::make_shared<EquipmentCard>(
                x, y, data.name, data.sellValue, data.iconPath,
                data.attack, data.health, data.defense,
                data.attackSpeed, data.hitChance, data.equipSlot, data.scale);

        case CardType::BUILDING:
            if (data.name == "Warehouse")
                return std::make_shared<WarehouseCard>(
                    x, y, data.sellValue, data.iconPath, data.scale, maxCardCount);
            return std::make_shared<BuildingCard>(
                x, y, data.name, data.sellValue, data.iconPath, data.scale);

        case CardType::FOOD:
            return std::make_shared<FoodCard>(
                x, y, data.name, data.sellValue, data.iconPath,
                data.nutritionValue, data.scale);

        case CardType::STRUCTURE:
            return std::make_shared<StructureCard>(
                x, y, data.name, data.sellValue, data.iconPath,
                data.resourceCount, data.spawnCards, data.scale);

        case CardType::ANIMAL:
            return std::make_shared<AnimalCard>(
                x, y, data.name, data.iconPath, data.scale,
                data.health, data.attack, data.defense,
                data.attackSpeed, data.hitChance,
                data.dropCards, data.abilityName, data.abilityCooldown,
                spawnCallback);

        case CardType::MONSTER:
            return std::make_shared<MonsterCard>(
                x, y, data.name, data.sellValue, data.iconPath, data.scale,
                data.health, data.attack, data.defense,
                data.attackSpeed, data.hitChance,
                data.dropCards);

        default:
            return std::make_shared<Card>(
                x, y, data.name, data.sellValue, data.type, data.scale);
    }
}
