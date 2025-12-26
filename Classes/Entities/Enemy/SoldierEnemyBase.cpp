#include "SoldierEnemyBase.h"




SoldierEnemyBase::SoldierEnemyBase()
    : attackRange_(0.0f)
    , movementSpeed_(0.0f)
    , attackCooldown_(0.0f)
    , attackTimer_(0.0f)
    , detectionRange_(0.0f)
    , isMovingLeft_(true)
    , idleTimer_(0.0f)
{}

SoldierEnemyBase::~SoldierEnemyBase()
{}

bool SoldierEnemyBase::init()
{
    if (!EnemyBase::init())
    {
        return false;
    }
    
    // 初始化默认参数
    attackRange_ = 100.0f;
    movementSpeed_ = 150.0f;
    attackCooldown_ = 2.0f;
    attackTimer_ = 0.0f;
    detectionRange_ = 200.0f;
    isMovingLeft_ = true;
    idleTimer_ = 0.0f;
    
    return true;
}

BehaviorResult SoldierEnemyBase::idle(float delta)
{
    // 增加闲置计时器
    idleTimer_ += delta;
    
    // 寻找玩家
    EnemyAi::findPlayer(this);
    
    // 如果玩家在检测范围内，就会被检测到并准备攻击
    // 每2秒改变一次移动方向，模拟巡逻
    if (idleTimer_ >= 2.0f)
    {
        idleTimer_ = 0.0f;
        isMovingLeft_ = !isMovingLeft_;
        
        // 更新精灵动画
        if (sprite_ != nullptr)
        {
            sprite_->stopAllActions();
            if (isMovingLeft_ && idleLeftAnimation_ != nullptr)
            {
                sprite_->runAction(RepeatForever::create(Animate::create(idleLeftAnimation_)));
            }
            else if (!isMovingLeft_ && idleRightAnimation_ != nullptr)
            {
                sprite_->runAction(RepeatForever::create(Animate::create(idleRightAnimation_)));
            }
        }
    }
    
    // 如果没有玩家或玩家不在检测范围内
    if (this->getPlayer() == nullptr || !EnemyAi::isPlayerInRange(this, detectionRange_))
    {
        if (physicsBody_ != nullptr)
        {
            float direction = isMovingLeft_ ? -1.0f : 1.0f;
            Vec2 velocity = physicsBody_->getVelocity();
            velocity.x = direction * movementSpeed_ * 0.5f; // ??????????????
            // 保持y轴速度，以便Fly类使用
            physicsBody_->setVelocity(velocity);
        }
    }
    else
    {
        // 如果玩家在检测范围内，停止移动
        if (physicsBody_ != nullptr)
        {
            Vec2 velocity = physicsBody_->getVelocity();
            velocity.x = 0.0f;
            // 保持y轴速度，以便Fly类使用
            physicsBody_->setVelocity(velocity);
        }
    }
    
    return { true, 0.0f };
}

// Getter方法
float SoldierEnemyBase::getAttackRange() const
{
    return attackRange_;
}

float SoldierEnemyBase::getMovementSpeed() const
{
    return movementSpeed_;
}

float SoldierEnemyBase::getAttackCooldown() const
{
    return attackCooldown_;
}

float SoldierEnemyBase::getAttackTimer() const
{
    return attackTimer_;
}

float SoldierEnemyBase::getDetectionRange() const
{
    return detectionRange_;
}

bool SoldierEnemyBase::getIsMovingLeft() const
{
    return isMovingLeft_;
}

float SoldierEnemyBase::getIdleTimer() const
{
    return idleTimer_;
}

// Setter方法
void SoldierEnemyBase::setAttackRange(float range)
{
    attackRange_ = range;
}

void SoldierEnemyBase::setMovementSpeed(float speed)
{
    movementSpeed_ = speed;
}

void SoldierEnemyBase::setAttackCooldown(float cooldown)
{
    attackCooldown_ = cooldown;
}

void SoldierEnemyBase::setAttackTimer(float timer)
{
    attackTimer_ = timer;
}

void SoldierEnemyBase::setDetectionRange(float range)
{
    detectionRange_ = range;
}

void SoldierEnemyBase::setIsMovingLeft(bool isMovingLeft)
{
    isMovingLeft_ = isMovingLeft;
}

void SoldierEnemyBase::setIdleTimer(float timer)
{
    idleTimer_ = timer;
}

void SoldierEnemyBase::setIdleLeftAnimation(RefPtr<Animation> animation)
{
    idleLeftAnimation_ = animation;
}

void SoldierEnemyBase::setIdleRightAnimation(RefPtr<Animation> animation)
{
    idleRightAnimation_ = animation;
}

void SoldierEnemyBase::setDeadAnimation(RefPtr<Animation> animation)
{
    deadAnimation_ = animation;
}

bool SoldierEnemyBase::onContactBegin(cocos2d::PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    // 确定哪一方是敌人
    Node* enemyNode = nullptr;
    Node* otherNode = nullptr;
    
    if (nodeA == this)
    {
        enemyNode = nodeA;
        otherNode = nodeB;
    }
    else if (nodeB == this)
    {
        enemyNode = nodeB;
        otherNode = nodeA;
    }
    
    if (enemyNode == nullptr || otherNode == nullptr)
    {
        return true;
    }
    
    // 如果碰撞到墙
    if (otherNode->getPhysicsBody()->getCategoryBitmask() == WALL_MASK)
    {
        // 允许与墙碰撞，不做特殊处理
        return true;
    }
    
    // 如果碰撞到玩家
    if (otherNode->getPhysicsBody()->getCategoryBitmask() == PLAYER_MASK)
    {
        // 给玩家施加一个力
        Vec2 direction = (otherNode->getPosition() - enemyNode->getPosition()).getNormalized();
        otherNode->getPhysicsBody()->setVelocity(direction * 100);
        
        return true;
    }
    
    // 如果碰撞到玩家子弹
    if (otherNode->getPhysicsBody()->getCategoryBitmask() == PLAYER_BULLET_MASK)
    {
        Bullet* bullet = dynamic_cast<Bullet*>(otherNode);
        if (bullet != nullptr)
        {
            Hitted(bullet->getDamage());
        }
        return true;
    }
    
    return EnemyBase::onContactBegin(contact);
}

bool SoldierEnemyBase::onContactSeparate(cocos2d::PhysicsContact& contact)
{
    return EnemyBase::onContactSeparate(contact);
}