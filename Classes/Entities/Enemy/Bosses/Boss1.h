#pragma once
#include "../EnemyBase.h"

class Boss1 : public EnemyBase
{
protected:
    cocos2d::Animation* idleAnimation_; 
    bool isIdleAnimationPlaying_;      
    cocos2d::Node* spriteParent_;     
    
    //attack1相关
    bool isAttack1Active_;            
    int bulletCount_;                
    float bulletTimer_;                
    bool hasTeleported_;              
    cocos2d::Vec2 initialPosition_;   
    void fireBullet1();            
    
    //attack2相关
    bool isAttack2Active_;            
    float attack2Timer_;              
    int attack2State_;                
    void fireSwordBeam();            
    
    //attack3,4相关
    bool isAttack3Active_;
    bool hasSummonedClone_;
    bool IsStage2_;            // 记录attack3是否已经使用过
    bool isStage3_;            // 记录是否进入三阶段   
    cocos2d::Vec2 clonePosition_;
    Boss1* clone_;
    void createClone();

    //boss1的行为
    BehaviorResult idle(float delta);
    BehaviorResult recovery(float delta);
    BehaviorResult attack1(float delta);
    BehaviorResult attack2(float delta);
    BehaviorResult attack3(float delta);
    BehaviorResult attack4(float delta);
    BehaviorResult turn(float delta);


public:
    Boss1();
    virtual ~Boss1();

    // 建立
    static Boss1* create();

    // 虚函数重写
    virtual bool init() override;
    virtual void Hitted(int damage, int poise_damage = 0) override;
    virtual void Dead() override;
    virtual void BehaviorInit() override;
    virtual std::string DecideNextBehavior(float delta) override;
    virtual void InitSprite() override;
    virtual void otherUpdate(float delta)override;
    // 碰撞回调
    virtual bool onContactBegin(cocos2d::PhysicsContact& contact) override;
    virtual bool onContactSeparate(cocos2d::PhysicsContact& contact) override;
};
