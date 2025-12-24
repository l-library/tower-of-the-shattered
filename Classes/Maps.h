#pragma once
#include "cocos2d.h"

class Maps : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();//创建场景

    virtual bool init();//初始化

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);

    // 生成多边形地图
    void Maps::buildPolyPhysicsFromLayer(cocos2d::TMXTiledMap* map);
    void Maps::switchLevelBox(cocos2d::TMXTiledMap* map);

    // implement the "static create()" method manually
    CREATE_FUNC(Maps);//生成一个create函数
};