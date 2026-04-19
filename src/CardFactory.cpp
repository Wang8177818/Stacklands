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
#include "FoodCard.hpp"
#include "StructureCard.hpp"

std::shared_ptr<Card> CardFactory::Create(float x, float y, const CardSpawnData& data) {
    switch (data.type) {
        case CardType::CHARACTER:
            return std::make_shared<CharacterCard>(
                x, y, data.name, data.sellValue, data.iconPath, data.scale,
                data.health, data.attack, data.defense,
                data.attackSpeed, data.hitChance, data.food);

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

        default:
            return std::make_shared<Card>(
                x, y, data.name, data.sellValue, data.type, data.scale);
    }
}
