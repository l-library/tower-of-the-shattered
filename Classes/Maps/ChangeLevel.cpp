#include "ChangeLevel.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"
#include "Roomdata.h"
#include "Scenes/GameManager.h"
#include "Entities/NPC/Npc1.h"
#include "Entities/NPC/Npc2.h"
#include "Entities/NPC/Npc3.h"

void initRoomDatabase()
{
    RoomData room1(1, "maps/map_start.tmx");
    room1.setExit(RD, 2);
    g_roomDatabase[1] = room1;

    RoomData room2(2, "maps/map_1.tmx");
    room2.setExit(LD, 1);
    room2.setExit(RU, 3);
    g_roomDatabase[2] = room2;

    RoomData room3(3, "maps/map_middle.tmx");
    room3.setExit(LU, 2);
    room3.setExit(UP, 4);
    room3.setExit(RU, 8);
    g_roomDatabase[3] = room3;

    RoomData room4(4, "maps/map_3.tmx");
    room4.setExit(DN, 3);
    room4.setExit(LU, 5);
    room4.setExit(RD, 7);
    g_roomDatabase[4] = room4;

    RoomData room5(5, "maps/map_2.tmx");
    room5.setExit(RU, 4);
    room5.setExit(LD, 6);
    g_roomDatabase[5] = room5;

    RoomData room6(6, "maps/map_npc.tmx");
    room6.setExit(RD, 5);
    g_roomDatabase[6] = room6;

    RoomData room7(7, "maps/map_shop.tmx");
    room7.setExit(LD, 4);
    g_roomDatabase[7] = room7;

    RoomData room8(8, "maps/map_4.tmx");
    room8.setExit(LU, 3);
    room8.setExit(RD, 9);
    g_roomDatabase[8] = room8;

    RoomData room9(9, "maps/map_boss.tmx");
    room9.setExit(LD, 8);
    g_roomDatabase[9] = room9;
}

// 生成多边形地形
void buildPolyPhysicsFromLayer(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map)
{
    // 多边形碰撞
    auto layer = map->getLayer("platform");
    auto mapsize = map->getMapSize();
    auto tilesize = map->getTileSize();
    map->setScale(MAP_SIZE);

    TMXObjectGroup* objectGroup = map->getObjectGroup("obj");
    if (objectGroup)
    {
        // 获取对象组中的所有对象
        ValueVector objects = objectGroup->getObjects();

        for (const auto& objValue : objects)
        {
            ValueMap objMap = objValue.asValueMap();

            // 检查是否存在points
            if (objMap.count("points"))
            {
                ValueVector points = objMap.at("points").asValueVector();

                // 获取坐标
                std::vector<Vec2> polygonVertices;
                for (const auto& pointValue : points)
                {
                    ValueMap pointMap = pointValue.asValueMap();
                    float x = pointMap.at("x").asFloat();
                    float y = -pointMap.at("y").asFloat();

                    float objectX = objMap.at("x").asFloat();
                    float objectY = objMap.at("y").asFloat();

                    Vec2 worldPoint(objectX + x, objectY + y);
                    polygonVertices.push_back(MAP_SIZE * worldPoint);
                }

                auto physicsBody = PhysicsBody::createPolygon(polygonVertices.data(),
                    polygonVertices.size());

                if (physicsBody) {
                    // 设置物理体的属性
                    physicsBody->setDynamic(false);
                    float objectX = objMap.at("x").asFloat();
                    float objectY = objMap.at("y").asFloat();
                    Vec2 objectPos(objectX, objectY);

                    std::vector<Vec2> localVertices;
                    for (const auto& worldPoint : polygonVertices) {
                        localVertices.push_back(worldPoint - objectPos);
                    }

                    auto localPhysicsBody = PhysicsBody::createPolygon(localVertices.data(),
                        localVertices.size());

                    localPhysicsBody->setDynamic(false);
                    // 其他属性设置
                    auto polygonNode = Node::create();
                    polygonNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
                    // 设置掩码
                    localPhysicsBody->setCategoryBitmask(WALL_MASK);
                    localPhysicsBody->setCollisionBitmask(PLAYER_MASK | ENEMY_MASK | PLAYER_BULLET_MASK | ITEM_MASK);
                    localPhysicsBody->setContactTestBitmask(PLAYER_MASK | ENEMY_MASK | PLAYER_BULLET_MASK);

                    polygonNode->setPhysicsBody(localPhysicsBody);
                    polygonNode->setPosition(objectPos); // 将节点位置设置为多边形的 TMX 坐标

                    scene->addChild(polygonNode, 1);
                }
            }
        }
    }
    else
    {
        log("Object group 'Objects' not found in TMX map.");
    }
}

