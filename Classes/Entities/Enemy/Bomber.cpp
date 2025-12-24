#include "Bomber.h"
#include "TowerOfTheShattered.h"
#include "cocos2d.h"
#include "Entities/Bullet/Bullet.h"

using namespace cocos2d;

bool Bomber::init()
{
    if (!SoldierEnemyBase::init())
    {
        return false;
    }
    
    // 初始化Bomber的基本属性
    this->setMaxVitality(120);
    this->setCurrentVitality(120);
    this->setStaggerResistance(10); // 有一定的抗打断能力
    this->setBaseAttackPower(15);
    this->setDefense(0);
    
    // 设置Bomber的行为参数
    this->setAttackRange(150.0f);
    this->setMovementSpeed(120.0f);
    this->setAttackCooldown(3.0f);
    this->setDetectionRange(250.0f);
    
    // 初始化状态变量
    isThrowingBomb_ = false;
    
    // 创建碰撞盒信息，设置为32x32的大小
    CollisionBoxInfo collisionInfo;
    collisionInfo.width = GRID_SIZE;
    collisionInfo.height = GRID_SIZE*2.5;
    collisionInfo.categoryBitmask = ENEMY_MASK;
    collisionInfo.contactTestBitmask = PLAYER_MASK | WALL_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.collisionBitmask = WALL_MASK | PLAYER_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.isDynamic = true;
    collisionInfo.mass = 1.0f;
    this->setCollisionBoxInfo(collisionInfo);
    
    // 初始化物理体
    this->InitPhysicsBody();
    
    // 初始化行为
    this->BehaviorInit();
    
    return true;
}

void Bomber::Hitted(int damage, int poise_damage)
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
}

void Bomber::Dead()
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

void Bomber::BehaviorInit()
{
    // 添加行为
    this->addBehavior("idle", std::bind(&SoldierEnemyBase::idle, this, std::placeholders::_1));
    this->addBehavior("recovery", std::bind(&Bomber::recovery, this, std::placeholders::_1));
    this->addBehavior("throwBombAttack", std::bind(&Bomber::throwBombAttack, this, std::placeholders::_1));
    
    // 设置初始行为
    this->currentBehavior_ = "idle";
}

std::string Bomber::DecideNextBehavior(float delta)
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
            return "throwBombAttack";
        }
    }
    
    // 否则保持待机状态
    return "idle";
}

