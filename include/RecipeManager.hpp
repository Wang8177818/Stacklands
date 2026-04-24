//
// Created by m0938 on 2026/4/14.
//

#ifndef STACKLANDS_RECIPEMANAGER_HPP
#define STACKLANDS_RECIPEMANAGER_HPP

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Card.hpp"

// 職業配方
struct ProfessionRecipe {
    std::vector<std::string> equipmentOptions;
    std::string output;
};

// 合成配方
struct CraftingRecipe {
    std::vector<std::string> inputs; //（"2x Wood" → {"Wood","Wood"}）
    std::string output;
    float time = 2.0f;  // 合成秒數
};

class RecipeManager {
public:
    void LoadProfessionRecipes(const std::string& filePath);
    void LoadCraftingRecipes(const std::string& filePath);

    // 檢查裝備名稱是否觸發職業配方
    std::string CheckProfession(const std::string& equipName) const;

    // 傳入堆疊底部，檢查整個堆疊是否符合合成配方
    // 回傳符合的 output（不符合則空字串），並透過 outTime 回傳等待秒數
    std::string CheckCrafting(std::shared_ptr<Card> stackBottom, float& outTime) const;

private:
    // 解析 "2x Wood"、"1 Villager"、"Wood" → (count, name)
    static std::pair<int, std::string> ParseInput(const std::string& raw);

    std::vector<ProfessionRecipe> m_ProfessionRecipes;
    std::vector<CraftingRecipe>   m_CraftingRecipes;
};

#endif // STACKLANDS_RECIPEMANAGER_HPP
