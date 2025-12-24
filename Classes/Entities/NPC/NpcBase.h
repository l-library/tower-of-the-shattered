#ifndef __NPC_BASE_H__
#define __NPC_BASE_H__

#include "cocos2d.h"
#include <vector>
#include <string>
#include<TowerOfTheShattered.h>
USING_NS_CC;

class NpcBase : public Node
{
public:
    NpcBase();
    virtual ~NpcBase();
    
    // 初始化函数
    virtual bool init(const std::string& spritePath);
    
    // 碰撞回调函数
    virtual bool onContactBegin(PhysicsContact& contact);
    virtual bool onContactSeparate(PhysicsContact& contact);
    
    // 设置对话内容
    void setDialogues(const std::vector<std::string>& dialogues);
    
    // 获取当前是否与玩家碰撞
    bool isPlayerCollided() const { return *isPlayerCollided_; }
    
protected:
    // 精灵指针
    Sprite* sprite_;
    
    // 碰撞箱指针
    PhysicsBody* physicsBody_;
    
    // 待机动画指针
    Animate* idleAnimation_;
    
    // 标志是否与玩家碰撞（使用指针以便在回调中修改）
    bool* isPlayerCollided_;
    
    // 存储NPC的对话
    std::vector<std::string> dialogues_;
    
    // 按键监听器
    EventListenerKeyboard* keyboardListener_;
    
    // 碰撞监听器
    EventListenerPhysicsContact* contactListener_;
    
    // 初始化精灵
    virtual bool InitSprite(const std::string& spritePath);
    
    // 处理玩家交互
    virtual void handlePlayerInteraction() = 0;
};

#endif // __NPC_BASE_H__