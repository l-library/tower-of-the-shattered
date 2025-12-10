#include "Bullet.h"

USING_NS_CC;

BulletBase::BulletBase()
    : collisionBoxWidth_(20.0f)
    , collisionBoxHeight_(20.0f)
    , damage_(10)
    , categoryBitmask_(0)
    , contactTestBitmask_(0)
    , collisionBitmask_(0)
    , physicsBody_(nullptr)
    , isPlayerBullet_(false)
    , canPenetrateWall_(false)
    , canBounce_(false)
{}

BulletBase::~BulletBase()
{
    if (physicsBody_ != nullptr) {
        physicsBody_->removeFromWorld();
        physicsBody_ = nullptr;
    }
}

bool BulletBase::init()
{
    if (!Node::init()) {
        return false;
    }
    
    // 基类初始化逻辑
    return true;
}

void BulletBase::setupPhysicsBody()
{
    // 创建物理形状
    auto shape = PhysicsShapeBox::create(Size(collisionBoxWidth_, collisionBoxHeight_));
    
    // 创建物理体
    physicsBody_ = PhysicsBody::create();
    physicsBody_->addShape(shape);
    
    // 设置物理体属性
    physicsBody_->setDynamic(true);
    physicsBody_->setMass(0.01f);
    physicsBody_->setRotationEnable(false);
    
    // 设置碰撞过滤掩码
    physicsBody_->setCategoryBitmask(categoryBitmask_);
    physicsBody_->setContactTestBitmask(contactTestBitmask_);
    physicsBody_->setCollisionBitmask(collisionBitmask_);
    
    // 设置物理体的标签为当前子弹对象的指针值（作为替代方案）
    physicsBody_->setTag(reinterpret_cast<uintptr_t>(this));
    
    // 附加物理体到节点
    this->setPhysicsBody(physicsBody_);
    
    // 注册碰撞回调
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(BulletBase::onContactBegin, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(BulletBase::onContactSeparate, this);
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
}

bool BulletBase::onContactBegin(PhysicsContact& contact)
{
    // 碰撞开始回调的默认实现
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    // 从标签中获取子弹对象
    BulletBase* bulletA = reinterpret_cast<BulletBase*>(bodyA->getTag());
    BulletBase* bulletB = reinterpret_cast<BulletBase*>(bodyB->getTag());
    
    // 处理碰撞逻辑
    // ...
    
    return true;
}

bool BulletBase::onContactSeparate(PhysicsContact& contact)
{
    // 碰撞结束回调的默认实现
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    // 从标签中获取子弹对象
    BulletBase* bulletA = reinterpret_cast<BulletBase*>(bodyA->getTag());
    BulletBase* bulletB = reinterpret_cast<BulletBase*>(bodyB->getTag());
    
    // 处理碰撞结束逻辑
    // ...
    return true;
}

// ============================== MeleeBullet ==============================

bool MeleeBullet::init()
{
    if (!BulletBase::init()) {
        return false;
    }
    
    // 近战子弹默认设置
    collisionBoxWidth_ = 50.0f;
    collisionBoxHeight_ = 50.0f;
    damage_ = 20;
    duration_ = 0.2f; // 攻击持续0.2秒
    attackRange_ = 60.0f;
    timer_ = 0.0f;
    
    // 设置碰撞掩码
    categoryBitmask_ = BULLET_MASK;
    contactTestBitmask_ = ENEMY_MASK | PLAYER_MASK;
    collisionBitmask_ = 0; // 近战子弹不参与物理碰撞，仅用于检测
    
    // 创建物理碰撞箱
    setupPhysicsBody();
    
    // 安排移除自身
    this->scheduleOnce([this](float) {
        this->removeFromParent();
    }, duration_, "removeBullet");
    
    return true;
}

// ============================== RangedBullet ==============================

bool RangedBullet::init()
{
    if (!BulletBase::init()) {
        return false;
    }
    
    // 远程子弹默认设置
    collisionBoxWidth_ = 15.0f;
    collisionBoxHeight_ = 15.0f;
    damage_ = 15;
    speed_ = 300.0f;
    direction_ = Vec2(1, 0); // 默认向右
    gravityScale_ = 0.0f; // 默认不考虑重力
    sprite_ = nullptr;
    trajectoryType_ = 0; // 默认直线轨迹
    
    // 设置碰撞掩码
    categoryBitmask_ = BULLET_MASK;
    contactTestBitmask_ = ENEMY_MASK | PLAYER_MASK | WALL_MASK | BORDER_MASK;
    collisionBitmask_ = WALL_MASK | BORDER_MASK;
    
    // 创建物理碰撞箱
    setupPhysicsBody();
    
    // 设置是否受重力影响
    physicsBody_->setGravityEnable(gravityScale_ > 0.0f);
    
    // 设置初始速度
    physicsBody_->setVelocity(direction_ * speed_);
    
    // 创建默认精灵
    if (sprite_ == nullptr) {
        sprite_ = Sprite::create();
        sprite_->setTextureRect(Rect(0, 0, collisionBoxWidth_, collisionBoxHeight_));
        sprite_->setColor(Color3B(255, 0, 0)); // 默认红色
        sprite_->setPosition(Vec2(0, 0)); // 精灵相对于节点的位置设置为(0,0)
        this->addChild(sprite_);
    }
    
    // 安排更新
    this->scheduleUpdate();
    
    // 防止子弹存在时间过长
    this->scheduleOnce([this](float) {
        this->removeFromParent();
    }, 5.0f, "removeBullet");
    
    return true;
}

void RangedBullet::setSpeed(float speed)
{
    speed_ = speed;
    
    // 如果物理体存在，更新速度
    if (physicsBody_ != nullptr) {
        physicsBody_->setVelocity(direction_.getNormalized() * speed_);
    }
}

void RangedBullet::setDirection(const Vec2& direction)
{
    direction_ = direction;
    
    // 如果物理体存在，更新速度
    if (physicsBody_ != nullptr) {
        physicsBody_->setVelocity(direction_.getNormalized() * speed_);
    }
}

void RangedBullet::update(float delta)
{
    // 根据轨迹类型更新子弹
    switch (trajectoryType_) {
        case 0: // 直线
            // 已经由物理引擎处理
            break;
        case 1: // 抛物线
            // 已经由重力处理
            break;
        case 2: // 曲线
            // 可以添加更复杂的轨迹计算
            break;
        default:
            break;
    }
    
    // 如果有精灵，确保精灵与物理体同步（作为子节点，自动跟随节点移动）
    // 不需要额外设置位置，因为精灵已经是节点的子节点，且位置设为(0,0)
}