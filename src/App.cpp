#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "CharacterCard.hpp"
#include "Sellslot.hpp"
#include "nlohmann/json.hpp"
#include <cstdio>
#include <fstream>
#include <random>

static std::mt19937 s_AppRng{ std::random_device{}() };

constexpr const char* SAVE_FILE_PATH = "save.json";

namespace {
// 讀取存檔的月份；找不到或解析失敗回傳 -1
int PeekSaveMonth() {
    std::ifstream f(SAVE_FILE_PATH);
    if (!f.is_open()) return -1;
    try {
        nlohmann::json j; f >> j;
        if (j.contains("month")) return j["month"].get<int>();
    } catch (...) {}
    return -1;
}
} // namespace

// ─────────────────────────────────────────────────────────────
void App::Start() {
    LOG_TRACE("Start");

    m_UIManager    = std::make_unique<UIManager>(m_Renderer);
    m_CardManager  = std::make_unique<CardManager>(m_Renderer);
    m_EventManager = std::make_unique<EventManager>();

    // 偵測存檔（找得到就顯示「繼續遊戲(N月)」按鈕）
    const int saveMonth = PeekSaveMonth();
    m_UIManager->InitMenu(saveMonth);

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
            m_EventManager->SetGameField(m_UIManager->GetGameFieldImage());
            m_EventManager->SetUIManager(m_UIManager.get());
            m_EventManager->SetCardManager(m_CardManager.get());
            m_CardManager->SetFieldBounds(m_UIManager->GetGameFieldImage());
            // 新遊戲：清掉殘留的月份 / 進度條（避免從繼續→返回→新遊戲時殘留）
            m_EventManager->month = 1;
            m_EventManager->tick  = 0;
            m_UIManager->UpdateMonth(1);
            // 重置遊戲時間狀態為 NORMAL（之前若進過暫停選單可能殘留 PAUSE）
            m_EventManager->SetGameState(EventManager::GameTime::NORMAL);
            m_EventManager->SetTimeBlocked(false);
            // 新遊戲卡片以 basic_scale 全新生成（等同 zoom=1），重置縮放避免殘留
            m_EventManager->SetZoomRatio(1.0f);
            // 啟用「第二包保底村民」
            m_CardManager->ResetNewGameState();
            m_LoadOnInit = false;
            m_CurrentState = State::GAME_INIT;
            break;

        case UIManager::MenuEvent::LOAD_GAME:
            LOG_INFO("Load Button Clicked!");
            m_UIManager->TransitionToGame();
            m_EventManager->SetGameField(m_UIManager->GetGameFieldImage());
            m_EventManager->SetUIManager(m_UIManager.get());
            m_EventManager->SetCardManager(m_CardManager.get());
            m_CardManager->SetFieldBounds(m_UIManager->GetGameFieldImage());
            // 讀檔同樣重置為 NORMAL（讀檔本身不保存遊戲時間狀態）
            m_EventManager->SetGameState(EventManager::GameTime::NORMAL);
            m_EventManager->SetTimeBlocked(false);
            m_LoadOnInit = true;
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
    // 先同步縮放：GameInit 在 App::Update 之前執行，若不同步，CardManager 內
    // 殘留的 m_ZoomRatio 會讓本階段生成的卡包存錯基準 scale（開包後變大/變小）
    m_CardManager->SetZoomRatio(m_EventManager->GetZoomRatio());

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

    m_CardManager->SpawnPackByName("A New World", basic_scale);

    // 如果是「繼續遊戲」進來，覆蓋上面新建的卡片
    if (m_LoadOnInit) {
        LoadGame();
    }

    m_CurrentState = State::UPDATE;
}

// ─────────────────────────────────────────────────────────────
void App::SaveGame() {
    if (!m_CardManager || !m_EventManager) return;
    nlohmann::json j = m_CardManager->ToJson();
    j["month"]     = static_cast<int>(m_EventManager->month);
    j["tick"]      = m_EventManager->tick;
    j["zoomRatio"] = m_EventManager->GetZoomRatio();
    // 鏡頭位置（GameField 的 transform）
    if (auto field = m_UIManager->GetGameFieldImage()) {
        j["camera"] = {
            {"x",  field->m_Transform.translation.x},
            {"y",  field->m_Transform.translation.y},
            {"sx", field->m_Transform.scale.x},
            {"sy", field->m_Transform.scale.y}
        };
    }
    // BlankSlot 上累積的硬幣數（依 m_BlankSlots 索引順序）
    nlohmann::json slotCoins = nlohmann::json::array();
    for (auto& slot : m_BlankSlots) slotCoins.push_back(slot->GetCoinCount());
    j["blankSlotCoins"] = slotCoins;
    std::ofstream f(SAVE_FILE_PATH);
    if (!f.is_open()) { LOG_ERROR("無法寫入存檔: {}", SAVE_FILE_PATH); return; }
    f << j.dump(2);
    LOG_INFO("已存檔（月份 {}, 卡片 {} 張）",
             j["month"].get<int>(),
             static_cast<int>(j.value("cards", nlohmann::json::array()).size()));
}

