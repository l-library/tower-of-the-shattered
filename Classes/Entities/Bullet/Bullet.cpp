#include "Bullet.h"

USING_NS_CC;

Bullet::Bullet()
    : damage_(0)
    , sprite_(nullptr)
    , isVisible_(true)
    , physicsBody_(nullptr)
    , updateLogic_(nullptr)
    , contactListener_(nullptr)
    , collisionWidth_(0.0f)
    , collisionHeight_(0.0f)
    , categoryBitmask_(BULLET_MASK)
    , contactTestBitmask_(PLAYER_MASK | ENEMY_MASK | WALL_MASK)
    , collisionBitmask_(BORDER_MASK | WALL_MASK | DAMAGE_WALL_MASK)
    , clearBitmask_(0)
    , existTime_(0.0f)
    , maxExistTime_(5.0f) // ?????????????5??
    , isNeedCleanup_(false)
{
}

Bullet::~Bullet()
{
    // ??????
    if (sprite_) {
        sprite_->release();
    }
    if (physicsBody_) {
        physicsBody_->release();
    }
    if (contactListener_) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(contactListener_);
        contactListener_->release();
    }
}

Bullet* Bullet::create(const std::string& spriteFrameName, int damage, 
                      const std::function<void(Bullet*, float)>& updateLogic)
{
    Bullet* bullet = new (std::nothrow) Bullet();
    if (bullet && bullet->init(spriteFrameName, damage, updateLogic)) {
        bullet->autorelease();
        return bullet;
    }
    CC_SAFE_DELETE(bullet);
    return nullptr;
}

bool Bullet::init(const std::string& spriteFrameName, int damage, 
                 const std::function<void(Bullet*, float)>& updateLogic)
{
    if (!Node::init()) {
        return false;
    }

    // ????????
    damage_ = damage;

    // ????????
    sprite_ = Sprite::create(spriteFrameName);
    if (!sprite_) {
        CCLOG("Failed to create bullet sprite with frame name: %s", spriteFrameName.c_str());
        return false;
    }
    sprite_->retain();
    sprite_->setContentSize(Size(GRID_SIZE, GRID_SIZE));
    sprite_->setPosition(Vec2::ZERO); // ???????????????
    this->addChild(sprite_);
    // ?????????§³????????§³
    collisionWidth_ = sprite_->getContentSize().width;
    collisionHeight_ = sprite_->getContentSize().height;

    // ?????????????
    recreatePhysicsBody();

    // ??????????
    updateLogic_ = updateLogic;

    // ????????????
    registerContactListener();

    // ????????
    this->scheduleUpdate();

    return true;
}

void Bullet::update(float delta)
{
    Node::update(delta);

    // ??????????
    existTime_ += delta;
    
    // ??????????????
    if (isNeedCleanup_ || existTime_ >= maxExistTime_) {
        if (isNeedCleanup_) {
            CCLOG("Bullet cleanup by collision!");
        } else {
            CCLOG("Bullet expired by exist time!");
        }
        cleanupBullet();
        return;
    }

    // ??????????????
    if (updateLogic_) {
        updateLogic_(this, delta);
    }
}

void Bullet::setVisible(bool visible)
{
    isVisible_ = visible;
    if (sprite_) {
        sprite_->setVisible(visible);
    }
    if (physicsBody_) {
        // ????????I???????????
        physicsBody_->setEnabled(visible);
    }
}

void Bullet::setCollisionWidth(float width)
{
    if (collisionWidth_ != width) {
        collisionWidth_ = width;
        recreatePhysicsBody();
    }
}

void Bullet::setCollisionHeight(float height)
{
    if (collisionHeight_ != height) {
        collisionHeight_ = height;
        recreatePhysicsBody();
    }
}

void Bullet::setCategoryBitmask(int bitmask)
{
    if (categoryBitmask_ != bitmask) {
        categoryBitmask_ = bitmask;
        if (physicsBody_) {
            physicsBody_->setCategoryBitmask(bitmask);
        }
    }
}

