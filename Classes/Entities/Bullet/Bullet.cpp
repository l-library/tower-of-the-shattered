#include "Bullet.h"

USING_NS_CC;

Bullet::Bullet()
    : damage_(0)
    , sprite_(nullptr)
    , isVisible_(true)
    , physicsBody_(nullptr)
    , updateLogic_(nullptr)
    , contactListener_(nullptr)
    , collisionWidth_(0.0f)
    , collisionHeight_(0.0f)
    , categoryBitmask_(BULLET_MASK)
    , contactTestBitmask_(PLAYER_MASK | ENEMY_MASK | WALL_MASK)
    , collisionBitmask_(BORDER_MASK | WALL_MASK | DAMAGE_WALL_MASK)
    , existTime_(0.0f)
    , maxExistTime_(5.0f) // 默认最大存在时间为5秒
{
}

Bullet::~Bullet()
{
    // 释放资源
    if (sprite_) {
        sprite_->release();
    }
    if (physicsBody_) {
        physicsBody_->release();
    }
    if (contactListener_) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(contactListener_);
        contactListener_->release();
    }
}

Bullet* Bullet::create(const std::string& spriteFrameName, int damage, 
                      const std::function<void(Bullet*, float)>& updateLogic)
{
    Bullet* bullet = new (std::nothrow) Bullet();
    if (bullet && bullet->init(spriteFrameName, damage, updateLogic)) {
        bullet->autorelease();
        return bullet;
    }
    CC_SAFE_DELETE(bullet);
    return nullptr;
}

bool Bullet::init(const std::string& spriteFrameName, int damage, 
                 const std::function<void(Bullet*, float)>& updateLogic)
{
    if (!Node::init()) {
        return false;
    }

    // 设置伤害值
    damage_ = damage;

    // 创建精灵
    sprite_ = Sprite::create(spriteFrameName);
    if (!sprite_) {
        CCLOG("Failed to create bullet sprite with frame name: %s", spriteFrameName.c_str());
        return false;
    }
    sprite_->retain();
    sprite_->setContentSize(Size(GRID_SIZE, GRID_SIZE));
    sprite_->setPosition(Vec2::ZERO); // 设置精灵在节点中心
    this->addChild(sprite_);
    // 初始碰撞体大小设为精灵大小
    collisionWidth_ = sprite_->getContentSize().width;
    collisionHeight_ = sprite_->getContentSize().height;

    // 创建物理碰撞体
    recreatePhysicsBody();

    // 设置更新逻辑
    updateLogic_ = updateLogic;

    // 注册碰撞监听器
    registerContactListener();

    // 启动更新
    this->scheduleUpdate();

    return true;
}

void Bullet::update(float delta)
{
    Node::update(delta);

    // 更新存在时间
    existTime_ += delta;
    
    // 检查是否超过最大存在时间
    if (existTime_ >= maxExistTime_) {
        CCLOG("Bullet expired by exist time!");
        cleanupBullet();
        return;
    }

    // 执行自定义更新逻辑
    if (updateLogic_) {
        updateLogic_(this, delta);
    }
}

void Bullet::setVisible(bool visible)
{
    isVisible_ = visible;
    if (sprite_) {
        sprite_->setVisible(visible);
    }
    if (physicsBody_) {
        // 物理碰撞体不可见时禁用碰撞
        physicsBody_->setEnabled(visible);
    }
}

void Bullet::setCollisionWidth(float width)
{
    if (collisionWidth_ != width) {
        collisionWidth_ = width;
        recreatePhysicsBody();
    }
}

void Bullet::setCollisionHeight(float height)
{
    if (collisionHeight_ != height) {
        collisionHeight_ = height;
        recreatePhysicsBody();
    }
}

void Bullet::setCategoryBitmask(int bitmask)
{
    if (categoryBitmask_ != bitmask) {
        categoryBitmask_ = bitmask;
        if (physicsBody_) {
            physicsBody_->setCategoryBitmask(bitmask);
        }
    }
}

void Bullet::setContactTestBitmask(int bitmask)
{
    if (contactTestBitmask_ != bitmask) {
        contactTestBitmask_ = bitmask;
        if (physicsBody_) {
            physicsBody_->setContactTestBitmask(bitmask);
        }
    }
}

