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
    bool prepareStage3_;
    void createClone();

    //attack5相关
    bool isAttack5Active_;
    float attack5Timer_;
    bool isFlipped_;
    float originalCollisionWidth_;
    float dashDistance_;
    Bullet* dashBullet_; // 冲刺攻击的判定框bullet
    
    //AI决策相关
    std::string lastBehavior_; // 记录上一个行为，防止attack1连续调用
    float lastAttack1Time_; // 记录上一次attack1调用的时间
    
    //UI相关
    cocos2d::Sprite* healthBarBorder_; // 血条边框
    cocos2d::ProgressTimer* healthBar_; // Boss血条
    bool isHealthBarCreated_; // 成员变量，标记血条是否已创建
    
    //boss1的行为
    BehaviorResult idle(float delta);
    BehaviorResult recovery(float delta);
    BehaviorResult attack1(float delta);
    BehaviorResult attack2(float delta);
    BehaviorResult attack3(float delta);
    BehaviorResult attack4(float delta);
    BehaviorResult attack5(float delta);
    BehaviorResult turn(float delta);
    
    //UI相关方法
    void initHealthBar();
    void updateHealthBar(float delta);


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
