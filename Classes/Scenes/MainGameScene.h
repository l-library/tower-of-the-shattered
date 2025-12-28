#pragma once
#include "cocos2d.h"
#include "Entities/Player/Player.h"
#include "Scenes/GameCamera.h"


// 该场景用于测试玩家的基本操作
class MainGameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();// 创建场景
    static cocos2d::Scene* createWithMap(const std::string& tmxFile, const cocos2d::Vec2 pos = { 360,100 });

    virtual bool init() override;// 初始化

    void setupInput();// 读取输入

    void MainGameScene::update(float dt);

    void setPlayerSpawnPosition(const cocos2d::Vec2 pos) 
    {
        _playerSpawnPosition = pos;
    }

    GameCamera* getGamera() { return _cameraController; }

    // implement the "static create()" method manually
    CREATE_FUNC(MainGameScene);// 生成一个create函数

    void MainGameScene::resetPlayerForNewScene(Player* player);

    void gameOver(std::string end_sprite = "player/dead.png", std::string end_sound = "sounds/Death.ogg");

    ~MainGameScene();
    std::string _currentMapFile = "maps/map_start.tmx";
private:
    cocos2d::Vec2 _playerSpawnPosition = { 360,100 };
    Player* _player;
    GameCamera* _cameraController;
    bool _isGameOverProcessing = false;
};