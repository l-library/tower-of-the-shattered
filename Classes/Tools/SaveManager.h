#pragma once
#include "cocos2d.h"
#include "ReadJson.h"
#include "WriteJson.h"
#include <string>

// 定义JSON文件名称
#define SAVE_FILE_NAME "user_save.json"

// 定义JSON字段的Key
#define KEY_ATK_MULTI   "atk_multiplier" // 攻击倍率
#define KEY_MOVE_SPEED  "move_speed"     // 移动速率
#define KEY_SKILL_STATE "skill_state"    // 技能解锁状态 (0-3)
#define KEY_SOUL_COUNT  "soul_count"     // 持有灵魂数

class SaveManager {
public:
    // 获取单例实例
    static SaveManager* getInstance();

    // 初始化（游戏启动时调用，负责读取文件到内存）
    void init();

    // 攻击力倍率
    int getAttackUp();
    void setAttackUp(int value);

    // 移动速率
    int getMoveSpeedUp();
    void setMoveSpeedUp(int value);

    // 技能状态 (0,1,2,3)
    /**
     * 获取当前技能解锁进度
     * 0: 无技能, 1: 解锁技能A, 2: 解锁技能A+B, 3: 全解锁
     */
    int getSkillState();

    /**
     * @brief 尝试解锁下一个技能
     * @details 如果当前是0，变1；是1变2；最大到3。
     * @return 解锁后的新状态
     */
    int unlockNextSkill();

    // 强制设置技能状态
    void setSkillState(int state);

    // 灵魂数
    int getSoulCount();
    // 增加灵魂 (负数则为消耗)
    void addSoul(int amount);
    // 检查是否足够支付，如果足够则扣除并返回true
    bool tryConsumeSoul(int cost);

    // 调试/重置
    // 删除存档或重置所有数据
    void resetSaveData();

    /**
     * @brief 静态检查是否存在存档文件
     * @return true=存在存档, false=不存在
     */
    static bool hasSaveFile();

    /**
     * @brief 当前场景是否从菜单场景切换过来
     * @return true=是，false=否
     */
    bool sceneComeFromMenu() { return _sceneComeFromMenu; }
    void setsceneComeFromMenu(bool input) { _sceneComeFromMenu = input; }

private:
    SaveManager(); // 私有构造
    ~SaveManager();

    // 辅助函数：保存单个整型字段
    void saveIntField(std::string key, int value);

    static SaveManager* _instance;

    // 内存中的缓存变量
    int _atkUp;
    int _moveSpeedUp;
    int _skillState;
    int _soulCount;
    bool _sceneComeFromMenu = false;
};