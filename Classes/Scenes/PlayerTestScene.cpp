#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"
#include "Entities/Enemy/Fly.h"
#include "Maps/ChangeLevel.h"

USING_NS_CC;

#define COOL_DOWN 900

Scene* PlayerTestScene::createScene()
{
    return PlayerTestScene::create();
}

Scene* PlayerTestScene::createWithMap(const std::string& mapFile) {
    // 1. ֱ�Ӵ���ʵ����������Ĭ�ϵ� create()
    PlayerTestScene* pRet = new(std::nothrow) PlayerTestScene();

    if (pRet) {
        // 2. �����õ�ͼ�ļ���
        pRet->_currentMapFile = mapFile;

        // 3. �ٵ��� init()
        if (pRet->init()) {
            pRet->autorelease();
            return pRet;
        }
        else {
            delete pRet;
            pRet = nullptr;
        }
    }

    return nullptr;
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

    this->getPhysicsWorld()->setAutoStep(true);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // map_1
    auto map_1 = TMXTiledMap::create(_currentMapFile);

    // ������ͼ���ɶ������ײ��
    buildPolyPhysicsFromLayer(this, map_1);
    switchLevelBox(this, map_1);
    this->addChild(map_1, -1);

    //���ض����ļ�
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/PlayerAnimation.plist");
    cache->addAnimationsWithFile("player/PlayerAttackBullet.plist");

    //����player��
    _player = Player::createNode();
    const Sprite* player_sprite = _player->getSprite();
    Size contentSize = player_sprite->getContentSize();
    _player->setPosition(Vec2(visibleSize.width / 4 + origin.x, visibleSize.height / 4 + origin.y));
    _player->setScale(2 * 32 / contentSize.width);
    this->addChild(_player, 1);///��Ⱦplayer
    setupInput();

    // ��������Slimeʵ�����ڲ���
    auto slime1 = Fly::create();
    slime1->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime1, 1);

    auto slime2 = Fly::create();
    slime2->setPosition(Vec2(visibleSize.width * 3 / 4 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime2, 1);

    setupCollisionListener(this);

    // ��ʼ��������� UI ������
    _cameraController = GameCamera::create(this, _player, map_1);
    _cameraController->retain(); // ��Ϊ�� Ref ���ͣ���Ҫ retain ��ֹ���Զ��ͷ�
    this->scheduleUpdate();

    // 播放背景音乐
    AudioManager::getInstance()->playIntroLoopBGM("sounds/BGM-Normal.ogg", "sounds/BGM-Normal-loop.ogg");
    AudioManager::getInstance()->setBGMVolume(0.9f);

    return true;
}

void PlayerTestScene::update(float dt) {
    // ÿһֻ֡��Ҫ֪ͨ����������
    _cameraController->update(dt);
}

void PlayerTestScene::setupInput() {
    // �����������
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
            case EventKeyboard::KeyCode::KEY_P:
            case EventKeyboard::KeyCode::KEY_3:
                _player->skillAttack("ArcaneShield");
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

PlayerTestScene::~PlayerTestScene()
{
    _cameraController->release();
}