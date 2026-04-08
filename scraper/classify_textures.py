"""
Texture2D 圖片分類器
- 從 Texture2D.zip 中找出對應 Cards_wiki.json 的卡牌圖片
- 依照卡牌類型分類放入對應資料夾
- 更新 JSON 的 iconPath 為實際圖片名稱
- 無對應卡牌的圖片不複製（視為刪除）
"""

import sys, json, zipfile, shutil, re, os
from pathlib import Path
from collections import defaultdict

if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

# ── 路徑設定 ──────────────────────────────────────────────────────────────────
ZIP_PATH    = Path(r"C:\Users\m0938\Downloads\res\Texture2D.zip")
JSON_PATH   = Path(r"D:\MyStuff\School\OOPL\Stacklands\Resources\Data\Cards_wiki.json")
OUTPUT_ROOT = Path(r"D:\MyStuff\School\OOPL\Stacklands\Resources\Image\card")

# ── 類型 → 資料夾映射 ─────────────────────────────────────────────────────────
TYPE_FOLDER = {
    "CHARACTER": "character",
    "RESOURCE":  "resource",
    "EQUIPMENT": "equipment",
    "FOOD":      "food",
    "BUILDING":  "building",
    "MONSTER":   "monster",
    "CREATURE":  "creature",
    "PACK":      "pack",
    "IDEA":      "idea",
    "COIN":      "resource",   # Coin 放在 resource 資料夾
}

# ── 圖片名稱 → 卡牌名稱 (明確對應表) ─────────────────────────────────────────
# key = zip 內的圖片名稱（不含 .png，大小寫完全對應）
# val = Cards_wiki.json 裡的 card["name"]
EXPLICIT_MAP = {
    # ── 荷蘭語名稱 ─────────────────────────────────────────────────
    "Aap":             "Monkey",
    "Banaan":          "Banana",
    "Haai":            "Shark",
    "Inktvis":         "Kraken",
    "Kampvuur":        "Campfire",
    "Kibbeling":       "Fish and Chips",   # 荷蘭炸魚料理
    "Krabbetje":       "Crab",
    "Makreel":         "Mackerel",
    "Oger":            "Ogre",
    "Spook":           "Ghost",
    "Vuurtoren":       "Lighthouse",
    "Zeemeermin":      "Merman",
    # ── 無空格 CamelCase → 含空格 ──────────────────────────────────
    "AnimalPen":       "Animal Pen",
    "BearClaw":        "Bear Claw",
    "BoneSpear":       "Bone Spear",
    "BoneStaff":       "Bone Staff",
    "BottleOfRum":     "Bottle of Rum",
    "BreedingPen":     "Breeding Pen",
    "BrokenBottle":    "Broken Bottle",
    "BunnyHat":        "Rabbit Hat",
    "CaneSugar":       "Cane Sugar",
    "ChellChest":      "Shell Chest",     # typo Shell
    "ChilliPepper":    "Chili Pepper",
    "Coin-Chest":      "Coin Chest",
    "CookedCrabMeat":  "Cooked Crab",
    "CrabArmor":       "Crab Scale Armor",
    "Chainmail":       "Chainmail Armor",
    "DarkAmulet":      "Dark Amulet",
    "DarkElf":         "Dark Elf",
    "Demonlord":       "Demon Lord",
    "DemonSword":      "Demon Sword",
    "Destillery":      "Distillery",      # typo
    "ElfArher":        "Elf Archer",      # typo
    "EmptyBottle":     "Empty Bottle",
    "EnchantedMushroom": "Enchanted Shroom",
    "FeralCat":        "Feral Cat",
    "FireCloak":       "Fire Cloak",
    "Fisherman":       "Fisher",
    "FishStew":        "Seafood Stew",
    "FishTrap":        "Fish Trap",
    "ForestAmulet":    "Forest Amulet",
    "FriendlyPirate":  "Friendly Pirate",
    "FrogHelmet":      "Frog Helmet",
    "Fritatta":        "Frittata",        # typo
    "Fruit-Salad":     "Fruit Salad",
    "GoblinArcher":    "Goblin Archer",
    "GoblinHat":       "Goblin Hat",
    "GoblinShaman":    "Goblin Shaman",
    "Goldbar":         "Gold Bar",
    "GoldDeposit":     "Gold Ore",
    "Goldore":         "Gold Ore",
    "GoldMine":        "Gold Mine",
    "GoldenGoblet":    "Golden Goblet",
    "GoldenShestPlate":"Golden Chestplate", # typo ShestPlate
    "GrilledFish":     "Grilled Fish",
    "Helm":            "Helmet",
    "HornedHelmet":    "Horned Helmet",
    "Iron Ore":        "Iron Ore",
    "Iron bar":        "Iron Bar",
    "IronDeposit":     "Iron Ore",
    "IronShield":      "Iron Shield",
    "IslandRelic":     "Island Relic",
    "KrakenToothAxe":  "Kraken Tooth Axe",
    "Leathertunic":    "Leather Tunic",
    "Lumbercamp":      "Lumber Camp",
    "MagicDust":       "Magic Dust",
    "MagicGlue":       "Magic Glue",
    "MagicRing":       "Magic Ring",
    "MagicStaff":      "Magic Staff",
    "MagicSword":      "Magic Blade",
    "MagicTome":       "Magic Tome",
    "Broom":           "Magic Broom",
    "MammaCrab":       "Momma Crab",
    "Messhal":         "Mess Hall",
    "Mimmic":          "Mimic",           # typo
    "MorningStar":     "Morning Star",
    "MountainAmulet":  "Mountain Amulet",
    "Big-Rat":         "Giant Rat",
    "Big-Slime":       "Slime",
    "Small-Rat":       "Rat",
    "Small-Slime":     "Small Slime",
    "Snail":           "Giant Snail",
    "Onions":          "Onion",
    "Pikaxe":          "Pickaxe",
    "PirateBoat":      "Pirate Boat",
    "PirateHat":       "Pirate Hat",
    "PirateSabre":     "Pirate Sabre",
    "Portal":          "Strange Portal",
    "StrongPortal":    "Rare Portal",
    "AngryPirate":     "Pirate",
    "RatKingCrown":    "Rat Crown",
    "RawCrabMeat":     "Raw Crab Meat",
    "Raw-Meat":        "Raw Meat",
    "RawFish":         "Raw Fish",
    "RowBoat":         "Rowboat",
    "SacredChest":     "Sacred Chest",
    "SacredKey":       "Sacred Key",
    "SandQuarry":      "Sand Quarry",
    "SkullHelmet":     "Skull Helmet",
    "SpikedPlank":     "Spiked Plank",
    "StaffOfFear":     "Staff of Fear",
    "Sugercane":       "Cane Sugar",      # 與 CaneSugar 重複，後者優先
    "Swordman":        "Swordsman",       # typo
    "TamagoSushi":     "Tamago Sushi",
    "TigerFurCoat":    "Tiger Fur Coat",
    "TreasureMap":     "Treasure Map",
    "Trowingstars":    "Throwing Stars",  # typo
    "Wand":            "Magic Wand",
    "WaterBottle":     "Bottle of Water",
    "WickedWitch":     "Wicked Witch",
    "WishingWell":     "Wishing Well",
    "WizardOrc":       "Orc Wizard",
    "WizardRobe":      "Wizard Robe",
    "Wolfhead":        "Wolf Head",
    "Woodenshield":    "Wooden Shield",
    "Magnet":          "Resource Magnet",
    "Cooked-Meat":     "Cooked Meat",
    "Fishing-rod":     "Fishing Rod",
    # ── 小寫 / 底線 命名 ─────────────────────────────────────────
    "charcoal":        "Charcoal",
    "egg":             "Egg",
    "factory_parts":   "Factory Parts",
    "road_builder":    "Road Builder",
    "sewer":           "Sewer",
    "water":           "Water",
    # ── Resource chest ─────────────────────────────────────────
    "Resource chest (outline)": "Resource Chest",
}

