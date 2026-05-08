//
// TimeBar.hpp — 時間讀條 UI
// 由兩個 1x1 像素的 PNG 拉伸而成：黑色底框 + 白色填充。
// 白色從右到左在指定秒數內填滿黑框。
//

#ifndef STACKLANDS_TIMEBAR_HPP
#define STACKLANDS_TIMEBAR_HPP

#include "BackgroundImage.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include <algorithm>
#include <memory>

class TimeBar {
public:
    // position    : 黑框中心位置（畫面座標）
    // blackSize   : 黑框顯示像素大小（寬, 高）
    // whiteSize   : 白條顯示像素大小（寬, 高），通常略小於黑框做出邊框感
    // durationSec : 白條從 0 填到 whiteSize.x 所需的秒數
    // zIndexBlack : 黑色圖層 z（白色會自動 +1，確保在上）
    TimeBar(Util::Renderer& renderer,
            const glm::vec2& position,
            const glm::vec2& blackSize,
            const glm::vec2& whiteSize,
            float durationSec,
            int zIndexBlack = 100)
        : m_Renderer(renderer),
          m_WhiteSize(whiteSize),
          m_DurationMs(durationSec * 1000.f) {

        // ── 黑色底框 ──────────────────────────────────────
        m_Black = std::make_shared<BackgroundImage>();
        m_Black->SetDrawable(std::make_shared<Util::Image>(
            RESOURCE_DIR"/Image/background/timebarBlack.png"));
        m_Black->SetScale(blackSize);
        m_Black->m_Transform.translation = position;
        m_Black->SetPivot({0.f, 0.f}); // 中心對齊
        m_Black->SetZIndex(zIndexBlack);
        renderer.AddChild(m_Black);

        // ── 白色讀條 ──────────────────────────────────────
        // 白條右邊緣對齊「白色完整時的右邊緣」（與黑框中心同一條 y 軸）
        // pivot.x = +0.5 讓 scale.x 縮放時固定右邊緣，左邊緣往左延伸。
        m_White = std::make_shared<BackgroundImage>();
        m_White->SetDrawable(std::make_shared<Util::Image>(
            RESOURCE_DIR"/Image/background/timebarWhite.png"));
        m_White->m_Transform.translation = {position.x + whiteSize.x * 0.5f, position.y};
        m_White->SetPivot({0.5f, 0.f});      // 錨點在右邊緣
        m_White->SetScale({0.f, whiteSize.y}); // 起始寬度 0
        m_White->SetZIndex(zIndexBlack + 1);
        renderer.AddChild(m_White);
    }

    ~TimeBar() {
        if (m_Black) m_Renderer.RemoveChild(m_Black);
        if (m_White) m_Renderer.RemoveChild(m_White);
    }

    // 不可複製（管理 Renderer 子節點）
    TimeBar(const TimeBar&) = delete;
    TimeBar& operator=(const TimeBar&) = delete;

    // 設定中心位置（白條右邊緣會自動跟著移動）
    void SetPosition(const glm::vec2& pos) {
        m_Black->m_Transform.translation = pos;
        m_White->m_Transform.translation = {pos.x + m_WhiteSize.x * 0.5f, pos.y};
    }

    // 重置進度並開始讀條
    void Start() {
        m_ElapsedMs = 0.f;
        m_Running   = true;
        ApplyWhiteScale();
    }

    void Pause()  { m_Running = false; }
    void Resume() { m_Running = true;  }
    void Reset()  { m_ElapsedMs = 0.f; ApplyWhiteScale(); }

    // 每幀呼叫，dtMs 為距上次更新的毫秒數
    void Update(float dtMs) {
        if (!m_Running) return;
        m_ElapsedMs = std::min(m_ElapsedMs + dtMs, m_DurationMs);
        ApplyWhiteScale();
        if (m_ElapsedMs >= m_DurationMs) m_Running = false;
    }

    bool IsFinished() const { return m_ElapsedMs >= m_DurationMs; }

    void SetVisible(bool v) {
        m_Black->SetVisible(v);
        m_White->SetVisible(v);
    }

    // 動態調整（如有需要）
    void SetDurationSec(float sec) { m_DurationMs = sec * 1000.f; }

    std::shared_ptr<BackgroundImage> GetBlack() const { return m_Black; }
    std::shared_ptr<BackgroundImage> GetWhite() const { return m_White; }

private:
    void ApplyWhiteScale() {
        const float t = (m_DurationMs > 0.f) ? (m_ElapsedMs / m_DurationMs) : 1.f;
        m_White->SetScale({m_WhiteSize.x * t, m_WhiteSize.y});
    }

    Util::Renderer& m_Renderer;
    std::shared_ptr<BackgroundImage> m_Black;
    std::shared_ptr<BackgroundImage> m_White;
    glm::vec2 m_WhiteSize;
    float     m_DurationMs;
    float     m_ElapsedMs = 0.f;
    bool      m_Running   = false;
};

#endif // STACKLANDS_TIMEBAR_HPP
