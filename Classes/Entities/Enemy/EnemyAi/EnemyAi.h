#pragma once
#include <string>
#include<vector>
// 前向声明EnemyBase类
class EnemyBase;

// 前向声明Player类
class Player;

// 行为结果类型定义
using BehaviorResult = std::pair<bool, float>;

// 敌人AI类 - 只包含静态方法，用于存储通用的AI辅助逻辑（查找玩家，检测玩家范围，读取玩家指令等）
// 具体行为决策由enemy实现，此类中的函数仅供辅助
class EnemyAi
{
private:
    // 构造函数和析构函数设为private，禁止创建对象
    EnemyAi();
    ~EnemyAi();

public:
    // 辅助方法 - 查找玩家
    static Player* findPlayer(EnemyBase* enemy);
    
    // 辅助方法 - 检测玩家是否在攻击范围内
    static bool isPlayerInRange(EnemyBase* enemy, float range);
    
    // 辅助方法 - 检测玩家是否在同一水平线
    static bool isPlayerHorizontal(EnemyBase* enemy, float yDiffThreshold);
    
    // 辅助方法 - 检测玩家是否可见（路径上没有墙）
    static bool isPlayerVisible(EnemyBase* enemy);
    
    // AI决策方法 - 随机决策
    static std::string simpleRandomDecision(EnemyBase* enemy, float delta, const std::vector<std::string>& behaviors);
    
    // AI决策方法 - 基于玩家距离的决策
    static std::string distanceBasedDecision(EnemyBase* enemy, float delta, float attackRange, float detectionRange);
};
