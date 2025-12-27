#pragma once

#include "cocos2d.h"

class Credits : public cocos2d::Scene
{
public:
    // 标准的 create 函数
    static cocos2d::Scene* createScene();

    virtual bool init();

    // 宏，实现 create() 方法
    CREATE_FUNC(Credits);
};