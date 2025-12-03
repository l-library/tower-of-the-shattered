#include "cocos2d.h"

//创建Scene的子类PlayerTest
class PlayerTestScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();//创建场景

    virtual bool init();//初始化

    // implement the "static create()" method manually
    CREATE_FUNC(PlayerTestScene);//生成一个create函数
};