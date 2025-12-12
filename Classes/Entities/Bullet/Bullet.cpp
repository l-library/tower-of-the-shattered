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
    
    // �����ʼ���߼�
    return true;
}

void BulletBase::setupPhysicsBody()
{
    // 创建物理形状
    auto shape = PhysicsShapeBox::create(Size(collisionBoxWidth_, collisionBoxHeight_));
    
    // 创建物理体
    physicsBody_ = PhysicsBody::create();
    physicsBody_->addShape(shape);
    
    // 设置物理属性
    physicsBody_->setDynamic(true);
    physicsBody_->setMass(0.01f);
    physicsBody_->setRotationEnable(false);
    
    // 根据子弹类型设置分类掩码
    if (isPlayerBullet_) {
        categoryBitmask_ = PLAYER_BULLET_MASK;
        // 玩家子弹应该与敌人和敌人的墙碰撞
        contactTestBitmask_ |= ENEMY_MASK | DAMAGE_WALL_MASK;
    } else {
        categoryBitmask_ = ENEMY_BULLET_MASK;
        // 敌人子弹应该与玩家和玩家的墙碰撞
        contactTestBitmask_ |= PLAYER_MASK;
    }
    
    // 设置碰撞掩码
    physicsBody_->setCategoryBitmask(categoryBitmask_);
    physicsBody_->setContactTestBitmask(contactTestBitmask_);
    physicsBody_->setCollisionBitmask(collisionBitmask_);
    
    // 将物理体的标签设置为当前子弹对象的指针值，以便于识别
    physicsBody_->setTag(reinterpret_cast<uintptr_t>(this));
    
    // 将物理体附加到节点
    this->setPhysicsBody(physicsBody_);
    
    // 注册碰撞回调
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(BulletBase::onContactBegin, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(BulletBase::onContactSeparate, this);
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
}

bool BulletBase::onContactBegin(PhysicsContact& contact)
{
    // 碰撞开始回调函数实现
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    // 从标签获取子弹对象
    BulletBase* bulletA = reinterpret_cast<BulletBase*>(bodyA->getTag());
    BulletBase* bulletB = reinterpret_cast<BulletBase*>(bodyB->getTag());
    
    // 确定当前子弹对象
    BulletBase* currentBullet = (this == bulletA) ? bulletA : (this == bulletB) ? bulletB : nullptr;
    if (currentBullet == nullptr) {
        return true;
    }
    
    // 获取碰撞的另一个对象
    PhysicsBody* otherBody = (this == bulletA) ? bodyB : bodyA;
    Node* otherNode = otherBody->getNode();
    
    // 处理与墙的碰撞
    if (otherBody->getCategoryBitmask() & WALL_MASK) {
        if (!currentBullet->getCanPenetrateWall()) {
            // 如果子弹不能穿墙，则移除子弹
            currentBullet->removeFromParent();
        }
    }
    
    // 处理与边界的碰撞
    if (otherBody->getCategoryBitmask() & BORDER_MASK) {
        currentBullet->removeFromParent();
    }
    
    // 处理与玩家的碰撞
    if (otherBody->getCategoryBitmask() & PLAYER_MASK) {
        // 玩家受到伤害的逻辑应该在玩家类中处理
    }
    
    // 处理与敌人的碰撞
    if (otherBody->getCategoryBitmask() & ENEMY_MASK) {
        // 敌人受到伤害的逻辑应该在敌人类中处理
    }
    
    return true;
}

bool BulletBase::onContactSeparate(PhysicsContact& contact)
{
    // 碰撞结束回调函数实现
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    // 从标签获取子弹对象
    BulletBase* bulletA = reinterpret_cast<BulletBase*>(bodyA->getTag());
    BulletBase* bulletB = reinterpret_cast<BulletBase*>(bodyB->getTag());
    
    // 确定当前子弹对象
    BulletBase* currentBullet = (this == bulletA) ? bulletA : (this == bulletB) ? bulletB : nullptr;
    if (currentBullet == nullptr) {
        return true;
    }
    
    // 获取碰撞的另一个对象
    PhysicsBody* otherBody = (this == bulletA) ? bodyB : bodyA;
    
    // 处理反弹逻辑
    if (currentBullet->getCanBounce() && (otherBody->getCategoryBitmask() & (WALL_MASK | BORDER_MASK))) {
        // 计算反弹方向
        Vec2 velocity = currentBullet->getPhysicsBody()->getVelocity();
        Vec2 normal = contact.getContactData()->normal;
        Vec2 reflectedVelocity = velocity - 2 * velocity.dot(normal) * normal;
        
        // 设置反弹后的速度
        currentBullet->getPhysicsBody()->setVelocity(reflectedVelocity);
    }
    
    return true;
}

// ============================== MeleeBullet ==============================

bool MeleeBullet::init()
{
    if (!BulletBase::init()) {
        return false;
    }
    
    // ��ս�ӵ�Ĭ������
    collisionBoxWidth_ = 50.0f;
    collisionBoxHeight_ = 50.0f;
    damage_ = 20;
    duration_ = 0.2f; // ��������0.2��
    attackRange_ = 60.0f;
    timer_ = 0.0f;
    
    // 设置碰撞掩码
    contactTestBitmask_ = ENEMY_MASK | PLAYER_MASK;
    collisionBitmask_ = 0; // 近战bullet不需要物理碰撞，只用于检测
    
    // ����������ײ��
    setupPhysicsBody();
    
    // �����Ƴ�����
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
    
    // Զ���ӵ�Ĭ������
    collisionBoxWidth_ = 15.0f;
    collisionBoxHeight_ = 15.0f;
    damage_ = 15;
    speed_ = 300.0f;
    direction_ = Vec2(1, 0); // Ĭ������
    gravityScale_ = 0.0f; // Ĭ�ϲ���������
    sprite_ = nullptr;
    trajectoryType_ = 0; // Ĭ��ֱ�߹켣
    
    // 设置碰撞掩码
    contactTestBitmask_ = ENEMY_MASK | PLAYER_MASK | WALL_MASK | BORDER_MASK;
    collisionBitmask_ = WALL_MASK | BORDER_MASK;
    
    // ����������ײ��
    setupPhysicsBody();
    
    // �����Ƿ�������Ӱ��
    physicsBody_->setGravityEnable(gravityScale_ > 0.0f);
    
    // ���ó�ʼ�ٶ�
    physicsBody_->setVelocity(direction_ * speed_);
    
    // ����Ĭ�Ͼ���
    if (sprite_ == nullptr) {
        sprite_ = Sprite::create();
        sprite_->setTextureRect(Rect(0, 0, collisionBoxWidth_, collisionBoxHeight_));
        sprite_->setColor(Color3B(255, 0, 0)); // Ĭ�Ϻ�ɫ
        sprite_->setPosition(Vec2(0, 0)); // ��������ڽڵ��λ������Ϊ(0,0)
        this->addChild(sprite_);
    }
    
    // ���Ÿ���
    this->scheduleUpdate();
    
    // ��ֹ�ӵ�����ʱ�����
    this->scheduleOnce([this](float) {
        this->removeFromParent();
    }, 5.0f, "removeBullet");
    
    return true;
}

void RangedBullet::setSpeed(float speed)
{
    speed_ = speed;
    
    // �����������ڣ������ٶ�
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

Sprite* RangedBullet::getSprite() const { return sprite_; }

void RangedBullet::setSprite(Sprite* sprite)
{
    // 移除旧的sprite
    if (sprite_ != nullptr) {
        sprite_->removeFromParent();
    }
    
    // 设置新的sprite
    sprite_ = sprite;
    
    // 添加新的sprite到节点
    if (sprite_ != nullptr) {
        sprite_->setPosition(Vec2(0, 0)); // 设置在节点中心
        this->addChild(sprite_);
    }
}

void RangedBullet::update(float delta)
{
    // ���ݹ켣���͸����ӵ�
    switch (trajectoryType_) {
        case 0: // ֱ��
            // �Ѿ����������洦��
            break;
        case 1: // ������
            // �Ѿ�����������
            break;
        case 2: // ����
            // �������Ӹ����ӵĹ켣����
            break;
        default:
            break;
    }
    
    // ����о��飬ȷ��������������ͬ������Ϊ�ӽڵ㣬�Զ�����ڵ��ƶ���
    // ����Ҫ��������λ�ã���Ϊ�����Ѿ��ǽڵ���ӽڵ㣬��λ����Ϊ(0,0)
}