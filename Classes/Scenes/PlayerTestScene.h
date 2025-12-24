#pragma once
#include "cocos2d.h"
#include "Entities/Player/Player.h"
#include "Scenes/GameCamera.h"

//创建Scene的子类PlayerTestScene
//该场景用于测试玩家的基本操作
class PlayerTestScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();//创建场景
    static cocos2d::Scene* createWithMap(const std::string& mapFile);

    virtual bool init() override;//初始化

    void setupInput();//读取输入

    void PlayerTestScene::update(float dt);

    // implement the "static create()" method manually
    CREATE_FUNC(PlayerTestScene);//生成一个create函数

    ~PlayerTestScene();
    std::string _currentMapFile = "maps/map_1.tmx";
private:
    Player* _player;
    GameCamera* _cameraController;

};