#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"

USING_NS_CC;

#define BLOOD_BAR 1002
#define MAGIC_BAR 1003

Scene* PlayerTestScene::createScene()
{
    Scene* scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -980));
    // 显示碰撞箱
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
    PlayerTestScene* layer = PlayerTestScene::create();
    scene->addChild(layer);
    return scene;
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

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // map_1
    auto map_1 = TMXTiledMap::create("maps/map_boss.tmx");

    // 遍历地图生成多边形碰撞箱
    buildPolyPhysicsFromLayer(map_1);
    this->addChild(map_1, -1);

    //Sprite* background = Sprite::create("player/PlayerTest.jpg");
    //background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    //this->addChild(background);

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
    initBar();

    // 添加两个Slime实例用于测试
    auto slime1 = Slime::create();
    slime1->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime1, 1);

    auto slime2 = Slime::create();
    slime2->setPosition(Vec2(visibleSize.width * 3 / 4 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime2, 1);

    return true;
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

void PlayerTestScene::initBar() {
    // 获取窗口大小
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // 血条
    auto sprite = Sprite::create("player/hp_border.png");   //创建进度框
    auto size = sprite->getContentSize();
    sprite->setPosition(Point(size.width/2 + 5, visibleSize.height - size.height/2 - 5)); //设置框的位置
    this->addChild(sprite);            //加到默认图层里面去
    auto sprBlood = Sprite::create("player/hp.png");  //创建血条
    ProgressTimer* progress_health = ProgressTimer::create(sprBlood); //创建progress对象
    progress_health->setType(ProgressTimer::Type::BAR);        //类型：条状
    progress_health->setPosition(Point(size.width/2 + 5, visibleSize.height - size.height/2 - 5));
    //从右到左减少血量
    progress_health->setMidpoint(Point(0, 0.5));     //如果是从左到右的话，改成(1,0.5)即可
    progress_health->setBarChangeRate(Point(1, 0));
    progress_health->setTag(BLOOD_BAR);       //做一个标记
    this->addChild(progress_health);
    schedule(CC_SCHEDULE_SELECTOR(PlayerTestScene::scheduleBlood), 0.1f);  //刷新函数，每隔0.1秒
    // 蓝条
    sprite = Sprite::create("player/mp_border.png");   //创建进度框
    size = sprite->getContentSize();
    sprite->setPosition(Point(size.width / 2 + 5, visibleSize.height - size.height * 3 / 2 - 5 - 5)); //设置框的位置
    this->addChild(sprite);            //加到默认图层里面去
    auto sprMagic = Sprite::create("player/mp.png");  //创建蓝条
    auto progress_magic = ProgressTimer::create(sprMagic); //创建progress对象
    progress_magic->setType(ProgressTimer::Type::BAR);        //类型：条状
    progress_magic->setPosition(Point(size.width / 2 + 5, visibleSize.height - size.height * 3 / 2 - 5 - 5));
    //从右到左减少蓝条
    progress_magic->setMidpoint(Point(0, 0.5));     //如果是从左到右的话，改成(1,0.5)即可
    progress_magic->setBarChangeRate(Point(1, 0));
    progress_magic->setTag(MAGIC_BAR);       //做一个标记
    this->addChild(progress_magic);
    schedule(CC_SCHEDULE_SELECTOR(PlayerTestScene::scheduleBlood), 0.1f);  //刷新函数，每隔0.1秒
}

void PlayerTestScene::scheduleBlood(float delta) {
    auto progress_health = (ProgressTimer*)this->getChildByTag(BLOOD_BAR);
    progress_health->setPercentage(static_cast<float>(_player->getHealth() / _player->getMaxHealth()) * 100);  //这里是百分制显示
    if (progress_health->getPercentage() < 0) {
        this->unschedule(CC_SCHEDULE_SELECTOR(PlayerTestScene::scheduleBlood));
    }
    auto progress_magic = (ProgressTimer*)this->getChildByTag(MAGIC_BAR);
    progress_magic->setPercentage(static_cast<float>(_player->getMagic() / _player->getMaxMagic()) * 100);  //这里是百分制显示
    if (progress_magic->getPercentage() < 0) {
        this->unschedule(CC_SCHEDULE_SELECTOR(PlayerTestScene::scheduleBlood));
    }
}

// 判断点集顺/逆时针方向
static bool isCounterClockwise(const std::vector<Vec2>& v)
{
    if (v.size() < 3) return true;
    float crossl = 0;

    Vec2 v_1 = v[1] - v[0];
    Vec2 v_2 = v[2] - v[1];
    double cross = v_1.x * v_2.y - v_1.y * v_2.x;

    return cross > 0;   // >0 逆时针
}

// 生成多边形地形
void PlayerTestScene::buildPolyPhysicsFromLayer(cocos2d::TMXTiledMap* map)
{
    // 多边形碰撞
    auto layer = map->getLayer("platform");
    auto mapsize = map->getMapSize();
    auto tilesize = map->getTileSize();
    auto siz = 1;// visibleSize.height / (mapsize.height * tilesize.height);
    map->setScale(siz);


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
                std::vector<Vec2> test_clock;
                for (const auto& pointValue : points)
                {
                    ValueMap pointMap = pointValue.asValueMap();
                    float x = pointMap.at("x").asFloat();
                    float y = pointMap.at("y").asFloat();
                    test_clock.push_back(Vec2(x, y));
                }
                const int is_clock = isCounterClockwise(test_clock);
                log("id=%d,isclock=%d", objMap.at("id").asInt(), is_clock);
                if (!is_clock)
                {
                    Vec2 v = test_clock[0];
                    std::reverse(test_clock.begin() + 1, test_clock.end());
                }

                int num = 0;
                std::vector<Vec2> polygonVertices;
                for (const auto& pointValue : points)
                {
                    ValueMap pointMap = pointValue.asValueMap();
                    float x = test_clock[num].x;
                    float y = -test_clock[num].y;

                    float objectX = objMap.at("x").asFloat();
                    float objectY = objMap.at("y").asFloat();

                    Vec2 worldPoint(objectX + x, objectY + y);
                    polygonVertices.push_back(siz * worldPoint);
                    num++;
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
