//
// MagicGlue.hpp — Magic Glue 建築卡
// 特殊功能：所有疊在它上方的卡片都無法被拖動（黏住整疊）
//

#ifndef STACKLANDS_MAGICGLUE_HPP
#define STACKLANDS_MAGICGLUE_HPP

#include "BuildingCard.hpp"

class MagicGlue final : public BuildingCard {
public:
    MagicGlue(float x, float y, int sellValue,
              const std::string& iconPath, float scale)
        : BuildingCard(x, y, "Magic Glue", sellValue, iconPath, scale)
    {}

    // 不能被疊到其他卡片上（只能單獨放在桌面上）
    bool CanStackOnto() override { return false; }

    // 標記為「黏住」：CardManager 推擠迴圈會跳過任何 stack 中含有 sticky 的卡，
    // 因此 Magic Glue 與疊在它上下的整疊都不會被其他卡推走（但仍可被玩家拖動）
    bool IsSticky() const override { return true; }
};

#endif // STACKLANDS_MAGICGLUE_HPP
