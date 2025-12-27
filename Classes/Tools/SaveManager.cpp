#include "SaveManager.h"

USING_NS_CC;

SaveManager* SaveManager::_instance = nullptr;

SaveManager* SaveManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new SaveManager();
        _instance->init(); // 构造时立即读取数据
    }
    return _instance;
}

SaveManager::SaveManager() {
    // 设置默认值，防止读取失败时变量未初始化
    _atkUp = 1; // 默认1倍
    _moveSpeedUp = 1;     // 默认1倍
    _skillState = 0;    // 默认未解锁
    _soulCount = 0;     // 默认0灵魂
}

SaveManager::~SaveManager() {
    // 析构逻辑
    CC_SAFE_DELETE(_instance);
}

void SaveManager::init() {
    std::string filename = SAVE_FILE_NAME;
    std::string writablePath = FileUtils::getInstance()->getWritablePath() + filename;

    // 检查存档文件是否已经存在
    bool isExist = FileUtils::getInstance()->isFileExist(writablePath);

    if (!isExist) {
        log("Save file not found. Creating default save file...");
        return;
    }

    // 如果存在，则正常读取
    auto strAtk = ReadJson::getString(filename, KEY_ATK_MULTI, false);
    if (!strAtk.empty()) _atkUp = std::atoi(strAtk.c_str());

    auto strSpd = ReadJson::getString(filename, KEY_MOVE_SPEED, false);
    if (!strSpd.empty()) _moveSpeedUp = std::atoi(strSpd.c_str());

    auto strSkill = ReadJson::getString(filename, KEY_SKILL_STATE, false);
    if (!strSkill.empty()) _skillState = std::atoi(strSkill.c_str());

    auto strSoul = ReadJson::getString(filename, KEY_SOUL_COUNT, false);
    if (!strSoul.empty()) _soulCount = std::atoi(strSoul.c_str());

    log("SaveManager Init: Atk=%d, Spd=%d, Skill=%d, Soul=%d",
        _atkUp, _moveSpeedUp, _skillState, _soulCount);
}

// 辅助：将 int 转 string 并调用 WriteJson
void SaveManager::saveIntField(std::string key, int value) {
    std::string valStr = std::to_string(value);
    bool success = WriteJson::setString(SAVE_FILE_NAME, key, valStr);
    if (!success) {
        log("SaveManager Error: Failed to save %s", key.c_str());
    }
}

// 攻击力实现
int SaveManager::getAttackUp() {
    return _atkUp;
}

void SaveManager::setAttackUp(int value) {
    if (_atkUp != value) {
        _atkUp = value;
        saveIntField(KEY_ATK_MULTI, _atkUp);
    }
}

// 移动速率实现
int SaveManager::getMoveSpeedUp() {
    return _moveSpeedUp;
}

void SaveManager::setMoveSpeedUp(int value) {
    if (_moveSpeedUp != value) {
        _moveSpeedUp = value;
        saveIntField(KEY_MOVE_SPEED, _moveSpeedUp);
    }
}

// 技能状态实现
int SaveManager::getSkillState() {
    return _skillState;
}

int SaveManager::unlockNextSkill() {
    if (_skillState < 3) {
        _skillState++;
        saveIntField(KEY_SKILL_STATE, _skillState);
        log("Skill Unlocked! Current Level: %d", _skillState);
    }
    else {
        log("All Skills already unlocked.");
    }
    return _skillState;
}

void SaveManager::setSkillState(int state) {
    // 简单的边界检查
    if (state < 0) state = 0;
    if (state > 3) state = 3;

    _skillState = state;
    saveIntField(KEY_SKILL_STATE, _skillState);
}

// 灵魂数实现
int SaveManager::getSoulCount() {
    return _soulCount;
}

void SaveManager::addSoul(int amount) {
    _soulCount += amount;
    if (_soulCount < 0) _soulCount = 0; // 防止负数
    saveIntField(KEY_SOUL_COUNT, _soulCount);
}

bool SaveManager::tryConsumeSoul(int cost) {
    if (_soulCount >= cost) {
        _soulCount -= cost;
        saveIntField(KEY_SOUL_COUNT, _soulCount);
        return true;
    }
    return false;
}

void SaveManager::resetSaveData() {
    _atkUp = 0;
    _moveSpeedUp = 0;
    _skillState = 0;
    _soulCount = 0;

    // 依次写入重置
    saveIntField(KEY_ATK_MULTI, _atkUp);
    saveIntField(KEY_MOVE_SPEED, _moveSpeedUp);
    saveIntField(KEY_SKILL_STATE, _skillState);
    saveIntField(KEY_SOUL_COUNT, _soulCount);

    log("Save Data Reset Complete.");
}

bool SaveManager::hasSaveFile() {
    // 检查文件是否存在
    std::string filename = SAVE_FILE_NAME;
    std::string writablePath = cocos2d::FileUtils::getInstance()->getWritablePath() + filename;
    return cocos2d::FileUtils::getInstance()->isFileExist(writablePath);
}