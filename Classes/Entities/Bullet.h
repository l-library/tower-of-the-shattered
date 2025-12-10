#pragma once
#ifndef __BULLET_H__
#define __BULLET_H__

#include "cocos2d.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

// 子弹基类 - 抽象类
class BulletBase : public Node
{
public:
    BulletBase();
    virtual ~BulletBase();
    
    // 初始化虚函数
    virtual bool init();
    
    // 碰撞回调函数
    virtual bool onContactBegin(PhysicsContact& contact);
    virtual bool onContactSeparate(PhysicsContact& contact);
    
    // Getter 和 Setter 方法
    float getCollisionBoxWidth() const { return collisionBoxWidth_; }
    void setCollisionBoxWidth(float width) { collisionBoxWidth_ = width; }
    
    float getCollisionBoxHeight() const { return collisionBoxHeight_; }
    void setCollisionBoxHeight(float height) { collisionBoxHeight_ = height; }
    
    int getDamage() const { return damage_; }
    void setDamage(int damage) { damage_ = damage; }
    
    uint32_t getCategoryBitmask() const { return categoryBitmask_; }
    void setCategoryBitmask(uint32_t mask) { categoryBitmask_ = mask; }
    
    uint32_t getContactTestBitmask() const { return contactTestBitmask_; }
    void setContactTestBitmask(uint32_t mask) { contactTestBitmask_ = mask; }
    
    uint32_t getCollisionBitmask() const { return collisionBitmask_; }
    void setCollisionBitmask(uint32_t mask) { collisionBitmask_ = mask; }
    
    // 设置发射者类型 (玩家或敌人)
    void setIsPlayerBullet(bool isPlayer) { isPlayerBullet_ = isPlayer; }
    bool getIsPlayerBullet() const { return isPlayerBullet_; }
    
    // 设置是否可以穿墙
    void setCanPenetrateWall(bool canPenetrate) { canPenetrateWall_ = canPenetrate; }
    bool getCanPenetrateWall() const { return canPenetrateWall_; }
    
    // 设置是否可以反弹
    void setCanBounce(bool canBounce) { canBounce_ = canBounce; }
    bool getCanBounce() const { return canBounce_; }
    
protected:
    // 创建物理碰撞箱
    virtual void setupPhysicsBody();
    
    // 碰撞箱大小
    float collisionBoxWidth_;
    float collisionBoxHeight_;
    
    // 伤害值
    int damage_;
    
    // 碰撞过滤掩码
    uint32_t categoryBitmask_;
    uint32_t contactTestBitmask_;
    uint32_t collisionBitmask_;
    
    // 物理体
    PhysicsBody* physicsBody_;
    
    // 发射者类型
    bool isPlayerBullet_;
    
    // 是否可以穿墙
    bool canPenetrateWall_;
    
    // 是否可以反弹
    bool canBounce_;
};

// 近战子弹类 - 临时碰撞框
class MeleeBullet : public BulletBase
{
public:
    CREATE_FUNC(MeleeBullet);
    
    virtual bool init() override;
    
    // 设置攻击方向 (用于决定碰撞箱位置)
    void setAttackDirection(const Vec2& direction) { attackDirection_ = direction; }
    Vec2 getAttackDirection() const { return attackDirection_; }
    
    // 设置攻击持续时间
    void setDuration(float duration) { duration_ = duration; }
    float getDuration() const { return duration_; }
    
    // 设置攻击范围
    void setAttackRange(float range) { attackRange_ = range; }
    float getAttackRange() const { return attackRange_; }
    
private:
    // 攻击方向
    Vec2 attackDirection_;
    
    // 攻击持续时间
    float duration_;
    
    // 攻击范围
    float attackRange_;
    
    // 更新计时器
    float timer_;
};

// 远程子弹类
class RangedBullet : public BulletBase
{
public:
    CREATE_FUNC(RangedBullet);
    
    virtual bool init() override;
    
    // Getter 和 Setter 方法
    float getSpeed() const { return speed_; }
    void setSpeed(float speed);
    
    Vec2 getDirection() const { return direction_; }
    void setDirection(const Vec2& direction);
    
    float getGravityScale() const { return gravityScale_; }
    void setGravityScale(float scale) { gravityScale_ = scale; }
    
    Sprite* getSprite() const { return sprite_; }
    void setSprite(Sprite* sprite) { sprite_ = sprite; }
    
    // 设置子弹轨迹
    void setTrajectoryType(int type) { trajectoryType_ = type; }
    int getTrajectoryType() const { return trajectoryType_; }
    
private:
    // 飞行速度
    float speed_;
    
    // 飞行方向
    Vec2 direction_;
    
    // 重力缩放因子
    float gravityScale_;
    
    // 精灵可视化部分
    Sprite* sprite_;
    
    // 轨迹类型 (0: 直线, 1: 抛物线, 2: 曲线等)
    int trajectoryType_;
    
    // 更新函数
    void update(float delta);
};

#endif // __BULLET_H__