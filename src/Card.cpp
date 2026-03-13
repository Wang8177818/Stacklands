//
// Created by m0938 on 2026/3/13.
//

#include "Card.hpp"

Card::Card(float x, float y, const std::string& name, int sellValue, CardType type)
    : m_X(x), m_Y(y), m_Name(name), m_SellValue(sellValue), m_Type(type), m_IsDragging(false) {

    // 1. 建立底圖物件
    m_Background = std::make_shared<Util::GameObject>();
    m_Background->SetZIndex(10); // 預設圖層高度

    // 2. 建立圖示物件
    m_Icon = std::make_shared<Util::GameObject>();
    m_Icon->SetZIndex(11); // 圖示要蓋在底圖上

    // 3. 建立名稱文字物件 (請確保有 arial.ttf 字體檔)
    m_NameText = std::make_shared<Util::GameObject>();
    m_NameText->SetDrawable(std::make_shared<Util::Text>(RESOURCE_DIR"/Font/arial.ttf", 24, m_Name, Util::Color(255, 255, 255)));
    m_NameText->SetZIndex(12); // 文字在最上層

    // 初始化座標位置
    UpdateVisualPositions();
}

void Card::SetBackgroundImage(const std::string& imagePath) {
    if (m_Background) {
        m_Background->SetDrawable(std::make_shared<Util::Image>(imagePath));
    }
}

void Card::SetIconImage(const std::string& imagePath) {
    if (m_Icon) {
        m_Icon->SetDrawable(std::make_shared<Util::Image>(imagePath));
    }
}

void Card::Update() {
    if (m_IsDragging) {
        // 取得目前的滑鼠座標
        glm::vec2 mousePos = Util::Input::GetCursorPosition();

        // 更新卡牌的中心點座標 (加上拖曳偏差值，手感才會順)
        m_X = mousePos.x + m_DragOffset.x;
        m_Y = mousePos.y + m_DragOffset.y;

        // 如果這張卡牌上面還有疊其他卡牌，也要連著一起更新座標！
        // (Stacklands 經典機制：拉動底下的卡，上面的會跟著走)
        if (m_CardAbove != nullptr) {
            // 設定上方卡牌的位置 (例如 Y 軸往下偏移 40，讓玩家看得到下面的卡)
            // 這裡可以等未來實作堆疊系統時再來微調
        }

        // 同步更新畫面上 GameObject 的位置
        UpdateVisualPositions();
    }
}

bool Card::IsMouseHovering(glm::vec2 mousePos) {
    // 計算卡牌的四個邊界
    float left = m_X - m_Width / 2;
    float right = m_X + m_Width / 2;
    float top = m_Y + m_Height / 2;
    float bottom = m_Y - m_Height / 2;

    // 判斷滑鼠座標是否落在邊界內
    return (mousePos.x >= left && mousePos.x <= right &&
            mousePos.y >= bottom && mousePos.y <= top);
}

void Card::StartDragging(glm::vec2 mousePos) {
    m_IsDragging = true;

    // 計算滑鼠點擊點與卡牌中心的距離差
    m_DragOffset = glm::vec2(m_X, m_Y) - mousePos;

    // 把正在拖曳的卡牌移到最上層，才不會被其他卡牌擋住
    m_Background->SetZIndex(100);
    m_Icon->SetZIndex(101);
    m_NameText->SetZIndex(102);
}

void Card::StopDragging() {
    m_IsDragging = false;

    // 放開後恢復原本的圖層高度 (這裡未來可以根據它疊在第幾層來動態計算)
    m_Background->SetZIndex(10);
    m_Icon->SetZIndex(11);
    m_NameText->SetZIndex(12);
}

// ==========================================
// 內部工具與預設方法
// ==========================================
void Card::UpdateVisualPositions() {
    // 更新底圖位置 (正中心)
    if (m_Background) m_Background->m_Transform.translation = glm::vec2(m_X, m_Y);

    // 更新圖示位置 (稍微往上偏移一點)
    if (m_Icon) m_Icon->m_Transform.translation = glm::vec2(m_X, m_Y + 15.0f);

    // 更新文字位置 (放在卡牌底部)
    if (m_NameText) m_NameText->m_Transform.translation = glm::vec2(m_X, m_Y - m_Height / 2 + 25.0f);
}

// 回傳 GameObject 給 App 渲染
std::vector<std::shared_ptr<Util::GameObject>> Card::GetGameObjects() {
    std::vector<std::shared_ptr<Util::GameObject>> objs;

    if (m_Background) objs.push_back(m_Background);
    if (m_Icon) objs.push_back(m_Icon);
    if (m_NameText) objs.push_back(m_NameText);

    return objs;
}

bool Card::OnStacked(std::shared_ptr<Card> /*cardAbove*/) {
    return false; // 預設不接受堆疊
}

void Card::OnMonthEnd() {
    // 預設什麼都不做
}