// 生成边界切换碰撞箱
void switchLevelBox(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map)
{
    // 多边形碰撞
    auto mapsize = map->getMapSize();
    auto tilesize = map->getTileSize();
    map->setScale(MAP_SIZE);

    TMXObjectGroup* objectGroup = map->getObjectGroup("sensor");
    if (objectGroup)
    {
        // 获取对象组中的所有对象
        ValueVector objects = objectGroup->getObjects();

        for (const auto& objValue : objects)
        {
            ValueMap objMap = objValue.asValueMap();

            // 检查是否存在points
            if (objMap.count("points"))
            {
                ValueVector points = objMap.at("points").asValueVector();

                // 获取坐标
                std::vector<Vec2> polygonVertices;
                for (const auto& pointValue : points)
                {
                    ValueMap pointMap = pointValue.asValueMap();
                    float x = pointMap.at("x").asFloat();
                    float y = -pointMap.at("y").asFloat();

                    float objectX = objMap.at("x").asFloat();
                    float objectY = objMap.at("y").asFloat();

                    Vec2 worldPoint(objectX + x, objectY + y);
                    polygonVertices.push_back(MAP_SIZE * worldPoint);
                }

                auto physicsBody = PhysicsBody::createPolygon(polygonVertices.data(),
                    polygonVertices.size());

                if (physicsBody) 
                {
                    // 设置物理体的属性
                    physicsBody->setDynamic(false);
                    float objectX = objMap.at("x").asFloat();
                    float objectY = objMap.at("y").asFloat();
                    Vec2 objectPos(objectX, objectY);

                    std::vector<Vec2> localVertices;
                    for (const auto& worldPoint : polygonVertices) {
                        localVertices.push_back(worldPoint - objectPos);
                    }

                    auto localPhysicsBody = PhysicsBody::createPolygon(localVertices.data(),
                        localVertices.size());

                    localPhysicsBody->setDynamic(false);

                    // 其他属性设置
                    auto polygonNode = Node::create();
                    polygonNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
                    // 设置掩码
                    localPhysicsBody->setCategoryBitmask(SENSOR_MASK);
                    localPhysicsBody->setCollisionBitmask(PLAYER_MASK);
                    localPhysicsBody->setContactTestBitmask(PLAYER_MASK);

                    polygonNode->setPhysicsBody(localPhysicsBody);
                    polygonNode->setPosition(objectPos); // 将节点位置设置为多边形的 TMX 坐标

                    std::string sensorName = "unknown_sensor";
                    if (objMap.find("name") != objMap.end()) {
                        sensorName = objMap.at("name").asString();
                        cocos2d::log("loading sensor: %s",
                            sensorName.c_str());
                    }

                    polygonNode->setName(sensorName);

                    scene->addChild(polygonNode, 1);
                }
            }
        }
    }
}

// 生成陷阱碰撞箱
void buildDamageBox(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map)
{
    // 多边形碰撞
    auto mapsize = map->getMapSize();
    auto tilesize = map->getTileSize();
    map->setScale(MAP_SIZE);

    TMXObjectGroup* objectGroup = map->getObjectGroup("damage");
    if (objectGroup)
    {
        // 获取对象组中的所有对象
        ValueVector objects = objectGroup->getObjects();

        for (const auto& objValue : objects)
        {
            ValueMap objMap = objValue.asValueMap();

            // 检查是否存在points
            if (objMap.count("points"))
            {
                ValueVector points = objMap.at("points").asValueVector();

                // 获取坐标
                std::vector<Vec2> polygonVertices;
                for (const auto& pointValue : points)
                {
                    ValueMap pointMap = pointValue.asValueMap();
                    float x = pointMap.at("x").asFloat();
                    float y = -pointMap.at("y").asFloat();

                    float objectX = objMap.at("x").asFloat();
                    float objectY = objMap.at("y").asFloat();

                    Vec2 worldPoint(objectX + x, objectY + y - 16 / 2);
                    polygonVertices.push_back(MAP_SIZE * worldPoint);
                }

                auto physicsBody = PhysicsBody::createPolygon(polygonVertices.data(),
                    polygonVertices.size());

                if (physicsBody)
                {
                    // 设置物理体的属性
                    physicsBody->setDynamic(false);
                    float objectX = objMap.at("x").asFloat();
                    float objectY = objMap.at("y").asFloat();
                    Vec2 objectPos(objectX, objectY);

                    std::vector<Vec2> localVertices;
                    for (const auto& worldPoint : polygonVertices) {
                        localVertices.push_back(worldPoint - objectPos);
                    }

                    auto localPhysicsBody = PhysicsBody::createPolygon(localVertices.data(),
                        localVertices.size());

                    localPhysicsBody->setDynamic(false);

                    // 其他属性设置
                    auto polygonNode = Node::create();
                    polygonNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
                    // 设置掩码
                    localPhysicsBody->setCategoryBitmask(WALL_MASK | DAMAGE_WALL_MASK);
                    localPhysicsBody->setCollisionBitmask(PLAYER_MASK);
                    localPhysicsBody->setContactTestBitmask(PLAYER_MASK);

                    polygonNode->setPhysicsBody(localPhysicsBody);
                    polygonNode->setPosition(objectPos);

                    scene->addChild(polygonNode, 1);
                }
            }
        }
    }
}

