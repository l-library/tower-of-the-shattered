#pragma once
#include "cocos2d.h"
#include "Entities/Player/Player.h"

//创建Scene的子类PlayerTestScene
//该场景用于测试玩家的基本操作
class PlayerTestScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();//创建场景

    virtual bool init();//初始化

    void setupInput();//读取输入

    // implement the "static create()" method manually
    CREATE_FUNC(PlayerTestScene);//生成一个create函数

private:
    Player* _player;
};