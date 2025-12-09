#include <cmath>
#include "Player.h"
#include "AudioEngine.h"

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
    _physicsSize = Size(contentSize.width * 0.5, contentSize.height * 0.85); // 碰撞体通常比贴图稍小

    _maxHealth = 100.0;
    _health = _maxHealth;
    _speed = 300.0;     // 水平移动最大速度
    _jumpForce = 600.0; // 跳跃冲量 
    _dodgeForce = 800.0;
    _acceleration = 2000.0;
    _deceleration = 3000.0; 

    _maxAttackCooldown = 0.3;
    _maxDodgeCooldown = 0.2;
    _dodgeTime = 0;

    // 状态标志
    _isGrounded = false; // 默认为false，由物理回调决定
    _isDodge = false;
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

    // 初始化碰撞监听 (用于检测地面)
    setupCollisionHandler();

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
    // 创建主身体（矩形或胶囊体），材质：摩擦力0(防止卡墙)，弹性0
    auto bodyMaterial = PhysicsMaterial(0.0f, 1.0f, 0.0f);
    _physicsBody = PhysicsBody::createBox(_physicsSize, bodyMaterial,offset);

    // 禁止旋转
    _physicsBody->setRotationEnable(false);
    _physicsBody->setMass(1.0f);

    // 设置掩码
    _physicsBody->setCategoryBitmask(PLAYER_CATEGORY_BITMASK);
    _physicsBody->setCollisionBitmask(GROUND_CATEGORY_BITMASK | PLATFORM_CATEGORY_BITMASK | ENEMY_CATEGORY_BITMASK);
    _physicsBody->setContactTestBitmask(GROUND_CATEGORY_BITMASK | PLATFORM_CATEGORY_BITMASK | ENEMY_CATEGORY_BITMASK | TRAP_CATEGORY_BITMASK);

    // 给主身体一个Tag
    _physicsBody->getShape(0)->setTag(TAG_BODY);

    // 添加“脚部传感器”
    // 这是一个比身体底部略小且略低的矩形，专门用于检测是否站在地上
    // 传感器(Sensor)只检测碰撞但不产生物理推力
    Size footSize = Size(_physicsSize.width * 0.8f, 10);
    Vec2 footOffset = Vec2(0, 5); // 位于身体底部

    auto footShape = PhysicsShapeBox::create(footSize, PhysicsMaterial(0, 0, 0), footOffset);
    footShape->setCategoryBitmask(PLAYER_CATEGORY_BITMASK);
    footShape->setCollisionBitmask(GROUND_CATEGORY_BITMASK | PLATFORM_CATEGORY_BITMASK);
    footShape->setContactTestBitmask(GROUND_CATEGORY_BITMASK | PLATFORM_CATEGORY_BITMASK);
    footShape->setTag(TAG_FEET); // 标记为脚

    _physicsBody->addShape(footShape);

    this->setPhysicsBody(_physicsBody);
    this->setAnchorPoint(Vec2(0.5f, 0.5f)); // 物理刚体通常中心对齐
}

