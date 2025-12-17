#include "Boss1.h"
#include "../EnemyAi/EnemyAi.h"
#include "../../Bullet/Bullet.h"
#include "TowerOfTheShattered.h"
#include "cocos2d.h"

using namespace cocos2d;

Boss1::Boss1()
{
    // 初始化待机动画相关变量
    idleAnimation_ = nullptr;
    isIdleAnimationPlaying_ = false;
    spriteParent_ = nullptr;
    
    // 初始化attack1相关变量
    isAttack1Active_ = false;
    bulletCount_ = 0;
    bulletTimer_ = -10.0f;
    hasTeleported_ = false;
    initialPosition_ = Vec2::ZERO;
    
    // 初始化attack2相关变量
    isAttack2Active_ = false;
    attack2Timer_ = 0.0f;
    attack2State_ = 0;
    
    // 初始化attack3相关变量
    isAttack3Active_ = false;
    hasSummonedClone_ = false;
    clonePosition_ = Vec2::ZERO;
    clone_ = nullptr;
    isClone_ = false;
}

Boss1::~Boss1()
{
    // 释放待机动画资源
    if (idleAnimation_ != nullptr)
    {
        idleAnimation_->release();
        idleAnimation_ = nullptr;
    }
    
    // 释放其他资源
    spriteParent_ = nullptr;
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
    this->setDefense(10); // 高防御力
    
    // 设置碰撞箱信息
    CollisionBoxInfo collisionInfo;
    collisionInfo.width = GRID_SIZE * 1.5;
    collisionInfo.height = GRID_SIZE * 3;
    collisionInfo.categoryBitmask = ENEMY_MASK;
    collisionInfo.contactTestBitmask = PLAYER_MASK | WALL_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.collisionBitmask = WALL_MASK | PLAYER_MASK | BORDER_MASK | PLAYER_BULLET_MASK;
    collisionInfo.isDynamic = true;
    collisionInfo.mass = 10000.0f; // 它应不能被推动
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
        if (isClone_) {
            // 分身快速死亡，立即清除
            auto fadeOut = FadeOut::create(0.5f);
            auto scaleDown = ScaleTo::create(0.5f, 0.1f);
            auto removeSelf = RemoveSelf::create();
            auto sequence = Sequence::create(Spawn::create(fadeOut, scaleDown, nullptr), removeSelf, nullptr);
            getSprite()->runAction(sequence);
        } else {
            // 本体正常死亡动画
            auto fadeOut = FadeOut::create(2.0f);
            auto scaleDown = ScaleTo::create(2.0f, 0.1f);
            auto spawn = Spawn::create(fadeOut, scaleDown, nullptr);
            getSprite()->runAction(spawn);
        }
    }
    
    // 立即移除碰撞箱
    if (physicsBody_ != nullptr)
    {
        this->removeComponent(physicsBody_);
        physicsBody_ = nullptr;
    }
    
    // 如果是分身，立即停止所有行为和调度器
    if (isClone_) {
        this->unscheduleAllCallbacks();
        this->stopAllActions();
        // 延迟移除节点，确保动画完成
        this->scheduleOnce([this](float dt) {
            this->removeFromParentAndCleanup(true);
        }, 0.5f, "remove_self");
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
    this->addBehavior("turn", [this](float delta) { return this->turn(delta); });
    
    // 设置初始行为
    this->currentBehavior_ = "idle";
}

std::string Boss1::DecideNextBehavior(float delta)
{
    if (isClone_)
        return "attack2";
    if (currentBehavior_ == "idle")
        return "attack3";
    // 默认返回待机行为
    return "idle";
}

