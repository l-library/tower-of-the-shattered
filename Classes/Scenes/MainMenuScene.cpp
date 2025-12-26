#include "MainMenuScene.h"
#include "../Audio/AudioManager.h" 
#include "Tools/ReadJson.h"

USING_NS_CC;

const Color3B kColorNormal = Color3B(200, 200, 200); // 灰色
const Color3B KColorHover = Color3B(255, 200, 0);   // 金色

Scene* MainMenuScene::createScene()
{
    return MainMenuScene::create();
}

bool MainMenuScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    _lastHighlightedItem = nullptr;

    initBackground();
    initMenu();
    initMouseListener();

    return true;
}


void MainMenuScene::initBackground()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景精灵（初始帧）
    auto bgSprite = Sprite::create("menu/menu_01.png"); // 替换为你的图片资源
    bgSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    // 如果背景需要铺满屏幕，设置缩放
     float scaleX = visibleSize.width / bgSprite->getContentSize().width;
     float scaleY = visibleSize.height / bgSprite->getContentSize().height;
     bgSprite->setScale(std::max(scaleX, scaleY));
    this->addChild(bgSprite, -1); // z-order -1 保证在最底层

    // 播放动画
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("menu/MenuAnimation.plist");
    auto animation = AnimationCache::getInstance()->getAnimation("menu_animation");
    if (animation)
        bgSprite->runAction(RepeatForever::create(Animate::create(animation)));

}


void MainMenuScene::initMenu()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 菜单选项文字
    auto createItem = [&](const std::string& text, const ccMenuCallback& callback){
        TTFConfig ttfConfig("fonts/Gothic.ttf", 32);
        auto label = Label::createWithTTF(ttfConfig, text);
        auto item = MenuItemLabel::create(label, callback);
        item->setColor(kColorNormal);
        return item;
    };

    std::string chinesePath = "config/ChineseMenu.json";
    auto itemStart = createItem(ReadJson::getString(chinesePath,"start"), CC_CALLBACK_1(MainMenuScene::onStartGame, this));
    auto itemLoad = createItem(ReadJson::getString(chinesePath, "load"), CC_CALLBACK_1(MainMenuScene::onLoadGame, this));
    auto itemRecord = createItem(ReadJson::getString(chinesePath, "record"), CC_CALLBACK_1(MainMenuScene::onRecords, this));
    auto itemExit = createItem(ReadJson::getString(chinesePath, "exit"), CC_CALLBACK_1(MainMenuScene::onExitGame, this));
    
    _mainMenu = Menu::create(itemStart, itemLoad, itemRecord, itemExit, nullptr);

    // 设置位置在屏幕底端 (高度的 15% 处)
    _mainMenu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.15f));

    // 水平排列，间隔 50 像素
    _mainMenu->alignItemsHorizontallyWithPadding(50.0f);

    this->addChild(_mainMenu, 1);

    // 预加载
    AudioManager::getInstance()->preload("sounds/button_hover.ogg");
    AudioManager::getInstance()->preload("sounds/button_click.ogg");

    // 播放bgm
    AudioManager::getInstance()->playBGM("sounds/menu.ogg");
}

void MainMenuScene::initMouseListener()
{
    // 创建鼠标监听器
    auto mouseListener = EventListenerMouse::create();

    mouseListener->onMouseMove = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;

        // 坐标转换
        Vec2 locInView = e->getLocationInView();

        // 转换为 Menu 内部的节点坐标
        Vec2 locInMenu = _mainMenu->convertToNodeSpace(locInView);

        MenuItem* currentItem = nullptr;

        // 遍历 Menu 的子节点
        for (auto& child : _mainMenu->getChildren())
        {
            auto item = dynamic_cast<MenuItem*>(child);
            if (item)
            {
                // 检查是否包含点
                if (item->getBoundingBox().containsPoint(locInMenu))
                {
                    currentItem = item;
                    break;
                }
            }
        }

        // 状态切换
        if (currentItem)
        {
            // 如果当前鼠标悬停的物品和上一次不同
            if (_lastHighlightedItem != currentItem)
            {
                // 如果之前有悬停的物品，先复原它
                if (_lastHighlightedItem) {
                    _lastHighlightedItem->setColor(kColorNormal);
                    _lastHighlightedItem->setScale(1.0f);
                }

                // 高亮当前物品
                currentItem->setColor(KColorHover);
                currentItem->setScale(1.2f);
                
                // 记录当前物品
                _lastHighlightedItem = currentItem;

                // 播放音效
                AudioManager::getInstance()->playEffect("sounds/button_hover.ogg");
            }
        }
        else
        {
            // 如果鼠标没有悬停在任何物品上，但之前有悬停记录
            if (_lastHighlightedItem) {
                _lastHighlightedItem->setColor(kColorNormal);
                _lastHighlightedItem->setScale(1.0f);
                _lastHighlightedItem = nullptr; // 清空记录
            }
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}


// 各回调函数实现

void MainMenuScene::onStartGame(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Start Game Clicked");
    // Director::getInstance()->replaceScene(TransitionFade::create(1.0f, GameScene::createScene()));
}

void MainMenuScene::onLoadGame(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Load Game Clicked");
    // 进入存档界面逻辑
}

void MainMenuScene::onRecords(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Records Clicked");
    // 进入记录界面逻辑
}

void MainMenuScene::onExitGame(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Exit Clicked");
    Director::getInstance()->end();
}