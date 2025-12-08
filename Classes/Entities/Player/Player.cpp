#include <cmath>
#include "Player.h"
#include "AudioEngine.h"

USING_NS_CC;

/*----创建玩家和初始化部分----*/
Player* Player::createNode()
{
	return Player::create();
}

bool Player::init()
{
	if (!Node::init())
	{
		return false;
	}

	//加载主角图像
	_sprite = Sprite::create("player/magician-idle-0.png");
    _sprite->setAnchorPoint(Vec2(0.5, 0));
	this->addChild(_sprite);
	//播放主角待机动画
	playAnimation("idle", true);

	/*---初始化主角各属性---*/
    _currentState = PlayerState::IDLE;
    _previousState = PlayerState::IDLE;
    _direction = Direction::RIGHT;
    // 基础属性
    _maxHealth = 100.0;
    _health = _maxHealth;
    _speed = 300.0;
    _jumpForce = 500.0;
    _dodgeForce = 600.0;
    _acceleration = 2000.0;
    _deceleration = 2500.0;
    _maxAttackCooldown = 0.3;
    _maxDodgeCooldown = 1.0;
    _dodgeTime = 0;
    // 状态标志
    _isGrounded = true;//标记：有了具体场景之后应当设置为false
    _isDodge = false;
    _isAttacking = false;
    _isInvincible = false;
    _controlEnabled = true;
    _attack_num = 0;
    // 计时器
    _jumpBufferTime = 0.0;
    _coyoteTime = 0.0;
    _dodgeCooldown = 0.0;
    _attackCooldown = 0.0;
    _invincibilityTime = 0.0;
    _attackEngageTime = 0.0;
    // 物理效果
    _velocity = Vec2::ZERO;
    //输入
    _moveInput = 0.0;

    //启用update函数
    this->scheduleUpdate();
	return true;
}

void Player::playAnimation(const std::string& name, bool loop) {
    //获取动画缓存单个实例中名为name的动画
    auto animation = AnimationCache::getInstance()->getAnimation(name);
    //如果成功获取
    if (animation) {
        //创建action
        auto action = Animate::create(animation);
        _sprite->stopAllActions();
        //根据是否循环确定播放方式
        if (loop) {
            _sprite->runAction(RepeatForever::create(action));
        }
        else {
            _sprite->runAction(Sequence::create(
                action,
                CallFunc::create([this]() {
                    _isAttacking = false;
                    }),//使用lambda回调函数，动作结束后标记攻击状态结束
                nullptr
            ));
        }
    }
}

/*----update及相关函数部分----*/
void Player::update(float dt) {
	if (!_controlEnabled) return;

	// 更新计时器
	updateTimers(dt);

	// 更新状态
	updateState();

	// 更新物理
	updatePhysics(dt);

	// 更新动画
	updateAnimation();
}

void Player::updateTimers(float dt) {
    // 跳跃缓冲时间
    if (_jumpBufferTime > 0) {
        _jumpBufferTime -= dt;
    }
    // 土狼时间（边缘跳跃缓冲）
    if (_isGrounded) {
        _coyoteTime = kCoyoteTime;
    }
    else {
        if (_coyoteTime > 0) {
            _coyoteTime -= dt;
        }
    }

    // 闪避冷却
    if (_dodgeCooldown > 0) {
        _dodgeCooldown -= dt;
    }

    //闪避持续
    if (_dodgeTime > 0) {
        _dodgeTime -= dt;
        if (_dodgeTime < 0) {
            _isDodge = false;
            changeState(PlayerState::IDLE);
        }
    }

    // 攻击冷却
    if (_attackCooldown > 0) {
        _attackCooldown -= dt;
    }

    //攻击衔接
    if (_attackCooldown <= 0 && _attackEngageTime > 0) {
        _attackEngageTime -= dt;
    }

    // 无敌时间
    if (_isInvincible) {
        _invincibilityTime -= dt;
        if (_invincibilityTime <= 0) {
            _isInvincible = false;
            this->setOpacity(255);
        }
        else {
            // 闪烁效果
            float blink = sin(_invincibilityTime * 20) * 0.5f + 0.5f;
            this->setOpacity(blink * 255);
        }
    }
}

void Player::updateState() {
    // 检查地面和墙壁
    checkCollisions();

    // 根据速度确定状态
    if (_isAttacking) {
        // 攻击状态优先
        changeState(PlayerState::ATTACKING);
        return;
    }
    else if (_isDodge) {
        // 闪避状态
        changeState(PlayerState::DODGING);
        return;
    }
    else if (!_isGrounded) {
        if (_velocity.y > 0) {
            changeState(PlayerState::JUMPING);
        }
        else {
            changeState(PlayerState::FALLING);
        }
    }
    else if (fabs(_velocity.x) > 10.0) {
        changeState(PlayerState::RUNNING);
    }
    else {
        changeState(PlayerState::IDLE);
    }
}

void Player::checkCollisions() {
    /*等待场景完善*/
}

void Player::changeState(PlayerState newState) {
    if (_currentState == newState) return;

    // 进入新状态
    switch (newState) {
        case PlayerState::FALLING:
            _coyoteTime = 0; // 重置土狼时间
            break;
        case PlayerState::LANDING:
            //音效相关待实现
            //AudioEngine::play2d("sounds/land.wav");
            break;
        case PlayerState::IDLE:
            _velocity.x = 0.0;
            break;
        default:
            break;
    }
    _currentState = newState;
}

