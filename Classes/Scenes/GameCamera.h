#pragma once

#include "cocos2d.h"
#include "Entities/Player/Player.h"
#include <string>
#include <vector>
#include <unordered_map>

constexpr int kUIZorder = 30;

class GameCamera : public cocos2d::Ref {
public:
    static GameCamera* create(cocos2d::Scene* scene, Player* player, cocos2d::TMXTiledMap* map);

    void update(float dt);

    cocos2d::Node* getUIRoot() { return _uiRoot; }

private:
    GameCamera();
    bool init(cocos2d::Scene* scene, Player* player, cocos2d::TMXTiledMap* map);

    // --- UI 初始化相关 ---
    void initUI();

    // 返回 ProgressTimer 指针，同时通过引用参数返回 Label 指针
    cocos2d::ProgressTimer* createStatusBar(const std::string& borderPath,
        const std::string& barPath,
        const cocos2d::Vec2& position,
        cocos2d::Label*& outLabel);

    // 创建单个技能槽
    void createSkillSlot(const std::string& skillName,
        const std::string& iconPath,
        const cocos2d::Vec2& position);

    void initBar();
    void initSkillIcons();
    void initGold();
    void initItemIcons();

    // --- 内部引用 ---
    cocos2d::Scene* _scene;
    Player* _player;
    cocos2d::TMXTiledMap* _map;
    cocos2d::Camera* _defaultCamera;
    cocos2d::Camera* _uiCamera;
    cocos2d::Node* _uiRoot;

    // --- UI 组件引用 ---
    cocos2d::ProgressTimer* _hpBar;
    cocos2d::Label* _hpLabel;

    cocos2d::ProgressTimer* _mpBar;
    cocos2d::Label* _mpLabel;

    cocos2d::Label* _goldLabel;

    cocos2d::Vec2 _nextIconPosition; // 存放下一个物品的坐标
    float _itemSize = 32.0f; // 物品图标大小

    // Key为技能名称，value为冷却条
    std::unordered_map<std::string, cocos2d::ProgressTimer*> _skillCDTimers;

    // 放大倍数
    float _zoomFactor;
};
