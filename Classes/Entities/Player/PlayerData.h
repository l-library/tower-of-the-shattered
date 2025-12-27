#pragma once
#include "cocos2d.h"

class PlayerData {
public:
    static PlayerData* getInstance();

    // 保存玩家当前状态（触碰传感器时调用）
    void savePlayerState(double health, double magic,
        const cocos2d::Vec2& position,
        int direction);

    // 获取保存的状态（新房间生成玩家时调用）
    double getSavedHealth() const { return _savedHealth; }
    double getSavedMagic() const { return _savedMagic; }
    cocos2d::Vec2 getSavedPosition() const { return _savedPosition; }

    // 是否有保存的数据
    bool hasSavedData() const { return _hasSavedData; }

    // 清除保存的数据（切换完成后）
    void clearSavedData() { _hasSavedData = false; }

    // 重置（开始新游戏）
    void reset();

private:
    PlayerData();
    ~PlayerData() = default;

    static PlayerData* _instance;

    double _savedHealth;
    double _savedMagic;
    cocos2d::Vec2 _savedPosition;

    bool _hasSavedData;

    // 默认值
    static constexpr double DEFAULT_HEALTH = 100.0;
    static constexpr double DEFAULT_MAGIC = 100.0;
};