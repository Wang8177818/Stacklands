"""
Stacklands Wiki Scraper  (MediaWiki API 版)
使用 Fandom MediaWiki API 繞過 Cloudflare 抓取所有卡牌資料
輸出格式與 Cards.json 相同

輸出順序: CHARACTER -> RESOURCE -> EQUIPMENT -> FOOD -> BUILDING
          -> MONSTER -> CREATURE -> PACK -> IDEA -> 其他
iconPath 依卡牌類型與名稱自動產生
"""

import sys, re, json, time
import requests
from collections import Counter, OrderedDict

if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

# ── 設定 ──────────────────────────────────────────────────────────────────────

API_URL    = "https://stacklands.fandom.com/api.php"
OUT_FILE   = "Cards_wiki.json"

HEADERS = {
    "User-Agent": (
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/124.0.0.0 Safari/537.36"
    )
}

# Wiki 分類 → 卡牌類型
# (分類名稱, 類型, 圖片資料夾)
CATEGORY_MAP = [
    # ── 人物卡 ──────────────────────────────────────────────────
    ("Humans",          "CHARACTER",  "character"),
    # ── 資源卡 ──────────────────────────────────────────────────
    ("Resources",       "RESOURCE",   "resource"),
    # ── 裝備卡 ──────────────────────────────────────────────────
    ("Equipment",       "EQUIPMENT",  "equipment"),
    # ── 食物卡 ──────────────────────────────────────────────────
    ("Food",            "FOOD",       "food"),
    # ── 建築卡 ──────────────────────────────────────────────────
    ("Buildings",       "BUILDING",   "building"),
    # ── 怪物卡（敵對生物）────────────────────────────────────────
    ("Hostile Mobs",    "MONSTER",    "monster"),
    # ── 友好生物 ─────────────────────────────────────────────────
    ("Friendly Mobs",   "CREATURE",   "creature"),
    ("Fish",            "CREATURE",   "creature"),
    ("Friendly Fishes", "CREATURE",   "creature"),
    # ── 寶包 ────────────────────────────────────────────────────
    ("Booster Packs",   "PACK",       "pack"),
    # ── 靈感/想法卡 ──────────────────────────────────────────────
    ("Ideas",           "IDEA",       "idea"),
]

# 輸出順序
TYPE_ORDER = [
    "CHARACTER", "RESOURCE", "EQUIPMENT",
    "FOOD", "BUILDING", "MONSTER", "CREATURE",
    "PACK", "IDEA",
]

# Infobox 模板 → 類型對照（用於修正跨類別的卡牌）
TEMPLATE_TYPE = {
    "stacklands human":       "CHARACTER",
    "stacklands hostile mob": "MONSTER",
    "stacklands friendly mob":"CREATURE",
    "stacklands material":    "RESOURCE",
    "stacklands treasure":    "RESOURCE",
    "stacklands equipment":   "EQUIPMENT",
    "stacklands_food":        "FOOD",
    "stacklands food":        "FOOD",
    "stacklands building":    "BUILDING",
    "stacklands idea":        "IDEA",
}

# 特殊卡牌類型覆寫（少數特例）
SPECIAL_TYPE = {
    "Coin": "COIN",
}

# ── 工具函式 ──────────────────────────────────────────────────────────────────

def make_icon(card_type: str, icon_folder: str, name: str) -> str:
    """依卡牌類型自動產生 iconPath"""
    safe = name.replace(" ", "_")
    return f"/Image/card/{icon_folder}/{safe}.png"

def parse_int(text: str):
    """從各種格式中提取整數，例如 'Weak (2)', '+2', '15', 'Blocks 2'"""
    if not text:
        return None
    # 處理 "Blocks X" 格式
    m = re.search(r'[Bb]locks?\s*(\d+)', text)
    if m:
        return int(m.group(1))
    # 提取第一個整數（包含前置正負號）
    m = re.search(r'[+\-]?\d+', text)
    if m:
        return int(m.group(0))
    return None

def parse_infobox(wikitext: str) -> tuple[str, dict]:
    """
    解析 wikitext 中的 infobox 模板
    回傳 (template_name_lower, {field: value})
    """
    # 找到第一個 {{ ... }} 模板
    depth = 0
    start = -1
    end   = -1
    for i, ch in enumerate(wikitext):
        if wikitext[i:i+2] == '{{':
            if depth == 0:
                start = i
            depth += 1
        elif wikitext[i:i+2] == '}}':
            depth -= 1
            if depth == 0:
                end = i + 2
                break

    if start == -1 or end == -1:
        return "", {}

    box = wikitext[start:end]

    # 提取模板名稱
    first_pipe = box.find('|')
    if first_pipe == -1:
        tmpl_name = box[2:-2].strip()
    else:
        tmpl_name = box[2:first_pipe].strip()
    tmpl_lower = tmpl_name.lower()

    # 提取所有 |field = value
    fields = {}
    # 去掉開頭模板名稱，逐行解析
    content = box[first_pipe+1:-2] if first_pipe != -1 else ""
    # 按 | 分割（注意：值中可能含 |，用 [[...]] 連結，需謹慎）
    # 簡單做法：按行解析
    for line in content.split('\n'):
        line = line.strip().lstrip('|').strip()
        if '=' not in line:
            continue
        key, _, val = line.partition('=')
        key = key.strip().lower()
        val = val.strip()
        # 去除 wiki 連結標記 [[...]]
        val = re.sub(r'\[\[([^\]|]+\|)?([^\]]+)\]\]', r'\2', val)
        # 去除 {{!}} → |
        val = val.replace('{{!}}', '|')
        # 去除 HTML 標籤
        val = re.sub(r'<[^>]+>', '', val)
        fields[key] = val

    return tmpl_lower, fields

