#include "NPC3.h"
#include "TowerOfTheShattered.h"

USING_NS_CC;

NPC3::NPC3()
{
}

NPC3::~NPC3()
{
}

NPC3* NPC3::create()
{
    NPC3* npc = new NPC3();
    if (npc && npc->init())
    {
        npc->autorelease();
        return npc;
    }
    delete npc;
    return nullptr;
}

bool NPC3::init()
{
    // 初始化父类，使用第一张图片作为初始精灵
    if (!NpcBase::init("NPC/NPC3/npc3_001.png"))
    {
        return false;
    }
    
    // 加载精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile("NPC/NPC3/npc3_idle.plist");
    
    // 创建待机动画
    Vector<SpriteFrame*> idleFrames;
    for (int i = 1; i <= 5; i++)
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
    std::string NPC3Path = "config/NPC/NPC3.json";
    // 设置对话内容
    std::vector<DialogueEntry> dialogues = 
    {
        DialogueEntry(ReadJson::getString(NPC3Path,"1"),
                     nullptr, // 通用回调函数(固定触发)
                     []() {}, // 按键1回调
                     []() {}, // 按键2回调
                     []() {}), // 按键3回调
        /*DialogueEntry(ReadJson::getString(NPC3Path,"2"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC3Path,"3"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),        
        DialogueEntry(ReadJson::getString(NPC3Path,"4"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC3Path,"5"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC3Path,"6"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC3Path,"7"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {})*/
    };
    this->setDialogues(dialogues);
    
    //设置立绘
    Illustration_ = Sprite::create("NPC/NPC3/npc3_illustration.png");
    if (Illustration_)
    {
        Illustration_->retain();
    }

    return true;
}

void NPC3::handlePlayerInteraction()
{
    // 显示对话背景并调用显示对话内容的函数
    showDialogueBackground();
}
