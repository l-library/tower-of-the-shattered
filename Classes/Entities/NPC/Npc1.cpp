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
    std::string NPC1Path = "config/NPC/NPC1.json";
    // 设置对话内容
    std::vector<DialogueEntry> dialogues = 
    {
        DialogueEntry(ReadJson::getString(NPC1Path,"1"),
                     nullptr, // 通用回调函数(固定触发)
                     []() {}, // 按键1回调
                     []() {}, // 按键2回调
                     []() {}), // 按键3回调
        DialogueEntry(ReadJson::getString(NPC1Path,"2"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC1Path,"3"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),        
        DialogueEntry(ReadJson::getString(NPC1Path,"4"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC1Path,"5"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC1Path,"6"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC1Path,"7"),
                     nullptr,
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC1Path,"8"),
                     nullptr,
                     [this]() {
                ItemManager::getInstance()->addSoul(1000);
            },
                     []() {},
                     []() {})

    };
    this->setDialogues(dialogues);
    
    //设置立绘
    Illustration_ = Sprite::create("NPC/NPC1/npc1_illustration.png");
    if (Illustration_)
    {
        Illustration_->retain();
    }

    return true;
}

void NPC1::handlePlayerInteraction()
{
    // 显示对话背景并调用显示对话内容的函数
    showDialogueBackground();
}

void NPC1::update(float delta) //直接说话（新手引导）
{
    if (!hasTalked&&this->player_)
    {
        isDialogueDisplaying_ = true;
        handlePlayerInteraction();
        hasTalked = true;
        dialogues_.erase(dialogues_.begin());
    }

}