"""
Booster Pack 圖片下載器
從 Fandom Wiki 下載所有寶包圖片，存到 /Image/card/pack/
並更新 Packs.json 的 iconPath
命名規則：Pack 名稱 → 底線格式（去除特殊字元）
"""

import sys, re, json, time
from pathlib import Path
import requests

if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

API_URL    = "https://stacklands.fandom.com/api.php"
PACK_DIR   = Path(r"D:\MyStuff\School\OOPL\Stacklands\Resources\Image\card\pack")
PACKS_JSON = Path(r"D:\MyStuff\School\OOPL\Stacklands\Resources\Data\Packs.json")

HEADERS = {
    "User-Agent": (
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0 Safari/537.36"
    ),
    "Referer": "https://stacklands.fandom.com/",
}

# ── Pack名稱 → wiki 原始圖片檔名 ──────────────────────────────────────────────
# 這些是 infobox |image1 = xxx.png 的值（帶有 wiki 上的原始拼法/大小寫）
WIKI_IMAGE = {
    "A New World":          "BoosterPack - Special - ANewWorld.png",
    "Humble Beginnings":    "BoosterPack - Mainland - HumbleBeginnnings.png",  # 原始有typo(3個n)
    "Seeking Wisdom":       "BoosterPack - Mainland - SeekingWisdom.png",
    "Logic and Reason":     "BoosterPack - Mainland - LogicAndReason.png",
    "Reap & Sow":           "BoosterPack - Mainland - Reap&Sow.png",
    "The Armory":           "BoosterPack - Mainland - TheArmory.png",
    "Curious Cuisine":      "BoosterPack - Mainland - CuriousCuisine.png",
    "Order and Structure":  "BoosterPack - Mainland - OrderAndStructure.png",
    "Explorers":            "BoosterPack - Mainland - Explorers.png",
    "Fresh Isle":           "BoosterPack - Special - FreshIsle.png",
    "Island of Ideas":      "BoosterPack - Island - IslandOfIdeas.png",
    "On The Shore":         "BoosterPack - Island - OnTheShore.png",
    "Grilling and Brewing": "BoosterPack - Island - GrillingAndBrewing.png",
    "Island Insights":      "BoosterPack - Island - IslandInsights.png",
    "Advanced Archipelago": "BoosterPack - Island - AdvancedArchipelago.png",
    "Enclave Explorers":    "BoosterPack - Island - EnclaveExplorers.png",
    "New Weaponry":         "BoosterPack - Special - NewWeaponry.png",
    "Noble Creatures":      "BoosterPack - WorldOfGreed - NobleCreatures.png",
    "Precious Resources":   "BoosterPack - City - PreciousResources.png",
    "Major Metals":         "BoosterPack - City - MajorMetals.png",
    "Department of Defense":"BoosterPack - City - DepartmentofDefense.png",
    "Impressive Industry":  "BoosterPack - City - ImpressiveIndustry.png",
    "Advanced Automation":  "BoosterPack - City - AdvancedAutomation.png",
    "Modern Solutions":     "BoosterPack - City - ModernSolutions.png",
    "Black Gold":           "BoosterPack - City - BlackGold.png",
    "Cities Pack":          "Cities Pack.png",
    "The Island?":          "BoosterPack - Special - TheIsland.png",
}

def pack_to_filename(pack_name: str) -> str:
    """
    Pack 名稱 → 乾淨的本地檔名
    "Humble Beginnings" → "Humble_Beginnings.png"
    "Reap & Sow"        → "Reap_and_Sow.png"
    "The Island?"       → "The_Island.png"
    """
    name = pack_name
    name = name.replace("&", "and")
    name = re.sub(r"[?!\"'<>|*\\/:]+", "", name)   # 移除不合法字元
    name = name.strip()
    name = re.sub(r"\s+", "_", name)                # 空格→底線
    return name + ".png"

def get_image_urls(wiki_filenames: list[str]) -> dict[str, str]:
    """
    批次向 Fandom API 查詢圖片真實下載 URL
    回傳 {wiki原始檔名: download_url}
    """
    result = {}
    batch  = 25   # API 每次最多25個 File:

    for i in range(0, len(wiki_filenames), batch):
        chunk  = wiki_filenames[i:i+batch]
        titles = "|".join(f"File:{fn}" for fn in chunk)
        r = requests.get(API_URL, params={
            "action": "query",
            "prop":   "imageinfo",
            "iiprop": "url",
            "titles": titles,
            "format": "json",
        }, headers=HEADERS, timeout=20)
        pages = r.json().get("query", {}).get("pages", {})
        for _, page in pages.items():
            title    = page.get("title", "").removeprefix("File:")
            imageinfo = page.get("imageinfo", [])
            if imageinfo:
                url = imageinfo[0].get("url", "")
                # 去除 /revision/... 後綴取得原始圖片
                url = re.sub(r"/revision/.*", "", url)
                result[title] = url
        time.sleep(0.2)
    return result

def download_image(url: str, dest: Path) -> bool:
    """下載圖片到指定路徑"""
    try:
        r = requests.get(url, headers=HEADERS, timeout=20, stream=True)
        if r.status_code == 200:
            dest.write_bytes(r.content)
            return True
        print(f"    HTTP {r.status_code}: {url}")
        return False
    except Exception as e:
        print(f"    下載失敗: {e}")
        return False

def main():
    PACK_DIR.mkdir(parents=True, exist_ok=True)

    # 讀取 Packs.json
    with open(PACKS_JSON, encoding="utf-8") as f:
        packs: list[dict] = json.load(f)

    pack_by_name = {p["name"]: p for p in packs}

    print("正在查詢 Wiki 圖片 URL...")
    wiki_fns   = list(WIKI_IMAGE.values())
    url_map    = get_image_urls(wiki_fns)

    print(f"  找到 {len(url_map)} / {len(wiki_fns)} 個圖片 URL\n")

    ok_count   = 0
    fail_count = 0

    for pack_name, wiki_fn in WIKI_IMAGE.items():
        local_fn   = pack_to_filename(pack_name)
        dest_path  = PACK_DIR / local_fn
        icon_path  = f"/Image/card/pack/{local_fn}"

        # 查 URL
        url = url_map.get(wiki_fn)
        if not url:
            # 嘗試空格版本
            alt = wiki_fn.replace("-", " - ") if " - " not in wiki_fn else wiki_fn
            url = url_map.get(alt)

        if not url:
            print(f"  ✗ {pack_name:30s}  → 找不到 URL ({wiki_fn})")
            fail_count += 1
            continue

        # 下載
        print(f"  ↓ {pack_name:30s}  → {local_fn}")
        if download_image(url, dest_path):
            ok_count += 1
            # 更新 Packs.json
            if pack_name in pack_by_name:
                pack_by_name[pack_name]["iconPath"] = icon_path
        else:
            fail_count += 1
        time.sleep(0.3)

    # 儲存更新後的 Packs.json
    with open(PACKS_JSON, "w", encoding="utf-8") as f:
        json.dump(packs, f, ensure_ascii=False, indent=2)

    print(f"\n✅ 下載完成: {ok_count} 成功 / {fail_count} 失敗")
    print(f"✅ Packs.json iconPath 已更新")
    print(f"\n目前 pack 資料夾:")
    for f in sorted(PACK_DIR.glob("*.png")):
        print(f"   {f.name}  ({f.stat().st_size:,} bytes)")

if __name__ == "__main__":
    main()
