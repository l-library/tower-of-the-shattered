#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"

USING_NS_CC;

#define BLOOD_BAR 1002

Scene* PlayerTestScene::createScene()
{
    Scene* scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -980));
    // 显示碰撞箱
    /*scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);*/
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
    
    auto map_1 = TMXTiledMap::create("maps/maps.tmx");
    auto layer = map_1->getLayer("layer_1");
    auto mapsize = map_1->getMapSize();
    auto tilesize = map_1->getTileSize();
    auto siz = visibleSize.height / (mapsize.height * tilesize.height);
    
    for (int y = 0; y < mapsize.height; ++y)
    {
        for (int x = 0; x < mapsize.width; ++x)
        {
            int gid = layer->getTileGIDAt(Vec2(static_cast<float>(x), static_cast<float>(y)));
            if (gid == 0)
                continue;          

            auto shape = PhysicsShapeBox::create(tilesize);
            auto body = PhysicsBody::create();
            body->addShape(shape);
            body->setDynamic(false);
            //设置掩码
            body->setCategoryBitmask(WALL_MASK);
            body->setCollisionBitmask(PLAYER_MASK | ENEMY_MASK | BULLET_MASK);
            body->setContactTestBitmask(WALL_MASK | ENEMY_MASK | BULLET_MASK);
            auto node = Sprite::create("maps/platform.png");
            node->setPosition(siz * Vec2(
                x * tilesize.width + tilesize.width / 2,
                (mapsize.height - 1 - y) * tilesize.height + tilesize.height / 2));
            node->setScale(siz);
            body->setCategoryBitmask(BORDER_MASK);
            body->setCollisionBitmask(PLAYER_MASK | ENEMY_MASK);
            body->setContactTestBitmask(PLAYER_MASK | ENEMY_MASK);
            node->setPhysicsBody(body);
            this->addChild(node);
        }
    }


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
    _player->setScale(2*32/contentSize.width);
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
            case EventKeyboard::KeyCode::KEY_S:
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
                //_player->crouch(true);
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
    auto sprite = Sprite::create("player/hp_border.png");   //创建进度框
    auto size = sprite->getContentSize();
    sprite->setPosition(Point(size.width/2 + 5, visibleSize.height - size.height/2 - 5)); //设置框的位置
    this->addChild(sprite);            //加到默认图层里面去
    auto sprBlood = Sprite::create("player/hp.png");  //创建血条
    ProgressTimer* progress = ProgressTimer::create(sprBlood); //创建progress对象
    progress->setType(ProgressTimer::Type::BAR);        //类型：条状
    progress->setPosition(Point(size.width/2 + 5, visibleSize.height - size.height/2 - 5));
    //从右到左减少血量
    progress->setMidpoint(Point(0, 0.5));     //如果是从左到右的话，改成(1,0.5)即可
    progress->setBarChangeRate(Point(1, 0));
    progress->setTag(BLOOD_BAR);       //做一个标记
    this->addChild(progress);
    schedule(CC_SCHEDULE_SELECTOR(PlayerTestScene::scheduleBlood), 0.1f);  //刷新函数，每隔0.1秒
}

void PlayerTestScene::scheduleBlood(float delta) {
    auto progress = (ProgressTimer*)this->getChildByTag(BLOOD_BAR);
    progress->setPercentage(static_cast<float>(_player->getHealth() / _player->getMaxHealth()) * 100);  //这里是百分制显示
    if (progress->getPercentage() < 0) {
        this->unschedule(CC_SCHEDULE_SELECTOR(PlayerTestScene::scheduleBlood));
    }
}
