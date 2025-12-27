#include "PauseMenuScene.h"
#include "MainMenuScene.h"
#include "../Audio/AudioManager.h"
#include "Tools/ReadJson.h"

USING_NS_CC;

const Color3B kColorNormal = Color3B(200, 200, 200); // 灰色
const Color3B KColorHover = Color3B(255, 200, 0);   // 金色

Scene* PauseMenuScene::createScene()
{
    return PauseMenuScene::create();
}

bool PauseMenuScene::init()
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

void PauseMenuScene::initBackground()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();


    auto bgLayer = LayerColor::create(Color4B(255, 0, 0, 2), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(origin);
    this->addChild(bgLayer, -1);
    std::string MenuPath = "config/ChineseMenu.json";
    // 创建暂停菜单标题
    auto title = Label::createWithSystemFont(ReadJson::getString(MenuPath, "PauseMenu"), "Arial", 48);
    if (title) {
        title->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height * 0.7f));
        title->setColor(Color3B(255, 255, 255));
        this->addChild(title, 1);
        log("Pause menu title created successfully");
    } else {
        log("Failed to create pause menu title");
    }
}

void PauseMenuScene::initMenu()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 菜单选项文字
    auto createItem = [&](const std::string& text, const ccMenuCallback& callback){
        auto label = Label::createWithSystemFont(text, "Arial", 32);
        if (label) {
            log("Created label for menu item: %s", text.c_str());
        } else {
            log("Failed to create label for menu item: %s", text.c_str());
        }
        auto item = MenuItemLabel::create(label, callback);
        if (item) {
            item->setColor(kColorNormal);
        }
        return item;
    };
    std::string MenuPath = "config/ChineseMenu.json";

    auto itemReturnGame = createItem(ReadJson::getString(MenuPath, "resume"), CC_CALLBACK_1(PauseMenuScene::onReturnToGame, this));
    auto itemReturnMain = createItem(ReadJson::getString(MenuPath, "quit"), CC_CALLBACK_1(PauseMenuScene::onReturnToMainMenu, this));
    
    _mainMenu = Menu::create(itemReturnGame, itemReturnMain, nullptr);
    if (_mainMenu) {
        // 设置位置在屏幕中央
        _mainMenu->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

        // 垂直排列，间隔 50 像素
        _mainMenu->alignItemsVerticallyWithPadding(50.0f);

        this->addChild(_mainMenu, 1);
        log("Menu created and added to scene");
    } else {
        log("Failed to create menu");
    }

    // 预加载音效
    AudioManager::getInstance()->preload("sounds/button_hover.ogg");
    AudioManager::getInstance()->preload("sounds/button_click.ogg");
}

void PauseMenuScene::initMouseListener()
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

void PauseMenuScene::onReturnToGame(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Return to Game Clicked");
    // 弹出当前场景，回到原来的游戏场景
    Director::getInstance()->popScene();
}

void PauseMenuScene::onReturnToMainMenu(Ref* sender)
{
    AudioManager::getInstance()->playEffect("sounds/button_click.ogg");
    log("Return to Main Menu Clicked");
    // 替换场景为主菜单场景
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, MainMenuScene::createScene()));
}
