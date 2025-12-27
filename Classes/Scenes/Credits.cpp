#include "Credits.h"
#include "Tools/ReadJson.h"
#include "MainMenuScene.h"
#include "../Audio/AudioManager.h" 

USING_NS_CC;

Scene* Credits::createScene()
{
    return Credits::create();
}

bool Credits::init()
{
    if (!Scene::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    std::string chinesePath = "config/credits.json";

    TTFConfig titleConfig("fonts/Gothic.ttf", 50);
    TTFConfig normalConfig("fonts/Gothic.ttf", 32);
    Label* title = Label::createWithTTF(titleConfig, ReadJson::getString(chinesePath, "credits"));
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.9));
    this->addChild(title);

    Label* member = Label::createWithTTF(normalConfig, ReadJson::getString(chinesePath, "member"));
    member->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.6));
    this->addChild(member);

    Label* address = Label::createWithTTF(normalConfig, ReadJson::getString(chinesePath, "address"));
    address->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.35));
    this->addChild(address);

    Label* realAdress = Label::createWithTTF(normalConfig, ReadJson::getString(chinesePath, "realAddress"));
    realAdress->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.35 - realAdress->getContentSize().height));
    this->addChild(realAdress);
    
    // ·µ»Ø°´Å¥
    auto backLabel = Label::createWithTTF(normalConfig, ReadJson::getString(chinesePath, "return"));
    auto backItem = MenuItemLabel::create(backLabel, [=](Ref* sender) {
        AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, MainMenuScene::createScene()));
        });

    backItem->setColor(Color3B(200, 200, 200));
    auto menu = Menu::create(backItem, nullptr);
    menu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.15f));
    this->addChild(menu);

    return true;
}