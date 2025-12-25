#include "TowerOfTheShattered.h"
#include "Items.h"

USING_NS_CC;

Items* Items::createWithId(int itemId) {
    Items* item = new (std::nothrow) Items();
    if (item && item->init(itemId)) {
        item->autorelease();
        return item;
    }
    CC_SAFE_DELETE(item);
    return nullptr;
}

bool Items::init(int itemId) {
    // 获取物品配置
    auto config = ItemManager::getInstance()->getItemConfig(itemId);
    if (!config) {
        log("Items Error: Invalid Item ID %d", itemId);
        return false;
    }

    // 初始化 Sprite (使用图标)
    if (!Sprite::initWithFile(config->iconPath)) {
        // 如果找不到图片，使用默认图片或色块防止崩溃
        if (!Sprite::init()) return false;
        this->setTextureRect(Rect(0, 0, 32, 32));
        this->setColor(Color3B::YELLOW);
    }

    _itemId = itemId;
    _isPickedUp = false;

    // 缩放调整（如果图标太大，可以适当缩小，比如以32像素为基准）
     const float targetSize = 32.0f;
     float scale = targetSize / this->getContentSize().width;
     this->setScale(scale);

    // 初始化物理
    initPhysics();

    // 播放浮动动画
    playFloatAnimation();

    return true;
}

void Items::initPhysics() {
    // 创建圆形物理身体，半径稍微比图片小一点
    auto bodySize = this->getContentSize();
    auto body = PhysicsBody::createCircle(bodySize.width * 0.4f);

    // 设置动态，受重力影响
    body->setDynamic(true);
    body->setGravityEnable(true);
    body->setRotationEnable(false); // 掉落物通常不旋转

    //掩码设置
    body->setCategoryBitmask(ITEM_MASK);
    body->setCollisionBitmask(WALL_MASK | BORDER_MASK);
    body->setContactTestBitmask(PLAYER_MASK);

    this->setPhysicsBody(body);
}

void Items::playFloatAnimation() {
    // 简单的上下浮动动画
    auto moveUp = MoveBy::create(1.0f, Vec2(0, 10));
    auto moveDown = moveUp->reverse();
    auto seq = Sequence::create(moveUp, moveDown, nullptr);
    this->runAction(RepeatForever::create(seq));
}

void Items::bePickedUp(Player* player) {
    if (_isPickedUp) return;
    _isPickedUp = true;

    // 播放音效
    AudioManager::getInstance()->playEffect("sounds/Pickup.ogg");

    // 调用 Manager 增加物品效果
    ItemManager::getInstance()->gainItem(player, _itemId);

    // 视觉反馈：拾取动画（例如向上飘并透明消失）
    this->getPhysicsBody()->setEnabled(false); // 立即关闭物理，防止二次碰撞
    this->stopAllActions(); // 停止浮动

    auto spawn = Spawn::create(
        MoveBy::create(0.5f, Vec2(0, 40)),
        FadeOut::create(0.5f),
        ScaleTo::create(0.5f, 0.1f),
        nullptr
    );

    auto callback = CallFunc::create([this]() {
        this->removeFromParent();
        });

    this->runAction(Sequence::create(spawn, callback, nullptr));

    log("Item %d picked up!", _itemId);
}