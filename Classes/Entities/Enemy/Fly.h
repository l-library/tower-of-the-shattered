#pragma once
#include "EnemyBase.h"
#include "cocos2d.h"
#include "Entities/Bullet/Bullet.h"
#include "EnemyAi/EnemyAi.h"
class Fly : public EnemyBase
{
private:
    // 攻击相关属性
    float attackRange_;        // 攻击范围
    float movementSpeed_;      // 移动速度
    float attackCooldown_;     // 攻击冷却时间
    float attackTimer_;        // 攻击计时器
    float detectionRange_;     // 玩家检测范围
    float chargeSpeed_;        // 冲撞速度
    float postChargeUpDistance_; // 冲撞后向上移动的距离
    
    // 动画相关属性
    cocos2d::RefPtr<cocos2d::Animation> idleLeftAnimation_;  // 向左待机动画
    cocos2d::RefPtr<cocos2d::Animation> idleRightAnimation_; // 向右待机动画
    cocos2d::RefPtr<cocos2d::Animation> deadAnimation_;      // 死亡动画
    bool isMovingLeft_;        // 是否向左移动
    float idleTimer_;          // 待机计时器
    
    // 行为状态
    bool isCharging_;          // 是否正在冲撞
    bool isChargeAttackCollided_; // 冲撞攻击是否已经碰撞
    
    // 行为函数
    BehaviorResult idle(float delta);
    BehaviorResult recovery(float delta);
    BehaviorResult chargeAttack(float delta);
    
    // 初始化精灵
    void InitSprite() override;
    
public:
    // 静态创建方法
    static Fly* create();
    
    // 构造函数和析构函数
    Fly();
    virtual ~Fly();
    
    // 重写的方法
    bool init() override;
    void Hitted(int damage, int poise_damage = 0) override;
    void Dead() override;
    void BehaviorInit() override;
    std::string DecideNextBehavior(float delta) override;
    bool onContactBegin(cocos2d::PhysicsContact& contact) override;
    bool onContactSeparate(cocos2d::PhysicsContact& contact) override;
};
