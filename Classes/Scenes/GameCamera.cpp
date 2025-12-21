#include "GameCamera.h"

USING_NS_CC;

GameCamera* GameCamera::create(Scene* scene, Player* player, TMXTiledMap* map) {
    auto ret = new GameCamera();
    if (ret && ret->init(scene, player, map)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool GameCamera::init(Scene* scene, Player* player, TMXTiledMap* map) {
    _scene = scene;
    _player = player;
    _map = map;
    _zoom_factor = 1.0f; // 默认缩放倍率，0.5 表示看到一半的视野
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 获取默认摄像机
    _defaultCamera = scene->getDefaultCamera();
    _defaultCamera->setDepth(0);
    _defaultCamera->initOrthographic(visibleSize.width * _zoom_factor, visibleSize.height * _zoom_factor, 1,1000);
    _defaultCamera->setCameraFlag(CameraFlag::DEFAULT);

    // UI 摄像机
    _uiCamera = Camera::createOrthographic(visibleSize.width, visibleSize.height, 1, 1100);
    _uiCamera->setCameraFlag(CameraFlag::USER2);
    // UI 摄像机固定在屏幕中心，永远不动
    _uiCamera->setDepth(1); // 保证在最上层
    _uiCamera->setPosition3D(Vec3(0, 0, 1100));
    _uiCamera->lookAt(Vec3(0, 0, 0));
    _scene->addChild(_uiCamera);

    // UI 根节点
    _uiRoot = Node::create();
    _uiRoot->setContentSize(visibleSize);
    _uiRoot->setCameraMask((uint16_t)CameraFlag::USER2, true);
    _scene->addChild(_uiRoot, 100);

    // 初始化 UI 元素
    initUI();

    _uiRoot->setCameraMask((uint16_t)CameraFlag::USER2, true);

    return true;
}

void GameCamera::initUI() {
    initBar();
    initSkillIcons();
}

void GameCamera::initBar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 血条
    auto hpBorder = Sprite::create("player/hp_border.png");
    auto size = hpBorder->getContentSize();
    hpBorder->setPosition(Vec2(size.width / 2 + 10, visibleSize.height - size.height / 2 - 10));
    _uiRoot->addChild(hpBorder);

    _hpBar = ProgressTimer::create(Sprite::create("player/hp.png"));
    _hpBar->setType(ProgressTimer::Type::BAR);
    _hpBar->setMidpoint(Vec2(0, 0.5f));
    _hpBar->setBarChangeRate(Vec2(1, 0));
    _hpBar->setPosition(hpBorder->getPosition());
    _uiRoot->addChild(_hpBar);

    // 蓝条
    auto mpBorder = Sprite::create("player/mp_border.png");
    mpBorder->setPosition(Vec2(size.width / 2 + 10, visibleSize.height - size.height * 1.5f - 20));
    _uiRoot->addChild(mpBorder);

    _mpBar = ProgressTimer::create(Sprite::create("player/mp.png"));
    _mpBar->setType(ProgressTimer::Type::BAR);
    _mpBar->setMidpoint(Vec2(0, 0.5f));
    _mpBar->setBarChangeRate(Vec2(1, 0));
    _mpBar->setPosition(mpBorder->getPosition());
    _uiRoot->addChild(_mpBar);
}

void GameCamera::initSkillIcons() {
    auto border_1 = Sprite::create("skill-icons/Border.png");
    border_1->setScale(2.0f);
    border_1->setPosition(Vec2(60, 60));
    _uiRoot->addChild(border_1);
    auto border_2 = Sprite::create("skill-icons/Border.png");
    border_2->setScale(2.0f);
    border_2->setPosition(Vec2(130, 60));
    _uiRoot->addChild(border_2);

    Sprite* ice_icon;
    Sprite* arcane_icon;
    if (_player->getSkillManager()->getSkill("IceSpear")->isUnlocked())
        ice_icon = Sprite::create("skill-icons/IceSpear.png");
    else
        ice_icon = Sprite::create("skill-icons/NULL.png");
    ice_icon->setScale(2.0f);
    ice_icon->setPosition(border_1->getPosition());
    _uiRoot->addChild(ice_icon);
    if (_player->getSkillManager()->getSkill("ArcaneJet")->isUnlocked())
        arcane_icon = Sprite::create("skill-icons/ArcaneJet.png");
    else
        arcane_icon = Sprite::create("skill-icons/NULL.png");
    arcane_icon->setScale(2.0f);
    arcane_icon->setPosition(border_2->getPosition());
    _uiRoot->addChild(arcane_icon);
    _skillCDTimer = ProgressTimer::create(Sprite::create("skill-icons/Stencil.png"));
    _skillCDTimer->setType(ProgressTimer::Type::RADIAL);
    _skillCDTimer->setReverseDirection(true);
    _skillCDTimer->setScale(2.0f);
    _skillCDTimer->setPosition(border_1->getPosition());
    _skillCDTimer->setVisible(false);
    _uiRoot->addChild(_skillCDTimer);
    _skillCDTimer_2 = ProgressTimer::create(Sprite::create("skill-icons/Stencil.png"));
    _skillCDTimer_2->setType(ProgressTimer::Type::RADIAL);
    _skillCDTimer_2->setReverseDirection(true);
    _skillCDTimer_2->setScale(2.0f);
    _skillCDTimer_2->setPosition(border_2->getPosition());
    _skillCDTimer_2->setVisible(false);
    _uiRoot->addChild(_skillCDTimer_2);
}

void GameCamera::update(float dt) {
    if (!_player || !_defaultCamera) return;
    // 相机跟随逻辑
    Vec2 targetPos = _player->getPosition();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 假设 _zoomFacto_f 是 0.5
    float viewW = visibleSize.width * _zoom_factor;
    float viewH = visibleSize.height * _zoom_factor;

    float mapWidth = _map->getMapSize().width * _map->getTileSize().width * _map->getScale();
    float mapHeight = _map->getMapSize().height * _map->getTileSize().height * _map->getScale();

    // 修正摄像机的坐标范围，防止看到地图之外的黑边
    float posX = std::max(viewW / 2, std::min(targetPos.x, mapWidth - viewW / 2));
    float posY = std::max(viewH / 2, std::min(targetPos.y, mapHeight - viewH / 2));

    posX -= visibleSize.width * _zoom_factor / 2;
    posY -= visibleSize.height * _zoom_factor / 2;

    _defaultCamera->setPosition3D(Vec3(posX, posY, 1000)); // Z值根据需要调整
    _defaultCamera->lookAt(Vec3(posX, posY, 0));

    // UI 刷新逻辑
    // 血条蓝条百分比更新
    if (_player->getHealth() >= 0)
        _hpBar->setPercentage((_player->getHealth() / _player->getMaxHealth()) * 100.0f);
    if (_player->getMagic() >= 0)
        _mpBar->setPercentage((_player->getMagic() / _player->getMaxMagic()) * 100.0f);

    // 技能冷却更新
    auto iceSkill = _player->getSkillManager()->getSkill("IceSpear");
    if (iceSkill->isUnlocked()) {
        float percent = iceSkill->getCooldownPercent() * 100;
        _skillCDTimer->setPercentage(percent);
        _skillCDTimer->setVisible(percent > 0);
    }
    auto arcaneSkill = _player->getSkillManager()->getSkill("ArcaneJet");
    if (arcaneSkill->isUnlocked()) {
        float percent = arcaneSkill->getCooldownPercent() * 100;
        _skillCDTimer_2->setPercentage(percent);
        _skillCDTimer_2->setVisible(percent > 0);
    }
}

GameCamera::GameCamera() : _hpBar(nullptr), _mpBar(nullptr), _skillCDTimer(nullptr) {}