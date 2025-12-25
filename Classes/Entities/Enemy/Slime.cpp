#include "Slime.h"
#include "SoldierEnemyBase.h"
#include "TowerOfTheShattered.h"
#include "cocos2d.h"
#include "Entities/Bullet/Bullet.h"

using namespace cocos2d;

bool Slime::init()
{
    if (!SoldierEnemyBase::init())
    {
        return false;
    }
    
    // 初始化Slime的基本属性
    this->setMaxVitality(100);
    this->setCurrentVitality(100);
    this->setStaggerResistance(INT_MAX); // 史莱姆不会被打断
    this->setBaseAttackPower(10);
    this->setDefense(0);
    
    // 设置Slime的行为参数
    this->setAttackRange(100.0f);
    this->setMovementSpeed(150.0f);
    this->setAttackCooldown(2.0f);
    this->setDetectionRange(200.0f);
    jumpSpeed_ = 300.0f;
    
    // 初始化行为??
    isJumping_ = false;
    isCharging_ = false;
    isJumpAttackCollided_ = false;
    isChargeAttackCollided_ = false;
    
    // 创建碰撞盒信息，设置为32x32的大小
    CollisionBoxInfo collisionInfo;
    collisionInfo.width = GRID_SIZE;
    collisionInfo.height = GRID_SIZE;
    collisionInfo.categoryBitmask = ENEMY_MASK;
    collisionInfo.contactTestBitmask = PLAYER_MASK | WALL_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.collisionBitmask = WALL_MASK | PLAYER_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.isDynamic = true;
    collisionInfo.mass = 1.0f;
    this->setCollisionBoxInfo(collisionInfo);
    
    // 初始化物理体
    this->InitPhysicsBody();
    
    // 减少生命值
    this->BehaviorInit();
    
    return true;
}

void Slime::Hitted(int damage, int poise_damage)
{
    // 寻找玩家?
    this->setCurrentVitality(this->getCurrentVitality() - damage);
    
    // 播放受伤动画
    if (sprite_ != nullptr)
    {
        auto redAction = TintTo::create(0.1f, 255, 0, 0);
        auto restoreAction = TintTo::create(0.2f, 255, 255, 255);
        sprite_->runAction(Sequence::create(redAction, restoreAction, nullptr));
    }
    
    // 这里可以添加硬直逻辑
}

void Slime::Dead()
{
    // 处理死亡效果
    if (sprite_ != nullptr)
    {
        sprite_->stopAllActions();
        sprite_->setColor(Color3B::WHITE); // 恢复颜色
        
        // 如果有死亡动画，播放死亡动画然后淡出
        if (deadAnimation_ != nullptr)
        {
            auto deathAction = Animate::create(deadAnimation_);
            if (deathAction != nullptr)
            {
                auto fadeOut = FadeOut::create(1.0f);
                if (fadeOut != nullptr)
                {
                    auto sequence = Sequence::create(deathAction, fadeOut, nullptr);
                    if (sequence != nullptr)
                    {
                        sprite_->runAction(sequence);
                    }
                }
            }
        }
        else
        {
            // 如果没有死亡动画，直接淡出
            auto fadeOut = FadeOut::create(1.0f);
            if (fadeOut != nullptr)
            {
                sprite_->runAction(fadeOut);
            }
        }
    }
    
    // 移除物理体，防止继续交互
    if (physicsBody_ != nullptr)
    {
        this->removeComponent(physicsBody_);
        physicsBody_ = nullptr;
    }
}

void Slime::BehaviorInit()
{
    // 添加行为
    this->addBehavior("idle", std::bind(&SoldierEnemyBase::idle, this, std::placeholders::_1));
    this->addBehavior("recovery", std::bind(&Slime::recovery, this, std::placeholders::_1));
    this->addBehavior("jumpAttack", std::bind(&Slime::jumpAttack, this, std::placeholders::_1));
    this->addBehavior("chargeAttack", std::bind(&Slime::chargeAttack, this, std::placeholders::_1));
    
    // 设置初始行为
    this->currentBehavior_ = "idle";
}

