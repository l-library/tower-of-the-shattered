#include "Mage.h"
#include "Entities/Bullet/Bullet.h"

USING_NS_CC;

Mage::Mage()
    : isCastingSpell_(false)
{
}

Mage::~Mage()
{
}

bool Mage::init()
{
    if (!SoldierEnemyBase::init())
    {
        return false;
    }
    
    // 初始化Mage的基本属性
    this->setMaxVitality(80);
    this->setCurrentVitality(80);
    this->setStaggerResistance(5); // 魔法护盾较弱，容易被打断
    this->setBaseAttackPower(15);
    this->setDefense(0);
    
    // 设置Mage的行为参数
    this->setAttackRange(250.0f);     // 魔法攻击距离较远
    this->setMovementSpeed(100.0f);   // 移动速度较慢
    this->setAttackCooldown(2.0f);    // 攻击冷却时间
    this->setDetectionRange(300.0f);  // 检测范围较大
    
    // 初始化状态
    isCastingSpell_ = false;
    
    // 创建碰撞盒信息
    CollisionBoxInfo collisionInfo;
    collisionInfo.width = GRID_SIZE*1;
    collisionInfo.height = GRID_SIZE*3;
    collisionInfo.categoryBitmask = ENEMY_MASK;
    collisionInfo.contactTestBitmask = PLAYER_MASK | WALL_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.collisionBitmask = WALL_MASK | PLAYER_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.isDynamic = true;
    collisionInfo.mass = 1.0f;
    this->setCollisionBoxInfo(collisionInfo);
    
    // 初始化物理体
    this->InitPhysicsBody();
    
    // 初始化精灵
    InitSprite();
    
    // 初始化行为
    BehaviorInit();
    
    return true;
}

void Mage::Hitted(int damage, int poise_damage)
{
    // 扣除生命值
    this->setCurrentVitality(this->getCurrentVitality() - damage);
    
    // 被击中时红一下
    if (sprite_ != nullptr)
    {
        auto redAction = TintTo::create(0.1f, 255, 0, 0);
        auto restoreAction = TintTo::create(0.2f, 255, 255, 255);
        sprite_->runAction(Sequence::create(redAction, restoreAction, nullptr));
    }
    
    // 检查是否死亡
    if (currentState_ == EnemyState::DEAD)
    {
        Dead();
    }
}

void Mage::Dead()
{
    // 设置状态为死亡
    currentState_ = EnemyState::DEAD;
    
    
    // 播放死亡动画
    if (sprite_ != nullptr && deadAnimation_ != nullptr)
    {
        sprite_->stopAllActions();
        auto deadAnimate = Animate::create(deadAnimation_);
        sprite_->runAction(deadAnimate);
    }
    
    // 移除旧物理体
    if (physicsBody_ != nullptr)
    {
        this->removeComponent(physicsBody_);
        physicsBody_ = nullptr;
    }
    

}

void Mage::BehaviorInit()
{
    // 添加行为
    this->addBehavior("idle", std::bind(&SoldierEnemyBase::idle, this, std::placeholders::_1));
    this->addBehavior("recovery", std::bind(&Mage::recovery, this, std::placeholders::_1));
    this->addBehavior("castFireball", std::bind(&Mage::castFireball, this, std::placeholders::_1));
    
    // 设置初始行为
    this->currentBehavior_ = "idle";
}

std::string Mage::DecideNextBehavior(float delta)
{
    // 寻找玩家
    EnemyAi::findPlayer(this);
    
    // 更新攻击计时器
    attackTimer_ += delta;
    
    // 如果玩家在攻击范围内且攻击冷却结束
    if (this->getPlayer() != nullptr && attackTimer_ >= attackCooldown_)
    {
        // 如果玩家在可见范围内、检测范围内
        if (EnemyAi::isPlayerVisible(this) && EnemyAi::isPlayerInRange(this, detectionRange_))
        {
            attackTimer_ = 0.0f;
            return "castFireball";
        }
    }
    
    // 否则保持待机状态
    return "idle";
}

