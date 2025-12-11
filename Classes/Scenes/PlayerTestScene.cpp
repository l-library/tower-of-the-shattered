#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"

USING_NS_CC;

Scene* PlayerTestScene::createScene()
{
    Scene* scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -980));
    //������ײ����ʾ
    /*scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);*/
    PlayerTestScene* layer = PlayerTestScene::create();
    scene->addChild(layer);
    return scene;
}

// ������ش��󣬴�ӡ������Ϣ
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

    //��ȡ��ǰ���ڵĿɼ������С
    auto visibleSize = Director::getInstance()->getVisibleSize();
    //��ȡ��ǰ���ڵ�ԭ������
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
                continue;          // �����հ׸�

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
            body->setCollisionBitmask(PLAYER_MASK | ENEMY_MASK);
            body->setContactTestBitmask(PLAYER_MASK | ENEMY_MASK);
            node->setPhysicsBody(body);
            this->addChild(node);
        }
    }


    ////���ñ���
    //Sprite* background = Sprite::create("player/PlayerTest.jpg");
    //background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    //this->addChild(background);

    //�ڻ����д��붯��
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/PlayerAnimation.plist");

    //�޸�player����
    _player = Player::createNode();
    const Sprite* player_sprite = _player->getSprite();
    Size contentSize = player_sprite->getContentSize();
    _player->setPosition(Vec2(visibleSize.width / 4 + origin.x, visibleSize.height / 4 + origin.y));
    _player->setScale(2*32/contentSize.width);
    this->addChild(_player, 1);///��Ⱦplayer
    setupInput();
    
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
    // ���̼���
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