void Bullet::setCollisionBitmask(int bitmask)
{
    if (collisionBitmask_ != bitmask) {
        collisionBitmask_ = bitmask;
        if (physicsBody_) {
            physicsBody_->setCollisionBitmask(bitmask);
        }
    }
}

void Bullet::recreatePhysicsBody()
{
    // 移除旧的物理碰撞体
    if (physicsBody_) {
        this->removeComponent(physicsBody_);
        physicsBody_->release();
        physicsBody_ = nullptr;
    }

    // 创建新的物理碰撞体
    physicsBody_ = PhysicsBody::createBox(Size(collisionWidth_, collisionHeight_), 
                                         PhysicsMaterial(0.0f, 0.0f, 0.0f));
    if (!physicsBody_) {
        CCLOG("Failed to create physics body for bullet");
        return;
    }

    physicsBody_->retain();
    physicsBody_->setDynamic(true);
    physicsBody_->setGravityEnable(false);
    physicsBody_->setLinearDamping(0.0f);
    physicsBody_->setAngularDamping(0.0f);
    physicsBody_->setRotationEnable(false);

    // 设置碰撞掩码
    physicsBody_->setCategoryBitmask(categoryBitmask_);
    physicsBody_->setContactTestBitmask(contactTestBitmask_);
    physicsBody_->setCollisionBitmask(collisionBitmask_);

    // 添加物理碰撞体到节点
    this->addComponent(physicsBody_);
}

void Bullet::registerContactListener()
{
    // 创建碰撞监听器
    contactListener_ = EventListenerPhysicsContact::create();
    if (!contactListener_) {
        CCLOG("Failed to create contact listener for bullet");
        return;
    }

    // 注册碰撞开始回调
    contactListener_->onContactBegin = CC_CALLBACK_1(Bullet::onContactBegin, this);
    
    // 注册碰撞结束回调
    contactListener_->onContactSeparate = CC_CALLBACK_1(Bullet::onContactSeparate, this);

    // 添加监听器到事件分发器
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener_, this);
    contactListener_->retain();
}

bool Bullet::onContactBegin(PhysicsContact& contact)
{
    PhysicsBody* bodyA = contact.getShapeA()->getBody();
    PhysicsBody* bodyB = contact.getShapeB()->getBody();
    
    // 检查碰撞是否涉及当前子弹
    if (bodyA == physicsBody_ || bodyB == physicsBody_) {
        PhysicsBody* otherBody = (bodyA == physicsBody_) ? bodyB : bodyA;
        
        // 只有当与Player碰撞时才清理子弹
        if (otherBody->getCategoryBitmask() == PLAYER_MASK) {
            CCLOG("Bullet collided with Player!");
            cleanupBullet();
            return true;
        }
        
        // 对于其他碰撞，只记录日志不清理
        CCLOG("Bullet collided with non-Player object!");
    }

    return true;
}

bool Bullet::onContactSeparate(PhysicsContact& contact)
{
    PhysicsBody* bodyA = contact.getShapeA()->getBody();
    PhysicsBody* bodyB = contact.getShapeB()->getBody();
    
    // 检查分离是否涉及当前子弹
    if (bodyA == physicsBody_ || bodyB == physicsBody_) {
        CCLOG("Bullet separated from object!");
        return true;
    }
    
    return true;
}

void Bullet::cleanupBullet()
{
    // 停止更新
    this->unscheduleUpdate();

    // 移除碰撞监听器
    if (contactListener_) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(contactListener_);
        contactListener_->release();
        contactListener_ = nullptr;
    }

    // 移除物理碰撞体
    if (physicsBody_) {
        this->removeComponent(physicsBody_);
        physicsBody_->release();
        physicsBody_ = nullptr;
    }

    // 移除精灵
    if (sprite_) {
        this->removeChild(sprite_, true);
        sprite_->release();
        sprite_ = nullptr;
    }

    // 从父节点移除子弹
    if (this->getParent()) {
        this->getParent()->removeChild(this, true);
    }
}