#pragma once
#ifndef __BULLET_H__
#define __BULLET_H__

#include "cocos2d.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

class Bullet : public Node
{
private:
    int damage_;                     // 伤害值
    Sprite* sprite_;                 // 子弹精灵
    bool isVisible_;                 // 是否可视
    PhysicsBody* physicsBody_;       // 物理碰撞体
    std::function<void(Bullet*, float)> updateLogic_;  // 更新逻辑函数指针
    EventListenerPhysicsContact* contactListener_;  // 物理碰撞监听器

    // 碰撞相关属性
    float collisionWidth_;           // 碰撞体宽度
    float collisionHeight_;          // 碰撞体高度
    int categoryBitmask_;            // 类别掩码
    int contactTestBitmask_;         // 接触测试掩码
    int collisionBitmask_;           // 碰撞掩码
    int clearBitmask_;               // 和该掩码下对象碰撞时清除子弹
    
    // 存在时间相关属性
    float existTime_;                // 当前存在时间
    float maxExistTime_;             // 最大存在时间
    
    // 消除标志
    bool isNeedCleanup_;             // 是否需要消除

public:
    // 静态创建方法
    static Bullet* create(const std::string& spriteFrameName, int damage, 
                         const std::function<void(Bullet*, float)>& updateLogic);
    
    // 设置最大存在时间
    void setMaxExistTime(float time) { maxExistTime_ = time; }
    float getMaxExistTime() const { return maxExistTime_; }

    void setExistTime(float time) { existTime_ = time; }
    float getExistTime() const { return existTime_; }

    virtual bool init(const std::string& spriteFrameName, int damage, 
                     const std::function<void(Bullet*, float)>& updateLogic);

    // 更新方法
    virtual void update(float delta) override;
    
    // 碰撞结束回调函数
    bool onContactSeparate(PhysicsContact& contact);

    // 清理方法
    void cleanupBullet();

    // Getter和Setter方法
    int getDamage() const { return damage_; }
    void setDamage(int damage) { damage_ = damage; }

    Sprite* getSprite() const { return sprite_; }

    bool isVisible() const { return isVisible_; }
    void setVisible(bool visible);

    PhysicsBody* getPhysicsBody() const { return physicsBody_; }

    // 碰撞体相关属性的Getter和Setter
    float getCollisionWidth() const { return collisionWidth_; }
    void setCollisionWidth(float width);

    float getCollisionHeight() const { return collisionHeight_; }
    void setCollisionHeight(float height);

    int getCategoryBitmask() const { return categoryBitmask_; }
    void setCategoryBitmask(int bitmask);

    int getContactTestBitmask() const { return contactTestBitmask_; }
    void setContactTestBitmask(int bitmask);

    int getCollisionBitmask() const { return collisionBitmask_; }
    void setCollisionBitmask(int bitmask);

    int getCLearBitmask() const { return clearBitmask_; }
    void setCLearBitmask(int bitmask);

    // 更新逻辑函数指针的Setter
    void setUpdateLogic(const std::function<void(Bullet*, float)>& updateLogic)
    {
        updateLogic_ = updateLogic;
    }

protected:
    // 重新创建物理碰撞体
    void recreatePhysicsBody();
    
    // 注册碰撞监听器
    void registerContactListener();
    
    // 碰撞回调函数
    bool onContactBegin(PhysicsContact& contact);

    Bullet();
    virtual ~Bullet();
};

#endif // __BULLET_H__