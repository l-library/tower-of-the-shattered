#ifndef __NPC_BASE_H__
#define __NPC_BASE_H__

#include "cocos2d.h"
#include <vector>
#include <string>
#include<TowerOfTheShattered.h>
USING_NS_CC;
// 定义对话结构体，包含对话内容和可选的回调函数
struct DialogueEntry {
    std::string content; // 对话内容
    std::function<void()> callback; // 通用回调函数指针
    std::function<void()> option1Callback; // 按键1回调函数
    std::function<void()> option2Callback; // 按键2回调函数
    std::function<void()> option3Callback; // 按键3回调函数

    // 默认构造函数，回调函数默认为空
    DialogueEntry(const std::string& c = "", 
                 std::function<void()> cb = nullptr,
                 std::function<void()> opt1 = nullptr,
                 std::function<void()> opt2 = nullptr,
                 std::function<void()> opt3 = nullptr)
        : content(c), callback(cb), 
          option1Callback(opt1), option2Callback(opt2), option3Callback(opt3) {
    }
};
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
    void setDialogues(const std::vector<DialogueEntry>& dialogues);
    
    // 获取当前是否与玩家碰撞
    bool isPlayerCollided() const { return *isPlayerCollided_; }
    
protected:
    // 精灵指针
    Sprite* sprite_;
    
    //玩家指针
    Player* player_;


    // 碰撞箱指针
    PhysicsBody* physicsBody_;
    
    // 对话背景精灵指针
    Sprite* dialogueBackground_;
    Sprite* Illustration_;

    // 对话文本标签指针
    Label* dialogueLabel_;
    
    // 待机动画指针
    Animate* idleAnimation_;
    
    // 标志是否与玩家碰撞（使用指针以便在回调中修改）
    bool* isPlayerCollided_;
    

    
    // 存储NPC的对话
    std::vector<DialogueEntry> dialogues_;
    
    // 当前显示的对话索引
    int currentDialogueIndex_;
    
    // 标志是否正在显示对话
    bool isDialogueDisplaying_;
    
    // 按键监听器
    EventListenerKeyboard* keyboardListener_;
    
    // 碰撞监听器
    EventListenerPhysicsContact* contactListener_;
    
    // 初始化精灵
    virtual bool InitSprite(const std::string& spritePath);
    
    // 处理玩家交互
    virtual void handlePlayerInteraction() = 0;
    
    // 显示对话背景并调用显示对话内容函数
    virtual void showDialogueBackground();
    
    // 显示对话内容
    virtual void displayDialogueContent(int dialogueIndex = 0);
};

#endif // __NPC_BASE_H__