# ── 標準化函式（用於自動比對）────────────────────────────────────────────────
def normalize(s: str) -> str:
    s = s.lower()
    s = re.sub(r"[-_ '()]", "", s)
    return s

# ── 主流程 ────────────────────────────────────────────────────────────────────

def main():
    # 讀取 JSON
    with open(JSON_PATH, encoding="utf-8") as f:
        cards: list[dict] = json.load(f)

    # 建立 卡牌名稱 → 卡牌資料 的查找表
    name_to_card: dict[str, dict] = {c["name"]: c for c in cards}
    norm_to_name: dict[str, str]  = {normalize(n): n for n in name_to_card}

    # 讀取 ZIP 內所有 png 檔案名稱
    with zipfile.ZipFile(ZIP_PATH) as z:
        all_entries = z.namelist()
    png_files = [
        e for e in all_entries
        if e.lower().endswith(".png") and not e.endswith("/")
    ]
    # basename → zip_path  (若重複取第一個)
    zip_by_basename: dict[str, str] = {}
    for entry in png_files:
        bn = Path(entry).name  # e.g. "BearClaw.png"
        if bn not in zip_by_basename:
            zip_by_basename[bn] = entry

    print(f"ZIP 內共 {len(zip_by_basename)} 張 PNG")
    print(f"JSON 內共 {len(cards)} 張卡牌\n")

    # ── 建立比對表（優先順序：精確名稱 > normalize > EXPLICIT_MAP） ─────────────
    # card_name → image_basename(含.png)
    card_to_img: dict[str, str] = {}

    # ── 第 1 輪：normalize 自動比對（英文直接名稱優先）─────────────────────────
    # 先做一輪，如果精確名稱(stem==card_name)存在，強制覆蓋 normalize 結果
    for bn in zip_by_basename:
        stem      = Path(bn).stem
        norm_stem = normalize(stem)
        if norm_stem not in norm_to_name:
            continue
        card_name = norm_to_name[norm_stem]

        if card_name not in card_to_img:
            card_to_img[card_name] = bn
        else:
            # 精確大小寫匹配 > normalize 匹配
            current = Path(card_to_img[card_name]).stem
            if stem == card_name and current != card_name:
                card_to_img[card_name] = bn   # 用精確名稱覆蓋

    # ── 第 2 輪：EXPLICIT_MAP 補丁（只補沒找到圖片的卡牌）────────────────────
    for img_base_noext, card_name in EXPLICIT_MAP.items():
        img_bn = img_base_noext + ".png"
        if (img_bn in zip_by_basename
                and card_name in name_to_card
                and card_name not in card_to_img):    # ← 只補未匹配的
            card_to_img[card_name] = img_bn

    # Berry_2.png 特例（檔名有 _2 後綴）
    if "Berry" not in card_to_img and "Berry_2.png" in zip_by_basename:
        card_to_img["Berry"] = "Berry_2.png"

    print(f"成功比對 {len(card_to_img)} 張卡牌\n")

    # ── 建立輸出資料夾 ────────────────────────────────────────────────────────
    for folder in TYPE_FOLDER.values():
        (OUTPUT_ROOT / folder).mkdir(parents=True, exist_ok=True)

    # ── 提取並複製圖片 + 更新 iconPath ───────────────────────────────────────
    with zipfile.ZipFile(ZIP_PATH) as z:
        copied   = 0
        skipped  = 0

        for card in cards:
            card_name = card["name"]
            card_type = card["type"]
            folder    = TYPE_FOLDER.get(card_type)

            if folder is None:
                skipped += 1
                continue

            img_bn = card_to_img.get(card_name)
            if img_bn is None:
                skipped += 1
                continue

            zip_entry = zip_by_basename[img_bn]
            dest_path = OUTPUT_ROOT / folder / img_bn

            # 解壓縮到目標位置
            img_data = z.read(zip_entry)
            dest_path.write_bytes(img_data)

            # 更新 iconPath（使用實際圖片檔名）
            card["iconPath"] = f"/Image/card/{folder}/{img_bn}"
            copied += 1

    print(f"✅ 已複製 {copied} 張卡牌圖片")
    print(f"   跳過（無圖片）: {skipped} 張")

    # ── 清除目標資料夾中不屬於本次複製的舊檔案 ────────────────────────────────
    valid_files = set(card_to_img.values())   # 本次有效的圖片名稱集合
    deleted_old = []
    for folder in TYPE_FOLDER.values():
        folder_path = OUTPUT_ROOT / folder
        if not folder_path.exists():
            continue
        for f in folder_path.glob("*.png"):
            if f.name not in valid_files:
                f.unlink()
                deleted_old.append(f"{folder}/{f.name}")

    if deleted_old:
        print(f"\n🗑️  已刪除 {len(deleted_old)} 個舊/無效圖片:")
        for p in sorted(deleted_old):
            print(f"   - {p}")

    # ── 儲存更新後的 JSON ────────────────────────────────────────────────────
    with open(JSON_PATH, "w", encoding="utf-8") as f:
        json.dump(cards, f, ensure_ascii=False, indent=2)
    print(f"\n✅ iconPath 已更新 → {JSON_PATH}")

    # ── 統計報告 ─────────────────────────────────────────────────────────────
    matched_set = set(card_to_img.values())
    unused_imgs = [bn for bn in zip_by_basename if bn not in matched_set]

    print(f"\n── 各類型複製數量 ────────────────────────────────────────────")
    type_counts: dict[str, int] = defaultdict(int)
    for card_name, img_bn in card_to_img.items():
        t = name_to_card[card_name]["type"]
        type_counts[t] += 1
    for t in ["CHARACTER","RESOURCE","EQUIPMENT","FOOD","BUILDING","MONSTER","CREATURE","PACK","IDEA","COIN"]:
        cnt = type_counts.get(t, 0)
        if cnt:
            print(f"  {t:12s}: {cnt:3d} 張")

    print(f"\n── 未對應到任何卡牌的圖片 ({len(unused_imgs)} 張，已不複製) ────")
    for bn in sorted(unused_imgs):
        print(f"  ✗ {bn}")

    print(f"\n── 未找到圖片的卡牌 ──────────────────────────────────────────")
    missing_cards = [(c["name"], c["type"]) for c in cards if c["name"] not in card_to_img]
    for name, t in sorted(missing_cards, key=lambda x: x[1]):
        print(f"  [{t:10s}] {name}")

# ── 執行 ──────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    print("="*60)
    print("  Texture2D 卡牌圖片分類器")
    print("="*60)
    main()
