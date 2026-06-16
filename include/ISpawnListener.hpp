#pragma once
#ifndef STACKLANDS_ISPAWNLISTENER_HPP
#define STACKLANDS_ISPAWNLISTENER_HPP

#include <string>

// 生成卡片的抽象介面（DIP：AnimalCard 依賴此介面，而非具體的 CardManager）
class ISpawnListener {
public:
    // scale：產出者當前的顯示縮放（已含鏡頭 zoom），讓產出物與產出者同大小
    virtual void OnSpawn(const std::string& name, float x, float y, float scale) = 0;
    virtual ~ISpawnListener() = default;
};

#endif // STACKLANDS_ISPAWNLISTENER_HPP
