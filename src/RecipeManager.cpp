//
// Created by m0938 on 2026/4/14.
//

#include "RecipeManager.hpp"
#include "nlohmann/json.hpp"
#include "Util/Logger.hpp"
#include <fstream>
#include <algorithm>
#include <cctype>

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
void RecipeManager::LoadProfessionRecipes(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("無法開啟職業配方檔: {}", filePath);
        return;
    }

    json j;
    file >> j;

    for (const auto& item : j) {
        ProfessionRecipe recipe;
        recipe.output = item.value("output", "");
        for (const auto& eq : item["inputs"]) {
            recipe.equipmentOptions.push_back(eq.get<std::string>());
        }
        m_ProfessionRecipes.push_back(recipe);
    }
    LOG_INFO("載入 {} 條職業配方", m_ProfessionRecipes.size());
}

std::string RecipeManager::CheckProfession(const std::string& equipName) const {
    for (const auto& recipe : m_ProfessionRecipes) {
        for (const auto& option : recipe.equipmentOptions) {
            if (option == equipName) return recipe.output;
        }
    }
    return "";
}

// ─────────────────────────────────────────────────────────────
// 解析數量標注：
//   "2x Wood"   → {2, "Wood"}
//   "1 Villager"→ {1, "Villager"}
//   "Iron Bar"  → {1, "Iron Bar"}
std::pair<int, std::string> RecipeManager::ParseInput(const std::string& raw) {
    // 嘗試 "Nx " 格式（數字 + x + 空格）
    size_t xPos = raw.find('x');
    if (xPos != std::string::npos && xPos > 0 && xPos + 1 < raw.size() && raw[xPos + 1] == ' ') {
        bool allDigits = true;
        for (size_t i = 0; i < xPos; i++)
            if (!std::isdigit(static_cast<unsigned char>(raw[i]))) { allDigits = false; break; }
        if (allDigits) {
            return { std::stoi(raw.substr(0, xPos)), raw.substr(xPos + 2) };
        }
    }

    // 嘗試 "N " 格式（數字 + 空格開頭）
    if (!raw.empty() && std::isdigit(static_cast<unsigned char>(raw[0]))) {
        size_t spacePos = raw.find(' ');
        if (spacePos != std::string::npos) {
            bool allDigits = true;
            for (size_t i = 0; i < spacePos; i++)
                if (!std::isdigit(static_cast<unsigned char>(raw[i]))) { allDigits = false; break; }
            if (allDigits)
                return { std::stoi(raw.substr(0, spacePos)), raw.substr(spacePos + 1) };
        }
    }

    return { 1, raw };
}

// ─────────────────────────────────────────────────────────────
void RecipeManager::LoadCraftingRecipes(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("無法開啟合成配方檔: {}", filePath);
        return;
    }

    json j;
    file >> j;

    for (const auto& item : j) {
        CraftingRecipe recipe;
        recipe.output = item.value("output", "");

        for (const auto& inp : item["inputs"]) {
            auto [count, name] = ParseInput(inp.get<std::string>());
            for (int i = 0; i < count; i++)
                recipe.inputs.push_back(name);
        }
        std::sort(recipe.inputs.begin(), recipe.inputs.end());
        recipe.time = item.value("time", 10.0f);
        m_CraftingRecipes.push_back(recipe);
    }
    LOG_INFO("載入 {} 條合成配方", m_CraftingRecipes.size());
}

// ─────────────────────────────────────────────────────────────
std::string RecipeManager::CheckCrafting(std::shared_ptr<Card> stackBottom, float& outTime) const {
    std::vector<std::string> stackNames;
    for (auto cur = stackBottom; cur; cur = cur->GetCardAbove())
        stackNames.push_back(cur->GetName());
    std::sort(stackNames.begin(), stackNames.end());

    for (const auto& recipe : m_CraftingRecipes) {
        if (recipe.inputs == stackNames) {
            outTime = recipe.time;
            return recipe.output;
        }
    }
    return "";
}
