"""偵錯腳本：顯示頁面實際狀況"""
import sys, time
if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

from playwright.sync_api import sync_playwright

BRAVE_PATH = r"C:\Program Files\BraveSoftware\Brave-Browser\Application\brave.exe"

URLS = [
    "https://stacklands.fandom.com/wiki/Villagers",
    "https://stacklands.fandom.com/wiki/Resources",
    "https://stacklands.fandom.com/wiki/Equipment",
]

def debug_page(page, url):
    print(f"\n{'='*60}")
    print(f"URL: {url}")
    print('='*60)
    try:
        page.goto(url, wait_until="domcontentloaded", timeout=30000)
    except Exception as e:
        print(f"  goto error: {e}")
    page.wait_for_timeout(5000)

    actual_url = page.url
    page_title = page.title()
    print(f"  實際URL  : {actual_url}")
    print(f"  頁面標題 : {page_title}")

    info = page.evaluate("""
        () => {
            const sels = ['.mw-parser-output','#mw-content-text','.page-content','.WikiaArticle',
                          '.article-content','main','#content','.fandom-community-header'];
            const result = {};
            for (const s of sels) {
                const el = document.querySelector(s);
                result[s] = el ? el.innerText.substring(0,200) : null;
            }
            result['bodyText'] = document.body.innerText.substring(0,500);
            result['allClasses'] = Array.from(document.querySelectorAll('[class]'))
                .map(e => e.className).filter(Boolean).slice(0,30);
            return result;
        }
    """)
    print(f"\n  [body 前500字]:\n{info.get('bodyText','')}")
    print(f"\n  [前30個 class]:\n{info.get('allClasses',[])}")
    for sel, text in info.items():
        if sel not in ('bodyText','allClasses') and text:
            print(f"\n  [{sel}] 前200字:\n{text}")

with sync_playwright() as pw:
    browser = pw.chromium.launch(
        executable_path=BRAVE_PATH,
        headless=False,
        args=["--no-sandbox"],
    )
    ctx = browser.new_context(
        user_agent="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/124.0 Safari/537.36",
        locale="en-US",
    )
    page = ctx.new_page()

    # 先到首頁
    try:
        page.goto("https://stacklands.fandom.com/", wait_until="domcontentloaded", timeout=30000)
        page.wait_for_timeout(2000)
    except Exception as e:
        print(f"首頁載入錯誤: {e}")

    for url in URLS:
        try:
            debug_page(page, url)
        except Exception as e:
            print(f"  FATAL ERROR: {e}")

    browser.close()