// 生成怪
void generateMonstersFromMap(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map)
{
    if (!scene || !map) return;

    map->setScale(MAP_SIZE);

    // 获取monster对象层
    TMXObjectGroup* monsterGroup = map->getObjectGroup("monster");
    if (!monsterGroup) {
        return;
    }

    // 获取所有怪物对象
    ValueVector monsters = monsterGroup->getObjects();

    for (const auto& monsterValue : monsters) {
        ValueMap monsterMap = monsterValue.asValueMap();

        // 获取位置
        float x = monsterMap.at("x").asFloat();
        float y = monsterMap.at("y").asFloat();

        cocos2d::Vec2 position(x * MAP_SIZE, y * MAP_SIZE);

        // float mapHeight = map->getMapSize().height * map->getTileSize().height;
        // float convertedY = mapHeight - y;
        // cocos2d::Vec2 position(x * MAP_SIZE, convertedY * MAP_SIZE);

        // 生成怪物
        auto slime = Slime::create();
        if (slime) {
            slime->setPosition(position);
            scene->addChild(slime, 1);

            CCLOG("生成史莱姆在 (%.1f, %.1f)", position.x, position.y);
        }
    }
}

// npc
void generateNPCsFromMap(cocos2d::Scene* scene, cocos2d::TMXTiledMap* map)
{
    if (!scene || !map) return;

    map->setScale(MAP_SIZE);

    TMXObjectGroup* npcGroup = map->getObjectGroup("npc");
    if (!npcGroup) return;

    ValueVector npcs = npcGroup->getObjects();

    for (const auto& npcValue : npcs) {
        ValueMap npcMap = npcValue.asValueMap();

        // 获取NPC名称
        std::string npcName = npcMap.find("name") != npcMap.end() ?
            npcMap.at("name").asString() : "npc_1";

        // 获取位置
        float x = npcMap.at("x").asFloat();
        float y = npcMap.at("y").asFloat();

        // 你的多边形是16x16的矩形，计算中心点
        // 多边形顶点: (0,0) (16,0) (16,16) (0,16)
        float centerX = x + 8;  // x + 16/2
        float centerY = y + 8;  // y + 16/2

        cocos2d::Vec2 position(centerX * MAP_SIZE, centerY * MAP_SIZE);

        CCLOG("生成NPC: %s 位置=(%.1f,%.1f)", npcName.c_str(), position.x, position.y);

        // 根据名称生成
        if (npcName == "npc_1") {
            if (auto npc = NPC1::create()) {
                npc->setPosition(position);
                scene->addChild(npc, 1);
            }
        }
        else if (npcName == "npc_2") {
            if (auto npc = NPC2::create()) {
                npc->setPosition(position);
                scene->addChild(npc, 1);
            }
        }
        else if (npcName == "npc_3") {
            if (auto npc = NPC3::create()) {
                npc->setPosition(position);
                scene->addChild(npc, 1);
            }
        }
    }
}

