#include "PlayerData.h"

USING_NS_CC;

PlayerData* PlayerData::_instance = nullptr;

PlayerData* PlayerData::getInstance() {
    if (!_instance) {
        _instance = new PlayerData();
    }
    return _instance;
}

PlayerData::PlayerData() {
    reset();
}

void PlayerData::reset() {
    _savedHealth = DEFAULT_HEALTH;
    _savedMagic = DEFAULT_MAGIC;
    _savedPosition = Vec2::ZERO;
    _hasSavedData = false;
}

void PlayerData::savePlayerState(double health, double magic,
    const cocos2d::Vec2& position,
    int direction) {
    _savedHealth = health;
    _savedMagic = magic;
    _savedPosition = position;
    _hasSavedData = true;

    CCLOG("保存玩家状态: 血量=%.1f, 蓝量=%.1f, 位置=(%.1f, %.1f), 方向=%d",
        health, magic, position.x, position.y, direction);
}