// 建立碰撞监听，核心逻辑：处理 _isGrounded
void Player::setupCollisionHandler()
{
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = [this](PhysicsContact& contact) -> bool {
        auto nodeA = contact.getShapeA()->getBody()->getNode();
        auto nodeB = contact.getShapeB()->getBody()->getNode();

        // 确保其中一个是玩家
        if (nodeA != this && nodeB != this) return true;

        auto shapeA = contact.getShapeA();
        auto shapeB = contact.getShapeB();

        // 检查是否是脚部碰撞
        bool isFeetContact = (nodeA == this && shapeA->getTag() == TAG_FEET) ||
            (nodeB == this && shapeB->getTag() == TAG_FEET);

        if (isFeetContact) {
            // 这里还可以判断碰撞对象的Bitmask，确保踩到的是地而不是敌人
            _footContactCount++;
            if (_footContactCount > 0) {
                _isGrounded = true;
            }
        }
        return true;
        };

    contactListener->onContactSeparate = [this](PhysicsContact& contact) {
        auto nodeA = contact.getShapeA()->getBody()->getNode();
        auto nodeB = contact.getShapeB()->getBody()->getNode();

        if (nodeA != this && nodeB != this) return;

        auto shapeA = contact.getShapeA();
        auto shapeB = contact.getShapeB();

        bool isFeetContact = (nodeA == this && shapeA->getTag() == TAG_FEET) ||
            (nodeB == this && shapeB->getTag() == TAG_FEET);

        if (isFeetContact) {
            _footContactCount--;
            if (_footContactCount <= 0) {
                _footContactCount = 0;
                _isGrounded = false;
            }
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}

void Player::update(float dt) {
    if (!_controlEnabled) return;

    // 1. 同步物理引擎的速度到逻辑变量 (这一步很重要，让逻辑基于真实物理)
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
    }
    else if (_coyoteTime > 0) {
        _coyoteTime -= dt;
    }

    if (_dodgeCooldown > 0) _dodgeCooldown -= dt;
    if (_attackCooldown > 0) _attackCooldown -= dt;

    if (_dodgeTime > 0) {
        _dodgeTime -= dt;
        if (_dodgeTime <= 0) {
            _isDodge = false;
            // 闪避结束，恢复掩码（如果闪避期间穿墙）
            // _physicsBody->setCollisionBitmask(originalMask); 
        }
    }

    // 攻击衔接
    if (_attackCooldown <= 0 && _attackEngageTime > 0)
    {
        _attackEngageTime -= dt;
    }
    
    // 无敌时间
    if (_isInvincible)
    {
        _invincibilityTime -= dt;
        if (_invincibilityTime <= 0)
        {
            _isInvincible = false;
            this->setOpacity(255);
        }
        else
        {
            // 闪烁效果
            float blink = sin(_invincibilityTime * 20) * 0.5f + 0.5f;
            this->setOpacity(blink * 255);
        }
    }
}

void Player::updatePhysics(float dt) {
    if (!_physicsBody) return;

    // 暂存当前速度
    float currentX = _velocity.x;
    float currentY = _velocity.y;

    float targetX = 0;
    
    //决定目标速度
    if (_isAttacking) {
        targetX = 0; // 攻击时目标速度为0
    }
    else if (_isDodge) {
        targetX = (_direction == Direction::RIGHT ? 1 : -1) * _dodgeForce;
    }
    else {
        targetX = _moveInput * _speed;
    }

    // 计算水平速度 (加速/减速)
    float newX = currentX;

    if (_isAttacking) {
        newX = currentX * 0.9f; //若在攻击，给予摩檫力
    }
    else {
        if (fabs(targetX) > 0.01f) {
            float direction = (targetX > currentX) ? 1.0f : -1.0f;
            newX = currentX + direction * _acceleration * dt;
            if ((direction > 0 && newX > targetX) || (direction < 0 && newX < targetX)) {
                newX = targetX;
            }
        }
        else {
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

    // 垂直速度 (重力逻辑）
    float newY = currentY;
    if (newY < -800.0f) newY = -800.0f;

    // 跳跃逻辑
    if (_jumpBufferTime > 0 && (_isGrounded || _coyoteTime > 0) && !_isAttacking) {
        newY = _jumpForce;
        _isGrounded = false;
        _coyoteTime = 0;
        _jumpBufferTime = 0;
    }

    _physicsBody->setVelocity(Vec2(newX, newY));
}

void Player::updateState() {
    if (_isAttacking) {
        changeState(PlayerState::ATTACKING);
        return;
    }
    if (_isDodge) {
        changeState(PlayerState::DODGING);
        return;
    }

    // 使用物理引擎判定的 _isGrounded
    if (!_isGrounded) {
        if (_velocity.y > 0.1f) {
            changeState(PlayerState::JUMPING);
        }
        else {
            changeState(PlayerState::FALLING);
        }
    }
    else {
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
    if (newState == PlayerState::FALLING) {
        // 这里不重置土狼时间，土狼时间在离地瞬间开始倒计时
    }
}


void Player::updateAnimation() {
    std::string animationName;
    bool loop = true;

    switch (_currentState) {
        case PlayerState::IDLE: animationName = "idle"; break;
        case PlayerState::RUNNING: animationName = "run"; break;
        case PlayerState::JUMPING: animationName = "jump"; loop = false; break;
        case PlayerState::FALLING: animationName = "fall"; loop = false; break;
        case PlayerState::ATTACKING:
            animationName = StringUtils::format("attack-%d", _attack_num + 1);
            loop = false;
            break;
        case PlayerState::DODGING: animationName = "dodge"; loop = false; break;
        default: animationName = "idle"; break;
    }

    if (_currentState != _previousState) {
        playAnimation(animationName, loop);
        _previousState = _currentState;
    }

    // 翻转逻辑：根据输入或速度方向翻转
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
        // 创建action
        auto action = Animate::create(animation);
        action->setTag(ANIMATION_ACTION_TAG);
        _sprite->stopAllActions();
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
                    { _isAttacking = false;}),
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

// --- 对外接口 ---

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
    // 只设置缓冲时间，实际起跳在 updatePhysics 中处理
    _jumpBufferTime = 0.1f;
}

void Player::attack() {
    if (_attackCooldown > 0 || _isAttacking) return;

    // 多段攻击
    if (_attackEngageTime > 0)
        _attack_num = (_attack_num + 1) % 3;
    else
        _attack_num = 0;

    _isAttacking = true;
    _attackCooldown = _maxAttackCooldown;
    _attackEngageTime = kMaxAttackEngageTime; // 多段攻击的衔接

    // 生成伤害判定框（Hitbox）通常在动画的特定帧回调中生成，或者在这里生成一个瞬时的Sensor
    // 这里留空，视具体Combat系统实现
}

void Player::dodge() {
    if (_dodgeCooldown > 0 || _isDodge) return; 

    _isDodge = true;
    _dodgeCooldown = _maxDodgeCooldown;
    _dodgeTime = 0.3; // 闪避持续时间

    // 可选：闪避时无敌
    // _isInvincible = true;
    // _invincibilityTime = 0.3f;
}