# ── API 呼叫 ──────────────────────────────────────────────────────────────────

def api_get(params: dict) -> dict:
    """發送 MediaWiki API 請求"""
    params["format"] = "json"
    try:
        resp = requests.get(API_URL, params=params, headers=HEADERS, timeout=20)
        resp.raise_for_status()
        return resp.json()
    except Exception as e:
        print(f"  [API錯誤] {e}")
        return {}

def get_category_members(category: str) -> list[str]:
    """取得分類成員（只取 namespace=0 的頁面，排除 File: Category: 等）"""
    members = []
    cmcontinue = None

    while True:
        params = {
            "action": "query",
            "list": "categorymembers",
            "cmtitle": f"Category:{category}",
            "cmlimit": "500",
            "cmnamespace": "0",   # 只要主命名空間（一般頁面）
        }
        if cmcontinue:
            params["cmcontinue"] = cmcontinue

        data = api_get(params)
        for m in data.get("query", {}).get("categorymembers", []):
            title = m.get("title", "")
            # 排除 File:、Category:、Template: 等命名空間頁面
            # 但保留 "Idea: Xxx" 這種正常頁面（namespace=0）
            ns = m.get("ns", 0)
            if ns == 0:
                members.append(title)

        if "continue" in data:
            cmcontinue = data["continue"].get("cmcontinue")
        else:
            break

    return members

def get_pages_wikitext(titles: list[str]) -> dict[str, str]:
    """
    批次取得多個頁面的 wikitext
    MediaWiki API 每次最多 50 個標題
    """
    result = {}
    batch_size = 50

    for i in range(0, len(titles), batch_size):
        batch = titles[i : i + batch_size]
        params = {
            "action": "query",
            "prop": "revisions",
            "rvprop": "content",
            "titles": "|".join(batch),
        }
        data = api_get(params)
        pages = data.get("query", {}).get("pages", {})
        for pid, page in pages.items():
            if pid == "-1":
                continue
            title = page.get("title", "")
            revs  = page.get("revisions", [])
            if revs:
                result[title] = revs[0].get("*", "")
        time.sleep(0.2)   # 禮貌性延遲

    return result

# ── 卡牌解析 ──────────────────────────────────────────────────────────────────

