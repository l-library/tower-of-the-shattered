#pragma once
#include "cocos2d.h"
#include "Entities/Player/Player.h"

// 前向声明
class Player;

class GameManager {
public:
    static GameManager* getInstance();

    // 保存Player对象（从场景中移除并持有）
    void savePlayer(Player* player);

    // 获取保存的Player对象（转移所有权）
    Player* getPlayer();

    // 检查是否有保存的Player
    bool hasPlayer() const { return _savedPlayer != nullptr; }

    // 清理保存的Player
    void clearPlayer();

    // 重置所有
    void reset();

private:
    GameManager();
    ~GameManager();

    static GameManager* _instance;

    Player* _savedPlayer;  // 直接保存Player对象
};