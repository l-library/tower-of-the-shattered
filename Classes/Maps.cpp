#include "Maps.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

Scene* Maps::createScene()
{
    Scene* scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -980));

    Maps* layer = Maps::create();
    scene->addChild(layer);
    return scene;
}

bool Maps::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto map_1 = TMXTiledMap::create("maps/map_middle.tmx");
    buildPolyPhysicsFromLayer(map_1);

    return true;
}

// 生成多边形地形
void Maps::buildPolyPhysicsFromLayer(cocos2d::TMXTiledMap* map)
{
    // 多边形碰撞
    auto layer = map->getLayer("platform");
    auto mapsize = map->getMapSize();
    auto tilesize = map->getTileSize();
    map->setScale(MAP_SIZE);


    TMXObjectGroup* objectGroup = map->getObjectGroup("obj"); // 替换为你的对象层名称
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
                    // 其他碰撞属性设置
                    auto polygonNode = Node::create();
                    polygonNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
                    //设置掩码
                    localPhysicsBody->setCategoryBitmask(WALL_MASK);
                    localPhysicsBody->setCollisionBitmask(PLAYER_MASK | ENEMY_MASK | PLAYER_BULLET_MASK);
                    localPhysicsBody->setContactTestBitmask(PLAYER_MASK | ENEMY_MASK | PLAYER_BULLET_MASK);

                    polygonNode->setPhysicsBody(localPhysicsBody);
                    polygonNode->setPosition(objectPos); // 将节点位置设置为多边形的 TMX 坐标

                    this->addChild(polygonNode, 1);
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
void Maps::switchLevelBox(cocos2d::TMXTiledMap* map)
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

                    this->addChild(polygonNode, 1);
                }
            }
        }
    }
    else
    {
        log("Object group 'sensor' not found in TMX map.");
    }
}