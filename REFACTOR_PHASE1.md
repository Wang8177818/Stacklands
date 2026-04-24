# 重構第一階段紀錄（Phase 1）

執行日期：2026/4/18–19  
分支：howhowdy2

---

## 改動總覽

| # | 改動類型 | 檔案 | 說明 |
|---|---------|------|------|
| 1 | **新增** | `include/GameConstants.hpp` | 集中管理所有 magic numbers |
| 2 | **修改** | `include/Card.hpp` | 加入 `InitLabelText` / `RebuildLabelText` 宣告；include GameConstants |
| 3 | **修改** | `src/Card.cpp` | 實作兩個輔助方法；全面替換 magic numbers；使用 `FONT_REGULAR` |
| 4 | **修改** | `include/ResourceCard.hpp` | 用 `InitLabelText` 取代手工初始化；用常數取代 `0.32f/-0.3f/42` |
| 5 | **修改** | `include/EquipmentCard.hpp` | 同上 |
| 6 | **修改** | `include/BuildingCard.hpp` | 同上；`SetScale` 改用 `RebuildLabelText` |
| 7 | **修改** | `include/FoodCard.hpp` | 兩個文字物件皆用輔助方法；用常數 |
| 8 | **修改** | `include/StructureCard.hpp` | 同上；`RefreshResourceText` 改用 `RebuildLabelText` |
| 9 | **修改** | `include/CharacterCard.hpp` | 用 `InitLabelText`；`0.34f/-0.3f` 改為常數 |
| 10 | **修改** | `src/CardPack.cpp` | `3000` 改 `PACK_FONT_SCALE`；`42` 改 `Z_DRAG_EXTRA`；用 `FONT_REGULAR` |
| 11 | **修改** | `src/EventManager.cpp` | `120000/265` 改為 `MONTH_DURATION_MS/PROGRESS_BAR_WIDTH` |
| 12 | **新增** | `include/CardFactory.hpp` | CardFactory 宣告 |
| 13 | **新增** | `src/CardFactory.cpp` | 用 `switch` 取代原本 8 個 `if-else`，依型別建立卡牌 |
| 14 | **修改** | `src/CardManager.cpp` | `CreateCardFromData` 改呼叫 `CardFactory::Create`；include CardFactory；`10000.0f` 改 `GATHER_TIME_MS` |
| 15 | **修改** | `files.cmake` | 加入 `CardFactory.cpp` 和 `CardFactory.hpp`、`GameConstants.hpp` |

---

## 各常數的對照表

| 原本的值 | 新常數 | 出現在 |
|---------|--------|--------|
| `850.0f` | `GameConstants::BASE_CARD_WIDTH` | Card.cpp（2處）|
| `1250.0f` | `GameConstants::BASE_CARD_HEIGHT` | Card.cpp（2處）|
| `1000` | `GameConstants::CARD_FONT_SCALE` | Card.cpp, 全部子類（10+處）|
| `3000` | `GameConstants::PACK_FONT_SCALE` | CardPack.cpp（3處）|
| `-0.32f` | `GameConstants::PRICE_OFFSET_X` | Resource/Equipment/Building/Food/Structure（5處）|
| `-0.3f` | `GameConstants::PRICE_OFFSET_Y` | 同上（5處）|
| `0.32f` | `GameConstants::SECONDARY_OFFSET_X` | FoodCard, StructureCard（2處）|
| `0.34f` | `GameConstants::HEALTH_OFFSET_X` | CharacterCard（1處）|
| `10` | `GameConstants::Z_BACKGROUND` | Card.cpp（3處）|
| `11` | `GameConstants::Z_ICON` | Card.cpp（2處）|
| `40` | `GameConstants::Z_DRAG_BG` | Card.cpp（1處）|
| `41` | `GameConstants::Z_DRAG_TEXT` | Card.cpp（2處）|
| `42` | `GameConstants::Z_DRAG_EXTRA` | 全部有 PriceText 的子類（7處）|
| `0.3f` | `GameConstants::FOLLOW_SPEED` | Card.cpp（1處）|
| `0.15f` | `GameConstants::STACK_OFFSET_Y` | Card.cpp（1處）|
| `0.6f` | `GameConstants::ICON_SCALE_FACTOR` | Card.cpp（2處）|
| `3` | `GameConstants::STACK_Z_STEP` | Card.cpp（3處）|
| `120000` | `GameConstants::MONTH_DURATION_MS` | EventManager.cpp（3處）|
| `265` | `GameConstants::PROGRESS_BAR_WIDTH` | EventManager.cpp（2處）|
| `10000.0f` | `GameConstants::GATHER_TIME_MS` | CardManager.cpp（1處）|
| `RESOURCE_DIR"/Font/msjh.ttc"` | `FONT_REGULAR` macro | Card.cpp, CardPack.cpp（10+處）|