std::string Slime::DecideNextBehavior(float delta)
{
    // ???????
    EnemyAi::findPlayer(this);
    
    // 更新攻击计时器
    attackTimer_ += delta;
    
    // 如果玩家在攻击范围内且攻击冷却结束
    if (this->getPlayer() != nullptr && attackTimer_ >= attackCooldown_)
    {
        // 如果玩家在可见范围内、检测范围内且在水平方向
        if (EnemyAi::isPlayerVisible(this) && EnemyAi::isPlayerInRange(this, detectionRange_) && EnemyAi::isPlayerHorizontal(this, GRID_SIZE * 2))
        {
            // 随机选择跳跃攻击或冲锋攻击
            int random = rand() % 2;
            attackTimer_ = 0.0f;
            
            if (random == 0)
            {
                return "jumpAttack";
            }
            else
            {
                return "chargeAttack";
            }
        }
    }
    
    // 否则保持待机状态
    return "idle";
}

void Slime::InitSprite()
{
    // 加载精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile("Enemy/Slime/slime_walk_left.plist", "Enemy/Slime/slime_walk_left.png");
    cache->addSpriteFramesWithFile("Enemy/Slime/slime_walk_right.plist", "Enemy/Slime/slime_walk_right.png");
    cache->addSpriteFramesWithFile("Enemy/Slime/slime_dead.plist", "Enemy/Slime/slime_dead.png");
    
    // 创建向左移动的动画
    Vector<SpriteFrame*> leftFrames;
    for (int i = 0; i < 2; i++)
    {
        char frameName[32];
        sprintf(frameName, "slime_walk_left%04d", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            leftFrames.pushBack(frame);
        }
    }
    if (!leftFrames.empty())
    {
        idleLeftAnimation_ = Animation::createWithSpriteFrames(leftFrames, 0.2f);
        if (idleLeftAnimation_ != nullptr)
        {
            idleLeftAnimation_->setLoops(-1); // 无限循环
        }
    }
    
    // 创建向右移动的动画
    Vector<SpriteFrame*> rightFrames;
    for (int i = 0; i < 2; i++)
    {
        char frameName[32];
        sprintf(frameName, "slime_walk_right%04d", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            rightFrames.pushBack(frame);
        }
        else
        {
            log("Failed to find sprite frame: %s", frameName);
        }
    }
    log("Right frames count: %d", rightFrames.size());
    if (!rightFrames.empty())
    {
        idleRightAnimation_ = Animation::createWithSpriteFrames(rightFrames, 0.2f);
        if (idleRightAnimation_ != nullptr)
        {
            idleRightAnimation_->setLoops(-1); // 无限循环
            log("Created idleRightAnimation_ successfully");
        }
        else
        {
            log("Failed to create idleRightAnimation_");
        }
    }
    
    // 创建死亡动画
    Vector<SpriteFrame*> deadFrames;
    for (int i = 0; i < 2; i++)
    {
        char frameName[32];
        sprintf(frameName, "slime_dead%04d", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            deadFrames.pushBack(frame);
        }
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
    sprite_ = Sprite::createWithSpriteFrame(cache->getSpriteFrameByName("slime_walk_left0000"));
    if (sprite_ != nullptr)
    {
        // 设置精灵大小为32x32像素
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



BehaviorResult Slime::recovery(float delta)
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

BehaviorResult Slime::jumpAttack(float delta)
{
    // 如果敌人已经死亡，结束行为
    if (currentState_ == EnemyState::DEAD)
    {
        return { true, 0.0f };
    }
    
    // 如果还没开始跳跃
    if (!isJumping_)
    {
        // 开始跳跃
        isJumping_ = true;
        // 重置碰撞标志
        isJumpAttackCollided_ = false;
        
        // 计算跳跃方向
        Vec2 direction = Vec2::ZERO;
        if (this->getPlayer() != nullptr)
        {
            direction = (this->getPlayer()->getPosition() - this->getPosition()).getNormalized();
        }
        
        // 更新精灵动画和颜色
        if (sprite_ != nullptr)
        {
            sprite_->stopAllActions();
            
            // 根据方向选择动画
            if (direction.x < 0 && idleLeftAnimation_ != nullptr) // 向左
            {
                auto animate = Animate::create(idleLeftAnimation_);
                if (animate != nullptr)
                {
                    sprite_->runAction(RepeatForever::create(animate));
                }
            }
            else if (direction.x >= 0 && idleRightAnimation_ != nullptr) // 向右
            {
                auto animate = Animate::create(idleRightAnimation_);
                if (animate != nullptr)
                {
                    sprite_->runAction(RepeatForever::create(animate));
                }
            }
            
            // 如果还没开始冲锋??
            auto tintAction = TintTo::create(0.1f, 255, 0, 0);
            if (tintAction != nullptr)
            {
                sprite_->runAction(tintAction);
            }
        }
        
        // 设置跳跃速度
        if (physicsBody_ != nullptr)
        {
            Vec2 jumpVelocity = Vec2(direction.x * movementSpeed_, jumpSpeed_);
            physicsBody_->setVelocity(jumpVelocity);
        }
        
        // 创建跳跃攻击的碰撞区域，比Slime大1.2倍
        auto jumpBullet = Bullet::create("", this->getBaseAttackPower(), [this](Bullet* bullet, float delta) {
            // 子弹更新回调
            if (this->currentState_ == EnemyState::DEAD || this->getParent() == nullptr)
            {
                bullet->cleanupBullet();
                return;
            }
            bullet->setPosition(this->getPosition());  // 同步位置
        });
        if (jumpBullet)
        {
            jumpBullet->setCollisionWidth(GRID_SIZE * 1.2f);
            jumpBullet->setCollisionHeight(GRID_SIZE * 1.2f);
            jumpBullet->setPosition(this->getPosition());
            jumpBullet->setCategoryBitmask(ENEMY_BULLET_MASK);
            jumpBullet->setContactTestBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
            jumpBullet->setCollisionBitmask(WALL_MASK | PLAYER_MASK | BORDER_MASK);
            jumpBullet->setCLearBitmask(PLAYER_MASK );
            jumpBullet->setDamage(50);
            this->getParent()->addChild(jumpBullet);
        }
    }
    
    // 如果跳跃结束或发生碰撞
    if (isJumping_ && physicsBody_ != nullptr && (physicsBody_->getVelocity().y == 0 || isJumpAttackCollided_))
    {
        isJumping_ = false;
        
        // 恢复精灵颜色
        if (sprite_ != nullptr)
        {
            sprite_->runAction(TintTo::create(0.1f, 255, 255, 255));
        }
        
        return { true, 2.0f }; // 完成跳跃攻击，冷却2秒
    }
    
    return { false, 0.0f };
}

BehaviorResult Slime::chargeAttack(float delta)
{
    // 如果敌人已经死亡，结束行为
    if (currentState_ == EnemyState::DEAD)
    {
        return { true, 0.0f };
    }
    
    if (!isCharging_)
    {
        // 开始冲锋
        isCharging_ = true;
        // 重置碰撞标志
        isChargeAttackCollided_ = false;
        
        // 计算冲锋方向
        Vec2 direction = Vec2::ZERO;
        if (this->getPlayer() != nullptr)
        {
            direction = (this->getPlayer()->getPosition() - this->getPosition()).getNormalized();
        }
        
        // 更新精灵动画和颜色
        if (sprite_ != nullptr)
        {
            sprite_->stopAllActions();
            
            // 根据方向选择动画
            if (direction.x < 0 && idleLeftAnimation_ != nullptr) // 向左
            {
                auto animate = Animate::create(idleLeftAnimation_);
                if (animate != nullptr)
                {
                    sprite_->runAction(RepeatForever::create(animate));
                }
            }
            else if (direction.x >= 0 && idleRightAnimation_ != nullptr) // 向右
            {
                auto animate = Animate::create(idleRightAnimation_);
                if (animate != nullptr)
                {
                    sprite_->runAction(RepeatForever::create(animate));
                }
            }
            
            // 停止冲锋
            auto tintAction = TintTo::create(0.1f, 255, 0, 0);
            if (tintAction != nullptr)
            {
                sprite_->runAction(tintAction);
            }
        }
        
        // 设置冲锋速度
        if (physicsBody_ != nullptr)
        {
            Vec2 chargeVelocity = Vec2(direction.x * movementSpeed_ * 2, 0);
            physicsBody_->setVelocity(chargeVelocity);
        }
        
        // 创建冲锋攻击的碰撞区域，比Slime大1.2倍
        auto chargeBullet = Bullet::create("", this->getBaseAttackPower(), [this](Bullet* bullet, float delta) {
            // 子弹更新回调
            if (this->currentState_ == EnemyState::DEAD || this->getParent() == nullptr)
            {
                bullet->cleanupBullet();
                return;
            }
            bullet->setPosition(this->getPosition());  // 同步位置
        });
        if (chargeBullet)
        {
            chargeBullet->setCollisionWidth(GRID_SIZE * 1.2f);
            chargeBullet->setCollisionHeight(GRID_SIZE * 1.2f);
            chargeBullet->setPosition(this->getPosition());
            chargeBullet->setCategoryBitmask(ENEMY_BULLET_MASK);
            chargeBullet->setContactTestBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
            chargeBullet->setCollisionBitmask(WALL_MASK | PLAYER_MASK | BORDER_MASK);
            chargeBullet->setCLearBitmask(PLAYER_MASK );
            chargeBullet->setDamage(50);
            this->getParent()->addChild(chargeBullet);
        }
    }
    
    // 控制冲锋持续时间
    static float chargeTimer = 0.0f;
    chargeTimer += delta;
    
    if (chargeTimer >= 1.0f)
    {
        chargeTimer = 0.0f;
        isCharging_ = false;
        
        if (physicsBody_ != nullptr)
        {
            physicsBody_->setVelocity(Vec2::ZERO);
        }
        
        // 恢复精灵颜色
        if (sprite_ != nullptr)
        {
            sprite_->runAction(TintTo::create(0.1f, 255, 255, 255));
        }
        return { true, 2.0f }; // 完成冲锋攻击，冷却2秒
    }

    return { false, 0.0f };
}



bool Slime::onContactBegin(PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    // 确定哪一方是Slime
    Node* slimeNode = nullptr;
    Node* otherNode = nullptr;
    
    if (nodeA == this)
    {
        slimeNode = nodeA;
        otherNode = nodeB;
    }
    else if (nodeB == this)
    {
        slimeNode = nodeB;
        otherNode = nodeA;
    }
    
    if (slimeNode == nullptr || otherNode == nullptr)
    {
        return true;
    }
    
    // Slime特有的碰撞逻辑：跳跃和冲锋攻击的碰撞检测
    if (isJumping_ || isCharging_)
    {
        // 如果碰撞到其他物体
        if (otherNode != this)
        {
            // 设置碰撞标志
            if (isJumping_)
            {
                isJumpAttackCollided_ = true;
            }
            if (isCharging_)
            {
                isChargeAttackCollided_ = true;
            }
        }
    }
    
    // 调用父类的碰撞处理逻辑
    return SoldierEnemyBase::onContactBegin(contact);
}

bool Slime::onContactSeparate(PhysicsContact& contact)
{
    return SoldierEnemyBase::onContactSeparate(contact);
}
