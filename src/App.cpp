#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "CharacterCard.hpp"
#include "Sellslot.hpp"
#include <random>

static std::mt19937 s_AppRng{ std::random_device{}() };

// ─────────────────────────────────────────────────────────────
void App::Start() {
    LOG_TRACE("Start");

    m_UIManager    = std::make_unique<UIManager>(m_Renderer);
    m_CardManager  = std::make_unique<CardManager>(m_Renderer);
    m_EventManager = std::make_unique<EventManager>();
    m_SellSlot     = std::make_shared<SellSlot>(-250, 210);

    // ── SellSlot 右側 7 個空白格子（顯示名稱 + 價格，功能未接）─────
    constexpr float SLOT_SPACING_X = 80.0f;  // 加大間距以容納較長的卡包名稱
    // {名稱, 價格} — 修改這 7 項即可調整每個格子
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
        float x = -250.0f + SLOT_SPACING_X * (i + 1);
        m_BlankSlots.push_back(std::make_shared<BlankSlot>(
            x, 210.0f, blankSlotData[i].first, blankSlotData[i].second));
    }

    m_UIManager->InitMenu();

    m_CurrentState = State::MAIN_MENU;
}

// ─────────────────────────────────────────────────────────────
void App::MainMenu() {
    LOG_TRACE("MainMenu");

    mousePos = Util::Input::GetCursorPosition();
    LOG_DEBUG("{} {}", mousePos.x, mousePos.y);

    auto event = m_UIManager->UpdateMenu(mousePos);

    switch (event) {
        case UIManager::MenuEvent::START_GAME:
            LOG_INFO("Start Button Clicked!");
            m_UIManager->TransitionToGame();
            // 遊戲 UI 建好後，把 GameField 與 UIManager 交給 EventManager
            m_EventManager->SetGameField(m_UIManager->GetGameFieldImage());
            m_EventManager->SetUIManager(m_UIManager.get());
            m_EventManager->SetCardManager(m_CardManager.get());
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
    m_CardManager->AddCard(m_SellSlot);
    for (auto& slot : m_BlankSlots) m_CardManager->AddCard(slot);
    m_Renderer.Update();
    // 讀 json
    m_CardManager->LoadCardDatabase(RESOURCE_DIR"/Data/Cards.json");
    m_CardManager->LoadPackDatabase(RESOURCE_DIR"/Data/Packs.json");
    m_CardManager->LoadProfessionRecipes(RESOURCE_DIR"/Data/Profession.json");
    m_CardManager->LoadCraftingRecipes(RESOURCE_DIR"/Data/Recipe.json");

    m_CardManager->SpawnCardByName("Villager", basic_scale);
    m_CardManager->SpawnCardByName("Chicken",basic_scale);
    m_CardManager->SpawnCardByName("Chainmail Armor",basic_scale);
    m_CardManager->SpawnCardByName("Bear",basic_scale);
    m_CardManager->SpawnCardByName("Smelter",basic_scale);
    m_CardManager->SpawnCardByName("Iron Ore",basic_scale);
    m_CardManager->SpawnCardByName("Stew",basic_scale);
    m_CardManager->SpawnCardByName("Magic Dust",basic_scale);
    m_CardManager->SpawnCardByName("Charcoal",basic_scale);
    m_CardManager->SpawnCardByName("Coin Chest",basic_scale);
    m_CardManager->SpawnCardByName("Hotpot",basic_scale);
    m_CardManager->SpawnCardByName("Resource Chest",basic_scale);

    m_CardManager->SpawnPackByName("A New World", basic_scale);
    m_CurrentState = State::UPDATE;
}

// ─────────────────────────────────────────────────────────────
void App::Update() {
    mousePos = Util::Input::GetCursorPosition();

    // ── 鏡頭移動 & 地圖縮放（完全委託給 EventManager）────
    auto cards = m_CardManager->GetAllCards();
    m_EventManager->Update(mousePos, m_CardManager->isDraggingCard(), cards);

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
        m_CardManager->SpawnPackByName(slot->GetName(), spawnScale, sx, sy - 100.f);

        // 找零：超付的部分以單顆 Coin 退回（在卡包旁邊散開）
        const int refund = coinCount - price;
        std::uniform_real_distribution<float> off(-60.f, 60.f);
        for (int i = 0; i < refund; ++i) {
            m_CardManager->SpawnCardByName("Coin", spawnScale,
                                           sx + off(s_AppRng),
                                           sy - 200.f + off(s_AppRng));
        }
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
