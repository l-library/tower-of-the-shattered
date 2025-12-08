#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"

// 玩家状态枚举
enum class PlayerState {
	IDLE,           // 待机
	RUNNING,        // 奔跑
	DODGING,		// 闪避
	JUMPING,        // 跳跃中
	FALLING,        // 下落中
	LANDING,		// 落地
	ATTACKING,      // 攻击中
	HURT,           // 受伤
	DEAD			// 死亡
};
//方向枚举
enum class Direction {
	RIGHT,
	LEFT
};

//规定32个像素为1米
#define PTM_RATIO 32

//规定作用在玩家身上的重力加速度
constexpr double kGravity = 10.0;
//默认土狼时间
constexpr double kCoyoteTime = 0.15;
//默认两端攻击之间最大时间差
constexpr double kMaxAttackEngageTime = 0.7;//在0.7秒内继续攻击则衔接下一段攻击


/**
* @brief 储存主角的各类信息 
* @details 这个类继承自Node
***/
class Player :public cocos2d::Node
{
public:
	/**
	* @brief 创建一个player对象
	* @param[in] 无
	* @return 指向创建好的对象的指针Node*
	***/
	static Player* createNode();

	/**
	* @brief 初始化主角对象
	* @details 这个函数会在create函数中被调用
	***/
	virtual bool init();

	/**
	* @brief 播放动画
	* @param[in] 动画名称string name，是否循环播放bool loop
	* @return 无
	***/
	void playAnimation(const std::string& name, bool loop = false);

	/**
	* @brief 获得玩家当前状态
	* @param[in] void
	* @return 玩家当前状态: idle, run, dodge, jump, fall, attack, hurt, dead
	***/
	std::string getCurrentState();

	/*----各操作实现/对外接口----*/
	void moveLeft();
	void moveRight();
	void stopMoving();
	void jump();
	void attack();
	void dodge();

	//利用宏生成一个create函数
	CREATE_FUNC(Player);

protected:
	/**
	* @brief 更新主角状态
	* @details 每帧调用一次
	***/
	void update(float dt);

	/**
	* @brief 限制速度
	* @details 保证角色的移动速度不会超过角色的最大速度
	***/
	void clampVelocity();

	/**
	* @brief 应用作用力
	* @details 模仿真实物理世界：作用力，速度，加速度，重力
	***/
	void applyMovementForce();

	/**
	* @brief 检查地面/墙壁/碰撞
	* @details 该函数待完善
	***/
	void checkCollisions();

	/**
	* @brief 改变主角状态
	* @details 将同时更新当前状态和前一个状态，若状态不变，则不会更新
	* @param[in] 要更改为的状态
	***/
	void changeState(PlayerState newState);

private:
	/*----各个更新函数----*/
	void updateTimers(float dt);//更新计时器
	void updateState();//更新状态
	void updatePhysics(float dt);//更新物理
	void updateAnimation();//更新动画

	/*----主角属性----*/
	//主角图像_sprite（待机动作的第一帧）
	cocos2d::Sprite* _sprite;
	//主角血量
	double _health;
	double _maxHealth;
	//主角法力值
	double _magic;
	double _maxMagic;
	//主角移动速度
	double _speed;
	//主角跳跃高度
	double _jumpForce;
	//主角闪避速度
	double _dodgeForce;
	//主角闪避的持续时间
	double _dodgeTime;
	//主角攻击冷却时间
	double _maxAttackCooldown;
	//主角的闪避冷却时间
	double _maxDodgeCooldown;

	/*----计时器----*/
	//跳跃缓冲时间
	double _jumpBufferTime;
	//土狼时间
	double _coyoteTime;
	//闪避冷却
	double _dodgeCooldown;
	//攻击冷却
	double _attackCooldown;
	//无敌时间
	double _invincibilityTime;
	//两端攻击之间衔接的时间差
	double _attackEngageTime;

	/*----主角当前状态----*/
	//是否允许控制
	bool _controlEnabled;
	//是否位于地面上
	bool _isGrounded;
	//是否无敌
	bool _isInvincible;
	//是否正在闪避
	bool _isDodge;
	//是否正在攻击
	bool _isAttacking;
	//当前状态
	PlayerState _currentState;
	//前一个状态
	PlayerState _previousState;
	//面朝方向
	Direction _direction;
	//速度
	cocos2d::Vec2 _velocity;
	//加速度
	float _acceleration;
	float _deceleration;
	//攻击段数
	int _attack_num;

	/*----输入----*/
	float _moveInput;
};