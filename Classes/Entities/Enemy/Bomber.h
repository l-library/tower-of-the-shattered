#pragma once
#include "SoldierEnemyBase.h"

// Bomber类 - 继承自SoldierEnemyBase，会向玩家投掷炸弹的敌人
class Bomber : public SoldierEnemyBase
{
private:
    // Bomber特有的属性
    bool isThrowingBomb_;          // 是否正在投掷炸弹
    
    // Bomber特有的行为
    BehaviorResult recovery(float delta);    // 后摇行为
    BehaviorResult throwBombAttack(float delta);  // 投掷炸弹攻击行为
    
public:
    CREATE_FUNC(Bomber);
    
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
