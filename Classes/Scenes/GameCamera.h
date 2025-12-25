#ifndef __GAME_CAMERA_H__
#define __GAME_CAMERA_H__

#include "cocos2d.h"
#include "Entities/Player/Player.h"

class GameCamera : public cocos2d::Ref {
public:
    static GameCamera* create(cocos2d::Scene* scene, Player* player, cocos2d::TMXTiledMap* map);

    void update(float dt);

private:
    GameCamera();
    bool init(cocos2d::Scene* scene, Player* player, cocos2d::TMXTiledMap* map);

    // UI 初始化子函数
    void initUI();
    void initBar();
    void initSkillIcons();
    void initGold();

    // 内部引用
    cocos2d::Scene* _scene;
    Player* _player;
    cocos2d::TMXTiledMap* _map;
    cocos2d::Camera* _defaultCamera;
    cocos2d::Camera* _uiCamera;
    cocos2d::Node* _uiRoot; // 所有 UI 元素的根节点

    // UI 组件引用，方便 update 中刷新
    cocos2d::ProgressTimer* _hpBar;
    cocos2d::ProgressTimer* _mpBar;
    cocos2d::ProgressTimer* _skillCDTimer;
    cocos2d::ProgressTimer* _skillCDTimer_2;
    cocos2d::ProgressTimer* _skillCDTimer_3;
    cocos2d::Label* _hp_label;
    cocos2d::Label* _mp_label;
    cocos2d::Label* _gold_num;
    
    
    //放大倍数
    float _zoom_factor;
};

#endif