// ─────────────────────────────────────────────────────────────
void App::LoadGame() {
    if (!m_CardManager || !m_EventManager) return;
    std::ifstream f(SAVE_FILE_PATH);
    if (!f.is_open()) { LOG_WARN("找不到存檔 {}", SAVE_FILE_PATH); return; }
    nlohmann::json j;
    try { f >> j; } catch (...) {
        LOG_ERROR("存檔解析失敗");
        return;
    }
    // 先還原縮放再重建卡片：LoadFromJson 內重建卡包時會用 m_ZoomRatio 換算基準 scale，
    // 必須在重建前同步，否則還原的卡包開出來大小會錯。
    if (j.contains("zoomRatio")) {
        const float z = j["zoomRatio"].get<float>();
        m_EventManager->SetZoomRatio(z);
        m_CardManager->SetZoomRatio(z);
    }
    m_CardManager->LoadFromJson(j, basic_scale);
    if (j.contains("month")) {
        m_EventManager->month = static_cast<float>(j["month"].get<int>());
        m_UIManager->UpdateMonth(static_cast<int>(m_EventManager->month));
    }
    if (j.contains("tick"))      m_EventManager->tick = j["tick"].get<float>();
    if (j.contains("camera")) {
        const auto& cam = j["camera"];
        if (auto field = m_UIManager->GetGameFieldImage()) {
            field->m_Transform.translation = glm::vec2(
                cam.value("x", 0.0f), cam.value("y", 0.0f));
            field->m_Transform.scale = glm::vec2(
                cam.value("sx", 1.0f), cam.value("sy", 1.0f));
        }
        // INTERACT 卡（SellSlot / BlankSlot）在 ToJson 被跳過，GameInit 把它們
        // 建在「初始」位置；讀檔還原 GameField 鏡頭後，要對它們補上同樣的
        // zoom + pan 變換才不會跑位。
        const float zoom = m_EventManager->GetZoomRatio();
        const glm::vec2 pan(cam.value("x", 0.0f), cam.value("y", 0.0f));
        auto applyCameraDelta = [&](const std::shared_ptr<Card>& c) {
            if (!c) return;
            if (zoom != 1.0f) c->ScaleAroundPivot(zoom, glm::vec2(0.f, 0.f));
            if (pan.x != 0.f || pan.y != 0.f) c->MoveBy(pan);
        };
        applyCameraDelta(m_SellSlot);
        for (auto& slot : m_BlankSlots) applyCameraDelta(slot);
    }
    // BlankSlot 硬幣還原（在每個 slot 上重建堆疊）
    if (j.contains("blankSlotCoins")) {
        const auto& counts = j["blankSlotCoins"];
        const float zoom   = m_EventManager->GetZoomRatio();
        const float scale  = basic_scale * zoom;
        for (std::size_t i = 0; i < counts.size() && i < m_BlankSlots.size(); ++i) {
            const int n = counts[i].is_number_integer() ? counts[i].get<int>() : 0;
            if (n <= 0) continue;
            auto slot = m_BlankSlots[i];
            std::shared_ptr<Card> top = slot;
            for (int k = 0; k < n; ++k) {
                auto coin = m_CardManager->SpawnCardByName("Coin", scale, slot->GetX(), slot->GetY());
                if (!coin) break;
                top->SetCardAbove(coin);
                coin->SetCardBelow(top);
                top = coin;
            }
        }
    }
    // 讀檔當下若卡片已超量，立即點亮警告並凍結時間（不必等下個月底）
    m_CardManager->ArmOverflowWarningIfOver();
    LOG_INFO("已讀檔（月份 {}）", static_cast<int>(m_EventManager->month));
}

