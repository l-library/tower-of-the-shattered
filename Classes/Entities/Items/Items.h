#pragma once
#include "cocos2d.h"
#include "ItemManager.h"
#include "../Player/Player.h"

class Items : public cocos2d::Sprite {
public:
    /**
     * @brief 创建一个掉落物对象
     * @param itemId 对应 ItemManager 中的物品ID
     * @return Items*
     */
    static Items* createWithId(int itemId);

    virtual bool init(int itemId);

    /**
     * @brief 物品被拾起时的逻辑
     * @details 播放特效、增加数据、然后销毁自己
     * @param player 捡起物品的玩家指针
     */
    void bePickedUp(Player* player);

    /**
     * @brief 获取物品ID
     */
    int getItemId() const { return _itemId; }

private:
    int _itemId;
    bool _isPickedUp; // 防止双重碰撞导致的多次拾取

    /**
     * @brief 初始化物理属性
     */
    void initPhysics();

    /**
     * @brief 播放待机浮动动画
     */
    void playFloatAnimation();
};