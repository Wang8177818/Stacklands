#include "core/App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "cards/CharacterCard.hpp"
#include "cards/Sellslot.hpp"
#include <random>

static std::mt19937 s_AppRng{ std::random_device{}() };

// ─────────────────────────────────────────────────────────────
void App::Start() {
    LOG_TRACE("Start");

    m_UIManager    = std::make_unique<UIManager>(m_Renderer);
    m_CardManager  = std::make_unique<CardManager>(m_Renderer);
    m_EventManager = std::make_unique<EventManager>();

    m_UIManager->InitMenu();

    m_CurrentState = State::MAIN_MENU;
}

// ─────────────────────────────────────────────────────────────
void App::MainMenu() {
    LOG_TRACE("MainMenu");

    mousePos = Util::Input::GetCursorPosition();

    auto event = m_UIManager->UpdateMenu(mousePos);

    switch (event) {
        case UIManager::MenuEvent::START_GAME:
            LOG_INFO("Start Button Clicked!");
            m_UIManager->TransitionToGame();
            // 遊戲 UI 建好後，把 GameField 與 UIManager 交給 EventManager
            m_EventManager->SetGameField(m_UIManager->GetGameFieldImage());
            m_EventManager->SetUIManager(m_UIManager.get());
            m_EventManager->SetCardManager(m_CardManager.get());
            m_CardManager->SetFieldBounds(m_UIManager->GetGameFieldImage());
            m_CurrentState = State::GAME_INIT;
            break;

        case UIManager::MenuEvent::EXIT:
            LOG_INFO("Exit Button Clicked!");
            m_CurrentState = State::END;
            break;

        default:
            break;
    }

    m_Renderer.Update();
}

// ─────────────────────────────────────────────────────────────
void App::GameInit() {
    //卡牌大小去App.hpp調basic_scale
    // 重建 SellSlot & BlankSlots（支援 Game Over 後重新開始）
    m_SellSlot = std::make_shared<SellSlot>(-500, 420);
    m_BlankSlots.clear();
    constexpr float SLOT_SPACING_X = 160.0f;
    const std::pair<std::string, int> blankSlotData[] = {
        {"Humble Beginnings",     3},
        {"Seeking Wisdom",    4},
        {"Reap & Sow",    10},
        {"Curious Cuisine",    10},
        {"Logic and Reason",    15},
        {"Explorers",   20},
        {"Order and Structure", 25},
    };
    for (int i = 0; i < 7; ++i) {
        float x = -500.0f + SLOT_SPACING_X * (i + 1);
        m_BlankSlots.push_back(std::make_shared<BlankSlot>(
            x, 420.0f, blankSlotData[i].first, blankSlotData[i].second));
    }

    m_CardManager->AddCard(m_SellSlot);
    for (auto& slot : m_BlankSlots) m_CardManager->AddCard(slot);
    m_Renderer.Update();
    // 讀 json
    m_CardManager->LoadCardDatabase(RESOURCE_DIR"/Data/Cards.json");
    m_CardManager->LoadPackDatabase(RESOURCE_DIR"/Data/Packs.json");
    m_CardManager->LoadProfessionRecipes(RESOURCE_DIR"/Data/Profession.json");
    m_CardManager->LoadCraftingRecipes(RESOURCE_DIR"/Data/Recipe.json");

    m_CardManager->SpawnCardByName("Villager", basic_scale);
    m_CardManager->SpawnCardByName("Villager", basic_scale);
    m_CardManager->SpawnCardByName("Villager", basic_scale);
    m_CardManager->SpawnCardByName("Chainmail Armor",basic_scale);
    m_CardManager->SpawnCardByName("Chainmail Armor",basic_scale);
    m_CardManager->SpawnCardByName("Chainmail Armor",basic_scale);
    m_CardManager->SpawnCardByName("Hammer",basic_scale);
    m_CardManager->SpawnCardByName("Hammer",basic_scale);
    m_CardManager->SpawnCardByName("Hammer",basic_scale);
    m_CardManager->SpawnCardByName("Stew",basic_scale);
    m_CardManager->SpawnCardByName("Stew",basic_scale);
    m_CardManager->SpawnCardByName("Stew",basic_scale);
    m_CardManager->SpawnCardByName("Stew",basic_scale);
    m_CardManager->SpawnCardByName("Demon",basic_scale);

    m_CurrentState = State::UPDATE;
}

