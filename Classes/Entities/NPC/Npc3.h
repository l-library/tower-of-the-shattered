#ifndef __NPC3_H__
#define __NPC3_H__

#include "NpcBase.h"

class NPC3 : public NpcBase//第三个NPC
{
public:
    NPC3();
    virtual ~NPC3();
    
    // 初始化函数
    static NPC3* create();
    bool init() override;


protected:
    // 实现玩家交互方法
    void handlePlayerInteraction() override;

    int goldspent_;
};

#endif // __NPC3_H__