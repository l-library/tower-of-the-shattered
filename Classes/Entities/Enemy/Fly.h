#pragma once
#include "SoldierEnemyBase.h"

class Fly : public SoldierEnemyBase
{
private:
    // Fly特有的属性
    float chargeSpeed_;        // 冲撞速度
    float postChargeUpDistance_; // 冲撞后向上移动的距离
    
    // 行为状态
    bool isCharging_;          // 是否正在冲撞
    bool isChargeAttackCollided_; // 冲撞攻击是否已经碰撞
    
    // 行为函数
    BehaviorResult recovery(float delta);
    BehaviorResult chargeAttack(float delta);
    

public:
    // 静态创建方法
    static Fly* create();
    
    // 构造函数和析构函数
    Fly();
    virtual ~Fly();
    
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
};
