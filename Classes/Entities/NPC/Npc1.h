#ifndef __NPC1_H__
#define __NPC1_H__

#include "NpcBase.h"

class NPC1 : public NpcBase//第一个NPC，在npcbase的基础上额外添加入场对话功能（新手引导）
{
public:
    NPC1();
    virtual ~NPC1();
    
    // 初始化函数
    static NPC1* create();
    bool init() override;

    //特殊处理：第一次默认进对话
    void update(float delta) override;


protected:
    // 实现玩家交互方法
    void handlePlayerInteraction() override;

    bool hasTalked = false;
};

#endif // __NPC1_H__