void Boss1::InitSprite()
{
    // 加载精灵帧缓存
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Enemy/Boss1/idle.plist");
    
    // 创建精灵帧数组
    Vector<SpriteFrame*> idleFrames;
    for (int i = 0; i < 11; ++i)
    {
        std::string frameName = StringUtils::format("idle_%d.png", i);
        SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            idleFrames.pushBack(frame);
        }
    }
    
    // 创建待机动画
    idleAnimation_ = Animation::createWithSpriteFrames(idleFrames, 0.1f); // 每帧0.1秒
    idleAnimation_->retain();
    
    // 创建一个父节点来包含精灵，这样可以通过移动父节点来实现位置偏移
    spriteParent_ = Node::create();
    spriteParent_->setPosition(Vec2::ZERO - Vec2(GRID_SIZE*1.5, 0)); // 设置父节点的偏移位置
    this->addChild(spriteParent_);
    
    // 创建精灵
    sprite_ = Sprite::createWithSpriteFrameName("idle_0.png");
    if (sprite_ != nullptr)
    {
        // 获取原始精灵大小
        Size originalSize = sprite_->getContentSize();
        // 计算缩放比例，将宽度调整为1.5
        float scaleFactor = GRID_SIZE*3 / originalSize.height;
        
        // 使用缩放比例控制精灵大小，这样动画播放时也会保持缩放
        sprite_->setScale(scaleFactor);
        sprite_->setPosition(Vec2::ZERO); // 精灵相对于父节点居中
        spriteParent_->addChild(sprite_);
        
        // 初始化动画状态
        isIdleAnimationPlaying_ = false;
        sprite_->setVisible(false);
    }
}

// 转向行为：立即使boss面对玩家方向
BehaviorResult Boss1::turn(float delta)
{
    // 使用EnemyAi类查找玩家
    Player* player = EnemyAi::findPlayer(this);
    
    // 如果找到玩家且精灵存在
    if (player != nullptr && sprite_ != nullptr)
    {
        // 获取玩家位置和boss位置
        Vec2 playerPos = player->getPosition();
        Vec2 bossPos = this->getPosition();
        
        // 判断玩家在boss的左侧还是右侧
        bool playerOnLeft = playerPos.x < bossPos.x;
        
        // 获取精灵的父节点
        Node* spriteParent = sprite_->getParent();
        if (spriteParent != nullptr)
        {
            // 根据玩家位置调整精灵的翻转和父节点位置
            if (playerOnLeft)
            {
                // 玩家在左侧，boss向左看
                sprite_->setFlippedX(true);
                // 精灵在boss右侧偏移
                spriteParent->setPosition(Vec2::ZERO + Vec2(GRID_SIZE*1.5, 0));
            }
            else
            {
                // 玩家在右侧，boss向右看
                sprite_->setFlippedX(false);
                // 精灵在boss左侧偏移
                spriteParent->setPosition(Vec2::ZERO - Vec2(GRID_SIZE*1.5, 0));
            }
        }
    }
    
    // 转向行为立即完成
    return { true, 0.0f };
}

// 行为方法的基本实现
BehaviorResult Boss1::idle(float delta)
{
    
    // 如果精灵存在且待机动画未播放
    if (sprite_ != nullptr && idleAnimation_ != nullptr && !isIdleAnimationPlaying_)
    {
        // 创建动画动作
        auto animateAction = Animate::create(idleAnimation_);
        auto repeatAction = RepeatForever::create(animateAction);
        
        // 显示精灵并播放动画
        sprite_->setVisible(true);
        sprite_->runAction(repeatAction);
        
        // 更新动画状态
        isIdleAnimationPlaying_ = true;
    }
    
    return { false, 0.0f };
}

BehaviorResult Boss1::recovery(float delta)
{
    // 如果精灵存在
    if (sprite_ != nullptr)
    {
        // 如果待机动画未播放，播放动画
        if (idleAnimation_ != nullptr && !isIdleAnimationPlaying_)
        {
            // 创建动画动作
            auto animateAction = Animate::create(idleAnimation_);
            auto repeatAction = RepeatForever::create(animateAction);

            // 显示精灵并播放动画
            sprite_->setVisible(true);
            sprite_->runAction(repeatAction);

        }
    }

    return { false, 0.0f };
}

