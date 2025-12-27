#include "BaseGameScene.h"

bool BaseGameScene::initWithInfo(const SceneInfo& info) {
    if (!Scene::init()) return false;

    _sceneInfo = info;

    // 根据场景类型进行不同设置
    switch (info.type) {
    case SceneType::NORMAL:
        setupNormalLevel();
        break;

    case SceneType::SHOP:
        setupShopLevel();
        break;

    case SceneType::BOSS:
        setupBossLevel();
        break;
    }

    // 加载地图
    loadMap(info.tmxFile);

    return true;
}

void BaseGameScene::setupNormalLevel() {
    // 普通关卡设置
    this->getPhysicsWorld()->setGravity(cocos2d::Vec2(0, -980));
    enableCombat(true);
    spawnEnemies();
}

void BaseGameScene::setupShopLevel() {
    // 商店设置
    this->getPhysicsWorld()->setGravity(cocos2d::Vec2(0, -980));
    enableCombat(false); // 商店里不能战斗
    spawnShopNPCs();
    createShopUI();
}

void BaseGameScene::setupBossLevel() {
    // Boss关卡设置
    this->getPhysicsWorld()->setGravity(cocos2d::Vec2(0, -980));
    enableCombat(true);
    lockDoors(); // 进入Boss战后锁门
    playBossIntro();
}