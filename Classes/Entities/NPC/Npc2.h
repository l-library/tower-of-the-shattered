#ifndef __NPC2_H__
#define __NPC2_H__

#include "NpcBase.h"

class NPC2 : public NpcBase//第二个NPC
{
public:
    NPC2();
    virtual ~NPC2();
    
    // 初始化函数
    static NPC2* create();
    bool init() override;


protected:
    // 实现玩家交互方法
    void handlePlayerInteraction() override;

    bool has_enough_soul;
};

#endif // __NPC2_H__