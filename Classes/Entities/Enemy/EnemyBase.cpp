#include "EnemyBase.h"
#include "TowerOfTheShattered.h"
#include "Entities/Player/Player.h"
#include <utility>
USING_NS_CC;

EnemyBase::EnemyBase()
    : sprite_(nullptr)
    , physicsBody_(nullptr)
    , max_vitality_(100)
    , current_vitality_(100)
    , stagger_resistance_(10)
    , current_stagger_resistance_(10)
    , currentState_(EnemyState::IDLE)
    , base_attack_power_(10)
    , defense_(5)
    , aiBehaviors_()
    , currentBehavior_("")
    , recoveryDuration_(0.0f)
    , recoveryTimer_(0.0f)
    , staggerDuration_(2.0f)  // 默认硬直持续2秒
    , staggerTimer_(0.0f)
    , player_(nullptr)
{
    // 初始化碰撞箱默认信息
    collisionBoxInfo_.width = 50.0f;
    collisionBoxInfo_.height = 50.0f;
    collisionBoxInfo_.categoryBitmask = 0x00000001;
    collisionBoxInfo_.contactTestBitmask = 0xFFFFFFFF;
    collisionBoxInfo_.collisionBitmask = 0xFFFFFFFF;
    collisionBoxInfo_.isDynamic = true;
    collisionBoxInfo_.mass = 1.0f;
}

EnemyBase::~EnemyBase()
{
    // 释放精灵资源
    if (sprite_ != nullptr)
    {
        sprite_->removeFromParent();
        sprite_ = nullptr;
    }
    // 清空AI行为映射
    aiBehaviors_.clear();
}

bool EnemyBase::init()
{
    if (!Node::init()) 
    {
        return false; 
    }
    
    // 调用精灵初始化虚函数
    this->InitSprite();
    
    // 初始化默认状态
    currentState_ = EnemyState::IDLE;
    
    // 初始化生命值和韧性
    current_vitality_ = max_vitality_;
    current_stagger_resistance_ = stagger_resistance_;
    
    // 初始化AI行为
    this->BehaviorInit();
    
    // 注册update，当对象创建后会每帧执行一次下面的update函数
    this->scheduleUpdate();
    
    return true;
}

// 精灵初始化虚函数的默认实现
void EnemyBase::InitSprite()
{
    // 初始化精灵
    sprite_ = Sprite::create();
    if (sprite_ != nullptr)
    {
        this->addChild(sprite_);
        
        // 为精灵创建默认的碰撞箱
        InitPhysicsBody();
    }
}

