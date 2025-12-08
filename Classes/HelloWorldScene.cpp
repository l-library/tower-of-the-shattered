/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "Maps.h"
#include "ui/CocosGUI.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;
using namespace cocos2d::ui;

// 简单的EnemyBase派生类用于测试
class TestEnemy : public EnemyBase
{
public:
    static TestEnemy* create() {
        TestEnemy* enemy = new TestEnemy();
        if (enemy && enemy->init()) {
            enemy->autorelease();
            return enemy;
        }
        CC_SAFE_DELETE(enemy);
        return nullptr;
    }
    
    // 重写精灵初始化虚函数
    virtual void InitSprite() override {
        // 初始化精灵并设置默认图像
        sprite_ = Sprite::create("HelloWorld.png");
        if (sprite_) {
            this->addChild(sprite_);
            // 设置精灵大小
            sprite_->setScale(0.5f);
        }
    }


    virtual void Hitted(int damage, int poise_damage = 0) override {
        // 被击中反应
        setCurrentVitality(getCurrentVitality() - damage);
        setCurrentStaggerResistance(getCurrentStaggerResistance() - poise_damage);
        
        // 简单的视觉反馈
        if (getSprite()) {
            getSprite()->setColor(Color3B::RED);
            auto resetColor = CallFunc::create([this]() {
                getSprite()->setColor(Color3B::WHITE);
            });
            getSprite()->runAction(Sequence::create(DelayTime::create(0.2f), resetColor, nullptr));
        }
    }

    virtual void Dead() override {
        // 死亡处理
        if (getSprite()) {
            auto fadeOut = FadeOut::create(1.0f);
            auto removeSelf = RemoveSelf::create(true); // 移除精灵
            auto removeEnemy = CallFunc::create([this]() {
                // 移除敌人对象本身
                this->removeFromParent();
            });
            getSprite()->runAction(Sequence::create(fadeOut, removeSelf, removeEnemy, nullptr));
        } else {
            // 如果没有精灵，直接移除敌人对象
            this->removeFromParent();
        }
    }

