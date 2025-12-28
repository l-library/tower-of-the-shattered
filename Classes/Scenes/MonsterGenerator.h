#pragma once
#include "cocos2d.h"

class MonsterGenerator {
public:
    static void generateMonstersFromMap(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map);

    static void generateMonster(cocos2d::Scene* scene, const std::string& monsterType,
        const cocos2d::Vec2& position);
};