#include "ChangeLevel.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"

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
                    //设置掩码
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
                    //设置掩码
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

// 切换场景调用
void onPlayerHitSensor(cocos2d::Scene* scene, cocos2d::Node* sensorNode)
{
    std::string sensorName = sensorNode->getName();
    cocos2d::Vec2 sensorPos = sensorNode->getPosition();

    if (sensorName == LD)
    {
        log("LD");
        auto newScene = PlayerTestScene::createWithMap("maps/map_3.tmx");
        cocos2d::Director::getInstance()->replaceScene(newScene);
    }
    else if (sensorName == LU)
    {

    }
    else if (sensorName == RD)
    {

    }
    else if (sensorName == RU)
    {
        log("RU");
        createGameSceneWithMap("maps/map_middle.tmx");
    }
    else if (sensorName == DN)
    {

    }
    else if (sensorName == UP)
    {

    }
}

// 检测碰撞
void setupCollisionListener(cocos2d::Scene* scene)
{
    auto contactListener = EventListenerPhysicsContact::create();

    contactListener->onContactBegin = [scene](PhysicsContact& contact) -> bool {
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
            log("111");
            onPlayerHitSensor(scene, sensorNode);
        }
        return true;
        };
    scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, scene);
}

cocos2d::Scene* createGameSceneWithMap(const std::string& mapFile) {
    auto scene = Scene::createWithPhysics();
    if (!Scene::createWithPhysics())
        log(-1);
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