#include "Enemy.h"
#include <utility>
USING_NS_CC;

EnemyBase::EnemyBase()
    : sprite_(nullptr)
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
{
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
    }
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