    virtual void BehaviorInit() override 
    {
        // 初始化AI行为
        addBehavior("idle", [this](float delta) -> BehaviorResult {
            // 空闲行为：轻微移动
            if (getSprite()) {
                getSprite()->setPosition(getSprite()->getPosition() - Vec2(1, 0));
                if (getSprite()->getPositionX() > Director::getInstance()->getVisibleSize().width) {
                    getSprite()->setPositionX(0);
                }
            }
            return {true, 0.0f}; // idle行为总是立即完成，无后摇
        });
        
        // 添加一个移动行为作为示例
        addBehavior("move", [this](float delta) -> BehaviorResult {
            // 移动行为：快速向右移动一段距离
            static float moveDuration = 0.0f;
            static bool hasStarted = false;
            static Vec2 startPos;
            
            if (!hasStarted) {
                startPos = getSprite()->getPosition();
                hasStarted = true;
                moveDuration = 0.0f;
            }
            
            moveDuration += delta;
            float totalDuration = 2.0f; // 移动持续2秒
            
            if (getSprite()) {
                float progress = std::min(moveDuration / totalDuration, 1.0f);
                float moveDistance = 100.0f;
                Vec2 newPos = startPos + Vec2(moveDistance * progress, 0);
                getSprite()->setPosition(newPos);
            }
            
            // 判断行为是否完成
            if (moveDuration >= totalDuration) {
                hasStarted = false;
                return {true, 0.5f}; // 行为完成，0.5秒后摇
            }
            
            return {false, 0.0f}; // 行为未完成，无后摇
        });
        
        // 添加硬直行为，后摇为0
        addBehavior("staggered", [this](float delta) -> BehaviorResult {
            // 硬直行为：显示硬直效果
            if (getSprite()) {
                // 简单的硬直效果：闪烁
                int frameCount = static_cast<int>(staggerTimer_ / delta);
                if (frameCount % 3 == 0) {
                    getSprite()->setOpacity(frameCount % 6 < 3 ? 128 : 255);
                }
            }
            return {true, 0.0f}; // 硬直行为总是立即完成，无后摇
        });
    }

    
    virtual std::string DecideNextBehavior(float delta) override {
        // 实际项目中可以根据游戏逻辑、敌人状态、玩家位置等因素决定
        static float decisionTimer = 0.0f;
        static std::string lastDecision = "idle";
        
        decisionTimer += delta; // 使用实际的delta时间更新计时器
        
        // 每2秒重新决定一次行为
        if (decisionTimer >= 2.0f) {
            decisionTimer = 0.0f;
            
            // 50%概率选择move行为，否则选择idle
            if (rand() % 2 == 0) {
                lastDecision = "move";
            } else {
                lastDecision = "idle";
            }
        }
        
        return lastDecision;
    }
};


Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...
    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }

    auto button = Button::create("CloseNormal.png", "CloseSelected.png", "CloseSelected.png");

    button->setTitleText("Map");
    button->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 4));

    button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::BEGAN:
            break;
        case ui::Widget::TouchEventType::ENDED:
            Director::getInstance()->replaceScene(Maps::createScene());
            break;
        default:
            break;
        }
        });

    this->addChild(button);

    // 创建测试敌人
    auto testEnemy = TestEnemy::create();
    if (testEnemy) {
        // 设置敌人位置
        testEnemy->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/4 + origin.y));
        
        // 添加敌人到场景
        this->addChild(testEnemy, 1);
        
        // 添加触摸事件监听器来测试敌人受伤
        auto touchListener = EventListenerTouchOneByOne::create();
        touchListener->onTouchBegan = [=](Touch* touch, Event* event) {
            // 当点击屏幕时，敌人受伤
            testEnemy->Hitted(10, 5);
            
            // 显示敌人当前状态
            auto statusLabel = Label::createWithTTF(
                StringUtils::format("Vitality: %d/%d, Stagger: %d/%d", 
                                    testEnemy->getCurrentVitality(), 
                                    testEnemy->getMaxVitality(),
                                    testEnemy->getCurrentStaggerResistance(),
                                    testEnemy->getStaggerResistance()), 
                "fonts/Marker Felt.ttf", 16);
            statusLabel->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height - 50));
            statusLabel->setColor(Color3B::GREEN);
            this->addChild(statusLabel, 2);
            
            // 1秒后移除状态标签
            statusLabel->runAction(Sequence::create(
                DelayTime::create(1.0f),
                RemoveSelf::create(),
                nullptr
            ));
            
            return true;
        };
        
        _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    }
    
    // 添加一个按钮用于测试韧性被清零的效果
    auto breakPoiseButton = MenuItemFont::create("Break Poise", [=](Ref* pSender) {
        // 一次性打空敌人的韧性
        int currentPoise = testEnemy->getCurrentStaggerResistance();
        testEnemy->Hitted(0, currentPoise); // 只造成韧性伤害，不造成生命值伤害
        
        // 显示敌人当前状态
        auto statusLabel = Label::createWithTTF(
            StringUtils::format("Vitality: %d/%d, Stagger: %d/%d", 
                                testEnemy->getCurrentVitality(), 
                                testEnemy->getMaxVitality(),
                                testEnemy->getCurrentStaggerResistance(),
                                testEnemy->getStaggerResistance()), 
            "fonts/Marker Felt.ttf", 16);
        statusLabel->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height - 50));
        statusLabel->setColor(Color3B::GREEN);
        this->addChild(statusLabel, 2);
        
        // 1秒后移除状态标签
        statusLabel->runAction(Sequence::create(
            DelayTime::create(1.0f),
            RemoveSelf::create(),
            nullptr
        ));
    });
    
    // 设置按钮位置
    breakPoiseButton->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + 50));
    breakPoiseButton->setColor(Color3B::BLUE);
    
    // 创建按钮菜单并添加到场景
    auto buttonMenu = Menu::create(breakPoiseButton, nullptr);
    buttonMenu->setPosition(Vec2::ZERO);
    this->addChild(buttonMenu, 2);
    
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
