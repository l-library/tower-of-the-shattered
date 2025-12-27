#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"
#include "Maps/ChangeLevel.h"
#include "Entities/NPC/Npc2.h"
#include "Entities/NPC/Npc1.h"
#include "Entities/Player/PlayerData.h"
#include "Maps/RoomData.h"
#include "Scenes/PauseMenuScene.h"
#include "Tools/SaveManager.h"
#include "Entities/Items/ItemManager.h"

USING_NS_CC;

#define COOL_DOWN 900

Scene* PlayerTestScene::createScene()
{
    initRoomDatabase();
    return PlayerTestScene::create();
}

Scene* PlayerTestScene::createWithMap(const std::string& mapFile, cocos2d::Vec2 pos)
{
    PlayerTestScene* pRet = new(std::nothrow) PlayerTestScene();
    pRet->setPlayerSpawnPosition(pos);
    if (pRet)
    {
        pRet->_currentMapFile = mapFile;
        if (pRet->init())
        {
            pRet->autorelease();
            return pRet;
        }
        else
        {
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
    if (SaveManager::getInstance()->sceneComeFromMenu()) {
        // 添加一个全屏黑色遮罩，Z序设为最高
        auto maskLayer = LayerColor::create(Color4B::BLACK);
        this->addChild(maskLayer, 9999);

        // 让遮罩层淡出
        auto seq = Sequence::create(
            DelayTime::create(0.1f),
            FadeOut::create(1.0f),
            RemoveSelf::create(),    // 动画结束后删除遮罩
            nullptr
        );
        maskLayer->runAction(seq);
        SaveManager::getInstance()->setsceneComeFromMenu(false);
    }

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

    // ������ײ��
    buildPolyPhysicsFromLayer(this, map_1);
    switchLevelBox(this, map_1);
    buildDamageBox(this, map_1);
    this->addChild(map_1, -1);

    // ���ض����ļ�
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/PlayerAnimation.plist");
    cache->addAnimationsWithFile("player/PlayerAttackBullet.plist");

    // ����player��
    //_player = Player::createNode();
    //const Sprite* player_sprite = _player->getSprite();
    //Size contentSize = player_sprite->getContentSize();
    //_player->setPosition(_playerSpawnPosition);
    //_player->setScale(2 * 32 / contentSize.width);
    //this->addChild(_player, 1);// ��Ⱦplayer
    // ����player��
    _player = Player::createNode();
    const Sprite* player_sprite = _player->getSprite();
    Size contentSize = player_sprite->getContentSize();

    auto playerData = PlayerData::getInstance();

    // ��ȡ��������λ��
    cocos2d::Vec2 spawnPos = _playerSpawnPosition;

    
    if (playerData->hasSavedData())
    {
        
        _player->setHealth(playerData->getSavedHealth());
        _player->setMagic(playerData->getSavedMagic());

        CCLOG("Ӧ�ñ�������: Ѫ��=%.1f, ħ��=%.1f",
            playerData->getSavedHealth(), playerData->getSavedMagic());

        
        playerData->clearSavedData();
    }
    else 
    {
        
        _player->setHealth(100.0);
        _player->setMagic(100.0);
        CCLOG("ʹ��Ĭ��״̬");
    }

    
    _player->setPosition(spawnPos);
    _player->setScale(2 * 32 / contentSize.width);
    _player->setName("player");


    this->addChild(_player, 1);// ��Ⱦplayer
    setupInput();

    auto slime1 = Slime::create();
    slime1->setPosition(_player->getPosition());
    this->addChild(slime1, 1);
    
    auto slime2 = Slime::create();
    slime2->setPosition(Vec2(visibleSize.width * 3 / 4 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime2, 1);

    setupCollisionListener(this);


    _cameraController = GameCamera::create(this, _player, map_1);
    _cameraController->retain(); 
    this->scheduleUpdate();

    
    AudioManager::getInstance()->playIntroLoopBGM("sounds/BGM-Normal.ogg", "sounds/BGM-Normal-loop.ogg");
    AudioManager::getInstance()->setBGMVolume(0.9f);

    
    ItemManager::getInstance()->init("config/items.json");
    
    auto item = Items::createWithId(110);
    if (item) {
        item->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

        
        item->getPhysicsBody()->setVelocity(Vec2(0, 200));

        this->addChild(item, 5);
    }
    return true;
}

void PlayerTestScene::update(float dt) {
    // ÿһֻ֡��Ҫ֪ͨ����������
    _cameraController->update(dt);
    if (_player->getCurrentState() == "dead")
        gameOver();
}

void PlayerTestScene::setupInput() {
    // ������������
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
            case EventKeyboard::KeyCode::KEY_ESCAPE:
                auto PauseScene = PauseMenuScene::createScene();
                Director::getInstance()->pushScene(PauseScene);
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

void PlayerTestScene::gameOver()
{
    // 防止每一帧重复调用
    static bool isGameOverProcessing = false;
    if (isGameOverProcessing) return;
    isGameOverProcessing = true;

    // 移除所有输入监听
    _eventDispatcher->removeEventListenersForTarget(this);

    // 停止物理世界的模拟
    this->getPhysicsWorld()->setAutoStep(false);

    // 停止背景音乐
    AudioManager::getInstance()->pauseBGM();

    // 创建死亡提示 Sprite
    Sprite* deadSprite = Sprite::create("player/dead.png");

    // 重置itemMananger
    ItemManager::getInstance()->resetRuntimeData();

    if (deadSprite)
    {
        //以此确保图片显示在当前相机的视野中心
        auto visibleSize = Director::getInstance()->getVisibleSize();
        deadSprite->setPosition(visibleSize / 2);
        deadSprite->setAnchorPoint(Vec2(0.5f,0.5f));
        deadSprite->setOpacity(0); // 初始完全透明
        deadSprite->setScale(0.5f); // 初始稍小，配合放大效果
        deadSprite->setCameraMask((uint16_t)CameraFlag::USER2, true);
        _cameraController->getUIRoot()->addChild(deadSprite, 99999);

        // 定义动画序列
        auto spawnAction = Spawn::create(
            FadeIn::create(2.0f),           // 2秒淡入
            ScaleTo::create(2.0f, 2.0f),    // 同时放大到原比例
            nullptr
        );

        auto sequence = Sequence::create(
            spawnAction,
            DelayTime::create(2.0f),
            CallFunc::create([this]() {
                // 重置静态变量
                isGameOverProcessing = false;
                AudioManager::getInstance()->playEffect("sounds/Death.ogg");
                // 创建并切换到菜单场景，使用淡出淡入转场
                auto menuScene = MainMenuScene::createScene();
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, menuScene, Color3B::BLACK));
                }),
            nullptr
        );

        deadSprite->runAction(sequence);
    }
    else
    {
        // 如果图片加载失败，直接切换场景
        log("Error: player/dead.png not found!");
        isGameOverProcessing = false;
        auto menuScene = MainMenuScene::createScene();
        Director::getInstance()->replaceScene(menuScene);
    }
}