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
    
    // 初始化精灵
    this->InitSprite();
    
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
    // 淡出效果
    if (getSprite()) {
        auto fadeOut = FadeOut::create(1.0f);
        getSprite()->runAction(fadeOut);
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
    this->findPlayer();
    
    // 更新攻击计时器
    attackTimer_ += delta;
    
    // 如果检测到玩家且攻击冷却结束
    if (this->getPlayer() != nullptr && attackTimer_ >= attackCooldown_)
    {
        // 检测玩家是否可见、在攻击范围内且水平
        if (this->isPlayerVisible() && this->isPlayerInRange() && this->isPlayerHorizontal())
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
    // 使用现有图片作为Slime的精灵
    sprite_ = Sprite::create("HelloWorld.png"); // 暂时使用现有图片
    if (sprite_ != nullptr)
    {
        // 设置精灵大小为32x32像素
        sprite_->setContentSize(Size(GRID_SIZE, GRID_SIZE));
        sprite_->setPosition(Vec2::ZERO); // 设置精灵在节点中心
        this->addChild(sprite_);
    }
}

BehaviorResult Slime::idle(float delta)
{
    // 待机行为：原地不动
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
        
        // 设置跳跃速度（向上跳跃，同时向玩家方向移动）
        if (physicsBody_ != nullptr)
        {
            Vec2 jumpVelocity = Vec2(direction.x * movementSpeed_, jumpSpeed_);
            physicsBody_->setVelocity(jumpVelocity);
        }
        
        // 攻击时让精灵变红
        if (sprite_ != nullptr)
        {
            sprite_->runAction(TintTo::create(0.1f, 255, 0, 0));
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
        
        // 设置冲撞速度（向玩家方向快速移动）
        if (physicsBody_ != nullptr)
        {
            Vec2 chargeVelocity = Vec2(direction.x * movementSpeed_ * 2, 0);
            physicsBody_->setVelocity(chargeVelocity);
        }
        
        // 攻击时让精灵变红
        if (sprite_ != nullptr)
        {
            sprite_->runAction(TintTo::create(0.1f, 255, 0, 0));
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

Player* Slime::findPlayer()
{
    // 查找玩家节点（使用typeid检测玩家类型）
    auto parent = this->getParent();
    if (parent != nullptr)
    {
        auto children = parent->getChildren();
        for (auto child : children)
        {
            // 使用typeid检测节点是否为Player类型
            if (typeid(*child) == typeid(Player))
            {
                Player* player = static_cast<Player*>(child);
                this->setPlayer(player); // 存储玩家指针到基类
                return player;
            }
        }
    }
    
    this->setPlayer(nullptr); // 如果没有找到玩家，将基类的玩家指针设为nullptr
    return nullptr;
}

bool Slime::isPlayerInRange()
{
    if (this->getPlayer() == nullptr)
    {
        return false;
    }
    
    // 检测玩家是否在攻击范围内
    float distance = this->getPosition().distance(this->getPlayer()->getPosition());
    return distance <= detectionRange_;
}

bool Slime::isPlayerHorizontal()
{
    if (this->getPlayer() == nullptr)
    {
        return false;
    }
    
    // 检测玩家是否在同一水平线上（Y坐标差小于一定值）
    float yDiff = abs(this->getPosition().y - this->getPlayer()->getPosition().y);
    return yDiff <= GRID_SIZE * 2;
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
            Hitted(100);

        }
        return true;

    }
    
    return EnemyBase::onContactBegin(contact);
}

bool Slime::onContactSeparate(PhysicsContact& contact)
{
    return EnemyBase::onContactSeparate(contact);
}
