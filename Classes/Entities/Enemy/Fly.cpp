#include "Fly.h"
#include "TowerOfTheShattered.h"
#include "cocos2d.h"
#include "Entities/Bullet/Bullet.h"

using namespace cocos2d;

Fly::Fly()
    : chargeSpeed_(250.0f)
    , postChargeUpDistance_(50.0f)
    , isCharging_(false)
    , isChargeAttackCollided_(false)
{}

Fly::~Fly()
{}

Fly* Fly::create()
{
    Fly* pRet = new(std::nothrow) Fly();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool Fly::init()
{
    if (!SoldierEnemyBase::init())
    {
        return false;
    }
    
    // 设置Fly的基本属性
    this->setMaxVitality(80);
    this->setCurrentVitality(80);
    this->setStaggerResistance(INT_MAX); // 韧性为无限大
    this->setBaseAttackPower(8);
    this->setDefense(0);
    
    // 设置SoldierEnemyBase的属性
    this->setAttackRange(100.0f);
    this->setMovementSpeed(120.0f);
    this->setAttackCooldown(2.0f);
    this->setDetectionRange(200.0f);
    
    // 设置行为状态
    isCharging_ = false;
    isChargeAttackCollided_ = false;
    
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
    
    // 初始化物理碰撞体（无重力）
    this->InitPhysicsBody();
    if (physicsBody_ != nullptr)
    {
        physicsBody_->setGravityEnable(false); // 设置为无重力
    }
    
    // 初始化行为
    this->BehaviorInit();
    
    return true;
}

void Fly::Hitted(int damage, int poise_damage)
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

void Fly::Dead()
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

void Fly::BehaviorInit()
{
    // 注册行为函数：使用父类的idle行为
    this->addBehavior("idle", std::bind(&SoldierEnemyBase::idle, this, std::placeholders::_1));
    this->addBehavior("recovery", std::bind(&Fly::recovery, this, std::placeholders::_1));
    this->addBehavior("chargeAttack", std::bind(&Fly::chargeAttack, this, std::placeholders::_1));
    
    // 设置初始行为
    this->currentBehavior_ = "idle";
}

std::string Fly::DecideNextBehavior(float delta)
{
    // 查找玩家
    EnemyAi::findPlayer(this);
    
    // 更新攻击计时器
    attackTimer_ += delta;
    
    // 如果检测到玩家且攻击冷却结束
    if (this->getPlayer() != nullptr && attackTimer_ >= attackCooldown_)
    {
        // 检测玩家是否在检测范围内
        if (EnemyAi::isPlayerInRange(this, detectionRange_))
        {
            // 执行冲撞攻击
            attackTimer_ = 0.0f;
            return "chargeAttack";
        }
    }
    
    // 默认返回待机行为
    return "idle";
}

void Fly::InitSprite()
{
    // 先移除基类创建的精灵（如果存在）
    if (sprite_ != nullptr)
    {
        sprite_->removeFromParent();
        sprite_ = nullptr;
    }
    
    // 加载纹理图集到精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    
    // 检查文件是否存在
    std::string leftPlistPath = "Enemy/Fly01/fly01_fly_left.plist";
    std::string leftImagePath = "Enemy/Fly01/fly01_fly_left.png";
    std::string rightPlistPath = "Enemy/Fly01/fly01_fly_right.plist";
    std::string rightImagePath = "Enemy/Fly01/fly01_fly_right.png";
    std::string deadPlistPath = "Enemy/Fly01/fly01_dead.plist";
    std::string deadImagePath = "Enemy/Fly01/fly01_dead.png";
    

    
    // 加载纹理图集
    cache->addSpriteFramesWithFile(leftPlistPath, leftImagePath);
    cache->addSpriteFramesWithFile(rightPlistPath, rightImagePath);
    cache->addSpriteFramesWithFile(deadPlistPath, deadImagePath);
    
    // 创建向左待机动画
    Vector<SpriteFrame*> leftFrames;
    for (int i = 0; i < 2; i++)
    {
        char frameName[32];
        sprintf(frameName, "fly01_fly_left%04d", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            leftFrames.pushBack(frame);

        }
        else
        {

        }
    }

    if (!leftFrames.empty())
    {
        idleLeftAnimation_ = Animation::createWithSpriteFrames(leftFrames, 0.2f);
        if (idleLeftAnimation_ != nullptr)
        {
            idleLeftAnimation_->setLoops(-1); // 无限循环

        }
        else
        {

        }
    }
    
    // 创建向右待机动画
    Vector<SpriteFrame*> rightFrames;
    for (int i = 0; i < 2; i++)
    {
        char frameName[32];
        sprintf(frameName, "fly01_fly_right%04d", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            rightFrames.pushBack(frame);
        }
        else
        {

        }
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
    for (int i = 0; i < 2; i++)
    {
        char frameName[32];
        sprintf(frameName, "fly01_dead%04d", i);
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
    
    // 创建精灵并设置初始帧
    auto initialFrame = cache->getSpriteFrameByName("fly01_fly_left0000");
    if (initialFrame == nullptr)
    {
    
    }
    
    sprite_ = Sprite::createWithSpriteFrame(initialFrame);
    if (sprite_ != nullptr)
    {

        // 设置精灵大小为32x32像素
        sprite_->setContentSize(Size(GRID_SIZE, GRID_SIZE));
        sprite_->setPosition(Vec2::ZERO); // 设置精灵在节点中心
        this->addChild(sprite_);

        
        // 开始播放向左待机动画
        if (idleLeftAnimation_ != nullptr)
        {
            sprite_->runAction(RepeatForever::create(Animate::create(idleLeftAnimation_)));
        }
    }
    else
    {
        log("Failed to create sprite with initial frame");
        // 尝试使用默认图片创建精灵，作为备用方案
        sprite_ = Sprite::create("HelloWorld.png");
        if (sprite_ != nullptr)
        {

            sprite_->setContentSize(Size(GRID_SIZE, GRID_SIZE));
            sprite_->setPosition(Vec2::ZERO);
            this->addChild(sprite_);
        }
        else
        {

        }
    }
}



BehaviorResult Fly::recovery(float delta)
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

BehaviorResult Fly::chargeAttack(float delta)
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
            Vec2 chargeVelocity = Vec2(direction.x * chargeSpeed_, direction.y * chargeSpeed_);
            physicsBody_->setVelocity(chargeVelocity);
        }
        
        // 创建冲撞攻击子弹，碰撞箱为Fly的1.2倍
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
            chargeBullet->setCLearBitmask(PLAYER_MASK);
            chargeBullet->setDamage(30);
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
        
        // 冲撞结束后向上移动一小段距离
        if (physicsBody_ != nullptr)
        {
            Vec2 velocity = physicsBody_->getVelocity();
            velocity.x = 0.0f;
            velocity.y = postChargeUpDistance_;
            physicsBody_->setVelocity(velocity);
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

bool Fly::onContactBegin(PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    // 确定当前Fly节点和对方节点
    Node* flyNode = nullptr;
    Node* otherNode = nullptr;
    
    if (nodeA == this)
    {
        flyNode = nodeA;
        otherNode = nodeB;
    }
    else if (nodeB == this)
    {
        flyNode = nodeB;
        otherNode = nodeA;
    }
    
    if (flyNode == nullptr || otherNode == nullptr)
    {
        return true;
    }
    
    // 检测是否在冲撞攻击中
    if (isCharging_)
    {
        // 检测是否撞到了其他碰撞箱（除了自身）
        if (otherNode != this)
        {
            // 设置相应的碰撞标志
            isChargeAttackCollided_ = true;
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
        Vec2 direction = (otherNode->getPosition() - flyNode->getPosition()).getNormalized();
        otherNode->getPhysicsBody()->setVelocity(direction * 100);
        
        return true;
    }
    
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

bool Fly::onContactSeparate(PhysicsContact& contact)
{
    return EnemyBase::onContactSeparate(contact);
}
