# 此檔為建構檔案清單的唯一來源（single source of truth）。
# 新增/刪除 .cpp 或 .hpp 時，只需在此維護，CMakeLists.txt 不必再改。
# main.cpp 由 CMakeLists.txt 另外加入，故不列於此。

set(SRC_FILES
    # 核心 / 入口
    App.cpp
    # 卡片
    Card.cpp
    CardPack.cpp
    CardFactory.cpp
    CombatCard.cpp
    WanderingCard.cpp
    AnimalCard.cpp
    MonsterCard.cpp
    # 戰鬥
    CombatArena.cpp
    # 管理器 / 系統
    CardManager.cpp
    EventManager.cpp
    RecipeManager.cpp
    TaskScheduler.cpp
    UIManager.cpp
    # UI / 輔助
    CheatMenu.cpp
    FloatingTextManager.cpp
)

set(INCLUDE_FILES
    # 核心 / 入口
    App.hpp
    GameConstants.hpp
    ISpawnListener.hpp
    # 卡片基底與型別
    Card.hpp
    CardData.hpp
    CardPack.hpp
    CardFactory.hpp
    CombatCard.hpp
    WanderingCard.hpp
    # 具體卡片
    AnimalCard.hpp
    MonsterCard.hpp
    CharacterCard.hpp
    BuildingCard.hpp
    StructureCard.hpp
    WarehouseCard.hpp
    ResourceCard.hpp
    FoodCard.hpp
    CoinCard.hpp
    IdeaCard.hpp
    LocationCard.hpp
    EquipmentCard.hpp
    # 戰鬥
    CombatArena.hpp
    AttackResolver.hpp
    EffectData.hpp
    # 管理器 / 系統
    CardManager.hpp
    EventManager.hpp
    RecipeManager.hpp
    TaskScheduler.hpp
    UIManager.hpp
    # UI / 輔助
    CheatMenu.hpp
    FloatingTextManager.hpp
    BackgroundImage.hpp
    Button.hpp
    TimeBar.hpp
    BlankSlot.hpp
    Sellslot.hpp
    # 第三方
    nlohmann/json.hpp
)

set(TEST_FILES
    CombatTests.cpp
)
