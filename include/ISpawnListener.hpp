#pragma once
#ifndef STACKLANDS_ISPAWNLISTENER_HPP
#define STACKLANDS_ISPAWNLISTENER_HPP

#include <string>

// 生成卡片的抽象介面（DIP：AnimalCard 依賴此介面，而非具體的 CardManager）
class ISpawnListener {
public:
    virtual void OnSpawn(const std::string& name, float x, float y) = 0;
    virtual ~ISpawnListener() = default;
};

#endif // STACKLANDS_ISPAWNLISTENER_HPP
