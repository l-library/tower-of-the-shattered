#pragma once

#include "cocos2d.h"

class PauseMenuScene : public cocos2d::Scene
{
public:
    // 标准的 create 函数
    static cocos2d::Scene* createScene();

    virtual bool init();

    // 宏，实现 create() 方法
    CREATE_FUNC(PauseMenuScene);

private:
    // 初始化背景
    void initBackground();
    // 初始化菜单项
    void initMenu();
    // 初始化鼠标监听（用于悬停效果）
    void initMouseListener();

    // 菜单回调函数
    void onReturnToGame(cocos2d::Ref* sender);
    void onReturnToMainMenu(cocos2d::Ref* sender);

    // 记录当前高亮的 Item，用于防止音效重复播放
    cocos2d::MenuItem* _lastHighlightedItem;

    // 保存 Menu 的引用方便在鼠标事件中访问
    cocos2d::Menu* _mainMenu;
};
