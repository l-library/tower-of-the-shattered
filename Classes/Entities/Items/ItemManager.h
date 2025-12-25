#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>
#include <map>

// 前向声明 Player，避免循环引用
class Player;

// 物品效果类型枚举
enum class ItemEffectType {
    NONE,
    HP_MAX_UP,      // 提升最大生命
    MP_MAX_UP,      // 提升最大法力
    ATTACK_UP,      // 提升攻击力
    SPEED_UP,       // 提升移动速度
    MP_RESTORE_UP,  // 提升回蓝速度
    JUMP_FORCE_UP,  // 提升跳跃高度
    DODGE_TIME_UP,  // 提升闪避时间
    DODGE_COOLDOWN, // 减少闪避冷却
    //DOUBLE_JUMP,    // 二段跳
    //DOUBLE_DOUDGE,  // 空中二段闪避
    SPECIAL         // 特殊机制（需要脚本或硬编码支持）
};

/**
 * @brief 物品数据结构（对应JSON中的一项）
 */
struct ItemData {
    int id;                     // 物品ID
    std::string name;           // 物品名称
    std::string description;    // 效果描述
    std::string flavorText;     // 吐槽/背景故事
    std::string iconPath;       // 图标资源路径

    ItemEffectType effectType;  // 效果类型
    double effectValue;         // 效果数值 (例如 +10)

    // 如果有更复杂的参数，可以使用 Map
    // std::map<std::string, std::string> extraParams;
};

/**
 * @brief 物品管理器 (单例)
 * @details 管理全局掉落物配置、玩家背包(遗物)、金币
 */
class ItemManager : public cocos2d::Ref {
public:
    static ItemManager* getInstance();
    static void destroyInstance();

    // 初始化：读取 JSON 配置
    bool init(const std::string& configFilename);

    // ---- 物品/遗物操作 ----

    /**
     * @brief 获得物品
     * @details 将物品加入背包并立即应用效果到 Player
     * @param player 玩家指针
     * @param itemId 物品ID
     */
    void gainItem(Player* player, int itemId);

    /**
     * @brief 移除物品（可选功能）
     * @details 移除效果
     */
    void removeItem(Player* player, int itemId);

    /**
     * @brief 检查是否拥有某物品
     */
    bool hasItem(int itemId) const;

    /**
     * @brief 获取所有物品的配置信息（用于图鉴等）
     */
    const ItemData* getItemConfig(int itemId) const;

    /**
     * @brief 获取玩家当前拥有的所有物品列表
     * @return 物品ID的列表
     */
    const std::vector<int>& getOwnedItems() const { return _ownedItemIds; }

    // ---- 金币操作 ----

    void addGold(int amount);
    bool spendGold(int amount);
    int getGold() const { return _gold; }

    // ---- UI 展示接口 ----

    /**
     * @brief 创建一个物品展示节点
     * @details 包含图标，点击可能需要自行绑定事件
     * @param itemId 物品ID
     * @return Sprite指针 (autorelease)
     */
    cocos2d::Sprite* createItemIcon(int itemId);

private:
    ItemManager();
    ~ItemManager();

    // 应用/移除效果的内部逻辑
    void applyEffect(Player* player, const ItemData& item, bool isAdd);

    static ItemManager* _instance;

    // 所有物品配置缓存 <ID, Data>
    std::map<int, ItemData> _itemConfigCache;

    // 当前拥有的物品 ID 列表
    std::vector<int> _ownedItemIds;

    // 当前金币
    int _gold;
};