#pragma once
#include "cocos2d.h"
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>


// 敌人状态枚举
enum class EnemyState 
{
    IDLE,          // 空闲
    ACTING,        //动作中
    RECOVERY,      // 后摇
    STAGGERED,     // 韧性被清零，硬直状态
    DEAD           //死掉啦
};


// 定义行为返回类型：第一个值表示行为是否完成，第二个值表示后摇持续时间
using BehaviorResult = std::pair<bool, float>;
using Behavior = std::function<BehaviorResult(float delta)>;//定义行为类型

//所有敌人的基类，包括小怪、boss等
class EnemyBase :public cocos2d::Node
{
protected:
    cocos2d::Sprite* sprite_;//敌人的精灵（动画/渲染）
    
    int max_vitality_;//生命值
    int current_vitality_;//当前生命值
    int stagger_resistance_;//韧性
    int current_stagger_resistance_;//当前韧性
    EnemyState currentState_;      // 当前状态
    int base_attack_power_;        // 基础攻击力
    int defense_;             // 防御力
    std::string currentBehavior_;  // 当前执行的行为名称
    float recoveryDuration_;       // 当前后摇持续时间
    float recoveryTimer_;          // 当前后摇计时器
    float staggerDuration_;        // 韧性被清零时的硬直持续时间
    float staggerTimer_;           // 硬直状态计时器

    //init初始化函数
    bool init() override;
    

    std::unordered_map<std::string, Behavior> aiBehaviors_;  // AI行为映射表，通过string作为键名，存储的是行为函数
public:


    //构造函数和析构函数
    EnemyBase();
    virtual ~EnemyBase();

    //update接口：(在本抽象类覆写，具体子类不用覆写）
    void update(float delta) override;


    //以下是每个子类要自行实现的函数（以及记得添加create）
    virtual void Hitted(int damage, int poise_damage = 0) = 0; // 被击中反应
    virtual void Dead() = 0;                                   // 死亡处理
    virtual void BehaviorInit() = 0;//初始化敌人的行为函数
    virtual std::string DecideNextBehavior(float delta) = 0; // 决定下一个行为的虚函数，接收delta参数
    virtual void InitSprite();    // 精灵初始化虚函数，包括大小、图片等

    
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
    
    // Setter方法
    void setMaxVitality(int maxVitality);
    void setCurrentVitality(int currentVitality);
    void setStaggerResistance(int staggerResistance);
    void setCurrentStaggerResistance(int currentStaggerResistance);
    void setCurrentState(EnemyState state);
    void setBaseAttackPower(int attackPower);
    void setDefense(int defense);
    void setStaggerDuration(float duration);
    

    // AI更新方法（封装AI相关逻辑），由update调用
    void updateAI(float delta);

    // AI行为管理方法
    void addBehavior(const std::string& name, const Behavior& behavior);
    void removeBehavior(const std::string& name);
    bool hasBehavior(const std::string& name) const;
    BehaviorResult Execute(const std::string& name, float delta);//执行行为

};