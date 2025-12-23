#include "Slime.h"
#include "TowerOfTheShattered.h"
#include "cocos2d.h"
#include "Entities/Bullet/Bullet.h"

using namespace cocos2d;

bool Slime::init()
{
    if (!EnemyBase::init())
    {
        return false;
    }
    
    // 设置Slime的基本属性
    this->setMaxVitality(100);
    this->setCurrentVitality(100);
    this->setStaggerResistance(INT_MAX); // 韧性为无限大
    this->setBaseAttackPower(10);
    this->setDefense(0);
    
    // 设置Slime特有的属性
    attackRange_ = 100.0f;
    movementSpeed_ = 150.0f;
    jumpSpeed_ = 300.0f;
    attackCooldown_ = 2.0f;
    attackTimer_ = 0.0f;
    detectionRange_ = 200.0f;
    
    // 初始化行为状态
    isJumping_ = false;
    isCharging_ = false;
    isJumpAttackCollided_ = false;
    isChargeAttackCollided_ = false;
    
    // 初始化动画相关属性
    isMovingLeft_ = true;
    idleTimer_ = 0.0f;
    
    // 设置碰撞箱信息（32x32像素，一个格子大小）
    CollisionBoxInfo collisionInfo;
    collisionInfo.width = GRID_SIZE;
    collisionInfo.height = GRID_SIZE;
    collisionInfo.categoryBitmask = ENEMY_MASK;
    collisionInfo.contactTestBitmask = PLAYER_MASK | WALL_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.collisionBitmask = WALL_MASK | PLAYER_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.isDynamic = true;
    collisionInfo.mass = 1.0f;
    this->setCollisionBoxInfo(collisionInfo);
    
    // 初始化物理碰撞体
    this->InitPhysicsBody();
    
    // 初始化行为
    this->BehaviorInit();
    
    return true;
}

void Slime::Hitted(int damage, int poise_damage)
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
    
    // 韧性无限大，不处理破韧
}

