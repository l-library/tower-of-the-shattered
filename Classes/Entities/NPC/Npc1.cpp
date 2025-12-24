#include "NPC1.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

NPC1::NPC1()
{
}

NPC1::~NPC1()
{
}

NPC1* NPC1::create()
{
    NPC1* npc = new NPC1();
    if (npc && npc->init())
    {
        npc->autorelease();
        return npc;
    }
    delete npc;
    return nullptr;
}

bool NPC1::init()
{
    // 初始化父类，使用第一张图片作为初始精灵
    if (!NpcBase::init("NPC/NPC1/001.png"))
    {
        return false;
    }
    
    // 加载精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile("NPC/NPC1/npc1_idle.plist");
    
    // 创建待机动画
    Vector<SpriteFrame*> idleFrames;
    for (int i = 1; i <= 6; i++)
    {
        char frameName[32];
        sprintf(frameName, "%03d.png", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            idleFrames.pushBack(frame);
        }
    }
    
    if (!idleFrames.empty())
    {
        // 创建动画
        auto idleAnimation = Animation::createWithSpriteFrames(idleFrames, 0.1f);
        if (idleAnimation != nullptr)
        {
            idleAnimation->setLoops(-1); // 无限循环
            idleAnimation->setRestoreOriginalFrame(true);
            
            // 创建动画动作
            idleAnimation_ = Animate::create(idleAnimation);
            if (idleAnimation_ != nullptr)
            {
                // 播放待机动画
                sprite_->runAction(idleAnimation_);
            }
        }
    }
    
    // 设置对话内容
    std::vector<std::string> dialogues = {
        "你好，冒险者！",
        "欢迎来到这个世界！",
        "有什么我可以帮助你的吗？"
    };
    this->setDialogues(dialogues);
    
    return true;
}

void NPC1::handlePlayerInteraction()
{
    // 检测功能是否正常，先CCLOG一个消息
    CCLOG("NPC1: 玩家与我交互了！");
    
    // 这里可以添加显示对话的逻辑
    if (!dialogues_.empty())
    {
        CCLOG("NPC1: %s", dialogues_[0].c_str());
    }
}