BehaviorResult Boss1::attack1(float delta)
{
    // 停止待机动画
    if (sprite_ != nullptr && isIdleAnimationPlaying_)
    {
        sprite_->stopAllActions();
        isIdleAnimationPlaying_ = false;
    }

    // 如果攻击还未激活，初始化攻击
    if (!isAttack1Active_)
    {
        isAttack1Active_ = true;
        bulletCount_ = 0;
        hasTeleported_ = false;

        // 保存初始位置
        initialPosition_ = this->getPosition();

        // 关闭重力，使Boss可以浮空
        if (physicsBody_ != nullptr)
        {
            physicsBody_->setGravityEnable(false);
        }

        // 第一次闪光1秒
        if (sprite_ != nullptr)
        {
            auto blink = Blink::create(1.0f, 5); // 1秒内闪烁5次
            sprite_->runAction(blink);
        }

        // 1秒后升空
        this->scheduleOnce([this](float dt) {
            // 瞬移到距离地面10个GRID_SIZE的高度
            Vec2 currentPos = this->getPosition();
            float newY = 10 * GRID_SIZE;
            this->setPosition(Vec2(currentPos.x, newY));
            hasTeleported_ = true;

            // 切换到clone动画
            if (sprite_ != nullptr)
            {
                // 停止当前所有动作
                sprite_->stopAllActions();

                // 设置精灵纹理为clone.png
                sprite_->setTexture("Enemy/Boss1/clone.png");

                // 使用EnemyAi类查找玩家
                Player* player = EnemyAi::findPlayer(this);
                if (player != nullptr)
                {
                    // 根据玩家位置判断是否翻转
                    Vec2 playerPos = player->getPosition();
                    Vec2 bossPos = this->getPosition();
                    if (playerPos.x < bossPos.x)
                    {
                        // 如果玩家在Boss左侧，翻转精灵
                        sprite_->setFlippedX(true);
                    }
                    else
                    {
                        // 如果玩家在Boss右侧，不翻转或恢复翻转
                        sprite_->setFlippedX(false);
                    }
                }
            }

            // 再次1秒后开始发射子弹
            this->scheduleOnce([this](float dt) {
                // 设置第一次发射子弹的计时器
                bulletTimer_ = 0.0f;
                }, 1.0f, "second_flash_end");
            }, 1.0f, "first_flash_end");
    }

    // 如果已经完成所有闪光和升空，处理子弹发射和实时翻转
    if (hasTeleported_)
    {
        // 实时根据玩家位置翻转clone动画
        if (sprite_ != nullptr)
        {
            Player* player = EnemyAi::findPlayer(this);
            if (player != nullptr)
            {
                // 根据玩家位置判断是否翻转（与子弹逻辑一致）
                Vec2 playerPos = player->getPosition();
                Vec2 bossPos = this->getPosition();
                if (playerPos.x < bossPos.x)
                {
                    // 如果玩家在Boss左侧，翻转精灵
                    sprite_->setFlippedX(true);
                }
                else
                {
                    // 如果玩家在Boss右侧，不翻转或恢复翻转
                    sprite_->setFlippedX(false);
                }
            }
        }

        // 处理子弹发射
        if (bulletTimer_ != -1.0f)
        {
            bulletTimer_ += delta;

            // 每0.5秒发射一次子弹
            if (bulletTimer_ >= 0.5f && bulletCount_ < 5)
            {
                this->fireBullet1();
                bulletTimer_ = 0.0f;
                bulletCount_++;
            }

            // 如果已经发射了5次子弹，结束攻击
            if (bulletCount_ >= 5)
            {
                isAttack1Active_ = false;
                bulletTimer_ = -1.0f; // 重置计时器

                // 恢复重力
                if (physicsBody_ != nullptr)
                {
                    physicsBody_->setGravityEnable(true);
                }

                // 闪现回升空前位置
                this->setPosition(initialPosition_);

                // 切换回原始精灵
                if (sprite_ != nullptr)
                {
                    // 从精灵帧缓存中获取初始帧，而不是直接设置纹理
                    SpriteFrame* initialFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("idle_0.png");
                    if (initialFrame != nullptr)
                    {
                        sprite_->setSpriteFrame(initialFrame);
                    }
                    // 确保精灵可见
                    sprite_->setVisible(true);
                }

                // 重置动画状态，以便在recovery时能正确播放动画
                isIdleAnimationPlaying_ = false;


                // 如果是分身，在攻击结束后自我消灭
                if (isClone_)
                {
                    this->Dead();
                }

                return  { true, 5.0f };
            }
        }

        return { false, 0.0f };
    }
}

