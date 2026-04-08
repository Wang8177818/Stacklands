"""
Stacklands Packs.json 生成器
從 Fandom Wiki 抓取所有 Booster Pack 的卡池資料
生成格式與 Packs.json 相同
"""

import sys, re, json, time
import requests

if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

API_URL = "https://stacklands.fandom.com/api.php"
HEADERS = {
    "User-Agent": (
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0 Safari/537.36"
    )
}
OUT_FILE = r"D:\MyStuff\School\OOPL\Stacklands\Resources\Data\Packs.json"

PACK_NAMES = [
    "A New World",       "Humble Beginnings",   "Seeking Wisdom",
    "Logic and Reason",  "Reap & Sow",          "The Armory",
    "Curious Cuisine",   "Order and Structure",  "Explorers",
    "Fresh Isle",        "Island of Ideas",      "On The Shore",
    "Grilling and Brewing", "Island Insights",   "Advanced Archipelago",
    "Enclave Explorers", "New Weaponry",         "Noble Creatures",
    "Precious Resources","Major Metals",          "Department of Defense",
    "Impressive Industry","Advanced Automation",  "Modern Solutions",
    "Black Gold",        "Island Insights",       "Cities Pack",
    "The Island?",
]
# 去重但保留順序
seen = set()
PACK_NAMES = [p for p in PACK_NAMES if not (p in seen or seen.add(p))]

# ── 卡牌名稱修正表 ─────────────────────────────────────────────────────────────
# wiki 名稱 → 遊戲/JSON 名稱
NAME_FIX = {
    "Sugar Cane":     "Cane Sugar",
    "Chilli Pepper":  "Chili Pepper",
    "Mamma Crab":     "Momma Crab",
    "Big Slime":      "Slime",
    "Small Rat":      "Rat",
    "Big Rat":        "Giant Rat",
    "Large Slime":    "Slime",
}

# ── Wiki 命名空間前綴（這些連結不是卡名）─────────────────────────────────────
# 注意：「Idea: Axe」不是命名空間，是真實卡名！只排除 Category: 等
WIKI_NAMESPACES = {
    "Category:", "Template:", "File:", "Special:",
    "Help:", "User:", "Talk:", "Wikipedia:",
}

# ── 排除非卡牌的純描述性文字（不是具體卡名）────────────────────────────────────
EXCLUDE_CARDS = {
    "Traveling Cart",
    "Travelling Cart",
}

# ── API 工具 ──────────────────────────────────────────────────────────────────

def get_wikitext_batch(titles: list[str]) -> dict[str, str]:
    result = {}
    batch = 50
    for i in range(0, len(titles), batch):
        chunk = titles[i:i+batch]
        r = requests.get(API_URL, params={
            "action": "query", "prop": "revisions",
            "rvprop": "content", "titles": "|".join(chunk), "format": "json"
        }, headers=HEADERS, timeout=20)
        for pid, page in r.json().get("query", {}).get("pages", {}).items():
            if pid != "-1":
                revs = page.get("revisions", [])
                if revs:
                    result[page["title"]] = revs[0].get("*", "")
        time.sleep(0.3)
    return result

# ── 解析函式 ──────────────────────────────────────────────────────────────────

def parse_infobox_field(wikitext: str, field: str) -> str:
    """從 infobox 提取指定欄位值"""
    m = re.search(rf'\|\s*{field}\s*=\s*([^\n|}}]+)', wikitext)
    return m.group(1).strip() if m else ""

def extract_pool(wikitext: str) -> list[str]:
    """
    從 wikitext 的 Cards 區段提取所有 [[Card Name]] 連結
    - 去重、保留首次出現順序
    - 應用名稱修正
    - 排除非卡牌物件
    """
    # 找 Cards 區段
    cards_section = wikitext
    m = re.search(r'==\s*Cards\s*==(.+?)(?:==\w|\Z)', wikitext, re.DOTALL)
    if m:
        cards_section = m.group(1)

    # 提取所有 [[Card Name]] 或 [[Card Name|Display]]
    raw_links = re.findall(r'\[\[([^\]|#]+)(?:\|[^\]]+)?\]\]', cards_section)

    pool = []
    seen = set()
    for link in raw_links:
        name = link.strip()

        # 排除 Wiki 命名空間連結（Category:、:Category:、File: 等）
        # 但保留 「Idea: Axe」、「Rumor: Combat」 等真實卡名
        stripped = name.lstrip(":")   # 去除開頭冒號 :Category: → Category:
        if any(stripped.startswith(ns) for ns in WIKI_NAMESPACES):
            continue

        # 去除純百分比說明等雜訊
        if not name or name.startswith("(") or re.match(r'^\d', name):
            continue

        # 名稱修正
        name = NAME_FIX.get(name, name)

        # 排除非卡牌
        if name in EXCLUDE_CARDS:
            continue

        if name not in seen:
            seen.add(name)
            pool.append(name)

    return pool

def make_icon_path(image1: str) -> str:
    """從 wiki 的 image1 欄位生成 iconPath"""
    # 去除副檔名再加回來，空格轉底線
    if not image1:
        return "/Image/card/pack/Traveling-Cart.png"
    name = image1.strip()
    # 空格轉底線，確保 .png 結尾
    safe = name.replace(" ", "_")
    if not safe.lower().endswith(".png"):
        safe += ".png"
    return f"/Image/card/pack/{safe}"

# ── 主流程 ────────────────────────────────────────────────────────────────────

def main():
    print("正在抓取 wikitext...")
    texts = get_wikitext_batch(PACK_NAMES)

    packs_json = []
    for name in PACK_NAMES:
        wt = texts.get(name, "")
        if not wt:
            print(f"  ✗ {name}: 無法取得 wikitext")
            continue

        # 解析欄位
        cards_count = parse_infobox_field(wt, "cards")
        image1      = parse_infobox_field(wt, "image1")
        try:
            total_cards = int(cards_count)
        except ValueError:
            total_cards = 3   # fallback

        pool = extract_pool(wt)
        icon = make_icon_path(image1)

        entry = {
            "name":       name,
            "iconPath":   icon,
            "totalCards": total_cards,
            "pool":       pool,
        }
        packs_json.append(entry)

        print(f"  ✓ {name:30s} cards={total_cards:2d}  pool={len(pool):3d}張")
        print(f"       pool: {pool}")

    # 儲存
    with open(OUT_FILE, "w", encoding="utf-8") as f:
        json.dump(packs_json, f, ensure_ascii=False, indent=2)
    print(f"\n✅ 已儲存 {len(packs_json)} 個 Pack → {OUT_FILE}")

if __name__ == "__main__":
    main()