// ─────────────────────────────────────────────────────────────
void App::Update() {
    LOG_DEBUG("{} {}", mousePos.x, mousePos.y);
    mousePos = Util::Input::GetCursorPosition();

    // ── 鏡頭移動 & 地圖縮放（完全委託給 EventManager）────
    // 滑鼠在作弊選單上時不能拖鏡頭也不能滾輪縮放
    auto cards = m_CardManager->GetAllCards();
    const bool overCheat = m_UIManager->GetCheatMenu().IsCapturingMouse(mousePos);
    m_EventManager->SetMouseBlocked(overCheat);
    m_EventManager->Update(mousePos,
                           m_CardManager->isDraggingCard() || overCheat,
                           cards);

    // ── 作弊選單（F1 開關）─────────────────────────────────
    if (Util::Input::IsKeyUp(Util::Keycode::F1)) {
        auto& cheat = m_UIManager->GetCheatMenu();
        if (!cheat.IsVisible()) {
            cheat.SetCardEntries(m_CardManager->GetAllCardEntries());
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

    // ── 卡片超量警告 + 強制暫停（敘述欄底部）──────────────
    // 月底結算超量會點亮；玩家賣到不超量才自動解除、遊戲恢復進行
    m_CardManager->ClearOverflowWarningIfResolved();
    const bool overflow = m_CardManager->IsCardOverflowWarning();
    int overflowCount = m_CardManager->GetCardCount() - m_CardManager->GetMaxCardCount();
    if (overflowCount < 0) overflowCount = 0;
    // 凍結遊戲時間直到玩家賣完（仍可拖曳賣卡）
    m_EventManager->SetTimeBlocked(overflow);
    // 暫停選單開啟時不顯示警告文字（避免蓋住選單），但時間仍凍結
    const bool showOverflowWarn =
        overflow && m_EventManager->GetGameState() != EventManager::GameTime::PAUSE;
    m_UIManager->SetCardOverflowWarning(showOverflowWarn, overflowCount);

    int sellPrice = m_SellSlot->GetTotalPrice();
    if (sellPrice > 0) {
        // 用 SellSlot 自身的實際縮放，確保金幣與場上卡片同大小
        // （避免 m_ZoomRatio 殘留導致 basic_scale*zoom 與場上卡片不一致）
        float spawnScale = m_SellSlot->GetScale();
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

        // 用 slot 自身的實際縮放，確保卡包 / 找零金幣與場上卡片同大小
        const float spawnScale = slot->GetScale();
        const float sx = slot->GetX();
        const float sy = slot->GetY();

        // 扣掉所有疊在上面的硬幣
        auto coins = slot->PopAllCoins();
        for (auto& c : coins) m_CardManager->RemoveCard(c);

        // 生卡包（位置稍微往下方，避免又疊回 slot 上）
        m_CardManager->SpawnPackByName(slot->GetName(), spawnScale, sx, sy - 200.f);

        // 找零：超付的部分以單顆 Coin 退回（在卡包旁邊散開）
        const int refund = coinCount - price;
        std::uniform_real_distribution<float> off(-120.f, 120.f);
        for (int i = 0; i < refund; ++i) {
            m_CardManager->SpawnCardByName("Coin", spawnScale,
                                           sx + off(s_AppRng),
                                           sy - 400.f + off(s_AppRng));
        }
    }

    // ── Game Over 檢查（所有村民死光）────────────────────
    int charCount = m_CardManager->GetCharacterCount();
    if (charCount > 0) m_HadCharacters = true;
    if (m_HadCharacters && charCount == 0) {
        LOG_INFO("Game Over: no characters remaining");
        m_HadCharacters = false;
        const int monthsPlayed = static_cast<int>(m_EventManager->month);
        // 顯示 Game Over 畫面，等玩家按返回；卡片暫不清除（讓畫面有背景）
        m_UIManager->ShowGameOver(monthsPlayed);
        m_CurrentState = State::GAME_OVER;
        return;
    }

    m_Renderer.Update();

    // 暫停選單按下「返回選單」→ 存檔 + 回主選單（不結束程式）
    if (m_EventManager->IsRequestingExit()) {
        SaveGame();
        const int monthsPlayed = static_cast<int>(m_EventManager->month);
        m_CardManager->ClearAllCards();
        m_BlankSlots.clear();
        m_SellSlot = nullptr;
        m_HadCharacters = false;
        m_EventManager->ClearExitRequest();  // 重置旗標，下次進遊戲不會立刻又跳回
        m_UIManager->RefreshLoadGameButton(monthsPlayed);
        m_UIManager->TransitionToMenu();
        m_CurrentState = State::MAIN_MENU;
        return;
    }
    // 視窗 X 按鈕關閉：結束程式（End() 會自動存檔）
    if (Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// ─────────────────────────────────────────────────────────────
void App::GameOver() {
    mousePos = Util::Input::GetCursorPosition();
    if (m_UIManager->UpdateGameOver(mousePos)) {
        // 玩家點了返回選單：清乾淨、回主選單；同時刪除存檔（不能繼續死局）
        m_UIManager->HideGameOver();
        m_CardManager->ClearAllCards();
        m_BlankSlots.clear();
        m_SellSlot = nullptr;
        std::remove(SAVE_FILE_PATH);
        // 既然存檔已刪，主選單的「繼續遊戲」按鈕也應該不再顯示
        m_UIManager->ClearLoadGameButton();
        m_UIManager->TransitionToMenu();
        m_CurrentState = State::MAIN_MENU;
    }
    m_Renderer.Update();
}

// ─────────────────────────────────────────────────────────────
void App::End() {
    LOG_TRACE("End");
    // 若曾經進入遊戲（已建立 EventManager 並有 GameField），離開前自動存檔
    // 但若已進入 Game Over，存檔已被刪除不需要重存
    if (m_EventManager && m_UIManager && m_UIManager->GetGameFieldImage()) {
        SaveGame();
    }
}