void Slime::Dead()
{
    // 播放死亡动画
    if (sprite_ != nullptr)
    {
        sprite_->stopAllActions();
        sprite_->setColor(Color3B::WHITE); // 恢复原色
        
        // 创建死亡动画序列：播放死亡动画后淡出
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
    
    // 立即移除碰撞箱，防止继续与其他物体碰撞
    if (physicsBody_ != nullptr)
    {
        this->removeComponent(physicsBody_);
        physicsBody_ = nullptr;
    }
}

void Slime::BehaviorInit()
{
    // 注册行为函数
    this->addBehavior("idle", std::bind(&Slime::idle, this, std::placeholders::_1));
    this->addBehavior("recovery", std::bind(&Slime::recovery, this, std::placeholders::_1));
    this->addBehavior("jumpAttack", std::bind(&Slime::jumpAttack, this, std::placeholders::_1));
    this->addBehavior("chargeAttack", std::bind(&Slime::chargeAttack, this, std::placeholders::_1));
    
    // 设置初始行为
    this->currentBehavior_ = "idle";
}

std::string Slime::DecideNextBehavior(float delta)
{
    // 查找玩家
    EnemyAi::findPlayer(this);
    
    // 更新攻击计时器
    attackTimer_ += delta;
    
    // 如果检测到玩家且攻击冷却结束
    if (this->getPlayer() != nullptr && attackTimer_ >= attackCooldown_)
    {
        // 检测玩家是否可见、在攻击范围内且水平
        if (EnemyAi::isPlayerVisible(this) && EnemyAi::isPlayerInRange(this, detectionRange_) && EnemyAi::isPlayerHorizontal(this, GRID_SIZE * 2))
        {
            // 随机选择跳跃攻击或冲撞攻击
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
    
    // 默认返回待机行为
    return "idle";
}

void Slime::InitSprite()
{
    // 加载纹理图集到精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile("Enemy/Slime/slime_walk_left.plist", "Enemy/Slime/slime_walk_left.png");
    cache->addSpriteFramesWithFile("Enemy/Slime/slime_walk_right.plist", "Enemy/Slime/slime_walk_right.png");
    cache->addSpriteFramesWithFile("Enemy/Slime/slime_dead.plist", "Enemy/Slime/slime_dead.png");
    
    // 创建向左行走动画
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
    
    // 创建向右行走动画
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
    
    // 先移除基类创建的精灵（如果存在）
    if (sprite_ != nullptr)
    {
        sprite_->removeFromParent();
        sprite_ = nullptr;
    }
    
    // 创建精灵并设置初始帧
    sprite_ = Sprite::createWithSpriteFrame(cache->getSpriteFrameByName("slime_walk_left0000"));
    if (sprite_ != nullptr)
    {
        // 设置精灵大小为32x32像素
        sprite_->setContentSize(Size(GRID_SIZE, GRID_SIZE));
        sprite_->setPosition(Vec2::ZERO); // 设置精灵在节点中心
        this->addChild(sprite_);
        
        // 开始播放向左行走动画
        if (idleLeftAnimation_ != nullptr)
        {
            sprite_->runAction(RepeatForever::create(Animate::create(idleLeftAnimation_)));
        }
    }
}

BehaviorResult Slime::idle(float delta)
{
    // 待机行为：交替左右移动
    idleTimer_ += delta;
    
    // 查找玩家
    EnemyAi::findPlayer(this);
    
    // 无论玩家是否在检测范围内，都执行方向切换逻辑，但只有玩家不在时才移动
    // 这样可以确保动画始终交替，即使玩家在远处
    if (idleTimer_ >= 2.0f)
    {
        idleTimer_ = 0.0f;
        isMovingLeft_ = !isMovingLeft_;
        
        // 切换动画方向
            if (sprite_ != nullptr)
            {
                sprite_->stopAllActions();
                if (isMovingLeft_ && idleLeftAnimation_ != nullptr)
                {
                    log("Playing idleLeftAnimation_");
                    auto animate = Animate::create(idleLeftAnimation_);
                    if (animate != nullptr)
                    {
                        sprite_->runAction(RepeatForever::create(animate));
                    }
                    else
                    {
                        log("Failed to create animate from idleLeftAnimation_");
                    }
                }
                else if (!isMovingLeft_ && idleRightAnimation_ != nullptr)
                {
                    log("Playing idleRightAnimation_");
                    auto animate = Animate::create(idleRightAnimation_);
                    if (animate != nullptr)
                    {
                        sprite_->runAction(RepeatForever::create(animate));
                    }
                    else
                    {
                        log("Failed to create animate from idleRightAnimation_");
                    }
                }
                else
                {
                    log("No valid animation to play: isMovingLeft_=%d, idleLeftAnimation_=%p, idleRightAnimation_=%p", isMovingLeft_, idleLeftAnimation_, idleRightAnimation_);
                }
            }
    }
    
    // 如果玩家不在检测范围内，根据当前方向移动
    if (this->getPlayer() == nullptr || !EnemyAi::isPlayerInRange(this, detectionRange_))
    {
        if (physicsBody_ != nullptr)
        {
            float direction = isMovingLeft_ ? -1.0f : 1.0f;
            Vec2 velocity = physicsBody_->getVelocity();
            velocity.x = direction * movementSpeed_ * 0.5f; // 待机时移动速度较慢
            physicsBody_->setVelocity(velocity);
        }
    }
    else
    {
        // 如果找到玩家且在检测范围内，停止移动
        if (physicsBody_ != nullptr)
        {
            Vec2 velocity = physicsBody_->getVelocity();
            velocity.x = 0.0f;
            physicsBody_->setVelocity(velocity);
        }
    }
    
    return { true, 0.0f };
}

BehaviorResult Slime::recovery(float delta)
{
    // 后摇行为：原地不动，持续0.5秒
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
    // 如果已经死亡，立即结束攻击行为
    if (currentState_ == EnemyState::DEAD)
    {
        return { true, 0.0f };
    }
    
    // 跳跃攻击行为
    if (!isJumping_)
    {
        // 开始跳跃
        isJumping_ = true;
        // 重置碰撞标志
        isJumpAttackCollided_ = false;
        
        // 计算向玩家的方向
        Vec2 direction = Vec2::ZERO;
        if (this->getPlayer() != nullptr)
        {
            direction = (this->getPlayer()->getPosition() - this->getPosition()).getNormalized();
        }
        
        // 根据玩家方向设置精灵动画
        if (sprite_ != nullptr)
        {
            sprite_->stopAllActions();
            
            // 根据方向设置正确的动画
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
            
            // 攻击时让精灵变红
            auto tintAction = TintTo::create(0.1f, 255, 0, 0);
            if (tintAction != nullptr)
            {
                sprite_->runAction(tintAction);
            }
        }
        
        // 设置跳跃速度（向上跳跃，同时向玩家方向移动）
        if (physicsBody_ != nullptr)
        {
            Vec2 jumpVelocity = Vec2(direction.x * movementSpeed_, jumpSpeed_);
            physicsBody_->setVelocity(jumpVelocity);
        }
        
        // 创建跳跃攻击子弹，碰撞箱为Slime的1.2倍
        auto jumpBullet = Bullet::create("HelloWorld.png", this->getBaseAttackPower(), [this](Bullet* bullet, float delta) {
            // 检查自身是否已经死亡或被移除
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
    
    // 检查是否落地或撞到其他碰撞箱
    if (isJumping_ && physicsBody_ != nullptr && (physicsBody_->getVelocity().y == 0 || isJumpAttackCollided_))
    {
        isJumping_ = false;
        
        // 攻击结束后恢复原色
        if (sprite_ != nullptr)
        {
            sprite_->runAction(TintTo::create(0.1f, 255, 255, 255));
        }
        
        return { true, 2.0f }; // 攻击完成，后摇2秒
    }
    
    return { false, 0.0f };
}

BehaviorResult Slime::chargeAttack(float delta)
{
    // 如果已经死亡，立即结束攻击行为
    if (currentState_ == EnemyState::DEAD)
    {
        return { true, 0.0f };
    }
    
    // 冲撞攻击行为
    if (!isCharging_)
    {
        // 开始冲撞
        isCharging_ = true;
        // 重置碰撞标志
        isChargeAttackCollided_ = false;
        
        // 计算向玩家的方向
        Vec2 direction = Vec2::ZERO;
        if (this->getPlayer() != nullptr)
        {
            direction = (this->getPlayer()->getPosition() - this->getPosition()).getNormalized();
        }
        
        // 根据玩家方向设置精灵动画
        if (sprite_ != nullptr)
        {
            sprite_->stopAllActions();
            
            // 根据方向设置正确的动画
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
            
            // 攻击时让精灵变红
            auto tintAction = TintTo::create(0.1f, 255, 0, 0);
            if (tintAction != nullptr)
            {
                sprite_->runAction(tintAction);
            }
        }
        
        // 设置冲撞速度（向玩家方向快速移动）
        if (physicsBody_ != nullptr)
        {
            Vec2 chargeVelocity = Vec2(direction.x * movementSpeed_ * 2, 0);
            physicsBody_->setVelocity(chargeVelocity);
        }
        
        // 创建冲撞攻击子弹，碰撞箱为Slime的1.2倍
        auto chargeBullet = Bullet::create("HelloWorld.png", this->getBaseAttackPower(), [this](Bullet* bullet, float delta) {
            // 检查自身是否已经死亡或被移除
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
    
    // 冲撞持续一段时间后结束或撞到其他碰撞箱
    static float chargeTimer = 0.0f;
    chargeTimer += delta;
    
    if (chargeTimer >= 1.0f)
    {
        chargeTimer = 0.0f;
        isCharging_ = false;
        
        // 停止冲撞
        if (physicsBody_ != nullptr)
        {
            physicsBody_->setVelocity(Vec2::ZERO);
        }
        
        // 攻击结束后恢复原色
        if (sprite_ != nullptr)
        {
            sprite_->runAction(TintTo::create(0.1f, 255, 255, 255));
        }
        return { true, 2.0f }; // 攻击完成，后摇2秒
    }

    return { false, 0.0f };
}



bool Slime::onContactBegin(PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    // 确定当前Slime节点和对方节点
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
    
    // 检测是否在跳跃攻击或冲撞攻击中
    if (isJumping_ || isCharging_)
    {
        // 检测是否撞到了其他碰撞箱（除了自身）
        if (otherNode != this)
        {
            // 设置相应的碰撞标志
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
    
    // 检测是否碰到墙
    if (otherNode->getPhysicsBody()->getCategoryBitmask() == WALL_MASK)
    {
        // 碰到墙，不处理特殊逻辑（物理引擎会自动处理穿墙问题）
        return true;
    }
    
    // 检测是否碰到主角
    if (otherNode->getPhysicsBody()->getCategoryBitmask() == PLAYER_MASK)
    {
        
        // 反方向弹开一点距离
        Vec2 direction = (otherNode->getPosition() - slimeNode->getPosition()).getNormalized();
        otherNode->getPhysicsBody()->setVelocity(direction * 100);
        
        return true;
    }
    if (otherNode->getPhysicsBody()->getCategoryBitmask() == PLAYER_BULLET_MASK)
    {
        Bullet* bullet = dynamic_cast<Bullet*>(otherNode);
        if (bullet!= nullptr)
        {
            Hitted(bullet->getDamage());

        }
        return true;

    }
    
    return EnemyBase::onContactBegin(contact);
}

bool Slime::onContactSeparate(PhysicsContact& contact)
{
    return EnemyBase::onContactSeparate(contact);
}
