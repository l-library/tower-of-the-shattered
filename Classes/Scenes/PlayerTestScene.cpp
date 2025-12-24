#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"

USING_NS_CC;

#define COOL_DOWN 900

Scene* PlayerTestScene::createScene()
{
    return PlayerTestScene::create();
}


static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in PlayerTestScene.cpp\n");
}

bool PlayerTestScene::init()
{
    if (!Scene::initWithPhysics())
    {
        return false;
    }

    this->getPhysicsWorld()->setGravity(Vec2(0, -980));
    this->getPhysicsWorld()->setSubsteps(3);
    this->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // map_1
    auto map_1 = TMXTiledMap::create("maps/map_1.tmx");

    // 遍历地图生成多边形碰撞箱
    buildPolyPhysicsFromLayer(map_1);
    switchLevelBox(map_1);
    this->addChild(map_1, -1);

    //加载动画文件
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/PlayerAnimation.plist");
    cache->addAnimationsWithFile("player/PlayerAttackBullet.plist");

    //创建player类
    _player = Player::createNode();
    const Sprite* player_sprite = _player->getSprite();
    Size contentSize = player_sprite->getContentSize();
    _player->setPosition(Vec2(visibleSize.width / 4 + origin.x, visibleSize.height / 4 + origin.y));
    _player->setScale(2 * 32 / contentSize.width);
    this->addChild(_player, 1);///渲染player
    setupInput();

    // 添加两个Slime实例用于测试
    auto slime1 = Slime::create();
    slime1->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime1, 1);

    auto slime2 = Slime::create();
    slime2->setPosition(Vec2(visibleSize.width * 3 / 4 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime2, 1);

    // 初始化摄像机和 UI 控制器
    _cameraController = GameCamera::create(this, _player, map_1);
    _cameraController->retain(); // 因为是 Ref 类型，需要 retain 防止被自动释放
    this->scheduleUpdate();
    return true;
}
void PlayerTestScene::update(float dt) {
    // 每一帧只需要通知控制器更新
    _cameraController->update(dt);
}

void PlayerTestScene::setupInput() {
    // 创建输入监听
    auto keyboardListener = EventListenerKeyboard::create();

    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event* event) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_A:
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
                _player->moveLeft();
                break;
            case EventKeyboard::KeyCode::KEY_D:
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
                _player->moveRight();
                break;
            case EventKeyboard::KeyCode::KEY_W:
            case EventKeyboard::KeyCode::KEY_UP_ARROW:
            case EventKeyboard::KeyCode::KEY_SPACE:
            case EventKeyboard::KeyCode::KEY_K:
                _player->jump();
                break;
            case EventKeyboard::KeyCode::KEY_J:
                _player->attack();
                break;
            case EventKeyboard::KeyCode::KEY_SHIFT:
            case EventKeyboard::KeyCode::KEY_L:
                _player->dodge();
                break;
            case EventKeyboard::KeyCode::KEY_I:
            case EventKeyboard::KeyCode::KEY_1:
                _player->skillAttack("IceSpear");
                break;
            case EventKeyboard::KeyCode::KEY_O:
            case EventKeyboard::KeyCode::KEY_2:
                _player->skillAttack("ArcaneJet");
                break;
        }
        };

    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event* event) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_A:
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            case EventKeyboard::KeyCode::KEY_D:
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
                _player->stopMoving();
                break;
            case EventKeyboard::KeyCode::KEY_S:
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
                //_player->crouch(false);
                break;
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

// 生成多边形地形
void PlayerTestScene::buildPolyPhysicsFromLayer(cocos2d::TMXTiledMap* map)
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
void PlayerTestScene::switchLevelBox(cocos2d::TMXTiledMap* map)
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
                    localPhysicsBody->setContactTestBitmask(EVERYTHING_MASK);

                    polygonNode->setPhysicsBody(localPhysicsBody);
                    polygonNode->setPosition(objectPos); // 将节点位置设置为多边形的 TMX 坐标

                    //检测碰撞

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

PlayerTestScene::~PlayerTestScene()
{
    _cameraController->release();
}