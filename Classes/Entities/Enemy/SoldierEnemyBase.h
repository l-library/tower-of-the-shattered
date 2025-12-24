#pragma once
#include "EnemyBase.h"
#include "cocos2d.h"

// 小兵基础类 - 继承自EnemyBase，是所有小兵的父类
class SoldierEnemyBase : public EnemyBase
{
protected:
    // 基础属性
    float attackRange_;        // 攻击范围
    float movementSpeed_;      // 移动速度
    float attackCooldown_;     // 攻击冷却时间
    float attackTimer_;        // 攻击计时器
    float detectionRange_;     // 检测范围
    
    // 动画相关
    cocos2d::RefPtr<cocos2d::Animation> idleLeftAnimation_;  // 向左移动动画
    cocos2d::RefPtr<cocos2d::Animation> idleRightAnimation_; // 向右移动动画
    cocos2d::RefPtr<cocos2d::Animation> deadAnimation_;      // 死亡动画
    
    // 状态变量
    bool isMovingLeft_;        // 是否向左移动
    float idleTimer_;          // 闲置计时器
    
    // 受保护的初始化方法
    bool init() override;
    
public:
    // 构造函数和析构函数
    SoldierEnemyBase();
    virtual ~SoldierEnemyBase();
    
    //小兵具有相似的待机行为
    virtual BehaviorResult idle(float delta);
    
    // 抽象基类的虚函数
    virtual void InitSprite() override = 0;
    virtual void Hitted(int damage, int poise_damage = 0) override = 0;
    virtual void Dead() override = 0;
    virtual void BehaviorInit() override = 0;
    virtual std::string DecideNextBehavior(float delta) override = 0;
    
    // Getter方法
    float getAttackRange() const;
    float getMovementSpeed() const;
    float getAttackCooldown() const;
    float getAttackTimer() const;
    float getDetectionRange() const;
    bool getIsMovingLeft() const;
    float getIdleTimer() const;
    
    // Setter方法
    void setAttackRange(float range);
    void setMovementSpeed(float speed);
    void setDetectionRange(float range);
    void setAttackCooldown(float cooldown);
    void setAttackTimer(float timer);
    void setIsMovingLeft(bool isMovingLeft);
    void setIdleTimer(float timer);
   

    void setIdleLeftAnimation(cocos2d::RefPtr<cocos2d::Animation> animation);
    void setIdleRightAnimation(cocos2d::RefPtr<cocos2d::Animation> animation);
    void setDeadAnimation(cocos2d::RefPtr<cocos2d::Animation> animation);
};