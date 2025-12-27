#include "MainMenuScene.h"
#include "../Audio/AudioManager.h" 
#include "Tools/ReadJson.h"
#include "../Scenes/PlayerTestScene.h"
#include "../Tools/SaveManager.h"
#include "Credits.h"

USING_NS_CC;

const Color3B kColorNormal = Color3B(200, 200, 200); // 灰色
const Color3B KColorHover = Color3B(255, 200, 0);   // 金色
const Color3B kColorGrey = Color3B(100, 100, 100); // 灰色

const int kHasUnlocked = 1000;
const int kHaslocked = 1001;

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
    auto bgSprite = Sprite::create("menu/menu_01.png");
    bgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    bgSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y - 50));
    // 设置缩放
     float scaleX = visibleSize.width / bgSprite->getContentSize().width * 0.8;
     float scaleY = visibleSize.height / bgSprite->getContentSize().height * 0.8;
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
    auto createItem = [&](const std::string& text, const ccMenuCallback& callback, bool isUnlocked = true){
        TTFConfig ttfConfig("fonts/Gothic.ttf", 32);
        auto label = Label::createWithTTF(ttfConfig, text);
        auto item = MenuItemLabel::create(label, callback);
        item->setColor(isUnlocked? kColorNormal:kColorGrey);
        return item;
    };

    std::string chinesePath = "config/ChineseMenu.json";
    auto itemStart = createItem(ReadJson::getString(chinesePath,"start"), CC_CALLBACK_1(MainMenuScene::onStartGame, this));
    auto itemLoad = createItem(ReadJson::getString(chinesePath, "load"), CC_CALLBACK_1(MainMenuScene::onLoadGame, this),
        SaveManager::getInstance()->hasSaveFile());
    auto itemRecord = createItem(ReadJson::getString(chinesePath, "record"), CC_CALLBACK_1(MainMenuScene::onRecords, this));
    auto itemExit = createItem(ReadJson::getString(chinesePath, "exit"), CC_CALLBACK_1(MainMenuScene::onExitGame, this));
    
    itemStart->setTag(kHasUnlocked);
    itemLoad->setTag(SaveManager::getInstance()->hasSaveFile() ? kHasUnlocked : kHaslocked);
    itemRecord->setTag(kHasUnlocked);
    itemExit->setTag(kHasUnlocked);

    _mainMenu = Menu::create(itemStart, itemLoad, itemRecord, itemExit, nullptr);

    // 设置位置在屏幕底端 (高度的 15% 处)
    _mainMenu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.15f));

    // 水平排列，间隔 50 像素
    _mainMenu->alignItemsHorizontallyWithPadding(70.0f);

    this->addChild(_mainMenu, 1);

    // 显示标题
    TTFConfig ttfConfig("fonts/title.ttf", 80);
    Label* title = Label::createWithTTF(ttfConfig, ReadJson::getString(chinesePath, "title"));
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.9));
    this->addChild(title, 5);

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
        if (currentItem && currentItem->getTag() == kHasUnlocked)
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
    // 新游戏，删除原存档（如果不存在存档此操作会创建一个存档）
    SaveManager::getInstance()->resetSaveData();
    SaveManager::getInstance()->setsceneComeFromMenu(true);
    Director::getInstance()->replaceScene(PlayerTestScene::createScene());
}

void MainMenuScene::onLoadGame(Ref* sender)
{
    log("Load Game Clicked");
    // 确定是否存在存档
    if (SaveManager::getInstance()->hasSaveFile()) {
        AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
        SaveManager::getInstance()->setsceneComeFromMenu(true);
        Director::getInstance()->replaceScene(PlayerTestScene::createScene());
    }
}

void MainMenuScene::onRecords(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Records Clicked");
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, Credits::create()));
}

void MainMenuScene::onExitGame(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Exit Clicked");
    Director::getInstance()->end();
}