#include "MonsterGenerator.h"
#include "Entities/Enemy/Slime.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Mage.h"
#include "Entities/Enemy/Fly.h"
#include "Entities/Enemy/Bomber.h"
USING_NS_CC;

void MonsterGenerator::generateMonstersFromMap(Scene* scene, TMXTiledMap* map) {
    if (!scene || !map) return;

    map->setScale(MAP_SIZE);

    // 获取monster对象层
    TMXObjectGroup* monsterGroup = map->getObjectGroup("monster");
    if (!monsterGroup) {
        CCLOG("警告：地图中没有monster对象层");
        return;
    }

    // 获取所有怪物对象
    ValueVector monsters = monsterGroup->getObjects();
    CCLOG("找到 %d 个怪物生成点", (int)monsters.size());

    for (const auto& monsterValue : monsters) {
        ValueMap monsterMap = monsterValue.asValueMap();
        // 获取怪物类型
        std::string name[4] = { "Slime","Mage","Bomber","Fly" };
        std::string monsterType = name[0]; // 默认类型
        if (monsterMap.find("type") != monsterMap.end()) {
            monsterType = monsterMap.at("type").asString();
        }

        // 获取位置
        float x = 0.0f, y = 0.0f;
        if (monsterMap.find("x") != monsterMap.end()) {
            x = monsterMap.at("x").asFloat();
        }
        if (monsterMap.find("y") != monsterMap.end()) {
            y = monsterMap.at("y").asFloat();
        }

        // Tiled的y坐标需要转换（Tiled是左上角原点，Cocos是左下角）
        float mapHeight = map->getMapSize().height * map->getTileSize().height;
        float convertedY = mapHeight - y;  // 转换y坐标

        // 应用地图缩放
        cocos2d::Vec2 position(x * MAP_SIZE, convertedY * MAP_SIZE);

        CCLOG("生成怪物: 类型=%s, 原始位置=(%.1f,%.1f), 转换后位置=(%.1f,%.1f)",
            monsterType.c_str(), x, y, position.x, position.y);

        // 生成怪物
        generateMonster(scene, monsterType, position);
    }
}

void MonsterGenerator::generateMonster(Scene* scene, const std::string& monsterType,
    const cocos2d::Vec2& position) {
    if (!scene) return;

    if (monsterType == "Slime") {
        auto slime = Slime::create();
        if (slime) {
            slime->setPosition(position);
            scene->addChild(slime, 1);
            CCLOG("生成史莱姆在位置 (%.1f, %.1f)", position.x, position.y);
        }
    }
    // 可以添加其他怪物类型
    else if (monsterType == "Fly") 
    {
         auto fly = Fly::create();
         if (fly) {
             fly->setPosition(position);
             scene->addChild(fly, 1);
         }
    }
    else if (monsterType == "Mage")
    {
        auto fly = Mage::create();
        if (fly) {
            fly->setPosition(position);
            scene->addChild(fly, 1);
        }
    }
    else if (monsterType == "Bomber")
    {
        auto fly = Bomber::create();
        if (fly) {
            fly->setPosition(position);
            scene->addChild(fly, 1);
        }
    }
    else 
    {
        CCLOG("未知怪物类型: %s", monsterType.c_str());
    }
}