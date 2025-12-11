#pragma once
#include "cocos2d.h"
#include "Entities/Player/Player.h" // 包含Player类头文件，用于typeid检测
#include "Entities/Bullet/Bullet.h" // 包含Bullet类头文件，用于创建MeleeBullet
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

// 前向声明Player类
class Player;


// ����״̬ö��
enum class EnemyState 
{
    IDLE,          // ����
    ACTING,        //������
    RECOVERY,      // ��ҡ
    STAGGERED,     // ���Ա����㣬Ӳֱ״̬
    DEAD           //������
};


// ������Ϊ�������ͣ���һ��ֵ��ʾ��Ϊ�Ƿ���ɣ��ڶ���ֵ��ʾ��ҡ����ʱ��
using BehaviorResult = std::pair<bool, float>;
using Behavior = std::function<BehaviorResult(float delta)>;//������Ϊ����

// 碰撞箱相关信息结构体
typedef struct CollisionBoxInfo {
    float width;          // 碰撞箱宽度
    float height;         // 碰撞箱高度
    int categoryBitmask;  // 碰撞类别掩码
    int contactTestBitmask; // 碰撞检测掩码
    int collisionBitmask; // 碰撞反应掩码
    bool isDynamic;       // 是否为动态碰撞体
    float mass;           // 质量
} CollisionBoxInfo;

//���е��˵Ļ��࣬����С�֡�boss��
class EnemyBase :public cocos2d::Node
{
protected:
    cocos2d::Sprite* sprite_;//���˵ľ��飨����/��Ⱦ��
    cocos2d::PhysicsBody* physicsBody_; // ������ײ��
    CollisionBoxInfo collisionBoxInfo_; // 碰撞箱相关信息
    
    int max_vitality_;//����ֵ
    int current_vitality_;//��ǰ����ֵ
    int stagger_resistance_;//����
    int current_stagger_resistance_;//��ǰ����
    EnemyState currentState_;      // ��ǰ״̬
    int base_attack_power_;        // ����������
    int defense_;             // ������
    std::string currentBehavior_;  // ��ǰִ�е���Ϊ����
    float recoveryDuration_;       // ��ǰ��ҡ����ʱ��
    float recoveryTimer_;          // ��ǰ��ҡ��ʱ��
    float staggerDuration_;        // ���Ա�����ʱ��Ӳֱ����ʱ��
    float staggerTimer_;           // Ӳֱ״̬��ʱ��
    Player* player_;               // 指向玩家的指针

    //init��ʼ������
    bool init() override;
    
    // ��ײ���ص�����
    virtual bool onContactBegin(cocos2d::PhysicsContact& contact);
    virtual bool onContactSeparate(cocos2d::PhysicsContact& contact);

    std::unordered_map<std::string, Behavior> aiBehaviors_;  // AI��Ϊӳ�����ͨ��string��Ϊ�������洢������Ϊ����
public:


    //���캯������������
    EnemyBase();
    virtual ~EnemyBase();

    //update�ӿڣ�(�ڱ������าд���������಻�ø�д��
    void update(float delta) override;


    //������ÿ������Ҫ����ʵ�ֵĺ������Լ��ǵ�����create��
    virtual void Hitted(int damage, int poise_damage = 0) = 0; // �����з�Ӧ
    virtual void Dead() = 0;                                   // ��������
    virtual void BehaviorInit() = 0;//��ʼ�����˵���Ϊ����
    virtual std::string DecideNextBehavior(float delta) = 0; // ������һ����Ϊ���麯��������delta����
    virtual void InitSprite();    // �����ʼ���麯����������С��ͼƬ��
    
    // ��ײ��ص�Getter����
    cocos2d::PhysicsBody* getPhysicsBody() const;
    
    // 初始化物理碰撞体（非虚函数）
    void InitPhysicsBody();

    
    // Getter����
    cocos2d::Sprite* getSprite() const;
    int getMaxVitality() const;
    int getCurrentVitality() const;
    int getStaggerResistance() const;
    int getCurrentStaggerResistance() const;
    EnemyState getCurrentState() const;
    int getBaseAttackPower() const;
    int getDefense() const;
    float getStaggerDuration() const;
    CollisionBoxInfo getCollisionBoxInfo() const;
    
    // 检测玩家是否可见（路径上没有墙）
    bool isPlayerVisible();
    
    // 设置玩家指针
    void setPlayer(Player* player);
    
    // 获取玩家指针
    Player* getPlayer() const;
    
    // Setter����
    void setCollisionBoxInfo(const CollisionBoxInfo& info);
    
    // Setter����
    void setMaxVitality(int maxVitality);
    void setCurrentVitality(int currentVitality);
    void setStaggerResistance(int staggerResistance);
    void setCurrentStaggerResistance(int currentStaggerResistance);
    void setCurrentState(EnemyState state);
    void setBaseAttackPower(int attackPower);
    void setDefense(int defense);
    void setStaggerDuration(float duration);
    

    // AI���·�������װAI����߼�������update����
    void updateAI(float delta);

    // AI��Ϊ��������
    void addBehavior(const std::string& name, const Behavior& behavior);
    void removeBehavior(const std::string& name);
    bool hasBehavior(const std::string& name) const;
    BehaviorResult Execute(const std::string& name, float delta);//ִ����Ϊ

};

// 小兵抽象类 - 继承自EnemyBase
class SoldierEnemyBase : public EnemyBase
{
protected:
    // 小兵特有的属性和方法
    
public:
    
    virtual bool init() override;
    
    // 实现父类的纯虚函数
    virtual void Hitted(int damage, int poise_damage = 0) override = 0;
    virtual void Dead() override = 0;
    virtual void BehaviorInit() override = 0;
    virtual std::string DecideNextBehavior(float delta) override = 0;
    virtual void InitSprite() override = 0;
    
    // 实现碰撞回调函数
    virtual bool onContactBegin(cocos2d::PhysicsContact& contact) override;
    virtual bool onContactSeparate(cocos2d::PhysicsContact& contact) override;
};