void Boss1::fireBullet1()
{
    // 使用EnemyAi类查找玩家
    Player* player = EnemyAi::findPlayer(this);
    if (player == nullptr) return;
    
    // 计算子弹移动方向
    Vec2 playerPos = player->getPosition();
    Vec2 bossPos = this->getPosition();
    Vec2 direction = playerPos - bossPos;
    direction.normalize();
    
    // 设置子弹移动速度
    float speed = 300.0f;
    Vec2 velocity = direction * speed;
    
    // 使用Bullet类创建子弹
    auto bullet = Bullet::create("Enemy/Boss1/clone.png", this->getBaseAttackPower(), 
        [velocity](Bullet* bullet, float delta) {
            // 更新逻辑：使用固定速度移动
            bullet->setPosition(bullet->getPosition() + velocity * delta);
        });
    
    if (bullet == nullptr) return;
    
    // 设置子弹位置为Boss的当前位置
    bullet->setPosition(this->getPosition());
    
    // 设置子弹大小
    auto bulletSprite = bullet->getSprite();
    if (bulletSprite != nullptr)
    {
        bulletSprite->setScale(GRID_SIZE * 4 / bulletSprite->getContentSize().width);
        
        // 根据玩家位置判断是否翻转子弹
        if (playerPos.x < bossPos.x)
        {
            // 如果玩家在Boss左侧，翻转子弹图片
            bulletSprite->setFlippedX(true);
        }
    }

    // 设置子弹的碰撞掩码
    bullet->setCategoryBitmask(ENEMY_BULLET_MASK);
    bullet->setContactTestBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
    bullet->setCollisionBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
    bullet->setCLearBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);

    // 设置子弹的最大存在时间
    float distance = bossPos.distance(playerPos);
    float time = distance / speed;
    bullet->setMaxExistTime(time + 1.0f); // 稍微延长一点时间，确保子弹能到达目标位置

    // 添加子弹到场景
    this->getParent()->addChild(bullet);
}

void Boss1::fireSwordBeam()
{
    // 检查精灵是否存在
    if (sprite_ == nullptr) return;
    
    // 根据Boss的朝向设置剑气方向
    float direction = sprite_->isFlippedX() ? -1.0f : 1.0f;
    
    // 设置剑气移动速度
    float speed = 500.0f;
    Vec2 velocity = Vec2(direction * speed, 0.0f);
    
    // 使用Bullet类创建剑气
    auto swordBeam = Bullet::create("Enemy/Boss1/attack2_bullet.png", this->getBaseAttackPower(), 
        [velocity](Bullet* bullet, float delta) {
            // 更新逻辑：使用固定速度水平移动
            bullet->setPosition(bullet->getPosition() + velocity * delta);
        });
    
    if (swordBeam == nullptr) return;
    
    // 设置剑气初始位置在Boss的位置
    swordBeam->setPosition(this->getPosition());
    
    // 设置剑气的翻转
    auto beamSprite = swordBeam->getSprite();
    if (beamSprite != nullptr)
    {
        beamSprite->setScale(GRID_SIZE * 4 / beamSprite->getContentSize().width);
        // 如果Boss向左翻转，剑气也向左翻转
        if (sprite_->isFlippedX())
        {
            beamSprite->setFlippedX(true);
        }
    }
    
    // 设置剑气的碰撞掩码
    swordBeam->setCategoryBitmask(ENEMY_BULLET_MASK);
    swordBeam->setContactTestBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
    swordBeam->setCollisionBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
    swordBeam->setCLearBitmask(PLAYER_MASK | WALL_MASK | BORDER_MASK);
    
    // 设置剑气的最大存在时间
    swordBeam->setMaxExistTime(4.0f);
    
    // 添加剑气到场景
    this->getParent()->addChild(swordBeam);
}