void Bullet::setContactTestBitmask(int bitmask)
{
    if (contactTestBitmask_ != bitmask) {
        contactTestBitmask_ = bitmask;
        if (physicsBody_) {
            physicsBody_->setContactTestBitmask(bitmask);
        }
    }
}

void Bullet::setCollisionBitmask(int bitmask)
{
    if (collisionBitmask_ != bitmask) {
        collisionBitmask_ = bitmask;
        if (physicsBody_) {
            physicsBody_->setCollisionBitmask(bitmask);
        }
    }
}

void Bullet::setCLearBitmask(int bitmask)
{
    if (clearBitmask_ != bitmask)
        clearBitmask_ = bitmask;
}

void Bullet::recreatePhysicsBody()
{
    // ???????????????
    if (physicsBody_) {
        this->removeComponent(physicsBody_);
        physicsBody_->release();
        physicsBody_ = nullptr;
    }

    // ????????????????
    physicsBody_ = PhysicsBody::createBox(Size(collisionWidth_, collisionHeight_), 
                                         PhysicsMaterial(0.0f, 0.0f, 0.0f));
    if (!physicsBody_) {
        CCLOG("Failed to create physics body for bullet");
        return;
    }

    physicsBody_->retain();
    physicsBody_->setDynamic(true);
    physicsBody_->setGravityEnable(false);
    physicsBody_->setLinearDamping(0.0f);
    physicsBody_->setAngularDamping(0.0f);
    physicsBody_->setRotationEnable(false);

    // ???????????
    physicsBody_->setCategoryBitmask(categoryBitmask_);
    physicsBody_->setContactTestBitmask(contactTestBitmask_);
    physicsBody_->setCollisionBitmask(collisionBitmask_);

    // ????????????Þ???
    this->addComponent(physicsBody_);
}

void Bullet::registerContactListener()
{
    // ?????????????
    contactListener_ = EventListenerPhysicsContact::create();
    if (!contactListener_) {
        CCLOG("Failed to create contact listener for bullet");
        return;
    }

    // ????????????
    contactListener_->onContactBegin = CC_CALLBACK_1(Bullet::onContactBegin, this);
    
    // ?????????????
    contactListener_->onContactSeparate = CC_CALLBACK_1(Bullet::onContactSeparate, this);

    // ???????????????????
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener_, this);
    contactListener_->retain();
}

bool Bullet::onContactBegin(PhysicsContact& contact)
{
    PhysicsBody* bodyA = contact.getShapeA()->getBody();
    PhysicsBody* bodyB = contact.getShapeB()->getBody();
    
    // ??????????p??????
    if (bodyA == physicsBody_ || bodyB == physicsBody_) {
        PhysicsBody* otherBody = (bodyA == physicsBody_) ? bodyB : bodyA;
        
        if (otherBody->getCategoryBitmask() & clearBitmask_) {
            CCLOG("Bullet collided clear!");
            isNeedCleanup_ = true;
            return true;
        }
        
        // ??????????????????????????
        CCLOG("Bullet collided with non-Player object!");
    }

    return true;
}

bool Bullet::onContactSeparate(PhysicsContact& contact)
{
    PhysicsBody* bodyA = contact.getShapeA()->getBody();
    PhysicsBody* bodyB = contact.getShapeB()->getBody();
    
    // ??????????p??????
    if (bodyA == physicsBody_ || bodyB == physicsBody_) {
        CCLOG("Bullet separated from object!");
        return true;
    }
    
    return true;
}

void Bullet::cleanupBullet()
{
    // ??????
    this->unscheduleUpdate();

    // ????????????
    if (contactListener_) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(contactListener_);
        contactListener_->release();
        contactListener_ = nullptr;
    }

    // ????????????
    if (physicsBody_) {
        this->removeComponent(physicsBody_);
        physicsBody_->release();
        physicsBody_ = nullptr;
    }

    // ???????
    if (sprite_) {
        this->removeChild(sprite_, true);
        sprite_->release();
        sprite_ = nullptr;
    }

    // ????????????
    if (this->getParent()) {
        this->getParent()->removeChild(this, true);
    }
}