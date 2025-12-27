#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"
#include "Entities/Enemy/Slime.h"
#include "Maps/ChangeLevel.h"
#include "Entities/NPC/Npc2.h"
#include "Entities/NPC/Npc1.h"
#include "Entities/Player/PlayerData.h"
#include "Maps/RoomData.h"
#include "Scenes/PauseMenuScene.h"
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
    // 添加一个全屏黑色遮罩，Z序设为最高
    auto maskLayer = LayerColor::create(Color4B::BLACK);
    this->addChild(maskLayer, 9999);

    // 让遮罩层淡出
    auto seq = Sequence::create(
        DelayTime::create(0.1f), // 可选：多等0.1秒确保万无一失
        FadeOut::create(1.0f),   // 1秒淡出，显示游戏画面
        RemoveSelf::create(),    // 动画结束后移除遮罩
        nullptr
    );
    maskLayer->runAction(seq);

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

    // 生成碰撞箱
    buildPolyPhysicsFromLayer(this, map_1);
    switchLevelBox(this, map_1);
    buildDamageBox(this, map_1);
    this->addChild(map_1, -1);

    // 加载动画文件
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/PlayerAnimation.plist");
    cache->addAnimationsWithFile("player/PlayerAttackBullet.plist");

    // 创建player类
    //_player = Player::createNode();
    //const Sprite* player_sprite = _player->getSprite();
    //Size contentSize = player_sprite->getContentSize();
    //_player->setPosition(_playerSpawnPosition);
    //_player->setScale(2 * 32 / contentSize.width);
    //this->addChild(_player, 1);// 渲染player
    // 创建player类
    _player = Player::createNode();
    const Sprite* player_sprite = _player->getSprite();
    Size contentSize = player_sprite->getContentSize();

    auto playerData = PlayerData::getInstance();

    // 获取玩家生成位置
    cocos2d::Vec2 spawnPos = _playerSpawnPosition;

    // 检查是否有保存的数据（来自存档或传送）
    if (playerData->hasSavedData())
    {
        // 使用保存的数据设置玩家状态
        _player->setHealth(playerData->getSavedHealth());
        _player->setMagic(playerData->getSavedMagic());

        CCLOG("应用保存数据: 血量=%.1f, 魔力=%.1f",
            playerData->getSavedHealth(), playerData->getSavedMagic());

        // 使用完保存数据后清除，避免重复使用
        playerData->clearSavedData();
    }
    else 
    {
        // 没有保存的数据，使用默认状态
        _player->setHealth(100.0);
        _player->setMagic(100.0);
        CCLOG("使用默认状态");
    }

    // 设置玩家位置，使用默认或保存的生成点
    _player->setPosition(spawnPos);
    _player->setScale(2 * 32 / contentSize.width);
    _player->setName("player");


    this->addChild(_player, 1);// 渲染player
    setupInput();

    auto slime1 = NPC2::create();
    slime1->setPosition(_player->getPosition());
    this->addChild(slime1, 1);
    
    auto slime2 = Slime::create();
    slime2->setPosition(Vec2(visibleSize.width * 3 / 4 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(slime2, 1);

    // 初始化摄像机和 UI 控制器
    _cameraController = GameCamera::create(this, _player, map_1);
    _cameraController->retain(); // 因为是 Ref 类型，需要 retain 防止被自动释放
    this->scheduleUpdate();

    // 播放背景音乐
    AudioManager::getInstance()->playIntroLoopBGM("sounds/BGM-Normal.ogg", "sounds/BGM-Normal-loop.ogg");
    AudioManager::getInstance()->setBGMVolume(0.9f);

    // 初始化物品管理器
    ItemManager::getInstance()->init("config/items.json");
    // 示例：以下初始化了一个物品供测试 物品id107
    auto item = Items::createWithId(2000);
    if (item) {
        item->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

        // 模拟爆出来的效果：给一个向上的初速度
        item->getPhysicsBody()->setVelocity(Vec2(0, 200));

        this->addChild(item, 5); // Z-order 在背景之上
    }
    // 示例：增加金币
    ItemManager::getInstance()->addGold(50);
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