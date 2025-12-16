#include <cmath>
#include "Player.h"
#include "AudioEngine.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;


Player* Player::createNode()
{
    Player* pRet = new(std::nothrow) Player();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool Player::init()
{
    if (!Node::init())
    {
        return false;
    }

    // 加载主角图像
    _sprite = Sprite::create("player/idle-0.png");
    // 设置锚点
    _sprite->setAnchorPoint(Vec2(0.5, 0));
    this->addChild(_sprite);

    /*---初始化主角各属性---*/
    _currentState = PlayerState::IDLE;
    _previousState = PlayerState::IDLE;
    _direction = Direction::RIGHT;

    // 基础属性
    Size contentSize = _sprite->getContentSize();
    _physicsSize = Size(contentSize.width * 0.5f, contentSize.height * 0.75f); // 碰撞体通常比贴图稍小

    _maxHealth = 100.0;
    _health = _maxHealth;
    _speed = 300.0;     // 水平移动最大速度
    _jumpForce = 500.0; // 跳跃冲量 
    _dodgeForce = 300.0;
    _acceleration = 1000.0;
    _deceleration = 2000.0; 
    _maxDodgeTime = 0.5;
    _maxDodgeTimes = 1;
    _dodgeTimes = _maxDodgeTimes;
    _playerAttackDamage = 30;

    _maxAttackCooldown = 0.3;
    _maxDodgeCooldown = 0.2;
    _dodgeTime = 0;

    // 状态标志
    _isGrounded = false;
    _isDodge = false;
    _isHurt = false;
    _isDead = false;
    _isAttacking = false;
    _isInvincible = false;
    _controlEnabled = true;
    _attack_num = 0;
    _footContactCount = 0; // 用于记录脚部接触物体的数量

    // 计时器
    _jumpBufferTime = 0.0;
    _coyoteTime = 0.0;
    _dodgeCooldown = 0.0;
    _attackCooldown = 0.0;
    _invincibilityTime = 0.0;
    _attackEngageTime = 0.0;

    // 输入
    _moveInput = 0.0;
    _velocity = Vec2::ZERO;

    // 初始化物理身体
    initPhysics();

    // 初始化碰撞监听
    // 注册碰撞回调
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(Player::onContactBegin, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(Player::onContactSeparate, this);

    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);

    // 播放初始动画
    playAnimation("idle", true);

    // 启用update函数
    this->scheduleUpdate();

    return true;
}

void Player::initPhysics()
{
    //创建偏移量（碰撞箱对应图片）
    Size originalSize = _sprite->getContentSize();
    Vec2 offset = Vec2(0, originalSize.height / 2);
    //创建主身体,材质：摩擦力1.0(防止卡墙)，弹性0
    auto bodyMaterial = PhysicsMaterial(0.1f, 0.0f, 0.0f);
    //根据碰撞箱大小、身体材质、偏移量创建碰撞箱
    _physicsBody = PhysicsBody::createBox(_physicsSize, bodyMaterial,offset);

    //禁止旋转
    _physicsBody->setRotationEnable(false);
    //设置质量
    _physicsBody->setMass(1.0f);

    //设置掩码
    _physicsBody->setCategoryBitmask(PLAYER_MASK);
    _physicsBody->setCollisionBitmask(WALL_MASK | BORDER_MASK | ENEMY_MASK | ENEMY_BULLET_MASK);
    _physicsBody->setContactTestBitmask(WALL_MASK | BORDER_MASK | ENEMY_MASK | DAMAGE_WALL_MASK | ENEMY_BULLET_MASK);
    _originalMask = WALL_MASK | BORDER_MASK | ENEMY_MASK | ENEMY_BULLET_MASK | DAMAGE_WALL_MASK;
    _dodgeMask = WALL_MASK | BORDER_MASK;

    //给主身体一个Tag
    _physicsBody->getShape(0)->setTag(TAG_BODY);

    //添加“脚部传感器”
    //这是一个比身体底部略小且略低的矩形，用于检测是否站在地上
    //只检测碰撞，不产生物理推力
    Size footSize = Size(_physicsSize.width * 0.8f, 10);
    Vec2 footOffset = Vec2(0, 2); // 位于身体底部

    auto footShape = PhysicsShapeBox::create(footSize, PhysicsMaterial(0, 0, 0), footOffset);
    footShape->setCategoryBitmask(PLAYER_MASK);
    footShape->setCollisionBitmask(WALL_MASK | BORDER_MASK | ENEMY_MASK);
    footShape->setContactTestBitmask(WALL_MASK | BORDER_MASK | ENEMY_MASK);
    footShape->setTag(TAG_FEET); // 标记为脚

    _physicsBody->addShape(footShape);

    this->setPhysicsBody(_physicsBody);
    this->setAnchorPoint(Vec2(0.5f, 0.5f)); // 物理刚体中心对齐

    // 注册碰撞回调
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(Player::onContactBegin, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(Player::onContactSeparate, this);

    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
}

bool Player::onContactBegin(cocos2d::PhysicsContact& contact)
{
    // 获取参与碰撞的两个形状
    auto shapeA = contact.getShapeA();
    auto shapeB = contact.getShapeB();

    // 获取对应的物体和节点
    auto bodyA = shapeA->getBody();
    auto bodyB = shapeB->getBody();
    auto nodeA = bodyA->getNode();
    auto nodeB = bodyB->getNode();

    // 确保其中有一个是当前玩家，并区分出玩家形状和对方形状
    PhysicsShape* playerShape = nullptr;
    PhysicsShape* otherShape = nullptr;
    Node* otherNode = nullptr;

    if (nodeA == this) {
        playerShape = shapeA;
        otherShape = shapeB;
        otherNode = nodeB;
    }
    else if (nodeB == this) {
        playerShape = shapeB;
        otherShape = shapeA;
        otherNode = nodeA;
    }
    else {
        return true;
    }

    // 地面检测
    if (playerShape->getTag() == TAG_FEET)
    {
        // 检查对方是否是墙壁、边界或敌人
        int otherMask = otherShape->getCategoryBitmask();

        if ((otherMask & WALL_MASK) || (otherMask & BORDER_MASK)||(otherMask & ENEMY_MASK))
        {
            _footContactCount++;
            if (_footContactCount > 0) {
                _isGrounded = true;
                // 如果是下落状态，播放落地音效
                if (_currentState == PlayerState::FALLING) {
                    // AudioEngine::play2d("land.mp3");
                }
            }
        }
    }
    // 伤害判定逻辑
    int otherCategory = otherShape->getCategoryBitmask();
    bool isEnemy = (otherCategory & ENEMY_BULLET_MASK);
    bool isTrap = (otherCategory & DAMAGE_WALL_MASK);

    if (isEnemy || isTrap)
    {
        // 如果没有处于无敌状态且没有死亡
        if (!_isInvincible && _currentState != PlayerState::DEAD)
        {
            // 扣血
            float damage = 10;
            if (isEnemy && otherNode)
            {
                // 尝试将 Node* 转换为 Enemy*
                auto enemy = dynamic_cast<Bullet*>(otherNode);
                if (enemy)
                {
                    damage = static_cast<float>(enemy->getDamage());
                }
                else
                {
                    damage = 10.0f; // 怪物的默认伤害
                }
            }
            else if (isTrap)
            {
                damage = 20.0f; // 陷阱的固定伤害
            }
            _health -= damage;

            // 状态判断
            if (_health <= 0) {
                _health = 0;
                changeState(PlayerState::DEAD);
                _isDead = true;
                _controlEnabled = false;//禁止控制
                _physicsBody->setVelocity(Vec2::ZERO);//速度减为0
                playAnimation("dead");
                this->removeComponent(_physicsBody);//移除所有物理效果
            }
            else {
                _isHurt = true;
                // 开启无敌时间
                _isInvincible = true;
                _invincibilityTime = 1.0f; // 无敌1秒

                // 击退效果
                Vec2 knockbackDir = Vec2::ZERO;
                if (otherNode) {
                    //以此物体中心和敌人中心计算击退方向
                    float diffX = this->getPositionX() - otherNode->getPositionX();
                    knockbackDir = Vec2(diffX > 0 ? 1 : -1.0f, 0.5f); // 向反方向弹开，带一点向上的力
                    _sprite->setFlippedX(diffX > 0 ? true : false);
                    _direction = diffX > 0 ? Direction::LEFT : Direction::RIGHT;
                }
                else {
                    // 如果是陷阱墙，根据朝向反弹
                    knockbackDir = Vec2(_direction == Direction::RIGHT ? -1 : 1.0f, 0.5f);
                }
                knockbackDir.normalize();

                // 设置击退速度 (覆盖当前速度)
                _physicsBody->setVelocity(knockbackDir * 500.0f);
                // 播放受伤音效
                // AudioEngine::play2d("hurt.mp3");
            }
        }
    }

    return true; // 返回 true 允许物理引擎处理碰撞（如阻挡）
}

bool Player::onContactSeparate(cocos2d::PhysicsContact& contact)
{
    auto shapeA = contact.getShapeA();
    auto shapeB = contact.getShapeB();
    auto bodyA = shapeA->getBody();
    auto bodyB = shapeB->getBody();
    auto nodeA = bodyA->getNode();
    auto nodeB = bodyB->getNode();

    PhysicsShape* playerShape = nullptr;
    PhysicsShape* otherShape = nullptr;

    if (nodeA == this) {
        playerShape = shapeA;
        otherShape = shapeB;
    }
    else if (nodeB == this) {
        playerShape = shapeB;
        otherShape = shapeA;
    }
    else {
        return true;
    }

    // 地面离开逻辑
    if (playerShape->getTag() == TAG_FEET)
    {
        int otherMask = otherShape->getCategoryBitmask();
        if ((otherMask & WALL_MASK) || (otherMask & BORDER_MASK))
        {
            _footContactCount--;
            // 只有当计数器归零，且确定不再接触任何地面时，才设为 false
            if (_footContactCount <= 0) {
                _footContactCount = 0; // 防止负数
                _isGrounded = false;
            }
        }
    }

    int otherCategory = otherShape->getCategoryBitmask();
    bool isEnemy = (otherCategory & ENEMY_MASK);
    bool isTrap = (otherCategory & DAMAGE_WALL_MASK);

    if (isEnemy || isTrap)
    {
        _isHurt = false;
    }

    return true;
}

void Player::update(float dt) {
    if (!_controlEnabled) return;

    // 同步物理引擎的速度到逻辑变量
    if (_physicsBody) {
        _velocity = _physicsBody->getVelocity();
    }
    // 时钟更新
    updateTimers(dt);

    // 物理更新：计算并应用新的速度
    updatePhysics(dt);

    // 状态更新：根据当前速度和输入决定状态
    updateState();

    // 动画更新：根据状态播放动画
    updateAnimation();
}

void Player::updateTimers(float dt) {
    if (_jumpBufferTime > 0) _jumpBufferTime -= dt;

    if (_isGrounded) {
        _coyoteTime = kCoyoteTime;
        _dodgeTimes = _maxDodgeTimes;
    }
    else if (_coyoteTime > 0) {
        _coyoteTime -= dt;
    }

    if (_dodgeCooldown > 0) _dodgeCooldown -= dt;
    if (_attackCooldown > 0) _attackCooldown -= dt;

    if (_dodgeTime > 0) {
        _dodgeTime -= dt;
    }
    if (_dodgeTime <= 0) {
        _isDodge = false;
        // 闪避结束，恢复掩码（冲刺穿过部分实体）
        _physicsBody->setCollisionBitmask(_originalMask); 
        _physicsBody->setContactTestBitmask(_originalMask | DAMAGE_WALL_MASK);
    }

    // 攻击衔接
    if (_attackCooldown <= 0 && _attackEngageTime > 0)
    {
        _attackEngageTime -= dt;
    }
    
    // 无敌时间（冲刺，受击）
    if (_isInvincible)
    {
        _invincibilityTime -= dt;
        if (_invincibilityTime <= 0)
        {
            _isInvincible = false;
            _sprite->setOpacity(255);
        }
        else
        {
            // 闪烁效果
            double blink = sin(_invincibilityTime * 20) * 0.5f + 0.5f;
            _sprite->setOpacity(static_cast<uint8_t>(blink * 255));
        }
    }
}

void Player::updatePhysics(float dt) {
    if (!_physicsBody) return;

    // 暂存当前速度
    float currentX = _velocity.x;
    float currentY = _velocity.y;
    // 物理引擎计算速度
    Vec2 realVelocity = _physicsBody->getVelocity();
    float targetX = 0;
    
    // 计算水平速度 (加速/减速)
    float newX = currentX;
    // 垂直速度 (重力逻辑）
    float newY = currentY;

    // 决定目标速度
    if (_isAttacking) {
        targetX = 0; // 攻击时目标速度为0
    }
    if (_isDodge) {
        float dashSpeed = static_cast<float>(_dodgeForce) * ((_direction == Direction::RIGHT) ? 1.0f : -1.0f);

        // 防穿墙逻辑：
        // 如果是冲刺刚开始（前0.05秒），强制设置速度（给一个初速度）
        float timeSinceDodgeStart = static_cast<float>(_maxDodgeTime - _dodgeTime);

        if (timeSinceDodgeStart < 0.05f) {
            // 刚开始冲刺，强制给速度
            currentX = dashSpeed;
            targetX = dashSpeed;
        }
        else {
            // 冲刺中途
            // 如果本来应该高速移动，但实际速度接近0，说明撞墙了
            if (std::abs(realVelocity.x) < 10.0f) {
                newX = 0;
            }
            else {
                targetX = dashSpeed;
            }
        }

        newY = 0; // 冲刺时忽略重力
    }
    else {
        targetX = _moveInput * static_cast<float>(_speed);
    }

    if (_isAttacking) {
        newX = currentX * 0.9f; //若在攻击，给予摩檫力
    }
    else {
        if (fabs(targetX) > 0.01f) {//加速
            float direction = (targetX > currentX) ? 1.0f : -1.0f;
            newX = currentX + direction * _acceleration * dt;
            if ((direction > 0 && newX > targetX) || (direction < 0 && newX < targetX)) {
                newX = targetX;
            }
        }
        else {//减速
            if (fabs(currentX) > 0.01f) {
                float direction = (currentX > 0) ? -1.0f : 1.0f;
                newX = currentX + direction * _deceleration * dt;
                if ((currentX > 0 && newX < 0) || (currentX < 0 && newX > 0)) {
                    newX = 0;
                }
            }
            else {
                newX = 0;
            }
        }
    }

    if (newY < -800.0f) newY = -800.0f;
    // 跳跃逻辑
    if (_jumpBufferTime > 0 && (_isGrounded || _coyoteTime > 0) && !_isAttacking) {
        newY = static_cast<float>(_jumpForce);
        _isGrounded = false;
        _coyoteTime = 0;
        _jumpBufferTime = 0;
    }

    _physicsBody->setVelocity(Vec2(newX, newY));
}

void Player::updateState() {
    //根据标识确定攻击/闪避
    if (_isAttacking) {
        changeState(PlayerState::ATTACKING);
        return;
    }
    else if (_isDodge) {
        changeState(PlayerState::DODGING);
        return;
    }
    else if (_isDead) {
        changeState(PlayerState::DEAD);
        return;
    }
    else if (_isHurt) {
        changeState(PlayerState::HURT);
        return;
    }

    // 使用物理引擎判定的 _isGrounded
    if (!_isGrounded) {
        //根据垂直速度方向判定跳跃/下落
        if (_velocity.y > 0.1f) {
            changeState(PlayerState::JUMPING);
        }
        else {
            changeState(PlayerState::FALLING);
        }
    }
    else {
        //根据水平速度大小判定奔跑/待机
        if (fabs(_velocity.x) > 10.0f) {
            changeState(PlayerState::RUNNING);
        }
        else {
            changeState(PlayerState::IDLE);
        }
    }
}

void Player::changeState(PlayerState newState) {
    if (_currentState == newState) return;

    _previousState = _currentState;
    _currentState = newState;

    // 状态进入逻辑
    // 可以考虑后续加入切换状态时的操作
    if (newState == PlayerState::FALLING) {
    }
}


void Player::updateAnimation() {
    std::string animationName;
    bool loop = true;

    // 所有动画制作完成
    switch (_currentState) {
        case PlayerState::IDLE: animationName = "idle"; break;
        case PlayerState::RUNNING: 
            animationName = "run";
            break;
        case PlayerState::JUMPING: animationName = "jump"; loop = false; break;
        case PlayerState::FALLING: animationName = "fall"; loop = false; break;
        case PlayerState::ATTACKING:
            animationName = StringUtils::format("attack-%d", _attack_num + 1);
            loop = false;
            break;
        case PlayerState::DODGING: animationName = "dodge"; loop = false; break;
        case PlayerState::HURT: animationName = "hurt"; loop = false; break;
        case PlayerState::DEAD: animationName = "dead"; loop = false; break;
        default: animationName = "idle"; break;
    }
    //如果状态变化，更新动画
    if (_currentState != _previousState) {
        playAnimation(animationName, loop);
        _previousState = _currentState;
    }

    //根据输入或速度方向翻转
    if (_moveInput > 0.1f) {
        _direction = Direction::RIGHT;
        _sprite->setFlippedX(false);
    }
    else if (_moveInput < -0.1f) {
        _direction = Direction::LEFT;
        _sprite->setFlippedX(true);
    }
}

void Player::playAnimation(const std::string& name, bool loop)
{
    // 获取动画缓存单个实例中名为name的动画
    auto animation = AnimationCache::getInstance()->getAnimation(name);
    // 如果成功获取
    if (animation)
    {
        _sprite->stopAllActions();
        // 如果要播放死亡动画，则要求停留在最后一帧
        if (name == "dead" || name =="jump" || name == "fall")
            animation->setRestoreOriginalFrame(false);
        // 创建action
        auto action = Animate::create(animation);
        action->setTag(ANIMATION_ACTION_TAG);
        // 根据是否循环确定播放方式
        if (loop)
        {
            _sprite->runAction(RepeatForever::create(action));
        }
        else
        {
            _sprite->runAction(Sequence::create(
                action,
                CallFunc::create([this]()
                    { _isAttacking = false; }),
                nullptr));
        }
    }
    //如果失败
    else
    {
        CCLOG("Error: Animation not found: %s", name.c_str());
        _isAttacking = false;
    }
}

/*---部分对外接口---*/

const Sprite* Player::getSprite() const {
    return _sprite;
}

void Player::moveLeft() {
    _moveInput = -1.0f;
}

void Player::moveRight() {
    _moveInput = 1.0f;
}

void Player::stopMoving() {
    _moveInput = 0.0f;
}

void Player::jump() {
    _jumpBufferTime = 0.1f;
}


void Player::shootBullet()
{
    // 创建子弹对象
    Bullet* attack = nullptr;
    switch (_attack_num) {
        case 0:
            attack = Bullet::create("player/FireBall-0.png", _playerAttackDamage, [this](Bullet* bullet, float delta) {});
            attack->setMaxExistTime(1.0f);
        break;
        case 1:
            attack = Bullet::create("player/FlameSlash-0.png", _playerAttackDamage, [this](Bullet* bullet, float delta) {});
            attack->setCollisionHeight(75);
            attack->setCollisionWidth(60);
            break;
        case 2:
            attack = Bullet::create("player/FrozenSpike-0.png", _playerAttackDamage, [this](Bullet* bullet, float delta) {});
            attack->setCollisionHeight(_sprite->getContentSize().height);
            attack->setCollisionWidth(65);
            break;
    }
    attack->setCLearBitmask(WALL_MASK | ENEMY_MASK | BORDER_MASK);
    // 加载动画资源
    char attack_name[20];
    sprintf(attack_name, "attack-bullet-%d", _attack_num + 1);
    auto animation = AnimationCache::getInstance()->getAnimation(attack_name);

    // 获取玩家当前位置
    Vec2 current_pos = _sprite->getPosition();
    if (!attack) return;

    // 根据攻击段数配置子弹逻辑
    float speed = 0;
    if (animation)
    {
        auto action = Animate::create(animation);
        if (_attack_num == 0) {
            // 第一段为火球
            attack->setCategoryBitmask(PLAYER_BULLET_MASK);
            attack->setCollisionBitmask(WALL_MASK | ENEMY_MASK | BORDER_MASK);
            attack->setContactTestBitmask(WALL_MASK | ENEMY_MASK | BORDER_MASK);
            speed = 200.0;
            attack->getSprite()->setScale(3.0f);       // 调整视觉大小
            attack->getSprite()->runAction(RepeatForever::create(action));
        }
        else {
            // 第二三段为近战攻击
            speed = 0;
            attack->setDamage(attack->getDamage() * (_attack_num+1) / 2);//第三段攻击为1.5倍伤害
            attack->setCategoryBitmask(PLAYER_BULLET_MASK);
            attack->setCollisionBitmask(ENEMY_MASK);
            attack->setContactTestBitmask(ENEMY_MASK);
            // 播放完动画后删除整个子弹对象
            auto finishCallback = CallFunc::create([attack]() {
                attack->cleanupBullet();
                });

            attack->getSprite()->runAction(Sequence::create(action, finishCallback, nullptr));
        }
    }

    // 设置方向 (同时处理物理速度方向和贴图翻转)
    Vec2 directionVec;
    if (_direction == Direction::RIGHT) {
        directionVec = Vec2(1, 0);
        attack->getPhysicsBody()->setVelocity(Vec2(speed, 0));
        attack->getSprite()->setFlippedX(false);
    }
    else {
        directionVec = Vec2(-1, 0);
        attack->getPhysicsBody()->setVelocity(Vec2(-speed, 0));
        attack->getSprite()->setFlippedX(true);
    }

    // 添加子弹到场景或玩家
    if (_attack_num == 0) {
        // --- 远程攻击：添加到世界场景 ---
        // 将玩家的局部坐标转换为世界坐标，防止子弹跟随玩家移动
        Vec2 worldPos = this->convertToWorldSpace(current_pos);

        // 微调发射位置，使其不完全重叠在玩家中心
        worldPos += (directionVec * 30.0f);
        worldPos.y += _sprite->getContentSize().height;

        attack->setPosition(worldPos);
        auto gameScene = Director::getInstance()->getRunningScene();
        if (gameScene) {
            gameScene->addChild(attack, 10);
        }
    }
    else {
        // --- 近战攻击：作为玩家的子节点 ---
        // 这样特效会跟随玩家移动
        // 根据朝向设置偏移量，使砍击特效出现在玩家前方
        attack->setAnchorPoint(Vec2(0.5f, 0.0f));
        attack->getSprite()->setAnchorPoint(Vec2(0.5f, 0.0f));
        Vec2 offset = directionVec * (_attack_num ==1? 20.0f:0.0f);
        attack->getSprite()->setPosition(current_pos + offset);
        attack->getPhysicsBody()->setPositionOffset(Vec2(directionVec.x * (_attack_num == 1 ? 70 : 0), 
            (_attack_num == 1 ? 20 : 0) + attack->getSprite()->getContentSize().height));
        this->addChild(attack, 10);
    }
}

void Player::attack() {
    if (_attackCooldown > 0 || _isAttacking || !_controlEnabled) return;

    // 多段攻击
    if (_attackEngageTime > 0)
        _attack_num = (_attack_num + 1) % 3;
    else
        _attack_num = 0;

    _isAttacking = true;
    _attackCooldown = _maxAttackCooldown;
    _attackEngageTime = kMaxAttackEngageTime; // 多段攻击的衔接

    shootBullet();

    // 生成伤害判定框（Hitbox）通常在动画的特定帧回调中生成，或者在这里生成一个瞬时的Sensor
    // 这里留空，视具体Combat系统实现
}

void Player::dodge() {
    if (_dodgeCooldown > 0 || _isDodge || _dodgeTimes <= 0) return; 

    _isDodge = true;
    _dodgeCooldown = _maxDodgeCooldown;
    _dodgeTime = _maxDodgeTime; // 闪避持续时间
    _dodgeTimes--;

    //闪避忽略初始速度
    Vec2 current_velocity = _physicsBody->getVelocity();
    current_velocity.x = 0;
    _physicsBody->setVelocity(current_velocity);

    //闪避可以穿过敌人
    _physicsBody->setCollisionBitmask(_dodgeMask);
    _physicsBody->setContactTestBitmask(_dodgeMask | DAMAGE_WALL_MASK);

    //闪避时无敌
    _isInvincible = true;
    _invincibilityTime = _dodgeTime;
}

const std::string Player::getCurrentState() const
{
    std::string current;
    switch (_currentState)
    {
        case PlayerState::IDLE:
            current = "idle";
            break;
        case PlayerState::RUNNING:
            current = "run";
            break;
        case PlayerState::JUMPING:
            current = "jump";
            break;
        case PlayerState::FALLING:
            current = "fall";
            break;
        case PlayerState::ATTACKING:
            char tmp[20]; // 存放攻击段数名
            sprintf(tmp, "attack-%d", _attack_num + 1);
            current = tmp;
            break;
        case PlayerState::DODGING:
            current = "dodge";
            break;
        case PlayerState::LANDING:
            current = "land";
            break;
        case PlayerState::HURT:
            current = "hurt";
            break;
        case PlayerState::DEAD:
            current = "dead";
            break;
        default:
            current = "idle";
    }
    return current;
}