void Player::updatePhysics(float dt) {
    //if (!_physicsBody) return;

    if (_isAttacking == true)return;

    // 应用移动力
    applyMovementForce();

    // 限制速度
    clampVelocity();

    // 更新Sprite位置
    Vec2 pos = this->getPosition();
    pos.x += _velocity.x * dt;//根据速度计算坐标
    pos.y += _velocity.y * dt;
    this->setPosition(pos);

    // 更新物理身体位置
    /*if (_physicsBody) {
        _physicsBody->setPositionOffset(b2Vec2(pos.x / PTM_RATIO, pos.y / PTM_RATIO), 0);
    }*/
}

void Player::applyMovementForce() {
    // 水平移动
    double targetSpeed = _moveInput * _speed;

    // 闪避状态
    if (_isDodge) {
        targetSpeed = (_direction == Direction::RIGHT ? 1 : -1) * _dodgeForce;
    }

    // 计算加速度
    double speedDiff = targetSpeed - _velocity.x;
    double accelRate = (fabs(targetSpeed) > 0.01f) ? _acceleration : _deceleration;

    // 应用力
    double movement = speedDiff * accelRate;
    _velocity.x += movement;

    // 重力
    if (!_isDodge&&!_isGrounded) {
        _velocity.y -= kGravity; // 重力加速度
    }
}

void Player::clampVelocity() {
    // 限制水平速度
    double maxSpeed = _isDodge ? _dodgeForce : _speed;
    if (fabs(_velocity.x) > maxSpeed) {
        _velocity.x = (_velocity.x > 0) ? maxSpeed : -maxSpeed;
    }

    // 虽然不符合物理但是符合游戏
    // 限制下落速度
    if (_velocity.y < -800.0) {
        _velocity.y = -800.0;
    }
}

void Player::updateAnimation()
{
    std::string animationName;
    bool loop = true;

    switch (_currentState) {
        case PlayerState::IDLE:
            animationName = "idle";
            break;
        case PlayerState::RUNNING:
            animationName = "run";
            break;
        case PlayerState::JUMPING:
            animationName = "jump";
            loop = false;
            break;
        case PlayerState::FALLING:
            animationName = "fall";
            loop = false;
            break;
        case PlayerState::ATTACKING:
            char tmp[20];//存放攻击段数名
            sprintf(tmp, "attack-%d", _attack_num + 1);
            animationName = tmp;
            loop = false;
            break;
        case PlayerState::DODGING:
            animationName = "dodge"; 
            loop = false;
            break;
        case PlayerState::LANDING:
            animationName = "land";
            loop = false;
            break;
        case PlayerState::HURT:
            animationName = "hurt";
            loop = false;
            break;
        case PlayerState::DEAD:
            animationName = "dead";
            loop = false;
            break;
        default:
            animationName = "idle";
    }
    // 如果状态改变，播放新动画
    if (_currentState != _previousState) {
        playAnimation(animationName, loop);
        _previousState = _currentState;
    }

    // 设置方向
    if (_moveInput > 0.1f) {
        _direction = Direction::RIGHT;
        _sprite->setFlippedX(false);
    }
    else if (_moveInput < -0.1f) {
        _direction = Direction::LEFT;
        _sprite->setFlippedX(true);
    }
}

/*----部分对外接口----*/
std::string Player::getCurrentState()
{
    std::string current;
    switch (_currentState) {
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
            char tmp[20];//存放攻击段数名
            sprintf(tmp,"attack-%d",_attack_num+1);
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

void Player::moveLeft()
{
    _moveInput = -1.0;
}

void Player::moveRight()
{
    _moveInput = 1.0;
}

void Player::stopMoving()
{
    _moveInput = 0.0;
    _velocity.set(Vec2(0, 0));
}

void Player::jump()
{
    // 跳跃缓冲
    _jumpBufferTime = 0.1f;

    if (_isGrounded || _coyoteTime > 0) {
        _velocity.y = _jumpForce;
        _coyoteTime = 0;
        _isGrounded = false;//测试用，有了完整场景后应删除
    }
}

void Player::attack() {
    if (_attackCooldown > 0 || _isAttacking) return;
    //多段攻击判断
    if (_attackEngageTime > 0 && !_isAttacking)
        _attack_num = (_attack_num + 1) % 3;
    else 
        _attack_num = 0;
    _isAttacking = true;
    _attackCooldown = _maxAttackCooldown;//攻击的冷却时间
    _attackEngageTime = kMaxAttackEngageTime;//多段攻击的衔接
    changeState(PlayerState::ATTACKING);

    //// 创建攻击碰撞区域
    //auto attackSize = Size(50, 30);
    //float offsetX = (_direction == Direction::RIGHT) ? 40 : -40;
    //auto attackRect = Rect(
    //    this->getPositionX() + offsetX - attackSize.width / 2,
    //    this->getPositionY() + 30,
    //    attackSize.width,
    //    attackSize.height
    //);

    // 这里应该通知游戏世界的碰撞检测系统
    //SimpleAudioEngine::getInstance()->playEffect("sounds/attack.wav");
}

void Player::dodge()
{
    if (_dodgeCooldown > 0 || _isDodge) return;
    _isDodge = true;
    _dodgeCooldown = _maxDodgeCooldown;
    _dodgeTime = 0.5;
    changeState(PlayerState::DODGING);
}