// 初始化物理碰撞体的实现
void EnemyBase::InitPhysicsBody()
{
    // 设置物理材质（弹性系数、摩擦系数、密度）
    // 0.0弹性防止弹跳，1.0摩擦防止滑动，0.0密度使质量不受体积影响
    auto bodyMaterial = PhysicsMaterial(0.0f, 1.0f, 0.0f);
    
    // 创建物理形状（矩形碰撞箱）
    auto shape = cocos2d::PhysicsShapeBox::create(cocos2d::Size(collisionBoxInfo_.width, collisionBoxInfo_.height), bodyMaterial);
    
    // 创建物理体
    physicsBody_ = cocos2d::PhysicsBody::create();
    physicsBody_->addShape(shape);
    
    // 设置物理体属性
    physicsBody_->setDynamic(collisionBoxInfo_.isDynamic); // 是否为动态
    physicsBody_->setMass(collisionBoxInfo_.mass); // 设置质量
    physicsBody_->setRotationEnable(false); // 禁止旋转
    physicsBody_->setCategoryBitmask(collisionBoxInfo_.categoryBitmask); // 碰撞类别
    physicsBody_->setContactTestBitmask(collisionBoxInfo_.contactTestBitmask); // 碰撞检测
    physicsBody_->setCollisionBitmask(collisionBoxInfo_.collisionBitmask); // 碰撞反应
    
    // 将物理体附加到敌人节点上
    this->setPhysicsBody(physicsBody_);
    
    // 确保精灵作为子节点位置正确
    if (sprite_ != nullptr)
    {
        sprite_->setPosition(Vec2::ZERO);
    }
    
    // 注册碰撞回调
    auto contactListener = cocos2d::EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(EnemyBase::onContactBegin, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(EnemyBase::onContactSeparate, this);
    
    // 添加事件监听器到当前节点
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
}

// 碰撞开始回调函数的默认实现
bool EnemyBase::onContactBegin(cocos2d::PhysicsContact& contact)
{
    return true;
}

// 碰撞结束回调函数的默认实现
bool EnemyBase::onContactSeparate(cocos2d::PhysicsContact& contact)
{
    return true;
}

void EnemyBase::updateAI(float delta)
{
    if (currentState_ == EnemyState::IDLE)
    {
        // 调用DecideNextBehavior()决定下一个行为
        currentBehavior_ = this->DecideNextBehavior(delta);
        
        // 执行决定的行为
        BehaviorResult behaviorResult = this->Execute(currentBehavior_, delta);
        bool behaviorCompleted = behaviorResult.first;
        
        // 如果执行的不是idle行为且行为未完成，则将状态设为ACTING
        if (currentBehavior_ != "idle" && !behaviorCompleted)
        {
            currentState_ = EnemyState::ACTING;
        }
    }
    
    if (currentState_ == EnemyState::ACTING)
    {
        // 执行当前记录的行为
        BehaviorResult behaviorResult = this->Execute(currentBehavior_, delta);
        bool behaviorCompleted = behaviorResult.first;
        float recoveryTime = behaviorResult.second;
        
        // 如果行为已完成
        if (behaviorCompleted)
        {
            // 如果有后摇时间，则进入RECOVERY状态
            if (recoveryTime > 0.0f)
            {
                currentState_ = EnemyState::RECOVERY;
                recoveryDuration_ = recoveryTime;
                recoveryTimer_ = 0.0f;
            }
            else
            {
                // 没有后摇时间，直接回到IDLE状态
                currentState_ = EnemyState::IDLE;
            }
        }
    }
    
    if (currentState_ == EnemyState::RECOVERY)
    {
        // 更新后摇计时器
        recoveryTimer_ += delta;
        
        // 如果后摇时间结束，进入IDLE状态
        if (recoveryTimer_ >= recoveryDuration_)
        {
            currentState_ = EnemyState::IDLE;
        }
    }
    
    if (currentState_ == EnemyState::STAGGERED)
    {
        // 执行硬直行为
        this->Execute("staggered", delta);
        
        // 更新硬直计时器
        staggerTimer_ += delta;
        
        // 如果硬直时间结束，进入IDLE状态并重置韧性
        if (staggerTimer_ >= staggerDuration_)
        {
            currentState_ = EnemyState::IDLE;
            current_stagger_resistance_ = stagger_resistance_; // 重置韧性
        }
    }
}
BehaviorResult EnemyBase::Execute(const std::string& name, float delta)
{
    if (hasBehavior(name)) {
        auto behavior = aiBehaviors_[name];
        return behavior(delta);
    }
    return { false, 0.0f };
}
void EnemyBase::update(float delta)
{
    // 如果已经死亡，不再更新
    if (currentState_ == EnemyState::DEAD)
    {
        return;
    }
    

    
    // 调用AI更新方法，封装了所有AI相关逻辑
    this->updateAI(delta);

}

// Getter方法
cocos2d::Sprite* EnemyBase::getSprite() const
{
    return sprite_;
}

CollisionBoxInfo EnemyBase::getCollisionBoxInfo() const
{
    return collisionBoxInfo_;
}

bool EnemyBase::isPlayerVisible()
{
    if (player_ == nullptr)
    {
        return false;
    }
    
    // 获取当前场景的物理世界
    auto scene = Director::getInstance()->getRunningScene();
    if (scene == nullptr) return false;
    
    auto physicsWorld = scene->getPhysicsWorld();
    if (physicsWorld == nullptr) return false;
    
    // 创建射线检测回调
    bool hasWall = false;
    
    physicsWorld->rayCast([&hasWall](PhysicsWorld& world, const PhysicsRayCastInfo& info, void* data) -> float {
        // 检查碰撞的物体是否是墙壁
        if (info.shape->getBody()->getCategoryBitmask() == WALL_MASK)
        {
            hasWall = true;
            return 0.0f; // 找到墙壁，停止检测
        }
        return -1.0f; // 继续检测
    }, this->getPosition(), player_->getPosition(), nullptr);
    
    // 如果没有墙壁阻挡，则玩家可见
    return !hasWall;
}

void EnemyBase::setPlayer(Player* player)
{
    player_ = player;
}

Player* EnemyBase::getPlayer() const
{
    return player_;
}

// Setter方法
void EnemyBase::setCollisionBoxInfo(const CollisionBoxInfo& info)
{
    collisionBoxInfo_ = info;
}

int EnemyBase::getMaxVitality() const
{
    return max_vitality_;
}

int EnemyBase::getCurrentVitality() const
{
    return current_vitality_;
}

int EnemyBase::getStaggerResistance() const
{
    return stagger_resistance_;
}

int EnemyBase::getCurrentStaggerResistance() const
{
    return current_stagger_resistance_;
}

EnemyState EnemyBase::getCurrentState() const
{
    return currentState_;
}

int EnemyBase::getBaseAttackPower() const
{
    return base_attack_power_;
}

int EnemyBase::getDefense() const
{
    return defense_;
}

float EnemyBase::getStaggerDuration() const
{
    return staggerDuration_;
}

cocos2d::PhysicsBody* EnemyBase::getPhysicsBody() const
{
    return physicsBody_;
}

void EnemyBase::setStaggerDuration(float duration)
{
    staggerDuration_ = std::max(0.0f, duration);
}

// Setter方法
void EnemyBase::setMaxVitality(int maxVitality)
{
    max_vitality_ = maxVitality;
    // 确保当前生命值不超过最大值
    if (current_vitality_ > max_vitality_)
    {
        current_vitality_ = max_vitality_;
    }
}

void EnemyBase::setCurrentVitality(int currentVitality)
{
    current_vitality_ = std::max(0, std::min(currentVitality, max_vitality_));
    // 如果生命值为0，触发死亡
    if (current_vitality_ <= 0)
    {
        currentState_ = EnemyState::DEAD;
        this->Dead();
    }
}

void EnemyBase::setStaggerResistance(int staggerResistance)
{
    stagger_resistance_ = staggerResistance;
    // 确保当前韧性不超过最大值
    if (current_stagger_resistance_ > stagger_resistance_)
    {
        current_stagger_resistance_ = stagger_resistance_;
    }
}

void EnemyBase::setCurrentStaggerResistance(int currentStaggerResistance)
{
    int oldValue = current_stagger_resistance_;
    current_stagger_resistance_ = std::max(0, std::min(currentStaggerResistance, stagger_resistance_));
    
    // 检查韧性是否被清零
    if (oldValue > 0 && current_stagger_resistance_ <= 0)
    {
        // 韧性被清零，无论当前是什么状态，强制进入STAGGERED状态
        currentState_ = EnemyState::STAGGERED;
        staggerTimer_ = 0.0f;
    }
}

void EnemyBase::setCurrentState(EnemyState state)
{
    currentState_ = state;
}

void EnemyBase::setBaseAttackPower(int attackPower)
{
    base_attack_power_ = std::max(0, attackPower);
}

void EnemyBase::setDefense(int defense)
{
    defense_ = std::max(0, defense);
}

// AI行为管理方法
void EnemyBase::addBehavior(const std::string& name, const Behavior& behavior)
{
    aiBehaviors_[name] = behavior;
}

void EnemyBase::removeBehavior(const std::string& name)
{
    auto it = aiBehaviors_.find(name);
    if (it != aiBehaviors_.end())
    {
        aiBehaviors_.erase(it);
    }
}

bool EnemyBase::hasBehavior(const std::string& name) const
{
    return aiBehaviors_.find(name) != aiBehaviors_.end();
}

// SoldierEnemyBase类的实现
bool SoldierEnemyBase::init()
{
    if (!EnemyBase::init())
    {
        return false;
    }
    
    return true;
}

bool SoldierEnemyBase::onContactBegin(cocos2d::PhysicsContact& contact)
{
    return EnemyBase::onContactBegin(contact);
}

bool SoldierEnemyBase::onContactSeparate(cocos2d::PhysicsContact& contact)
{
    return EnemyBase::onContactSeparate(contact);
}