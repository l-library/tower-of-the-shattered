#pragma once
#include "cocos2d.h"
#include "EnemyAi/EnemyAi.h"
#include "TowerOfTheShattered.h"

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

// 前向声明Player类
class Player;


// 敌人状态枚举
enum class EnemyState 
{
    IDLE,          // 空闲状态
    ACTING,        // 行动状态
    RECOVERY,      // 恢复状态
    STAGGERED,     // 被击晕状态
    DEAD           // 死亡状态
};


// 行为结果类型定义，包含是否成功和持续时间
using BehaviorResult = std::pair<bool, float>;
using Behavior = std::function<BehaviorResult(float delta)>; // 行为函数类型定义

// 碰撞箱相关信息结构体
typedef struct CollisionBoxInfo {
    float width;          // 碰撞箱宽度
    float height;         // 碰撞箱高度
    int categoryBitmask;  // 碰撞类别掩码
    int contactTestBitmask; // 碰撞检测掩码
    int collisionBitmask; // 碰撞反应掩码
    bool isDynamic;       // 是否为动态碰撞体
    float mass;           // 质量
} CollisionBoxInfo;

// 敌人基类，所有敌人的父类（包括小boss）
class EnemyBase :public cocos2d::Node
{
protected:
    cocos2d::Sprite* sprite_; // 敌人精灵，用于显示敌人模型/动画
    cocos2d::PhysicsBody* physicsBody_; // 敌人物理碰撞体
    CollisionBoxInfo collisionBoxInfo_; // 碰撞箱相关信息
    
    int max_vitality_;          // 最大生命值
    int current_vitality_;      // 当前生命值
    int stagger_resistance_;    // 抗晕值
    int current_stagger_resistance_; // 当前抗晕值
    EnemyState currentState_;      // 当前敌人状态
    int base_attack_power_;        // 基础攻击力
    int defense_;             // 防御力
    std::string currentBehavior_;  // 当前执行的行为名称
    float recoveryDuration_;       // 恢复状态持续时间
    float recoveryTimer_;          // 恢复状态计时器
    float staggerDuration_;        // 被击晕状态持续时间
    float staggerTimer_;           // 被击晕状态计时器
    Player* player_;               // 指向玩家的指针
    bool isDead_;                  // 是否已经死亡
    float deathTimer_;             // 死亡计时器
    float maxDeathTimer_;          // 最大死亡时间（用于延迟移除）

    // 初始化方法（重写）
    bool init() override;
    
    // 碰撞回调函数
    virtual bool onContactBegin(cocos2d::PhysicsContact& contact);
    virtual bool onContactSeparate(cocos2d::PhysicsContact& contact);

    std::unordered_map<std::string, Behavior> aiBehaviors_;  // AI行为映射表，以string为键，存储不同的行为函数
public:


    // 构造函数和析构函数
    EnemyBase();
    virtual ~EnemyBase();

    // 更新方法（建议子类重写，父类提供基础逻辑）
    void update(float delta) override;


    // 纯虚函数声明（子类必须在create方法中实现）
    virtual void Hitted(int damage, int poise_damage = 0) = 0; // 被击中处理
    virtual void Dead() = 0;                                   // 死亡处理
    virtual void BehaviorInit() = 0; // 行为初始化，子类必须实现
    virtual std::string DecideNextBehavior(float delta) = 0; // 决定下一个行为，传入delta时间
    virtual void InitSprite();    // 初始化精灵，子类可以重写以设置不同的精灵

    void DropLootOnDeath();
    virtual void otherUpdate(float delta) {};

    // Getter方法 - 物理碰撞体
    cocos2d::PhysicsBody* getPhysicsBody() const;
    
    // 初始化物理碰撞体（非虚函数）
    void InitPhysicsBody();

    
    // Getter方法
    cocos2d::Sprite* getSprite() const;
    int getMaxVitality() const;
    int getCurrentVitality() const;
    int getStaggerResistance() const;
    int getCurrentStaggerResistance() const;
    EnemyState getCurrentState() const;
    int getBaseAttackPower() const;
    int getDefense() const;
    float getStaggerDuration() const;
    CollisionBoxInfo getCollisionBoxInfo() const;
    
    
    // 设置玩家指针
    void setPlayer(Player* player);
    
    // 获取玩家指针
    Player* getPlayer() const;
    
    
    // Setter方法
    void setCollisionBoxInfo(const CollisionBoxInfo& info);
    void setMaxVitality(int maxVitality);
    void setCurrentVitality(int currentVitality);
    void setStaggerResistance(int staggerResistance);
    void setCurrentStaggerResistance(int currentStaggerResistance);
    void setCurrentState(EnemyState state);
    void setBaseAttackPower(int attackPower);
    void setDefense(int defense);
    void setStaggerDuration(float duration);
    void setCurrentBehavior(std::string name);

    // AI更新方法
    void updateAI(float delta);

    // AI行为方法
    void addBehavior(const std::string& name, const Behavior& behavior);
    void removeBehavior(const std::string& name);
    bool hasBehavior(const std::string& name) const;
    BehaviorResult Execute(const std::string& name, float delta);//??????

};


