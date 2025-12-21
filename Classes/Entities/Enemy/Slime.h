#pragma once
#include "EnemyBase.h"
#include "TowerOfTheShattered.h" // 包含GRID_SIZE宏

// Slime类 - 第一个具体的小兵类，继承自EnemyBase
class Slime : public EnemyBase
{
public:
    CREATE_FUNC(Slime);
    
    virtual bool init() override;
    
    // 实现父类的纯虚函数
    virtual void Hitted(int damage, int poise_damage = 0) override;
    virtual void Dead() override;
    virtual void BehaviorInit() override;
    virtual std::string DecideNextBehavior(float delta) override;
    virtual void InitSprite() override;
    
    // 实现碰撞回调函数
    virtual bool onContactBegin(cocos2d::PhysicsContact& contact) override;
    virtual bool onContactSeparate(cocos2d::PhysicsContact& contact) override;
    
private:
    // Slime特有的属性
    float attackRange_;        // 攻击范围
    float movementSpeed_;      // 移动速度
    float jumpSpeed_;          // 跳跃速度
    float attackCooldown_;     // 攻击冷却时间
    float attackTimer_;        // 攻击计时器
    
    // 行为状态
    bool isJumping_;           // 是否正在跳跃
    bool isCharging_;          // 是否正在冲撞
    bool isJumpAttackCollided_;  // 跳跃攻击是否发生碰撞
    bool isChargeAttackCollided_; // 冲撞攻击是否发生碰撞
    
    // 检测玩家的距离
    float detectionRange_;     // 检测玩家的距离
    
    // Slime特有的行为
    BehaviorResult idle(float delta);       // 待机行为
    BehaviorResult recovery(float delta);    // 后摇行为
    BehaviorResult jumpAttack(float delta);  // 跳跃攻击行为
    BehaviorResult chargeAttack(float delta); // 冲撞攻击行为
    
    // 辅助方法
      Player* findPlayer();             // 查找玩家
      bool isPlayerInRange();                  // 检测玩家是否在攻击范围内
    bool isPlayerHorizontal();               // 检测玩家是否在同一水平线上
};
