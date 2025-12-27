#include "NPC3.h"
#include"Entities/Enemy/Bosses/Boss1.h"

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
    goldspent_ = 0;
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
        DialogueEntry(ReadJson::getString(NPC3Path,"2"),
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
                     [this]() 
            {
                goldspent_ = ItemManager::getInstance()->getGold();
                ItemManager::getInstance()->spendGold(goldspent_);

            },
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC3Path,"5"),
                     nullptr,
                     [this]() {player_->modifyMaxHealth(goldspent_); player_->modifyMaxMagic(goldspent_); },
                     [this]() 
            {
                for (int i = 0; i < goldspent_ / 50; i++)
                {

                    auto item = Items::createWithId(110);
                    if (item)
                    {
                        item->setPosition(this->getPosition());

                        // 模拟爆出来的效果：给一个向上的初速度
                        item->getPhysicsBody()->setVelocity(Vec2(0, 200));
                        this->getParent()->addChild(item, 5); // Z-order 在背景之上
                    }
                }
            },
                     [this]() 
            {
                ItemManager::getInstance()->addSoul(goldspent_ / 10);

            }),
        DialogueEntry(ReadJson::getString(NPC3Path,"6"),
                     [this]() 
            {
                this->setVisible(false);
                this->getPhysicsBody()->setEnabled(false);
            },
                     []() {},
                     []() {},
                     []() {}), 
        DialogueEntry(ReadJson::getString(NPC3Path,"7"),
                     [this]()
            {
                if (dialogueBackground_)
                        {
                            dialogueBackground_->removeFromParentAndCleanup(true);
                            dialogueBackground_ = nullptr;
                        }

                        if (dialogueLabel_)
                        {
                            dialogueLabel_->removeFromParentAndCleanup(true);
                            dialogueLabel_ = nullptr;
                        }

                        if (Illustration_)
                        {
                            Illustration_->removeFromParent();
                        }

                        isDialogueDisplaying_ = false;
                        currentDialogueIndex_ = 0;
                        if (player_ != nullptr)
                            player_->setControlEnabled(true);
                auto boss = Boss1::create();
                boss->setPosition(this->getPosition());
                this->getParent()->addChild(boss, 1);
            },
                     []() {},
                     []() {},
                     []() {})
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
