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
    static cocos2d::Scene* createWithMap(const std::string& tmxFile, const cocos2d::Vec2 pos = { 360,100 });

    virtual bool init() override;//初始化

    void setupInput();//读取输入

    void PlayerTestScene::update(float dt);

    void setPlayerSpawnPosition(const cocos2d::Vec2 pos) 
    {
        _playerSpawnPosition = pos;
    }

    GameCamera* getGamera() { return _cameraController; }

    // implement the "static create()" method manually
    CREATE_FUNC(PlayerTestScene);//生成一个create函数

    void gameOver();

    ~PlayerTestScene();
    std::string _currentMapFile = "maps/map_start.tmx";
private:
    cocos2d::Vec2 _playerSpawnPosition = { 360,100 };
    Player* _player;
    GameCamera* _cameraController;
};