---

## 新增的輔助方法

### `Card::InitLabelText(text, color, zOffset=1)`
**位置：** `include/Card.hpp:protected` / `src/Card.cpp`

建立一個新的附屬文字 GameObject，自動套用：
- 字體 `FONT_REGULAR`
- 字級 = `max(1, CARD_FONT_SCALE * m_Scale)`
- Transform scale = `{m_Scale, m_Scale}`
- ZIndex = `m_Background->GetZIndex() + zOffset`

**取代了什麼：** 各子類建構子中的 6 行重複程式碼（共 7 個類別）

```cpp
// 舊寫法（每個子類都要複製）
m_PriceText = std::make_shared<Util::GameObject>();
int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
m_PriceText->SetDrawable(std::make_shared<Util::Text>(
    RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(sellValue), color));
m_PriceText->m_Transform.scale = {m_Scale, m_Scale};
m_PriceText->SetZIndex(m_Background->GetZIndex() + 1);

// 新寫法
m_PriceText = InitLabelText(std::to_string(sellValue), color);
```

---

### `Card::RebuildLabelText(obj, text, color)`
**位置：** `include/Card.hpp:protected` / `src/Card.cpp`

重建已有 GameObject 的 Drawable（scale 改變後呼叫），自動套用新字級和縮放。

**取代了什麼：** 各子類 `SetScale()` 和 `RefreshResourceText()` 中的 4 行重複程式碼

```cpp
// 舊寫法
int fontSize = std::max(1, static_cast<int>(1000 * m_Scale));
m_PriceText->SetDrawable(std::make_shared<Util::Text>(
    RESOURCE_DIR"/Font/msjh.ttc", fontSize, std::to_string(m_SellValue), color));
m_PriceText->m_Transform.scale = {m_Scale, m_Scale};

// 新寫法
RebuildLabelText(m_PriceText, std::to_string(m_SellValue), color);
```

---

## CardFactory 說明

### 為何拆出來？

`CardManager::CreateCardFromData` 原本包含 8 個 `if-else if`，負責決定要建立哪種子類別。這違反了**單一職責原則**（CardManager 不應該知道每種卡牌的建構簽名）。

### 改動後的結構

```
CardManager::CreateCardFromData(x, y, data)
    └─ CardFactory::Create(x, y, data)  ← 所有子類 include 移到這裡
           switch (data.type)
           ├─ CHARACTER  → CharacterCard(...)
           ├─ RESOURCE   → ResourceCard(...)
           ├─ COIN       → CoinCard(...)
           ├─ EQUIPMENT  → EquipmentCard(...)
           ├─ BUILDING   → BuildingCard(...)
           ├─ FOOD       → FoodCard(...)
           ├─ STRUCTURE  → StructureCard(...)
           └─ default    → Card(...)
```

### 新增卡牌類型時的改動範圍

| 方式 | 改動的檔案數 |
|------|------------|
| 重構前 | CardManager.cpp（加 if-else）+ 新卡牌的 .hpp/.cpp |
| 重構後 | **CardFactory.cpp**（加一個 case）+ 新卡牌的 .hpp/.cpp |

CardManager.cpp 不再需要動。

---

## 注意事項

- `FONT_REGULAR` / `FONT_BOLD` 是 macro（非 constexpr），因為需要與 `RESOURCE_DIR` macro 做編譯期字串拼接
- `GameConstants::STACK_Z_STEP` 原本是 `3` 的 int literal，型別宣告為 `constexpr int`
- `CardPack.cpp` 中 `m_CountText` 的初始化**沒有**改用 `InitLabelText`，因為它使用 `PACK_FONT_SCALE(3000)` 而非 `CARD_FONT_SCALE(1000)`，邏輯不同
