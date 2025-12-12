#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

Scene* PlayerTestScene::createScene()
{
    Scene* scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -980));
    //开启碰撞箱显示
    /*scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);*/
    PlayerTestScene* layer = PlayerTestScene::create();
    scene->addChild(layer);
    return scene;
}

// 如果加载错误，打印错误信息
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

    //获取当前窗口的可见区域大小
    auto visibleSize = Director::getInstance()->getVisibleSize();
    //获取当前窗口的原点坐标
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
                continue;          // 跳过空白格

            auto shape = PhysicsShapeBox::create(tilesize);
            auto body = PhysicsBody::create();
            body->addShape(shape);
            body->setDynamic(false);
            auto node = Sprite::create("maps/platform.png");
            node->setPosition(siz * Vec2(
                x * tilesize.width + tilesize.width / 2,
                (mapsize.height - 1 - y) * tilesize.height + tilesize.height / 2));
            node->setScale(siz);
            body->setCategoryBitmask(BORDER_MASK);
            body->setCollisionBitmask(PLAYER_MASK);
            body->setContactTestBitmask(PLAYER_MASK);
            node->setPhysicsBody(body);
            this->addChild(node);
        }
    }


    ////设置背景
    //Sprite* background = Sprite::create("player/PlayerTest.jpg");
    //background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    //this->addChild(background);

    //在缓存中存入动画
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/PlayerAnimation.plist");

    //修改player对象
    _player = Player::createNode();
    const Sprite* player_sprite = _player->getSprite();
    Size contentSize = player_sprite->getContentSize();
    _player->setPosition(Vec2(visibleSize.width / 4 + origin.x, visibleSize.height / 4 + origin.y));
    _player->setScale(2*32/contentSize.width);
    this->addChild(_player, 1);///渲染player
    setupInput();

    return true;
}


void PlayerTestScene::setupInput() {
    // 键盘监听
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