// ─────────────────────────────────────────────────────────────
void App::Update() {
    LOG_DEBUG("{} {}", mousePos.x, mousePos.y);
    mousePos = Util::Input::GetCursorPosition();

    // ── 鏡頭移動 & 地圖縮放（完全委託給 EventManager）────
    auto cards = m_CardManager->GetAllCards();
    m_EventManager->Update(mousePos, m_CardManager->isDraggingCard(), cards);

    // ── 作弊選單（F1 開關）─────────────────────────────────
    if (Util::Input::IsKeyUp(Util::Keycode::F1)) {
        auto& cheat = m_UIManager->GetCheatMenu();
        if (!cheat.IsVisible()) {
            cheat.SetCardNames(m_CardManager->GetAllCardNames());
        }
        cheat.Toggle();
    }
    {
        auto& cheat = m_UIManager->GetCheatMenu();
        std::string spawnName = cheat.Update(mousePos);
        if (!spawnName.empty()) {
            float spawnScale = basic_scale * m_EventManager->GetZoomRatio();
            m_CardManager->SpawnCardByName(spawnName, spawnScale);
        }
    }

    // ── 卡片更新 ──────────────────────────────────────────
    m_CardManager->SetZoomRatio(m_EventManager->GetZoomRatio());
    m_CardManager->Update(mousePos);

    // ── 卡片數量 UI 更新 ─────────────────────────────────
    m_UIManager->UpdateCardCount(m_CardManager->GetCardCount(),m_CardManager->GetMaxCardCount());

    // ── 金幣數量 UI 更新 ─────────────────────────────────
    m_UIManager->UpdateCoinCount(m_CardManager->GetCoinCount());

    // ── 食物數量 UI 更新 ─────────────────────────────────
    m_UIManager->UpdateFood(m_CardManager->GetTotalFoodSupply(),m_CardManager->GetNeededFoodCount());

    // ── 懸停卡片敘述更新 ─────────────────────────────────
    // 暫停時不顯示懸停敘述（敘述欄底圖也已經被隱藏）
    if (m_EventManager->GetGameState() == EventManager::GameTime::PAUSE) {
        m_UIManager->UpdateDescriptionName("");
        m_UIManager->UpdateDescriptionText("");
    } else {
        std::shared_ptr<Card> hoveredCard = nullptr;
        int highestZ = -9999;
        for (auto& card : m_CardManager->GetAllCards()) {
            if (card->GetType() == CardType::INTERACT) continue;
            if (card->IsMouseHovering(mousePos)) {
                int z = card->GetGameObjects()[0]->GetZIndex();
                if (z > highestZ) {
                    highestZ = z;
                    hoveredCard = card;
                }
            }
        }
        m_UIManager->UpdateDescriptionName(
            hoveredCard ? hoveredCard->GetName() : "");
        m_UIManager->UpdateDescriptionText(
            hoveredCard ? hoveredCard->GetDescription() : "");
    }

    int sellPrice = m_SellSlot->GetTotalPrice();
    if (sellPrice > 0) {
        float spawnScale = basic_scale * m_EventManager->GetZoomRatio();
        // 生成一Coin作為堆疊底部
        auto topCoin = m_CardManager->SpawnCardByName("Coin", spawnScale);
        // 嘗試將第一枚硬幣疊加到附近已有的硬幣上
        m_CardManager->TryAutoStack(topCoin);
        for (int i = 1; i < sellPrice; i++) {
            auto newCoin = m_CardManager->SpawnCardByName("Coin", spawnScale);
            topCoin->SetCardAbove(newCoin);
            newCoin->SetCardBelow(topCoin);
            topCoin = newCoin;
        }
        auto soldCards = m_SellSlot->PopAllCards();
        for (auto& card : soldCards) {
            card->OnSold();
            m_CardManager->RemoveCard(card);
        }
    }

    // ── BlankSlot 購買檢查 ────────────────────────────────
    // 每個格子若堆疊的硬幣數 >= 售價 → 扣硬幣、生成對應卡包；多餘的退回
    for (auto& slot : m_BlankSlots) {
        const int price = slot->GetBuyPrice();
        if (price <= 0) continue;
        const int coinCount = slot->GetCoinCount();
        if (coinCount < price) continue;

        const float spawnScale = basic_scale * m_EventManager->GetZoomRatio();
        const float sx = slot->GetX();
        const float sy = slot->GetY();

        // 扣掉所有疊在上面的硬幣
        auto coins = slot->PopAllCoins();
        for (auto& c : coins) m_CardManager->RemoveCard(c);

        // 生卡包（位置稍微往下方，避免又疊回 slot 上）
        m_CardManager->SpawnPackByName(slot->GetName(), spawnScale, sx, sy - 200.f);

        // 找零：超付的部分以單顆 Coin 退回，自動疊加成一疊
        const int refund = coinCount - price;
        if (refund > 0) {
            auto topCoin = m_CardManager->SpawnCardByName("Coin", spawnScale, sx, sy - 400.f);
            m_CardManager->TryAutoStack(topCoin);
            for (int i = 1; i < refund; ++i) {
                auto newCoin = m_CardManager->SpawnCardByName("Coin", spawnScale);
                topCoin->SetCardAbove(newCoin);
                newCoin->SetCardBelow(topCoin);
                topCoin = newCoin;
            }
        }
    }

    // ── Game Over 檢查 ─────────────────────────────────────
    int charCount = m_CardManager->GetCharacterCount();
    if (charCount > 0) m_HadCharacters = true;
    if (m_HadCharacters && charCount == 0) {
        LOG_INFO("Game Over: no characters remaining");
        m_HadCharacters = false;
        // 清除場上所有卡片與浮動文字
        m_CardManager->ClearFloatingText();
        m_CardManager->ClearAllCards();
        m_BlankSlots.clear();
        m_SellSlot = nullptr;
        // 重置月份進度條
        m_EventManager->Reset();
        m_UIManager->TransitionToMenu();
        m_CurrentState = State::MAIN_MENU;
        return;
    }

    m_Renderer.Update();

    if (m_EventManager->IsRequestingExit() || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// ─────────────────────────────────────────────────────────────
void App::End() {
    LOG_TRACE("End");
}
