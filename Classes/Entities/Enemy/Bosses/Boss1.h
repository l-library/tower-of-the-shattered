#pragma once
#include "../EnemyBase.h"

class Boss1 : public EnemyBase
{
protected:
    // Boss1特有的属性





    // 行为方法
    BehaviorResult idle(float delta);
    BehaviorResult recovery(float delta);
    BehaviorResult attack1(float delta);
    BehaviorResult attack2(float delta);
    BehaviorResult attack3(float delta);


public:
    // 构造函数和析构函数
    Boss1();
    virtual ~Boss1();

    // 创建实例的静态方法
    static Boss1* create();

    // 重写父类的虚函数
    virtual bool init() override;
    virtual void Hitted(int damage, int poise_damage = 0) override;
    virtual void Dead() override;
    virtual void BehaviorInit() override;
    virtual std::string DecideNextBehavior(float delta) override;
    virtual void InitSprite() override;

    // 重写碰撞回调函数
    virtual bool onContactBegin(cocos2d::PhysicsContact& contact) override;
    virtual bool onContactSeparate(cocos2d::PhysicsContact& contact) override;
};
