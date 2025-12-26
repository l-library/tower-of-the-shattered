#include "EnemyAi.h"


// 构造函数和析构函数的实现（private，不会被外部调用）
EnemyAi::EnemyAi() {};
EnemyAi::~EnemyAi() {};

// 查找玩家
Player* EnemyAi::findPlayer(EnemyBase* enemy)
{
    if (!enemy) return nullptr;
    
    // 查找玩家节点
    auto parent = enemy->getParent();
    if (parent != nullptr)
    {
        auto children = parent->getChildren();
        for (auto child : children)
        {
            if (typeid(*child) == typeid(Player))
            {
                Player* player = static_cast<Player*>(child);
                enemy->setPlayer(player);
                return player;
            }
        }
    }
    
    enemy->setPlayer(nullptr);
    return nullptr;
}

// 检测玩家是否在攻击范围内
bool EnemyAi::isPlayerInRange(EnemyBase* enemy, float range)
{
    if (!enemy || !enemy->getPlayer())
    {
        return false;
    }
    
    // 检测玩家是否在攻击范围内
    float distance = enemy->getPosition().distance(enemy->getPlayer()->getPosition());
    return distance <= range;
}

// 检测玩家是否在同一水平线
bool EnemyAi::isPlayerHorizontal(EnemyBase* enemy, float yDiffThreshold)
{
    if (!enemy || !enemy->getPlayer())
    {
        return false;
    }
    
    // 检测玩家是否在同一水平线（Y坐标差小于阈值）
    float yDiff = abs(enemy->getPosition().y - enemy->getPlayer()->getPosition().y);
    return yDiff <= yDiffThreshold;
}

// 检测玩家是否可见（路径上没有墙）
bool EnemyAi::isPlayerVisible(EnemyBase* enemy)
{
    if (!enemy) return false;
    
    Player* player = findPlayer(enemy);
    if (player == nullptr)
    {
        return false;
    }
    
    // 获取当前场景的物理世界
    auto scene = Director::getInstance()->getRunningScene();
    if (scene == nullptr) return false;
    
    auto physicsWorld = scene->getPhysicsWorld();
    if (physicsWorld == nullptr) return false;
    
    // 创建射线检测回调
    bool hasWall = false;
    
    physicsWorld->rayCast([&hasWall](PhysicsWorld& world, const PhysicsRayCastInfo& info, void* data) -> float {
        // 检查碰撞的物体是否是墙壁
        if (info.shape->getBody()->getCategoryBitmask() == WALL_MASK)
        {
            hasWall = true;
            return 0.0f; // 找到墙壁，停止检测
        }
        return -1.0f; // 继续检测
    }, enemy->getPosition(), player->getPosition(), nullptr);
    
    // 如果没有墙壁阻挡，则玩家可见
    return !hasWall;
}

// 简单随机决策
std::string EnemyAi::simpleRandomDecision(EnemyBase* enemy, float delta, const std::vector<std::string>& behaviors)
{
    if (behaviors.empty())
    {
        return "idle";
    }
    
    // 随机选择一个行为
    int randomIndex = rand() % behaviors.size();
    return behaviors[randomIndex];
}

// 基于玩家距离的决策
std::string EnemyAi::distanceBasedDecision(EnemyBase* enemy, float delta, float attackRange, float detectionRange)
{
    // 查找玩家
    findPlayer(enemy);
    
    // 如果检测到玩家
    if (enemy->getPlayer() != nullptr)
    {
        // 检测玩家是否可见、在攻击范围内
        if (isPlayerVisible(enemy))
        {
            float distance = enemy->getPosition().distance(enemy->getPlayer()->getPosition());
            
            if (distance <= attackRange)
            {
                // 在攻击范围内，随机选择攻击行为
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
            else if (distance <= detectionRange)
            {
                // 在检测范围内，接近玩家
                return "approach";
            }
        }
    }
    
    // 默认返回待机行为
    return "idle";
}
