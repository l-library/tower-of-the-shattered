#pragma once
#include "SoldierEnemyBase.h"


// Mage类 - 派生自SoldierEnemyBase，使用火球进行直线攻击的法师敌人
class Mage : public SoldierEnemyBase
{
private:
    // Mage的内部状态变量
    bool isCastingSpell_;          // 是否正在施法
    
    // Mage的行为函数
    BehaviorResult recovery(float delta);    // 恢复状态
    BehaviorResult castFireball(float delta);  // 释放火球攻击
    
public:
    CREATE_FUNC(Mage);
    
    virtual bool init() override;
    
    // 重写基类的纯虚函数
    virtual void Hitted(int damage, int poise_damage = 0) override;
    virtual void Dead() override;
    virtual void BehaviorInit() override;
    virtual std::string DecideNextBehavior(float delta) override;
    virtual void InitSprite() override;
    
    // 重写碰撞回调函数
    virtual bool onContactBegin(cocos2d::PhysicsContact& contact) override;
    virtual bool onContactSeparate(cocos2d::PhysicsContact& contact) override;
    
    Mage();
    ~Mage();
};