BehaviorResult Boss1::attack2(float delta)
{
    // 停止待机动画
    if (sprite_ != nullptr && isIdleAnimationPlaying_)
    {
        sprite_->stopAllActions();
        isIdleAnimationPlaying_ = false;
    }
    
    // 如果攻击还未激活，初始化攻击
    if (!isAttack2Active_)
    {
        isAttack2Active_ = true;
        attack2Timer_ = 0.0f;
        attack2State_ = 0; // 0: 第一张攻击图片, 1: 第二张攻击图片, 2: 第三张攻击图片并发射剑气, 3: 发射剑气后延迟0.5秒, 4: 后摇
        
        return { false, 0.0f };
    }
    
    // 更新攻击计时器
    attack2Timer_ += delta;
    
    // 根据攻击状态执行不同的逻辑
    switch (attack2State_)
    {
        case 0: // 第一张攻击图片
            if (attack2Timer_ >= 0.5f)
            {
                attack2Timer_ = 0.0f;
                attack2State_ = 1;
                
                // 设置第一张攻击图片
                if (sprite_ != nullptr)
                {
                    sprite_->setVisible(true);
                    sprite_->setTexture("Enemy/Boss1/8.png");
                    // 临时调整位置：向左上移动
                    sprite_->setPosition(Vec2::ZERO + Vec2(0, 50));
                }
            }
            break;
            
        case 1: // 第二张攻击图片
            if (attack2Timer_ >= 0.5f)
            {
                attack2Timer_ = 0.0f;
                attack2State_ = 2;
                
                // 设置第二张攻击图片
                if (sprite_ != nullptr)
                {
                    sprite_->setTexture("Enemy/Boss1/9.png");
                    // 临时调整位置：向左上移动
                    sprite_->setPosition(Vec2::ZERO + Vec2(0, 40));
                }
            }
            break;
            
        case 2: // 第三张攻击图片并发射剑气
            if (attack2Timer_ >= 0.5f)
            {
                attack2Timer_ = 0.0f;
                attack2State_ = 3;
                
                // 设置第三张攻击图片
                if (sprite_ != nullptr)
                {
                    sprite_->setTexture("Enemy/Boss1/10.png");
                    // 临时调整位置：向左上移动
                    sprite_->setPosition(Vec2::ZERO + Vec2(0, 30));
                }
                
                // 发射剑气
                fireSwordBeam();
            }
            break;
            
        case 3: // 发射剑气后延迟0.9秒
            if (attack2Timer_ >= 0.9f)
            {
                attack2Timer_ = 0.0f;
                attack2State_ = 4;
            }
            break;
            
        case 4: // 攻击动作完成，准备结束
            {
                // 攻击结束，重置状态
                isAttack2Active_ = false;
                attack2State_ = 0;
                
                // 恢复精灵原始位置
                if (sprite_ != nullptr)
                {
                    sprite_->setPosition(Vec2::ZERO);
                }
                
                // 返回true表示攻击行为结束，进入recovery行为5秒
                BehaviorResult result = { true, 5.0f };
                
                // 如果是分身，在攻击结束后自我消灭
                if (isClone_)
                {
                    this->Dead();
                }
                
                return result;
            }
            break;
    }
    
    return { false, 0.0f };
}