// 传送房间
bool exitRoom(int currentRoomId, const std::string& exitDir)
{
    // 查找当前房间
    auto currentIt = g_roomDatabase.find(currentRoomId);
    if (currentIt == g_roomDatabase.end())
    {
        return false;
    }

    RoomData& currentRoom = currentIt->second;

    // 查找这个出口
    Exit* exit = currentRoom.getExit(exitDir);
    if (!exit)
    {
        return false;
    }

    // 查找目标房间
    auto targetIt = g_roomDatabase.find(exit->targetRoomId);
    if (targetIt == g_roomDatabase.end()) 
    {
        return false;
    }
    
    RoomData& targetRoom = targetIt->second;
    auto objects = TMXTiledMap::create(targetRoom.tmxFile)->getObjectGroup("sensor")->getObjects();
    ValueMap objMap;
    for (const auto& objValue : objects)
    {
        objMap = objValue.asValueMap();
        if (objMap.find("name") != objMap.end() && objMap.at("name").asString() == targetEnterID(exitDir))
            break;
    }

    // 获取出生点
    cocos2d::Vec2 spawnPos = cocos2d::Vec2{ MAP_SIZE * objMap.at("x").asFloat(),MAP_SIZE * (objMap.at("y").asFloat() - 16 * 4) };
    if (targetEnterID(exitDir) == LD || targetEnterID(exitDir) == LU)
        spawnPos.x += 16 * 3 * MAP_SIZE;
    else if (targetEnterID(exitDir) == RD || targetEnterID(exitDir) == RU)
        spawnPos.x -= 16 * 3 * MAP_SIZE;
    else if (targetEnterID(exitDir) == UP)
        spawnPos.y -= 16 * 3 * MAP_SIZE;
    else
    {
        spawnPos.x -= 16 * 3 * MAP_SIZE;
        spawnPos.y += 16 * 5 * MAP_SIZE;
    }

    // 如果返回零向量，使用默认
    if (spawnPos == cocos2d::Vec2::ZERO)
    {
        auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        spawnPos = cocos2d::Vec2(visibleSize.width / 4, visibleSize.height / 4);
    }

    // 创建新场景并设置出生点
    auto newScene = PlayerTestScene::createWithMap(targetRoom.tmxFile, spawnPos);
    if (!newScene) 
    {
        return false;
    }

    // 更新当前房间ID
    g_currentRoomId = exit->targetRoomId;

    // 切换场景
    cocos2d::Director::getInstance()->replaceScene(newScene);

    return true;
}

// 切换场景调用
void onPlayerHitSensor(cocos2d::Scene* scene, cocos2d::Node* sensorNode) {
    auto player = scene->getChildByName<Player*>("player");
    if (player) {
        auto physicsBody = player->getPhysicsBody();
        if (physicsBody) {
            player->removeComponent(physicsBody);
            CCLOG("remove Player's PhysicsBody");
        }
        GameManager::getInstance()->savePlayer(player);
    }
    std::string sensorName = sensorNode->getName();
    exitRoom(g_currentRoomId, sensorName);
}

// 检测碰撞
void setupCollisionListener(cocos2d::Scene* scene)
{
    auto contactListener = EventListenerPhysicsContact::create();

    contactListener->onContactBegin = [scene](PhysicsContact& contact) -> bool 
        {
        auto shapeA = contact.getShapeA();
        auto shapeB = contact.getShapeB();
        auto bodyA = shapeA->getBody();
        auto bodyB = shapeB->getBody();

        // 检查是否是玩家和传感器的碰撞
        unsigned int maskA = bodyA->getCategoryBitmask();
        unsigned int maskB = bodyB->getCategoryBitmask();

        // 玩家碰到了传感器
        if ((maskA & PLAYER_MASK && maskB & SENSOR_MASK) ||
            (maskA & SENSOR_MASK && maskB & PLAYER_MASK)) {

            // 找出哪个是传感器节点
            Node* sensorNode = nullptr;
            Node* playerNode = nullptr;

            if (maskA == SENSOR_MASK) {
                sensorNode = bodyA->getNode();
                playerNode = bodyB->getNode();
            }
            else {
                sensorNode = bodyB->getNode();
                playerNode = bodyA->getNode();
            }
            onPlayerHitSensor(scene, sensorNode);
        }
        return true;
        };
    scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, scene);
}

cocos2d::Scene* createGameSceneWithMap(const std::string& mapFile)
{
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -980));
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 加载地图
    auto map = TMXTiledMap::create(mapFile);
    map->setScale(MAP_SIZE);
    buildPolyPhysicsFromLayer(scene, map);
    switchLevelBox(scene, map);
    scene->addChild(map, -1);

    // 创建player
    auto player = Player::createNode();
    player->setPosition(Vec2(visibleSize.width / 4 + origin.x, visibleSize.height / 4 + origin.y));
    scene->addChild(player, 1);

    return scene;
}