def build_card(name: str, wikitext: str, default_type: str, icon_folder: str) -> dict:
    """
    從 wikitext 建立卡牌 dict
    """
    tmpl_lower, fields = parse_infobox(wikitext)

    # 決定最終類型（以 infobox 模板 > 特殊對照 > 分類預設）
    card_type = TEMPLATE_TYPE.get(tmpl_lower, default_type)
    card_type = SPECIAL_TYPE.get(name, card_type)

    # 依 iconFolder 邏輯決定資料夾
    # 如果類型被模板覆寫，更新 icon_folder
    type_to_folder = {
        t: f for _, t, f in CATEGORY_MAP
    }
    # 同類型可能有多個 folder（取第一個）
    tf = {}
    for _, t, f in CATEGORY_MAP:
        if t not in tf:
            tf[t] = f
    real_folder = tf.get(card_type, icon_folder)

    card: dict = OrderedDict()
    card["name"]     = name
    card["type"]     = card_type
    card["iconPath"] = make_icon(card_type, real_folder, name)

    # ── CHARACTER 屬性 ───────────────────────────────────────────
    if card_type == "CHARACTER":
        hp = parse_int(fields.get("health_points", ""))
        if hp is not None:
            card["health"] = hp

        food = parse_int(fields.get("food", ""))
        if food is not None:
            card["food"] = food

        combat_lv = parse_int(fields.get("combat_level", ""))
        if combat_lv is not None:
            card["combatLevel"] = combat_lv

        # damage 欄位（如 "Weak (2)"）
        dmg_raw = fields.get("damage", "")
        dmg = parse_int(dmg_raw)
        if dmg is not None:
            card["damage"] = dmg

        # defense（如 "Weak (Blocks 1)"）
        def_raw = fields.get("defense", "")
        defense = parse_int(def_raw)
        if defense is not None:
            card["defense"] = defense

        if fields.get("effect"):
            card["effect"] = fields["effect"]

    # ── RESOURCE 屬性 ────────────────────────────────────────────
    elif card_type == "RESOURCE":
        sell = parse_int(fields.get("sell_price", ""))
        if sell is not None:
            card["sellValue"] = sell

    # ── COIN 屬性 ────────────────────────────────────────────────
    elif card_type == "COIN":
        sell = parse_int(fields.get("sell_price", ""))
        if sell is not None:
            card["sellValue"] = sell

    # ── EQUIPMENT 屬性 ──────────────────────────────────────────
    elif card_type == "EQUIPMENT":
        sell = parse_int(fields.get("sell_prize", "")  # 注意 wiki 原文是 sell_prize
                         or fields.get("sell_price", ""))
        if sell is not None:
            card["sellValue"] = sell

        dmg = parse_int(fields.get("damage", ""))
        if dmg is not None:
            card["attack"] = dmg

        def_ = parse_int(fields.get("defense", ""))
        if def_ is not None:
            card["defense"] = def_

        slot = fields.get("slot", "")
        if slot:
            card["slot"] = slot

        atk_spd = fields.get("attack_speed", "")
        if atk_spd:
            card["attackSpeed"] = atk_spd

        if fields.get("special"):
            card["special"] = fields["special"]

    # ── FOOD 屬性 ────────────────────────────────────────────────
    elif card_type == "FOOD":
        sell = parse_int(fields.get("sell_price", ""))
        if sell is not None:
            card["sellValue"] = sell

        fv = parse_int(fields.get("units_of_food", ""))
        if fv is not None:
            card["foodValue"] = fv

    # ── BUILDING 屬性 ────────────────────────────────────────────
    elif card_type == "BUILDING":
        sell = parse_int(fields.get("sell_price", ""))
        if sell is not None:
            card["sellValue"] = sell

    # ── MONSTER 屬性 ────────────────────────────────────────────
    elif card_type in ("MONSTER", "CREATURE"):
        hp = parse_int(fields.get("health_points", ""))
        if hp is not None:
            card["health"] = hp

        combat_lv = parse_int(fields.get("combat_level", ""))
        if combat_lv is not None:
            card["combatLevel"] = combat_lv

        dmg_raw = fields.get("damage", "")
        dmg = parse_int(dmg_raw)
        if dmg is not None:
            card["damage"] = dmg

        def_raw = fields.get("defense", "")
        defense = parse_int(def_raw)
        if defense is not None:
            card["defense"] = defense

        if fields.get("effect"):
            card["effect"] = fields["effect"]

    # ── IDEA 屬性 ────────────────────────────────────────────────
    elif card_type == "IDEA":
        sell = parse_int(fields.get("sell_price", ""))
        if sell is not None:
            card["sellValue"] = sell

    return card

# ── 主流程 ────────────────────────────────────────────────────────────────────

def scrape_all() -> list[dict]:
    all_by_type: dict[str, list[dict]] = {t: [] for t in TYPE_ORDER}
    all_by_type["COIN"] = []
    seen_names: set[str] = set()

    for (category, c_type, icon_folder) in CATEGORY_MAP:
        print(f"\n[{c_type}] 分類: {category}")

        members = get_category_members(category)
        print(f"  → 找到 {len(members)} 個成員，正在取得 wikitext...")

        wikitext_map = get_pages_wikitext(members)

        for name in members:
            if name in seen_names:
                continue

            wikitext = wikitext_map.get(name, "")
            if not wikitext:
                # 無 wikitext，建立基本卡牌
                card = OrderedDict([
                    ("name",     name),
                    ("type",     c_type),
                    ("iconPath", make_icon(c_type, icon_folder, name)),
                ])
            else:
                card = build_card(name, wikitext, c_type, icon_folder)

            final_type = card["type"]
            seen_names.add(name)

            bucket = all_by_type.get(final_type)
            if bucket is None:
                all_by_type[final_type] = []
                bucket = all_by_type[final_type]
            bucket.append(card)
            print(f"  + [{final_type}] {name}")

        time.sleep(0.5)

    # ── 依順序合併 ─────────────────────────────────────────────
    result = []
    ordered = TYPE_ORDER + ["COIN"]
    for t in ordered:
        result.extend(all_by_type.get(t, []))
    # 補上未知類型
    for t, cards in all_by_type.items():
        if t not in ordered:
            result.extend(cards)

    return result

def save_json(cards: list[dict], path: str):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(cards, f, ensure_ascii=False, indent=2)
    print(f"\n✅ 已儲存 {len(cards)} 張卡牌 → {path}")

# ── 執行入口 ──────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    print("=" * 60)
    print("  Stacklands Wiki Scraper  (MediaWiki API)")
    print("  輸出順序: CHARACTER -> RESOURCE -> EQUIPMENT -> ...")
    print("=" * 60)

    cards = scrape_all()

    if not cards:
        print("\n❌ 未抓到任何資料")
        sys.exit(1)

    save_json(cards, OUT_FILE)

    print("\n── 統計 ─────────────────────────────────────────────────")
    counts = Counter(c["type"] for c in cards)
    for t in TYPE_ORDER + ["COIN"]:
        if counts.get(t):
            print(f"  {t:12s}: {counts[t]:3d} 張")
    print(f"  {'合計':12s}: {len(cards):3d} 張")
