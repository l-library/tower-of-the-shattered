#include "GameManager.h"
#include "Entities/Player/Player.h"

USING_NS_CC;

GameManager* GameManager::_instance = nullptr;

GameManager* GameManager::getInstance() {
    if (!_instance) {
        _instance = new GameManager();
    }
    return _instance;
}

GameManager::GameManager() : _savedPlayer(nullptr) {
    CCLOG("GameManager初始化");
}

GameManager::~GameManager() {
    clearPlayer();
    CCLOG("GameManager销毁");
}

void GameManager::savePlayer(Player* player) {
    if (!player) {
        CCLOG("警告：尝试保存空Player");
        return;
    }

    // 清理旧的
    clearPlayer();

    // 保存指针
    _savedPlayer = player;

    // 手动retain（防止自动释放）
    _savedPlayer->retain();    
    
    // 从父节点移除（防止随场景销毁）
    _savedPlayer->removeFromParent();

    CCLOG("Save Player");
}

Player* GameManager::getPlayer() {
    if (!_savedPlayer) {
        CCLOG("No Saved Player");
        return nullptr;
    }

    Player* player = _savedPlayer;

    // 转移所有权
    _savedPlayer = nullptr;

    CCLOG("Return Player");
    return player;
}

void GameManager::clearPlayer() {
    if (_savedPlayer) {
        // 释放我们retain的引用
        _savedPlayer->release();
        _savedPlayer = nullptr;
        CCLOG("Clear Player");
    }
}

void GameManager::reset() {
    clearPlayer();
    CCLOG("GameManager Reset");
}