void Bomber::InitSprite()
{
    // 加载精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile("Enemy/Bomber/bomber_walk_left.plist", "Enemy/Bomber/bomber_walk_left.png");
    cache->addSpriteFramesWithFile("Enemy/Bomber/bomber_walk_right.plist", "Enemy/Bomber/bomber_walk_right.png");
    cache->addSpriteFramesWithFile("Enemy/Bomber/bomber_dead.plist", "Enemy/Bomber/bomber_dead.png");
    cache->addSpriteFramesWithFile("Enemy/Bomber/bomber_stop.plist", "Enemy/Bomber/bomber_stop.png");
    
    // 创建向左移动的动画
    Vector<SpriteFrame*> leftFrames;
    for (int i = 0; i < 2; i++)
    {
        char frameName[32];
        sprintf(frameName, "bomber_walk_left%04d", i);
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
        sprintf(frameName, "bomber_walk_right%04d", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            rightFrames.pushBack(frame);
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
        sprintf(frameName, "bomber_dead%04d", i);
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
    sprite_ = Sprite::createWithSpriteFrame(cache->getSpriteFrameByName("bomber_walk_left0000"));
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

BehaviorResult Bomber::recovery(float delta)
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

BehaviorResult Bomber::throwBombAttack(float delta)
{
    // 如果敌人已经死亡，结束行为
    if (currentState_ == EnemyState::DEAD)
    {
        return { true, 0.0f };
    }
    
    // 如果还没开始投掷炸弹
    if (!isThrowingBomb_)
    {
        // 开始投掷炸弹
        isThrowingBomb_ = true;
        
        // 计算投掷方向
        Vec2 direction = Vec2::ZERO;
        if (this->getPlayer() != nullptr)
        {
            direction = (this->getPlayer()->getPosition() - this->getPosition()).getNormalized();
        }
        
        // 更新精灵动画为停止动画
        if (sprite_ != nullptr)
        {
            sprite_->stopAllActions();
            
            // 获取停止动画的精灵帧
            auto cache = SpriteFrameCache::getInstance();
            SpriteFrame* stopFrame = nullptr;
            
            if (direction.x < 0)
            {
                stopFrame = cache->getSpriteFrameByName("hero_stop0000");
            }
            else
            {
                stopFrame = cache->getSpriteFrameByName("hero_stop0000");
            }
            
            if (stopFrame != nullptr)
            {
                sprite_->setSpriteFrame(stopFrame);
            }
        }
        
        // 停止移动
        if (physicsBody_ != nullptr)
        {
            Vec2 velocity = physicsBody_->getVelocity();
            velocity.x = 0.0f;
            physicsBody_->setVelocity(velocity);
        }
        
        // 创建并投掷炸弹
        auto bomb = Bullet::create("Enemy/Bomber/bomb.png", this->getBaseAttackPower(), [this](Bullet* bullet, float delta) {
            // 检查炸弹是否即将清理
            if (bullet->getPhysicsBody() == nullptr) return;
            
            // 创建一个临时变量来存储炸弹状态
            static bool exploded = false;
            if (!exploded) {
                // 检查是否需要清理
                if (!bullet || !bullet->getParent()) {
                    exploded = true;
                    return;
                }
                
                Vec2 velocity = bullet->getPhysicsBody()->getVelocity();
                if (bullet->getExistTime() > 1.9f) {
                    
                    // 在炸弹位置创建一个较大的瞬时bullet
                    auto explosionBullet = Bullet::create("Enemy/Bomber/bomb.png", this->getBaseAttackPower(), nullptr);
                    if (explosionBullet) {
                        // 设置爆炸bullet的属性
                        explosionBullet->setCollisionWidth(GRID_SIZE * 3.0f); // 较大的碰撞范围
                        explosionBullet->setCollisionHeight(GRID_SIZE * 3.0f);
                        explosionBullet->setPosition(bullet->getPosition());
                        explosionBullet->setCategoryBitmask(ENEMY_BULLET_MASK);
                        explosionBullet->setContactTestBitmask(PLAYER_MASK);
                        explosionBullet->setCollisionBitmask(PLAYER_MASK);
                        explosionBullet->setCLearBitmask(0); // 无clearmask，不会被碰撞清除
                        explosionBullet->setDamage(30); // 爆炸伤害
                        explosionBullet->setMaxExistTime(0.2f); // 只存在0.2秒
                        
                        // 添加到场景
                        bullet->getParent()->addChild(explosionBullet);
                        
                        // 禁用物理体移动
                        auto physicsBody = explosionBullet->getPhysicsBody();
                        if (physicsBody) {
                            physicsBody->setVelocity(Vec2::ZERO);
                            physicsBody->setGravityEnable(false);
                        }
                    }
                    
                    // 播放炸弹爆炸特效
                    auto cache = SpriteFrameCache::getInstance();
                    
                    // 加载爆炸特效的精灵帧
                    cache->addSpriteFramesWithFile("Enemy/Bomber/bomb_dead.plist", "Enemy/Bomber/bomb_dead.png");
                    
                    // 创建爆炸动画
                    Vector<SpriteFrame*> explosionFrames;
                    for (int i = 0; i < 10; i++) { 
                        char frameName[32];
                        sprintf(frameName, "bomb_dead%04d", i);
                        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
                        if (frame) {
                            explosionFrames.pushBack(frame);
                        }
                    }
                    
                    if (!explosionFrames.empty()) {
                        // 创建爆炸精灵
                        auto explosionSprite = Sprite::createWithSpriteFrame(explosionFrames.front());
                        if (explosionSprite) {
                            explosionSprite->setPosition(bullet->getPosition());
                            bullet->getParent()->addChild(explosionSprite);
                            
                            // 创建并播放爆炸动画
                            auto explosionAnimation = Animation::createWithSpriteFrames(explosionFrames, 0.02f);
                            auto explosionAnimate = Animate::create(explosionAnimation);
                            
                            // 动画完成后移除爆炸精灵
                            auto removeAction = RemoveSelf::create();
                            auto sequence = Sequence::create(explosionAnimate, removeAction, nullptr);
                            explosionSprite->runAction(sequence);
                        }
                    }
                }
            }
        });

        
        if (bomb)
        {
            // 设置炸弹属性
            bomb->setCollisionWidth(GRID_SIZE *1.0f);
            bomb->setCollisionHeight(GRID_SIZE *1.0f);
            bomb->setPosition(this->getPosition() + Vec2(direction.x * 20, 10));
            bomb->setCategoryBitmask(ENEMY_BULLET_MASK);
            bomb->setContactTestBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
            bomb->setCollisionBitmask(WALL_MASK | PLAYER_MASK | BORDER_MASK);
            bomb->setCLearBitmask(0);
            bomb->setDamage(5);
            bomb->getPhysicsBody()->setGravityEnable(true);      
            bomb->getPhysicsBody()->setMass(70.0f);
            // 设置炸弹的最大存在时间（爆炸时间）
            bomb->setMaxExistTime(2.0f);
            // 添加到父节点
            this->getParent()->addChild(bomb);
            
            // 给炸弹设置斜上抛的初速度（增加了速度）
            Vec2 bombVelocity(direction.x * 300, 400);
            bomb->getPhysicsBody()->setVelocity(bombVelocity);
        }
    }
    
    // 控制投掷动作持续时间
    static float throwTimer = 0.0f;
    throwTimer += delta;
    
    if (throwTimer >= 1.0f)
    {
        throwTimer = 0.0f;
        isThrowingBomb_ = false;
        
        // 恢复精灵动画
        if (sprite_ != nullptr)
        {
            // 根据当前移动方向选择动画
            if (isMovingLeft_ && idleLeftAnimation_ != nullptr)
            {
                sprite_->runAction(RepeatForever::create(Animate::create(idleLeftAnimation_)));
            }
            else if (!isMovingLeft_ && idleRightAnimation_ != nullptr)
            {
                sprite_->runAction(RepeatForever::create(Animate::create(idleRightAnimation_)));
            }
        }
        
        return { true, 2.0f }; // 完成投掷攻击，冷却2秒
    }
    
    return { false, 0.0f };
}

bool Bomber::onContactBegin(cocos2d::PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    // 确定哪一方是Bomber
    Node* bomberNode = nullptr;
    Node* otherNode = nullptr;
    
    if (nodeA == this)
    {
        bomberNode = nodeA;
        otherNode = nodeB;
    }
    else if (nodeB == this)
    {
        bomberNode = nodeB;
        otherNode = nodeA;
    }
    
    if (bomberNode == nullptr || otherNode == nullptr)
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
        Vec2 direction = (otherNode->getPosition() - bomberNode->getPosition()).getNormalized();
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

bool Bomber::onContactSeparate(cocos2d::PhysicsContact& contact)
{
    return EnemyBase::onContactSeparate(contact);
}
