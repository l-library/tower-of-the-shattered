#pragma once
#include "cocos2d.h"
#include "SkillManager.h"

// 玩家状态枚举
enum class PlayerState {
	IDLE,           // 待机
	RUNNING,        // 奔跑
	DODGING,		// 闪避
	JUMPING,        // 跳跃中
	FALLING,        // 下落中
	LANDING,		// 落地
	ATTACKING,      // 攻击中
	SKILLING,		// 特殊攻击中
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
//动画标签
#define ANIMATION_ACTION_TAG 1001

//默认土狼时间
constexpr double kCoyoteTime = 0.15;
//默认两端攻击之间最大时间差
constexpr double kMaxAttackEngageTime = 0.7;//在0.7秒内继续攻击则衔接下一段攻击
// 定义脚部传感器Tag，用于碰撞检测区分身体和脚
const int TAG_BODY = 10;
const int TAG_FEET = 11;

// 定义两次脚步声播放的间隔
const float kStepSoundsInterval = 0.5f;

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

	/*----各操作实现/对外接口----*/
	void moveLeft();
	void moveRight();
	void stopMoving();
	void jump();
	void attack();
	void dodge();

	/**
	* @brief 获得玩家当前状态
	* @param[in] void
	* @return (const)玩家当前状态: idle, run, dodge, jump, fall, attack, hurt, dead
	***/
	const std::string getCurrentState() const;

	/**
	* @brief 获得玩家当前血量
	* @param[in] void
	* @return (const double)玩家当前血量
	***/
	const double getHealth() const { return _health; };

	/**
	* @brief 获得玩家当前最大血量
	* @param[in] void
	* @return (const double)玩家最大血量
	***/
	const double getMaxHealth() const { return _maxHealth; };

	/**
	* @brief 获得玩家当前法力值
	* @param[in] void
	* @return (const double)玩家当前法力值
	***/
	const double getMagic() const { return _magic; };
	const void setMagic(double magic) { _magic = magic; }

	/**
	* @brief 获得玩家当前最大法力值
	* @param[in] void
	* @return (const double)玩家最大法力值
	***/
	const double getMaxMagic() const { return _maxMagic; };

	/**
	* @brief 获得玩家是否运行控制
	* @param[in] void
	* @return (const)玩家控制状态 true，false
	***/
	const bool canBeControled() const { return _controlEnabled && !_isAttacking && !_isSkilling && !_isDodge; };

	const Direction getDirection() const { return _direction; };

	SkillManager* getSkillManager() const { return _skillManager; };

	/**
	* @brief 获得玩家图像
	* @return 玩家当前的图像（常量指针）
	***/
	const cocos2d::Sprite* getSprite() const;

	/**
	* @brief 玩家释放技能
	* @param[in] string技能名称 常量引用
	* @return 释放成功/失败（bool）
	***/
	bool skillAttack(const std::string& name);

	/**
	* @brief 获得某个技能是否被解锁了
	* @param[in] string技能名称 常量引用
	* @return 解锁/没有解锁
	***/
	bool isUnlocked(const std::string& name);

	// 设置主角属性
	void modifyMaxHealth(double value) { _maxHealth += value; _health += value; } // 增加上限并回血
	void modifyAttackDamage(double value) { _playerAttackDamage += value; }
	void modifySpeed(double value) { _speed += value; }
	void modifyMagicRestore(double value) { _magicRestore += value; }

	//利用宏生成一个create函数
	CREATE_FUNC(Player);

protected:
	/**
	* @brief 更新主角状态
	* @details 每帧调用一次
	***/
	void update(float dt);

	/**
	* @brief 改变主角状态
	* @details 将同时更新当前状态和前一个状态，若状态不变，则不会更新
	* @param[in] 要更改为的状态
	***/
	void changeState(PlayerState newState);

	/**
	* @brief 初始化主角物理效果
	* @details 将主角物理属性初始化
	***/
	void initPhysics();

	void shootBullet();

private:
	/*----各个更新函数----*/
	void updateTimers(float dt);//更新计时器
	void updateState();//更新状态
	void updatePhysics(float dt);//更新物理
	void updateAnimation();//更新动画

	//碰撞回调函数
	bool onContactBegin(cocos2d::PhysicsContact& contact);
	bool onContactSeparate(cocos2d::PhysicsContact& contact);

	/*----主角属性----*/
	//主角图像_sprite（待机动作的第一帧）
	cocos2d::Sprite* _sprite;
	//主角物理模型
	cocos2d::PhysicsBody* _physicsBody;
	//主角模型大小
	cocos2d::Size _physicsSize;
	//主角血量
	double _health;
	double _maxHealth;
	//主角法力值
	double _magic;
	double _maxMagic;
	//主角闪避时间（无敌时间）
	double _maxDodgeTime;
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
	//主角的空中闪避次数
	int _maxDodgeTimes;
	//主角每秒恢复的魔法值
	double _magicRestore;

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
	//两次脚步声之间的声音间隔
	float _stepSoundsInterval;
	//普通状态下的碰撞掩码
	int _originalMask;
	//冲刺状态下的碰撞掩码
	int _dodgeMask;

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
	//是否正在特殊攻击
	bool _isSkilling;
	//是否正在受击
	bool _isHurt;
	//是否死亡
	bool _isDead;
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
	int _footContactCount;
	//空中闪避次数
	int _dodgeTimes;

	/*----攻击和技能数值----*/
	double _playerAttackDamage;// 普通攻击伤害

	/*----输入----*/
	float _moveInput;

	/*----技能管理器----*/
	SkillManager* _skillManager;
	
};