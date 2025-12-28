#include "PauseMenuScene.h"
#include "MainMenuScene.h"
#include "../Audio/AudioManager.h"
#include "Tools/ReadJson.h"
#include "Entities/Items/ItemManager.h"
#include "Maps/GameSceneManager.h"
#include "Maps/RoomData.h"

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
    initItemDisplay();

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
        title->setPosition(Vec2(origin.x + visibleSize.width * 0.75f, origin.y + visibleSize.height * 0.7f));
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
        _mainMenu->setPosition(Vec2(origin.x + visibleSize.width * 0.75f, origin.y + visibleSize.height * 0.5f));
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
    ItemManager::getInstance()->resetRuntimeData();
    g_currentRoomId = 1;
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, MainMenuScene::createScene()));
}

void PauseMenuScene::initItemDisplay()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 获取 ItemManager 单例和拥有的物品列表
    auto itemManager = ItemManager::getInstance();
    const std::vector<int>& ownedItems = itemManager->getOwnedItems();

    // 如果没有物品，直接返回
    if (ownedItems.empty()) {
        return;
    }

    // 布局参数设置
    float startX = origin.x + 20.0f;           // 距离左边缘的距离
    float startY = origin.y + visibleSize.height - 120.0f; // 距离顶部的起始位置
    float itemBoxWidth = 300.0f;               // 展示框宽度
    float itemBoxHeight = 100.0f;               // 展示框高度
    float gapX = 20.0f;                        // 列间距
    float gapY = 10.0f;                        // 列表垂直间距
    float bottomLimit = origin.y + 50.0f;      // 底部边界，低于这个位置就换列

    // 当前绘制坐标
    float currentX = startX;
    float currentY = startY;

    // 遍历拥有的物品并生成 UI
    for (size_t i = 0; i < ownedItems.size(); ++i)
    {
        // 检查是否需要换列
        // 如果当前高度减去盒子高度 低于 底部边界，则换到新的一列
        if (currentY < bottomLimit) {
            currentY = startY;              // 重置回到顶部
            currentX += itemBoxWidth + gapX;// 向右移动一列的距离
        }
        int itemId = ownedItems[i];
        const ItemData* data = itemManager->getItemConfig(itemId);

        // 创建背景底板
        auto bgLayer = LayerColor::create(Color4B(150, 150, 150, 150), itemBoxWidth, itemBoxHeight);
        bgLayer->setPosition(Vec2(currentX, currentY));
        this->addChild(bgLayer, 1);

        // 创建图标
        auto icon = Sprite::create(data->iconPath);
        if (icon) {
            // 将图标缩放到合适大小
            float targetIconSize = 64.0f;
            float scale = targetIconSize / icon->getContentSize().width;
            icon->setScale(scale);
            // 图标居中于左侧区域
            icon->setPosition(Vec2(currentX + 40, currentY + itemBoxHeight / 2));
            this->addChild(icon, 2);
        }
        else {
            log("Error: Icon not found for item %d at path %s", itemId, data->iconPath.c_str());
        }

        // 创建名称Label
        auto nameLabel = Label::createWithTTF(data->name, "fonts/Gothic.ttf", 20);
        nameLabel->setAnchorPoint(Vec2(0, 1)); // 左上角对齐
        nameLabel->setPosition(Vec2(currentX + 80, currentY + itemBoxHeight - 10));
        nameLabel->setColor(KColorHover); // 使用金色显示名称
        this->addChild(nameLabel, 2);

        // 创建描述Label
        auto descLabel = Label::createWithTTF(data->description, "fonts/Gothic.ttf", 14);
        descLabel->setAnchorPoint(Vec2(0, 1)); // 左上角对齐
        descLabel->setPosition(Vec2(currentX + 80, currentY + itemBoxHeight - 35));
        descLabel->setColor(Color3B::WHITE);
        // 设置文字显示区域，实现自动换行
        descLabel->setDimensions(itemBoxWidth - 80, 40);
        descLabel->setOverflow(Label::Overflow::CLAMP);
        this->addChild(descLabel, 2);

        // 创建更多信息Label
        auto flovorLabel = Label::createWithTTF(data->flavorText, "fonts/Gothic.ttf", 14);
        flovorLabel->setAnchorPoint(Vec2(0, 1)); // 左上角对齐
        flovorLabel->setPosition(Vec2(currentX + 80, currentY + itemBoxHeight - 50));
        flovorLabel->setColor(Color3B::GRAY);
        // 设置文字显示区域，实现自动换行
        flovorLabel->setDimensions(itemBoxWidth - 80, 40);
        flovorLabel->setOverflow(Label::Overflow::CLAMP);
        this->addChild(flovorLabel, 2);

        // 更新坐标
        currentY -= (itemBoxHeight + gapY);
    }

    log("Item display initialized with %d items.", (int)ownedItems.size());
}