#include "GameCamera.h"
#include "../Entities/Items/ItemManager.h"
#include <string>

USING_NS_CC;

GameCamera::GameCamera()
    : _hpBar(nullptr)
    , _mpBar(nullptr)
    , _hpLabel(nullptr)
    , _mpLabel(nullptr)
    , _goldLabel(nullptr)
    , _zoomFactor(0.7f)
{
}

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
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 获取默认摄像机
    _defaultCamera = scene->getDefaultCamera();
    _defaultCamera->setDepth(0);
    _defaultCamera->initOrthographic(visibleSize.width * _zoomFactor, visibleSize.height * _zoomFactor, 1, 1000);
    _defaultCamera->setCameraFlag(CameraFlag::DEFAULT);

    // UI 摄像机
    _uiCamera = Camera::createOrthographic(visibleSize.width, visibleSize.height, 1, 1100);
    _uiCamera->setCameraFlag(CameraFlag::USER2);
    _uiCamera->setDepth(1);
    _uiCamera->setPosition3D(Vec3(0, 0, 1100));
    _uiCamera->lookAt(Vec3(0, 0, 0));
    _scene->addChild(_uiCamera);

    // UI 根节点
    _uiRoot = Node::create();
    _uiRoot->setContentSize(visibleSize);
    _uiRoot->setCameraMask((uint16_t)CameraFlag::USER2, true);
    _scene->addChild(_uiRoot, kUIZorder);

    initUI();

    _uiRoot->setCameraMask((uint16_t)CameraFlag::USER2, true);

    return true;
}

void GameCamera::initUI() {
    initBar();
    initSkillIcons();
    initGold();
    initItemIcons();
}

// 通用的状态条创建函数
ProgressTimer* GameCamera::createStatusBar(const std::string& borderPath, const std::string& barPath, const Vec2& position, Label*& outLabel) {
    auto border = Sprite::create(borderPath);
    if (!border) return nullptr;

    border->setPosition(position);
    _uiRoot->addChild(border, kUIZorder);

    auto bar = ProgressTimer::create(Sprite::create(barPath));
    bar->setType(ProgressTimer::Type::BAR);
    bar->setMidpoint(Vec2(0, 0.5f));
    bar->setBarChangeRate(Vec2(1, 0));
    bar->setPosition(position);
    _uiRoot->addChild(bar, kUIZorder);

    // 初始化 Label
    outLabel = Label::createWithTTF("0 / 0", "fonts/Marker Felt.ttf", 24);
    if (outLabel) {
        outLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
        outLabel->setPosition(position);
        _uiRoot->addChild(outLabel, kUIZorder+1);
    }

    return bar;
}

void GameCamera::initBar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 预计算位置
    auto tempSprite = Sprite::create("player/hp_border.png");
    Size borderSize = tempSprite ? tempSprite->getContentSize() : Size(200, 30);

    Vec2 hpPos(borderSize.width / 2 + 10, visibleSize.height - borderSize.height / 2 - 10);
    Vec2 mpPos(borderSize.width / 2 + 10, visibleSize.height - borderSize.height * 1.5f - 20);

    // 创建 HP 条
    _hpBar = createStatusBar("player/hp_border.png", "player/hp.png", hpPos, _hpLabel);

    // 创建 MP 条
    _mpBar = createStatusBar("player/mp_border.png", "player/mp.png", mpPos, _mpLabel);
}


void GameCamera::createSkillSlot(const std::string& skillName, const std::string& iconPath, const Vec2& position) {
    // 边框
    auto border = Sprite::create("skill-icons/Border.png");
    border->setScale(2.0f);
    border->setPosition(position);
    _uiRoot->addChild(border, kUIZorder);

    // 技能图标
    std::string finalIconPath = "skill-icons/NULL.png";
    if (_player->getSkillManager()->getSkill(skillName)->isUnlocked()) {
        finalIconPath = iconPath;
    }

    auto icon = Sprite::create(finalIconPath);
    if (icon) {
        icon->setScale(2.0f);
        icon->setPosition(position);
        _uiRoot->addChild(icon, kUIZorder);
    }

    // 冷却遮罩 (Timer)
    auto cdTimer = ProgressTimer::create(Sprite::create("skill-icons/Stencil.png"));
    cdTimer->setType(ProgressTimer::Type::RADIAL);
    cdTimer->setReverseDirection(true);
    cdTimer->setScale(2.0f);
    cdTimer->setPosition(position);
    cdTimer->setVisible(false);
    _uiRoot->addChild(cdTimer, kUIZorder+1);

    // 存入 Map 以便 update 使用
    _skillCDTimers[skillName] = cdTimer;
}

