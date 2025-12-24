#ifndef __NPC1_H__
#define __NPC1_H__

#include "NpcBase.h"

class NPC1 : public NpcBase
{
public:
    NPC1();
    virtual ~NPC1();
    
    // 初始化函数
    static NPC1* create();
    bool init() override;
    
protected:
    // 实现玩家交互方法
    void handlePlayerInteraction() override;
};

#endif // __NPC1_H__