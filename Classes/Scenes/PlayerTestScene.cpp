#include "PlayerTestScene.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

Scene* PlayerTestScene::createScene()
{
    return PlayerTestScene::create();
}

// 如果加载错误，打印错误信息
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in PlayerTestScene.cpp\n");
}

bool PlayerTestScene::init()
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

    //在缓存中存入动画
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("player/Animation_test.plist");

    //创建一个player对象
    auto player = Player::createNode();
    player->setPosition(Vec2(visibleSize.width / 4 + origin.x, visibleSize.height / 4 + origin.y));
    player->setScale(5.0f);
    this->addChild(player, 1);///渲染player

    return true;
}
