#pragma once
#include "cocos2d.h"

void initRoomDatabase();

// 生成多边形地形
void buildPolyPhysicsFromLayer(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map);

// 生成边界切换碰撞箱
void switchLevelBox(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map);

// 生成陷阱碰撞箱
void buildDamageBox(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map);

// 切换场景调用
void onPlayerHitSensor(cocos2d::Scene* scene, cocos2d::Node* sensorNode);

// 检测碰撞
void setupCollisionListener(cocos2d::Scene* scene);

cocos2d::Scene* createGameSceneWithMap(const std::string& mapFile);