void Mage::InitSprite()
{
    // 创建精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    
    // 创建向左移动的动画（单帧）
    Vector<SpriteFrame*> leftFrames;
    auto leftFrame = Sprite::create("Enemy/Mage/mage_left.png")->getSpriteFrame();
    if (leftFrame != nullptr)
    {
        leftFrames.pushBack(leftFrame);
    }
    if (!leftFrames.empty())
    {
        idleLeftAnimation_ = Animation::createWithSpriteFrames(leftFrames, 0.2f);
        if (idleLeftAnimation_ != nullptr)
        {
            idleLeftAnimation_->setLoops(-1); // 无限循环
        }
    }
    
    // 创建向右移动的动画（单帧）
    Vector<SpriteFrame*> rightFrames;
    auto rightFrame = Sprite::create("Enemy/Mage/mage_right.png")->getSpriteFrame();
    if (rightFrame != nullptr)
    {
        rightFrames.pushBack(rightFrame);
    }
    if (!rightFrames.empty())
    {
        idleRightAnimation_ = Animation::createWithSpriteFrames(rightFrames, 0.2f);
        if (idleRightAnimation_ != nullptr)
        {
            idleRightAnimation_->setLoops(-1); // 无限循环
        }
    }
    
    // 创建死亡动画
    Vector<SpriteFrame*> deadFrames;
    auto deadFrame = Sprite::create("Enemy/Mage/mage_dead.png")->getSpriteFrame();
    if (deadFrame != nullptr)
    {
        deadFrames.pushBack(deadFrame);
    }
    if (!deadFrames.empty())
    {
        deadAnimation_ = Animation::createWithSpriteFrames(deadFrames, 0.2f);
        if (deadAnimation_ != nullptr)
        {
            deadAnimation_->setLoops(1); // 只播放一次
        }
    }
    
    // 移除旧精灵，避免内存泄漏
    if (sprite_ != nullptr)
    {
        sprite_->removeFromParent();
        sprite_ = nullptr;
    }
    
    // 创建新精灵实例
    sprite_ = Sprite::create("Enemy/Mage/mage_left.png");
    if (sprite_ != nullptr)
    {
        // 设置精灵大小
        sprite_->setContentSize(Size(GRID_SIZE, GRID_SIZE));
        sprite_->setPosition(Vec2::ZERO); // 设置精灵相对于父节点的位置
        this->addChild(sprite_);
        
        // 播放初始的向左移动动画
        if (idleLeftAnimation_ != nullptr)
        {
            sprite_->runAction(RepeatForever::create(Animate::create(idleLeftAnimation_)));
        }
    }
}

BehaviorResult Mage::recovery(float delta)
{
    // 恢复状态持续0.5秒
    static float recoveryTimer = 0.0f;
    recoveryTimer += delta;
    
    if (recoveryTimer >= 0.5f)
    {
        recoveryTimer = 0.0f;
        return { true, 0.0f };
    }
    
    return { false, 0.0f };
}

BehaviorResult Mage::castFireball(float delta)
{
    // 如果敌人已经死亡，结束行为
    if (currentState_ == EnemyState::DEAD)
    {
        return { true, 0.0f };
    }
    
    // 如果还没开始施法
    if (!isCastingSpell_)
    {
        // 开始施法
        isCastingSpell_ = true;
        
        // 计算攻击方向
        Vec2 direction = Vec2::ZERO;
        if (this->getPlayer() != nullptr)
        {
            direction = (this->getPlayer()->getPosition() - this->getPosition()).getNormalized();
            direction.y = 0;
        }
        
        // 更新精灵朝向
        if (sprite_ != nullptr)
        {
            sprite_->stopAllActions();
            
            // 根据方向选择精灵
            if (direction.x < 0) // 向左
            {
                sprite_->setTexture("Enemy/Mage/mage_left.png");
            }
            else // 向右
            {
                sprite_->setTexture("Enemy/Mage/mage_right.png");
            }
        }
        
        // 停止移动
        if (physicsBody_ != nullptr)
        {
            Vec2 velocity = physicsBody_->getVelocity();
            velocity.x = 0.0f;
            physicsBody_->setVelocity(velocity);
        }
        
        // 创建并发射火球
        auto fireball = Bullet::create("Enemy/Mage/magebullet.png", this->getBaseAttackPower(), nullptr);
        
        if (fireball)
        {
            // 设置火球属性
            fireball->setCollisionWidth(GRID_SIZE * 0.3f);
            fireball->setCollisionHeight(GRID_SIZE * 0.3f);
            fireball->setPosition(this->getPosition() + Vec2(direction.x * 20, 0));
            fireball->setCategoryBitmask(ENEMY_BULLET_MASK);
            fireball->setContactTestBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
            fireball->setCollisionBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
            fireball->setCLearBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
            fireball->setDamage(15);
            fireball->setMaxExistTime(3.0f);
            
            // 设置火球速度（直线飞行）
            auto fireballBody = fireball->getPhysicsBody();
            if (fireballBody)
            {
                fireballBody->setGravityEnable(false);
                fireballBody->setVelocity(direction * 300.0f); // 火球速度较快
            }
            
            // 添加到场景
            this->getParent()->addChild(fireball);
        }
        
        // 重置施法状态
        isCastingSpell_ = false;
        
        // 结束攻击行为，进入恢复状态
        return { true, 2.0f };
    }
    
    return { false, 0.0f };
}

bool Mage::onContactBegin(PhysicsContact& contact)
{
    // 调用父类的碰撞处理逻辑
    return SoldierEnemyBase::onContactBegin(contact);
}

bool Mage::onContactSeparate(PhysicsContact& contact)
{
    return SoldierEnemyBase::onContactSeparate(contact);
}