BehaviorResult Boss1::attack3(float delta)
{
    // 停止待机动画
    if (sprite_ != nullptr && isIdleAnimationPlaying_)
    {
        sprite_->stopAllActions();
        isIdleAnimationPlaying_ = false;
    }
    
    // 如果攻击还未激活，初始化攻击
    if (!isAttack3Active_)
    {
        isAttack3Active_ = true;
        hasSummonedClone_ = false;
        
        // 计算分身的位置：与Boss对称
        Vec2 bossPos = this->getPosition();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        clonePosition_ = Vec2(visibleSize.width - bossPos.x, bossPos.y);
        
        // 在分身位置播放3秒的闪光动画
        auto flash = Sprite::create();
        if (flash != nullptr)
        {
            flash->setPosition(clonePosition_);
            flash->setScale(2.0f);
            this->getParent()->addChild(flash);
            
            // 创建闪光动画
            auto blink = Blink::create(3.0f, 15); // 3秒内闪烁15次
            auto fadeOut = FadeOut::create(0.5f);
            auto removeSelf = RemoveSelf::create();
            auto sequence = Sequence::create(blink, fadeOut, removeSelf, nullptr);
            flash->runAction(sequence);
        }
        
        // 查找玩家位置
        Player* player = EnemyAi::findPlayer(this);
        Vec2 playerDir = Vec2::ZERO;
        if (player != nullptr)
        {
            playerDir = player->getPosition() - this->getPosition();
            playerDir.normalize();
        }
        
        // 在boss身上显示1.png图片，持续3秒并向玩家方向修正和翻转
        if (spriteParent_ != nullptr)
        {
            sprite_->setVisible(false);
            auto summonEffect = Sprite::create("Enemy/Boss1/1.png");
            if (summonEffect != nullptr)
            {
                // 根据玩家位置翻转图片
                bool flipX = false;
                if (player != nullptr)
                {
                    flipX = player->getPosition().x < this->getPosition().x;
                }
                summonEffect->setFlippedX(flipX);
                
                // 向玩家方向修正位置
                Vec2 bossEffectOffset = playerDir * 35.0f; // 调整35像素向玩家方向
                summonEffect->setPosition(Vec2::ZERO + bossEffectOffset); // 相对于spriteParent_居中并修正
                summonEffect->setScale(1.5f); // 根据需要调整大小
                spriteParent_->addChild(summonEffect, 10); // 添加到较高层级
                
                // 3秒后移除这个图片
                this->scheduleOnce([summonEffect,this](float dt) {
                    summonEffect->removeFromParentAndCleanup(true);
                    sprite_->setVisible(true);
                }, 3.0f, "remove_summon_effect");
            }
        }
        
        // 在召唤位置显示蓝色调的1.png，持续3秒并向玩家方向修正和翻转
        auto cloneEffect = Sprite::create("Enemy/Boss1/1.png");
        if (cloneEffect != nullptr)
        {
            // 应用蓝色调
            cloneEffect->setColor(Color3B::BLUE);
            cloneEffect->setOpacity(200); // 半透明效果
            
            // 根据玩家位置翻转图片
            bool flipX = false;
            if (player != nullptr)
            {
                flipX = player->getPosition().x < clonePosition_.x;
            }
            cloneEffect->setFlippedX(flipX);
            
            // 向玩家方向修正位置
            Vec2 playerToCloneDir = Vec2::ZERO;
            if (player != nullptr)
            {
                playerToCloneDir = player->getPosition() - clonePosition_;
                playerToCloneDir.normalize();
            }
            Vec2 cloneEffectOffset = playerToCloneDir * 35.0f; // 调整35像素向玩家方向
            cloneEffect->setPosition(clonePosition_ + cloneEffectOffset);
            cloneEffect->setScale(1.5f); // 根据需要调整大小
            
            // 添加到场景
            if (this->getParent() != nullptr)
            {
                this->getParent()->addChild(cloneEffect, 10); // 添加到较高层级
                
                // 3秒后移除这个图片
                this->scheduleOnce([cloneEffect](float dt) {
                    cloneEffect->removeFromParentAndCleanup(true);
                }, 3.0f, "remove_clone_effect");
            }
        }
        
        // 3秒后创建分身
        this->scheduleOnce([this](float dt) {
            this->createClone();
            hasSummonedClone_ = true;
        }, 3.0f, "create_clone");
        
        return { false, 0.0f };
    }
    
    // 如果已经创建了分身，等待一段时间后结束攻击
    if (hasSummonedClone_)
    {
        // 攻击结束，重置状态
        isAttack3Active_ = false;
        
        // 返回true表示攻击行为结束，进入recovery行为3秒
        BehaviorResult result = { true, 3.0f };
        
        // 如果是分身，在攻击结束后自我消灭
        if (isClone_)
        {
            this->Dead();
        }
        
        return result;
    }
    
    return { false, 0.0f };
}

// 创建分身
void Boss1::createClone()
{
    // 创建Boss1的克隆体
    clone_ = Boss1::create();
    if (clone_ != nullptr)
    {
        // 设置分身位置为对称位置
        clone_->setPosition(clonePosition_);
        
        // 设置分身颜色为蓝色
        if (clone_->getSprite() != nullptr)
        {
            clone_->getSprite()->setColor(cocos2d::Color3B(0, 100, 255));
        }
        
        // 设置分身具有无限血量
        clone_->setMaxVitality(INT_MAX);
        clone_->setCurrentVitality(INT_MAX);
        
        // 设置为分身
        clone_->isClone_ = true;
        
        // 重置attack2相关状态变量，确保每个分身独立执行攻击
        clone_->isAttack2Active_ = false;
        clone_->attack2Timer_ = 0.0f;
        clone_->attack2State_ = 0;
        
        // 添加分身在场景中
        this->getParent()->addChild(clone_);
    }
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
