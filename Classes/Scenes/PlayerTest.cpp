#include "PlayerTest.h"

USING_NS_CC;

Scene* PlayerTest::createScene()
{
    return PlayerTest::create();
}

// 如果加载错误，打印错误信息
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in PlayerTestScene.cpp\n");
}

bool PlayerTest::init()
{
    if (!Scene::init())
    {
        return false;
    }

    //获取当前窗口的可见区域大小
    auto visibleSize = Director::getInstance()->getVisibleSize();
    //获取当前窗口的原点坐标
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //设置背景
    Sprite* background = Sprite::create("player/PlayerTest.jpg");
    background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(background);

    //创建一个精灵用于显示动画
    Sprite* grossini = Sprite::create("player/Knight_idle_first.png");

    grossini->setPosition(Vec2(visibleSize.width / 4 + origin.x, visibleSize.height / 4 + origin.y));

    grossini->setScale(0.5f);

    this->addChild(grossini, 1);
    //在缓存中存入动画
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/HeroKnight_idle_.plist");
    //从缓存中取出动画
    auto animation2 = cache->getAnimation("idle");

    auto action2 = Animate::create(animation2);
    grossini->runAction(Sequence::create(action2, action2->reverse(), NULL));

    return true;
}


void PlayerTest::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