void GameCamera::initSkillIcons() {
    // 定义技能配置结构
    struct SkillConfigUI {
        std::string name;
        std::string icon;
        float xOffset;
    };

    // 技能列表
    std::vector<SkillConfigUI> skills = {
        {"IceSpear",     "skill-icons/IceSpear.png",     60.0f},
        {"ArcaneJet",    "skill-icons/ArcaneJet.png",    130.0f},
        {"ArcaneShield", "skill-icons/ArcaneShield.png", 200.0f}
    };

    float yPos = 60.0f;

    for (const auto& config : skills) {
        createSkillSlot(config.name, config.icon, Vec2(config.xOffset, yPos));
    }
}

void GameCamera::initGold() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto gold = Sprite::create("items/gold.png");
    if (!gold) return;

    auto position = Vec2(visibleSize.width - 100, visibleSize.height - gold->getContentSize().height);
    gold->setPosition(position);
    gold->setScale(2.0f);
    _uiRoot->addChild(gold);

    std::string goldStr = std::to_string(ItemManager::getInstance()->getGold());
    _goldLabel = Label::createWithTTF(goldStr, "fonts/Marker Felt.ttf", 24);

    if (_goldLabel) {
        auto labelPos = Vec2(position.x + gold->getContentSize().width + _goldLabel->getContentSize().width, position.y);
        _goldLabel->setPosition(labelPos);
        _uiRoot->addChild(_goldLabel, kUIZorder+1);
    }
}

void GameCamera::initItemIcons()
{
    _nextIconPosition = Vec2(_itemSize / 2, _mpBar->getPosition().y - _mpBar->getContentSize().height);
}

void GameCamera::update(float dt) {
    if (!_player || !_defaultCamera) return;

    // --- 相机跟随逻辑 ---
    Vec2 targetPos = _player->getPosition();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    float viewW = visibleSize.width * _zoomFactor;
    float viewH = visibleSize.height * _zoomFactor;

    float mapWidth = _map->getMapSize().width * _map->getTileSize().width * _map->getScale();
    float mapHeight = _map->getMapSize().height * _map->getTileSize().height * _map->getScale();

    float posX = std::max(viewW / 2, std::min(targetPos.x, mapWidth - viewW / 2));
    float posY = std::max(viewH / 2, std::min(targetPos.y, mapHeight - viewH / 2));

    posX -= visibleSize.width * _zoomFactor / 2;
    posY -= visibleSize.height * _zoomFactor / 2;

    _defaultCamera->setPosition3D(Vec3(posX, posY, 1000));
    _defaultCamera->lookAt(Vec3(posX, posY, 0));

    // UI 刷新逻辑

    // 刷新HP/MP
    auto updateBarInfo = [](float current, float max, ProgressTimer* bar, Label* label) {
        if (max > 0) {
            float percent = (current / max) * 100.0f;
            if (current >= 0 && bar) bar->setPercentage(percent);
            if (label) label->setString(StringUtils::format("%.2f / %.2f", current, max));
        }
        };

    updateBarInfo(_player->getHealth(), _player->getMaxHealth(), _hpBar, _hpLabel);
    updateBarInfo(_player->getMagic(), _player->getMaxMagic(), _mpBar, _mpLabel);

    // 刷新技能冷却
    // 遍历所有注册的技能 UI
    for (auto& pair : _skillCDTimers) {
        std::string skillName = pair.first;
        ProgressTimer* timer = pair.second;

        auto skill = _player->getSkillManager()->getSkill(skillName);
        if (skill && skill->isUnlocked() && timer) {
            float percent = skill->getCooldownPercent() * 100.0f;
            timer->setPercentage(percent);
            timer->setVisible(percent > 0);
        }
    }

    // 刷新金币
    if (_goldLabel) {
        _goldLabel->setString(std::to_string(ItemManager::getInstance()->getGold()));
    }

    // 刷新物品UI
    // 获取玩家所有的物品列表
    if (ItemManager::getInstance()->hasNewItems(_player)) {
        std::vector<int> Items = ItemManager::getInstance()->getOwnedItems();
        int item = Items.back();
        auto itemIcon = ItemManager::getInstance()->createItemIcon(item);
        itemIcon->setCameraMask((unsigned short)CameraFlag::USER2);
        float scale = _itemSize / itemIcon->getContentSize().width;
        itemIcon->setScale(scale);
        itemIcon->setPosition(_nextIconPosition);
        _nextIconPosition.x += _itemSize + 5.0f;
        _uiRoot->addChild(itemIcon,kUIZorder);
    }
}
