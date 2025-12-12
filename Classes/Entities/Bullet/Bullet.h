#pragma once
#ifndef __BULLET_H__
#define __BULLET_H__

#include "cocos2d.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

// �ӵ����� - ������
class BulletBase : public Node
{
public:
    BulletBase();
    virtual ~BulletBase();
    
    // ��ʼ���麯��
    virtual bool init();
    
    // ��ײ�ص�����
    virtual bool onContactBegin(PhysicsContact& contact);
    virtual bool onContactSeparate(PhysicsContact& contact);
    
    // Getter �� Setter ����
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
    
    // ���÷��������� (��һ����)
    void setIsPlayerBullet(bool isPlayer) { isPlayerBullet_ = isPlayer; }
    bool getIsPlayerBullet() const { return isPlayerBullet_; }
    
    // �����Ƿ���Դ�ǽ
    void setCanPenetrateWall(bool canPenetrate) { canPenetrateWall_ = canPenetrate; }
    bool getCanPenetrateWall() const { return canPenetrateWall_; }
    
    // �����Ƿ���Է���
    void setCanBounce(bool canBounce) { canBounce_ = canBounce; }
    bool getCanBounce() const { return canBounce_; }
    
    
protected:
    // ����������ײ��
    virtual void setupPhysicsBody();
    
    // ��ײ���С
    float collisionBoxWidth_;
    float collisionBoxHeight_;
    
    // �˺�ֵ
    int damage_;
    
    // ��ײ��������
    uint32_t categoryBitmask_;
    uint32_t contactTestBitmask_;
    uint32_t collisionBitmask_;
    
    // ������
    PhysicsBody* physicsBody_;
    
    // ����������
    bool isPlayerBullet_;
    
    // �Ƿ���Դ�ǽ
    bool canPenetrateWall_;
    
    // �Ƿ���Է���
    bool canBounce_;
    
};

// ��ս�ӵ��� - ��ʱ��ײ��
class MeleeBullet : public BulletBase
{
public:
    CREATE_FUNC(MeleeBullet);
    
    virtual bool init() override;
    
    // ���ù������� (���ھ�����ײ��λ��)
    void setAttackDirection(const Vec2& direction) { attackDirection_ = direction; }
    Vec2 getAttackDirection() const { return attackDirection_; }
    
    // ���ù�������ʱ��
    void setDuration(float duration) { duration_ = duration; }
    float getDuration() const { return duration_; }
    
    // ���ù�����Χ
    void setAttackRange(float range) { attackRange_ = range; }
    float getAttackRange() const { return attackRange_; }
    
private:
    // ��������
    Vec2 attackDirection_;
    
    // ��������ʱ��
    float duration_;
    
    // ������Χ
    float attackRange_;
    
    // ���¼�ʱ��
    float timer_;
};

// Զ���ӵ���
class RangedBullet : public BulletBase
{
public:
    CREATE_FUNC(RangedBullet);
    
    virtual bool init() override;
    
    // Getter �� Setter ����
    float getSpeed() const { return speed_; }
    void setSpeed(float speed);
    
    Vec2 getDirection() const { return direction_; }
    void setDirection(const Vec2& direction);
    
    float getGravityScale() const { return gravityScale_; }
    void setGravityScale(float scale) { gravityScale_ = scale; }
    
    Sprite* getSprite() const;
    void setSprite(Sprite* sprite);
    
    // �����ӵ��켣
    void setTrajectoryType(int type) { trajectoryType_ = type; }
    int getTrajectoryType() const { return trajectoryType_; }
    
private:
    // �����ٶ�
    float speed_;
    
    // ���з���
    Vec2 direction_;
    
    // ������������
    float gravityScale_;
    
    // ������ӻ�����
    Sprite* sprite_;
    
    // �켣���� (0: ֱ��, 1: ������, 2: ���ߵ�)
    int trajectoryType_;
    
    // ���º���
    void update(float delta);
};

#endif // __BULLET_H__