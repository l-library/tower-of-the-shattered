#include "Boss1.h"
#include "../EnemyAi/EnemyAi.h"
#include "TowerOfTheShattered.h"
#include "cocos2d.h"

using namespace cocos2d;

Boss1::Boss1()
{
    // 构造函数
}

Boss1::~Boss1()
{
    // 析构函数
}

Boss1* Boss1::create()
{
    Boss1* boss = new Boss1();
    if (boss && boss->init())
    {
        boss->autorelease();
        return boss;
    }
    else
    {
        delete boss;
        boss = nullptr;
        return nullptr;
    }
}

bool Boss1::init()
{
    if (!EnemyBase::init())
    {
        return false;
    }
    
    // 设置Boss1的基本属性
    this->setMaxVitality(1000); // 高生命值
    this->setCurrentVitality(1000);
    this->setStaggerResistance(500); // 高韧性
    this->setBaseAttackPower(50); // 高攻击力
    this->setDefense(20); // 高防御力
    
    // 设置碰撞箱信息
    CollisionBoxInfo collisionInfo;
    collisionInfo.width = GRID_SIZE * 4; // 更大的碰撞箱
    collisionInfo.height = GRID_SIZE * 4;
    collisionInfo.categoryBitmask = ENEMY_MASK;
    collisionInfo.contactTestBitmask = PLAYER_MASK | WALL_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.collisionBitmask = WALL_MASK | PLAYER_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.isDynamic = true;
    collisionInfo.mass = 10.0f; // 更大的质量
    this->setCollisionBoxInfo(collisionInfo);
    
    // 初始化精灵
    this->InitSprite();
    
    // 初始化物理碰撞体
    this->InitPhysicsBody();
    
    // 初始化行为
    this->BehaviorInit();
    
    return true;
}

void Boss1::Hitted(int damage, int poise_damage)
{
    // 扣除生命值
    int actualDamage = damage - this->getDefense();
    if (actualDamage < 1) actualDamage = 1;
    this->setCurrentVitality(this->getCurrentVitality() - actualDamage);
    
    // 处理破韧
    int currentStagger = this->getCurrentStaggerResistance();
    currentStagger -= poise_damage;
    this->setCurrentStaggerResistance(currentStagger);
    
    // 被击中时的视觉效果
    if (sprite_ != nullptr)
    {
        auto redAction = TintTo::create(0.1f, 255, 0, 0);
        auto restoreAction = TintTo::create(0.2f, 255, 255, 255);
        sprite_->runAction(Sequence::create(redAction, restoreAction, nullptr));
    }
    
    // 如果生命值为0，进入死亡状态
    if (this->getCurrentVitality() <= 0)
    {
        this->Dead();
    }
    // 如果韧性为0，进入被击晕状态
    else if (this->getCurrentStaggerResistance() <= 0)
    {
        this->setCurrentState(EnemyState::STAGGERED);
        this->setCurrentStaggerResistance(this->getStaggerResistance());
    }
}

void Boss1::Dead()
{
    // 设置死亡状态
    this->setCurrentState(EnemyState::DEAD);
    
    // 死亡视觉效果
    if (getSprite()) {
        auto fadeOut = FadeOut::create(2.0f);
        auto scaleDown = ScaleTo::create(2.0f, 0.1f);
        auto spawn = Spawn::create(fadeOut, scaleDown, nullptr);
        getSprite()->runAction(spawn);
    }
    
    // 立即移除碰撞箱
    if (physicsBody_ != nullptr)
    {
        this->removeComponent(physicsBody_);
        physicsBody_ = nullptr;
    }
}

void Boss1::BehaviorInit()
{
    // 注册行为函数
    this->addBehavior("idle", [this](float delta) { return this->idle(delta); });
    this->addBehavior("recovery", [this](float delta) { return this->recovery(delta); });
    this->addBehavior("attack1", [this](float delta) { return this->attack1(delta); });
    this->addBehavior("attack2", [this](float delta) { return this->attack2(delta); });
    this->addBehavior("attack3", [this](float delta) { return this->attack3(delta); });
    
    // 设置初始行为
    this->currentBehavior_ = "idle";
}

std::string Boss1::DecideNextBehavior(float delta)
{
    // 查找玩家
    this->setPlayer(EnemyAi::findPlayer(this));
    
    // 如果检测到玩家
    if (this->getPlayer() != nullptr)
    {

        // 简单的随机攻击选择
        int random = rand() % 3;
        if (random == 0)
        {
            return "attack1";
        }
        else if (random == 1)
        {
            return "attack2";
        }
        else
        {
            return "attack3";
        }
    }
    
    // 默认返回待机行为
    return "idle";
}

void Boss1::InitSprite()
{
    // 初始化Boss1的精灵
    sprite_ = Sprite::create("HelloWorld.png"); // 暂时使用现有图片
    if (sprite_ != nullptr)
    {
        // 设置精灵大小
        sprite_->setContentSize(Size(GRID_SIZE * 4, GRID_SIZE * 4));
        sprite_->setPosition(Vec2::ZERO); // 设置精灵在节点中心
        this->addChild(sprite_);
    }
}

// 行为方法的基本实现
BehaviorResult Boss1::idle(float delta)
{
        return { false, 0.0f };
}

BehaviorResult Boss1::recovery(float delta)
{
    return { false, 0.0f };
}

BehaviorResult Boss1::attack1(float delta)
{
    return { false, 0.0f };
}

BehaviorResult Boss1::attack2(float delta)
{
    return { false, 0.0f };
}

BehaviorResult Boss1::attack3(float delta)
{
    return { false, 0.0f };
}

// 碰撞回调函数
bool Boss1::onContactBegin(PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    // 确定当前Boss节点和对方节点
    Node* bossNode = nullptr;
    Node* otherNode = nullptr;
    
    if (nodeA == this)
    {
        bossNode = nodeA;
        otherNode = nodeB;
    }
    else if (nodeB == this)
    {
        bossNode = nodeB;
        otherNode = nodeA;
    }
    
    //需完善碰撞逻辑

    if (bossNode == nullptr || otherNode == nullptr)
    {
        return true;
    }
    
    // 处理与玩家子弹的碰撞
    if (otherNode->getPhysicsBody()->getCategoryBitmask() == PLAYER_BULLET_MASK)
    {
        Bullet* bullet = dynamic_cast<Bullet*>(otherNode);
        if (bullet != nullptr)
        {
            Hitted(bullet->getDamage(), bullet->getDamage() * 2); // 子弹造成双倍韧性伤害
        }
        return true;
    }
    
    return EnemyBase::onContactBegin(contact);
}

bool Boss1::onContactSeparate(PhysicsContact& contact)
{
    return EnemyBase::onContactSeparate(contact);
}
